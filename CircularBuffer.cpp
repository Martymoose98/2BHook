#include "CircularBuffer.h"

// VirtualAlloc2 and MapViewOfFile3 are KernelBase exports from Windows 10 1803 on, declared only for
// that target and linked from onecore.lib, so they are looked up instead: the DLL still loads where
// they are missing, and the buffer falls back. The MEM_EXTENDED_PARAMETER arguments are never used,
// and are void* here.
typedef PVOID(WINAPI* VirtualAlloc2_t)(HANDLE hProcess, PVOID pBaseAddress, SIZE_T Size, ULONG AllocationType,
	ULONG PageProtection, void* pExtendedParameters, ULONG ParameterCount);
typedef PVOID(WINAPI* MapViewOfFile3_t)(HANDLE hFileMapping, HANDLE hProcess, PVOID pBaseAddress, ULONG64 Offset,
	SIZE_T ViewSize, ULONG AllocationType, ULONG PageProtection, void* pExtendedParameters, ULONG ParameterCount);

size_t CircularBufferGranularity(void)
{
	SYSTEM_INFO Info;

	GetSystemInfo(&Info);

	return Info.dwAllocationGranularity;
}

// The ring twice over, back to back, both halves views of the same section, or null.
static uint8_t* MapMirroredRing(size_t cbRing)
{
	const HMODULE hKernelBase = GetModuleHandleW(L"kernelbase.dll");

	if (!hKernelBase)
		return nullptr;

	const VirtualAlloc2_t pVirtualAlloc2 = reinterpret_cast<VirtualAlloc2_t>(GetProcAddress(hKernelBase, "VirtualAlloc2"));
	const MapViewOfFile3_t pMapViewOfFile3 = reinterpret_cast<MapViewOfFile3_t>(GetProcAddress(hKernelBase, "MapViewOfFile3"));

	if (!pVirtualAlloc2 || !pMapViewOfFile3)
		return nullptr;

	const HANDLE hProcess = GetCurrentProcess();

	// Reserve both halves as one placeholder, so nothing else can land between them, then split it
	// into a placeholder per half. Each view then replaces its placeholder exactly.
	uint8_t* pFirst = static_cast<uint8_t*>(pVirtualAlloc2(hProcess, nullptr, 2 * cbRing,
		MEM_RESERVE | MEM_RESERVE_PLACEHOLDER, PAGE_NOACCESS, nullptr, 0));

	if (!pFirst)
		return nullptr;

	uint8_t* pSecond = pFirst + cbRing;

	if (!VirtualFree(pFirst, cbRing, MEM_RELEASE | MEM_PRESERVE_PLACEHOLDER))
	{
		VirtualFree(pFirst, 0, MEM_RELEASE);
		return nullptr;
	}

	// Pagefile-backed, committed in full here.
	const HANDLE hSection = CreateFileMappingW(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE,
		(DWORD)((uint64_t)cbRing >> 32), (DWORD)cbRing, nullptr);

	if (!hSection)
	{
		VirtualFree(pFirst, 0, MEM_RELEASE);
		VirtualFree(pSecond, 0, MEM_RELEASE);
		return nullptr;
	}

	void* pFirstView = pMapViewOfFile3(hSection, hProcess, pFirst, 0, cbRing,
		MEM_REPLACE_PLACEHOLDER, PAGE_READWRITE, nullptr, 0);

	void* pSecondView = (pFirstView) ? pMapViewOfFile3(hSection, hProcess, pSecond, 0, cbRing,
		MEM_REPLACE_PLACEHOLDER, PAGE_READWRITE, nullptr, 0) : nullptr;

	// The views keep the section alive.
	CloseHandle(hSection);

	// Everything the buffer does rests on the halves being one, so see that they are before using
	// it, in case a call claimed success without delivering.
	if (pFirstView && pSecondView)
	{
		pFirst[0] = 0x5A;

		if (pSecond[0] == 0x5A)
			return pFirst;
	}

	// A placeholder that wasn't replaced is still there to free; a view has to be unmapped, which
	// leaves nothing behind.
	if (pFirstView)
		UnmapViewOfFile(pFirstView);
	else
		VirtualFree(pFirst, 0, MEM_RELEASE);

	if (pSecondView)
		UnmapViewOfFile(pSecondView);
	else
		VirtualFree(pSecond, 0, MEM_RELEASE);

	return nullptr;
}

CircularBufferMemory CircularBufferAllocate(size_t cbRing)
{
	CircularBufferMemory Memory = {};

	Memory.m_cbRing = cbRing;
	Memory.m_pRing = MapMirroredRing(cbRing);
	Memory.m_bMirrored = (Memory.m_pRing != nullptr);

	if (!Memory.m_pRing)
		Memory.m_pRing = static_cast<uint8_t*>(VirtualAlloc(nullptr, 2 * cbRing, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE));

	return Memory;
}

void CircularBufferFree(CircularBufferMemory& Memory)
{
	if (!Memory.m_pRing)
		return;

	if (Memory.m_bMirrored)
	{
		UnmapViewOfFile(Memory.m_pRing + Memory.m_cbRing);
		UnmapViewOfFile(Memory.m_pRing);
	}
	else
	{
		VirtualFree(Memory.m_pRing, 0, MEM_RELEASE);
	}

	Memory = {};
}
