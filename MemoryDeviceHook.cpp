#include "MemoryDeviceHook.h"

MemoryDeviceHook::CMemorySystem_CreateHeapFn MemoryDeviceHook::CreateHeap;
MemoryDeviceHook::CMemoryDevice_ReleaseHeapFn MemoryDeviceHook::ReleaseHeap;
MemoryDeviceHook::CMemoryDevice_CreateChildHeapInternalFn MemoryDeviceHook::CreateChildHeapInternal;
MemoryDeviceHook::CMemorySystem_CreateChildHeapFn MemoryDeviceHook::CreateChildHeap;

BOOL CMemoryDevice_ReAlloc(CMemoryDevice* pThis, void** ppMemoryPool, SIZE_T cbSize)
{
	if (!ppMemoryPool)
		return NULL;

	void* pPool = HeapReAlloc(pThis->m_hHeap, HEAP_NO_SERIALIZE, *ppMemoryPool, cbSize);
	*ppMemoryPool = pPool;

	return pPool != NULL;
}

// void *pMemoryPool
BOOL hkCHeapInstanceSetHeapInfo(CHeapInstance* pThis, CHeapInfo* pInfo, unsigned __int64 cbSize, int nChildren, int flags)
{
	typedef BOOL (*OriginalFn)(CHeapInstance*, const void*, unsigned __int64, int, int);

	BOOL bStatus = ((OriginalFn)(g_pMemoryDeviceHook->GetHeapHook().GetFunctionAddress(1)))(pThis, pInfo, cbSize, nChildren, flags);

	return bStatus;
}

CHeapAllocInfo* hkCHeapInstanceAllocPartition(CHeapInstance* pThis, __int64 nBytes, unsigned __int64 align, int flags)
{
	typedef CHeapAllocInfo* (*OriginalFn)(CHeapInstance*, __int64, unsigned __int64, int);

	SIZE_T cbSize = (nBytes << 10); // uSanitizedPartitionSize = (char *)pNextPartition - pPreviousPartition;

	//  *(CHeapAllocInfo**)((char*)pPool - 8)
	CHeapAllocInfo* pAlloc = ((OriginalFn)(g_pMemoryDeviceHook->GetHeapHook().GetFunctionAddress(4)))(pThis, cbSize, align, flags);

	//void* pPool = MakePtr(void*, pAlloc, flags ? pAlloc->m_cbAlignedSize : sizeof(CHeapAllocInfo));
	//QWORD qwPoolSize = pThis->GetPartitionSize(pPool);
	//CHeapAllocInfo* pPoolInfo = pThis->fn2(pPool);
	//CHeapAllocInfo* pRootInfo = pThis->fn2(nullptr); // i think
	//CMemoryDevice::CreateChildHeapInternal(a1, (CHeapInfo *)a1, 1, cbSize, 0, a3);

	if (!pAlloc)
	{
		//CHeapInfo* pParentInfo = pThis->m_pParentInfo;
	
		//void* pOldPool = pThis->m_pMemoryPool;
		//QWORD qwPoolSize = pThis->GetPartitionSize(pOldPool);

		if (CMemoryDevice_ReAlloc(g_pMemoryDevice, (void**)&pThis->m_pMemoryPool, cbSize))
		//if (g_pMemoryDevice->Alloc((void**)&pThis->m_pMemoryPool, cbSize))
		{
			__debugbreak();
		}
	}


	if (g_pMemoryDeviceHook->IsChildHeapAllocation(pThis))
	{
		if (!pAlloc)
		{
			//(0x14091FCC0) - CreateHeap
			// figure out how to iterate heap list and work your way up the heirarchy modifying the heap sizes
			// DeviceHeap has all the heaps but in load order
			// we can also hook the device heap and from there on hook all the new additional heaps
			// another way: search for the root heap in the linked list and rebuild heirarchy that way
			for (CHeapAllocInfo* pAllocInfo = pThis->m_pAllocationInfo; pAllocInfo; pAllocInfo = pAllocInfo->m_pNext)
			{
			}
		}
		else
		{
			//g_pMemoryDeviceHook->AddHook(pAlloc->m_pHeap);
		}
	}
	else
	{
		//g_pMemoryDeviceHook->ExtendHeap(pThis, nBytes);
	}

	return pAlloc;
}

// ---------------------------------------------------------------------------------------------
// HeapScale (see MemoryDeviceHook.h)
// ---------------------------------------------------------------------------------------------

#include <unordered_set>

namespace HeapScale
{
	typedef __int64(*CreateHeapFn)(CHeapInfo* pInfo, unsigned __int64 cbSize, DWORD uHeapIndex);
	typedef __int64(*CreateChildHeapInternalFn)(void* pThis, CHeapInfo* pInfo, unsigned int nChildren,
		unsigned __int64 cbSize, __int64 fFlags, CHeapInfo* pParentInfo);

	// The first whole instructions of each, which the hooks move into their trampolines (and
	// check before patching): the same in v1.02 and build 26534214
	static const BYTE s_CreateHeapHead[18] = { 0x40, 0x56, 0x57, 0x41, 0x56, 0x48, 0x83, 0xEC, 0x40,
		0x48, 0xC7, 0x44, 0x24, 0x30, 0xFE, 0xFF, 0xFF, 0xFF };
	static const BYTE s_CreateChildHeapInternalHead[14] = { 0x48, 0x89, 0x4C, 0x24, 0x08, 0x57, 0x41,
		0x56, 0x41, 0x57, 0x48, 0x83, 0xEC, 0x40 };

	static HOOK_FUNC s_CreateHeapHook;
	static HOOK_FUNC s_CreateChildHeapHook;
	static CreateHeapFn s_oCreateHeap;
	static CreateChildHeapInternalFn s_oCreateChildHeapInternal;

	static int s_iScale = 1;
	static Status_t s_Status = HEAPSCALE_OFF;

	// The heaps that were made bigger. A child is only made bigger when its parent was, so it
	// always fits. Map blocks free and remake their heaps in the same CHeapInfo slots.
	static std::unordered_set<const CHeapInfo*> s_ScaledHeaps;
	static SRWLOCK s_Lock = SRWLOCK_INIT;

	static unsigned __int64 Scaled(unsigned __int64 cbSize)
	{
		return cbSize * (unsigned __int64)s_iScale;
	}

	static __int64 hkCreateHeap(CHeapInfo* pInfo, unsigned __int64 cbSize, DWORD uHeapIndex)
	{
		__int64 bCreated = s_oCreateHeap(pInfo, Scaled(cbSize), uHeapIndex);

		AcquireSRWLockExclusive(&s_Lock);

		if (bCreated)
			s_ScaledHeaps.insert(pInfo);
		else
			s_ScaledHeaps.erase(pInfo);

		ReleaseSRWLockExclusive(&s_Lock);
		return bCreated;
	}

	static __int64 hkCreateChildHeapInternal(void* pThis, CHeapInfo* pInfo, unsigned int nChildren,
		unsigned __int64 cbSize, __int64 fFlags, CHeapInfo* pParentInfo)
	{
		AcquireSRWLockShared(&s_Lock);
		const bool bScale = s_ScaledHeaps.count(pParentInfo) != 0;
		ReleaseSRWLockShared(&s_Lock);

		__int64 bCreated = s_oCreateChildHeapInternal(pThis, pInfo, nChildren, bScale ? Scaled(cbSize) : cbSize, fFlags, pParentInfo);

		AcquireSRWLockExclusive(&s_Lock);

		if (bCreated && bScale)
			s_ScaledHeaps.insert(pInfo);
		else
			s_ScaledHeaps.erase(pInfo);

		ReleaseSRWLockExclusive(&s_Lock);
		return bCreated;
	}

	// i_heap_scale from the config the menu loads at startup, default.xml beside the DLL. A plain
	// text search: XmlLite would mean loading a DLL inside DllMain.
	static int ReadScaleSetting(HINSTANCE hInstance)
	{
		WCHAR szPath[MAX_PATH];
		DWORD cchPath = GetModuleFileNameW(hInstance, szPath, MAX_PATH);

		if (!cchPath || cchPath >= MAX_PATH)
			return 1;

		WCHAR* pSlash = wcsrchr(szPath, L'\\');

		if (!pSlash || wcscpy_s(pSlash + 1, MAX_PATH - (pSlash + 1 - szPath), L"default.xml"))
			return 1;

		HANDLE hFile = CreateFileW(szPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

		if (hFile == INVALID_HANDLE_VALUE)
			return 1;

		char szXml[16384];
		DWORD cbRead = 0;
		BOOL bRead = ReadFile(hFile, szXml, sizeof(szXml) - 1, &cbRead, NULL);

		CloseHandle(hFile);

		if (!bRead)
			return 1;

		szXml[cbRead] = '\0';

		const char* szValue = strstr(szXml, "<i_heap_scale>");

		if (!szValue)
			return 1;

		int iScale = atoi(szValue + sizeof("<i_heap_scale>") - 1);

		return (iScale < 1) ? 1 : (iScale > MAX_SCALE) ? MAX_SCALE : iScale;
	}

	void InstallEarly(HINSTANCE hInstance)
	{
		s_iScale = ReadScaleSetting(hInstance);

		if (s_iScale <= 1)
		{
			s_iScale = 1;
			s_Status = HEAPSCALE_OFF;
			return;
		}

		// CMemorySystem::CreateHeap (0x1402623B0, v1.02 0x1402622B0), the whole prologue
		ULONG_PTR uCreateHeap = FindPattern(NULL, "40 56 57 41 56 48 83 EC 40 48 C7 44 24 30 FE FF FF FF 48 89 5C 24 60 48 89 6C 24 70 41 8B F0");
		// CMemoryDevice::CreateChildHeapInternal (0x140261DE0, v1.02 0x140261CE0)
		ULONG_PTR uCreateChildHeap = FindPattern(NULL, "48 89 4C 24 08 57 41 56 41 57 48 83 EC 40 48 C7 44 24 30 FE FF FF FF 48 89 5C 24 68 48 89 6C 24 70 48 89 74 24 78 49 8B F1");
		// g_MemoryDevice.m_pRootInfo, the list CreateHeap puts each root on: mov rax, m_pRootInfo /
		// test rax, rax / jz / mov [rax+18h], rbx in CreateHeap (0x14026247A, v1.02 0x14026237A)
		CHeapInfo** ppRootInfo = (CHeapInfo**)FindPatternPtr(NULL, "48 8B 05 ? ? ? ? 48 85 C0 74 0B 48 89 58 18 48 8B 05", 3);

		if (!uCreateHeap || uCreateHeap == (ULONG_PTR)-1 || !uCreateChildHeap || uCreateChildHeap == (ULONG_PTR)-1 || !ppRootInfo)
		{
			s_iScale = 1;
			s_Status = HEAPSCALE_NOT_FOUND;
			return;
		}

		// A root heap already made means Mem::Startup has run: too late to make any bigger, and a
		// bigger child of a root that isn't wouldn't fit
		if (*ppRootInfo)
		{
			s_iScale = 1;
			s_Status = HEAPSCALE_TOO_LATE;
			return;
		}

		ZeroMemory(&s_CreateHeapHook, sizeof(s_CreateHeapHook));
		ZeroMemory(&s_CreateChildHeapHook, sizeof(s_CreateChildHeapHook));

		if (!HookFunc64((LPVOID)uCreateHeap, (LPVOID)&hkCreateHeap, sizeof(s_CreateHeapHead), &s_CreateHeapHook,
				s_CreateHeapHead, (LPVOID*)&s_oCreateHeap))
		{
			s_iScale = 1;
			s_Status = HEAPSCALE_NOT_FOUND;
			return;
		}

		if (!HookFunc64((LPVOID)uCreateChildHeap, (LPVOID)&hkCreateChildHeapInternal, sizeof(s_CreateChildHeapInternalHead),
				&s_CreateChildHeapHook, s_CreateChildHeapInternalHead, (LPVOID*)&s_oCreateChildHeapInternal))
		{
			// Roots alone are harmless (only more room), but say so
			UnhookFunc(&s_CreateHeapHook);
			s_iScale = 1;
			s_Status = HEAPSCALE_NOT_FOUND;
			return;
		}

		s_Status = HEAPSCALE_APPLIED;
	}

	void Uninstall(void)
	{
		if (s_Status != HEAPSCALE_APPLIED)
			return;

		// The game keeps making child heaps (map blocks), so these can't point into an unloaded DLL
		UnhookFunc(&s_CreateChildHeapHook);
		UnhookFunc(&s_CreateHeapHook);
	}

	int GetScale(void)
	{
		return (s_Status == HEAPSCALE_APPLIED) ? s_iScale : 1;
	}

	Status_t GetStatus(void)
	{
		return s_Status;
	}
}
