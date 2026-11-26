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

// CMemorySystem::CreateHeap (0x1402623B0, v1.02 0x1402622B0), the whole prologue
static const char s_szCreateHeapPattern[] = "40 56 57 41 56 48 83 EC 40 48 C7 44 24 30 FE FF FF FF 48 89 5C 24 60 48 89 6C 24 70 41 8B F0";

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

		ULONG_PTR uCreateHeap = FindPattern(NULL, s_szCreateHeapPattern);
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

// ---------------------------------------------------------------------------------------------
// HeapOverflow (see MemoryDeviceHook.h)
// ---------------------------------------------------------------------------------------------

#include <vector>

#include "Console.h"

namespace HeapOverflow
{
	typedef CHeapAllocInfo* (*AllocPartitionFn)(CHeapInstance* pThis, __int64 nBytes, unsigned __int64 uAlign, int fFlags);
	typedef void (*FreePartitionFn)(CHeapInstance* pThis, void* pHeader);
	typedef CHeapInstance* (*DestroyFn)(CHeapInstance* pThis, char uFlags);

	enum
	{
		SLOT_DESTROY = 0,			// CHeapInstance_Dtor (0x140260170), both vtables
		SLOT_ALLOC_PARTITION = 4,	// CHeapInstance::AllocPartition (0x140260A60), VRAM 0x1402613B0
		SLOT_FREE_PARTITION = 5,	// CHeapInstance::FreePartition (0x140260D30), VRAM 0x1402615B0
	};

	// A block's header, as the IDB names it (CHeapAllocInfo). The SDK's CHeapAllocInfo has the same
	// layout under older names. Blocks lie back to back from the heap's m_pPrevPartition (the pool's
	// start) to its m_pNextPartition (its end), each header m_cbAlignedSize + 0x48 after the last:
	// how FreePartition finds the block after the one it frees.
	struct BlockHeader_t
	{
		BOOL				m_bAllocated;		//0x00
		DWORD				dw04;				//0x04
		CHeapInstance*		m_pPartitionHeap;	//0x08 | carved from: its FreePartition frees it
		CHeapInstance*		m_pAllocationHeap;	//0x10 | allocated for: on its list, set after AllocPartition
		BlockHeader_t*		m_pNext;			//0x18
		BlockHeader_t*		m_pPrevious;		//0x20
		void*				m_pPayload;			//0x28
		unsigned __int64	m_cbAlignedSize;	//0x30
		unsigned __int64	m_cbSize;			//0x38
		BlockHeader_t*		m_pPayloadHeader;	//0x40
	};
	VALIDATE_OFFSET(BlockHeader_t, m_pAllocationHeap, 0x10);
	VALIDATE_OFFSET(BlockHeader_t, m_cbAlignedSize, 0x30);
	VALIDATE_SIZE(BlockHeader_t, 0x48);

	VALIDATE_OFFSET(CHeapInstance, m_pPrevPartition, 0x48);
	VALIDATE_OFFSET(CHeapInstance, m_pNextPartition, 0x50);
	VALIDATE_OFFSET(CHeapInstance, m_uIndex, 0x80);
	VALIDATE_OFFSET(CHeapInfo, m_pNext, 0x20);
	VALIDATE_OFFSET(CHeapInfo, m_uPartitionSize, 0x28);
	VALIDATE_OFFSET(CMemoryDevice, m_DeviceHeap, 0x38);
	VALIDATE_OFFSET(CMemoryDevice, m_pRootInfo, 0xD8);

	// The two heap vtables, found by their RTTI names, which are the same in every build
	struct HeapKind_t
	{
		const char*			szTypeName;
		ULONG_PTR*			pVirtualTable;
		ULONG_PTR			uAllocPartitionHook;
		ULONG_PTR			uDestroyHook;
		AllocPartitionFn	oAllocPartition;
		FreePartitionFn		oFreePartition;
		DestroyFn			oDestroy;
	};

	struct OverflowHeap_t
	{
		CHeapInfo*			pInfo;		// m_pHeap is the overflow heap
		HeapKind_t*			pKind;
		unsigned int		uIndex;
		unsigned __int64	cbSize;
	};

	static CHeapAllocInfo* hkAllocPartition(CHeapInstance* pThis, __int64 nBytes, unsigned __int64 uAlign, int fFlags);
	static CHeapAllocInfo* hkVramAllocPartition(CHeapInstance* pThis, __int64 nBytes, unsigned __int64 uAlign, int fFlags);
	static CHeapInstance* hkDestroy(CHeapInstance* pThis, char uFlags);
	static CHeapInstance* hkVramDestroy(CHeapInstance* pThis, char uFlags);

	static HeapKind_t s_Kinds[2] =
	{
		{ ".?AVcHeapInstance@Hw@@", nullptr, (ULONG_PTR)&hkAllocPartition, (ULONG_PTR)&hkDestroy },
		{ ".?AVcHeapVramInstance@Hw@@", nullptr, (ULONG_PTR)&hkVramAllocPartition, (ULONG_PTR)&hkVramDestroy },
	};

	static HeapScale::CreateHeapFn s_pCreateHeap;
	static Status_t s_Status = HEAPOVERFLOW_OFF;

	// Everything below is guarded by the memory device's own lock, which the game holds while it
	// makes and releases heaps, so it is always taken first: the device, then a heap.
	static std::vector<OverflowHeap_t> s_Heaps;
	static bool s_bCreating;
	static bool s_bWarnedFull;

	// Read by the menu without the lock: whole aligned 64-bit values, never torn
	static Stats_t s_Stats;

	// The heap's own CHeapInfo (the SDK calls it m_pParentInfo), at its IDA address when it is one of
	// the game's globals, so the console says which heap ran out in terms IDA can look up.
	static void DescribeHeap(CHeapInstance* pHeap, char* szBuffer, size_t cchBuffer)
	{
		const ULONG_PTR uInfo = (ULONG_PTR)pHeap->m_pParentInfo;
		const ULONG_PTR uImage = (ULONG_PTR)GetModuleHandleW(NULL);
		const IMAGE_NT_HEADERS* pNtHeaders = (const IMAGE_NT_HEADERS*)(uImage + ((const IMAGE_DOS_HEADER*)uImage)->e_lfanew);

		if (uInfo >= uImage && uInfo < uImage + pNtHeaders->OptionalHeader.SizeOfImage)
			_snprintf_s(szBuffer, cchBuffer, _TRUNCATE, "the heap at 0x%llX (index %u)",
				(unsigned long long)(uInfo - uImage + 0x140000000ull), pHeap->m_uIndex);
		else
			_snprintf_s(szBuffer, cchBuffer, _TRUNCATE, "a heap made at run time (index %u)", pHeap->m_uIndex);
	}

	// The heap ReserveMemory tries when this one is full: CHeapInfo::m_pFallbackInfo in the IDB, which
	// the SDK has as m_uPartitionSize. Mem::Startup gives one heap one.
	static CHeapInfo* FallbackOf(CHeapInstance* pHeap)
	{
		const CHeapInfo* pInfo = pHeap->m_pParentInfo;

		return (pInfo) ? (CHeapInfo*)pInfo->m_uPartitionSize : nullptr;
	}

	static HeapKind_t* KindOf(CHeapInstance* pHeap)
	{
		for (HeapKind_t& Kind : s_Kinds)
			if (*(ULONG_PTR**)pHeap == Kind.pVirtualTable)
				return &Kind;

		return nullptr;
	}

	// A new overflow heap with room for cbNeeded more, or null. With the device lock held.
	static OverflowHeap_t* CreateOverflowHeap(CHeapInstance* pFullHeap, unsigned __int64 cbNeeded)
	{
		// Whole MB, with a MB to spare for headers and alignment
		unsigned __int64 cbSize = (cbNeeded + (2ull << 20) - 1) & ~((1ull << 20) - 1);

		if (cbSize < OVERFLOW_HEAP_SIZE)
			cbSize = OVERFLOW_HEAP_SIZE;

		if (s_Stats.cbHeaps + cbSize > MAX_OVERFLOW_TOTAL)
		{
			if (!s_bWarnedFull)
			{
				s_bWarnedFull = true;
				g_pConsole->Warn("Heap Overflow: the overflow heaps are at their %llu MB limit, so a full heap fails as the game's own would\n",
					MAX_OVERFLOW_TOTAL >> 20);
			}

			return nullptr;
		}

		// The game's root list keeps this for as long as the game runs, after 2BHook is unloaded too,
		// so it comes from the process heap and is never freed
		CHeapInfo* pInfo = (CHeapInfo*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(CHeapInfo));

		if (!pInfo)
			return nullptr;

		// CreateHeap takes the heap's own memory from the device's Win32 heap, but the instance and
		// its free lists from the device heap. That one is left out of the overflow, and this stops
		// any other way back in here.
		s_bCreating = true;
		const __int64 bCreated = s_pCreateHeap(pInfo, cbSize, pFullHeap->m_uIndex);
		s_bCreating = false;

		if (!bCreated)
		{
			// Not on the root list: CreateHeap unlinks it when it fails
			HeapFree(GetProcessHeap(), 0, pInfo);
			return nullptr;
		}

		// The index makes the kind (4 is the VRAM one), and the overflow heap takes it from the heap
		// that was full, so it is of the same kind
		HeapKind_t* pKind = KindOf(pInfo->m_pHeap);

		if (!pKind)
			return nullptr;

		s_Heaps.push_back({ pInfo, pKind, pFullHeap->m_uIndex, cbSize });
		++s_Stats.nHeaps;
		s_Stats.cbHeaps += cbSize;

		char szHeap[64];

		DescribeHeap(pFullHeap, szHeap, sizeof(szHeap));
		g_pConsole->Warn("Heap Overflow: %s is full, so overflow heap %u (%llu MB, %llu MB in all) takes its allocations\n",
			szHeap, s_Stats.nHeaps, cbSize >> 20, s_Stats.cbHeaps >> 20);

		return &s_Heaps.back();
	}

	static CHeapAllocInfo* AllocOverflow(HeapKind_t& Kind, CHeapInstance* pFullHeap, __int64 nBytes, unsigned __int64 uAlign, int fFlags)
	{
		CHeapAllocInfo* pBlock = nullptr;

		g_pMemoryDevice->m_Lock.Lock();

		if (!s_bCreating)
		{
			for (OverflowHeap_t& Heap : s_Heaps)
			{
				if (Heap.pKind == &Kind && Heap.uIndex == pFullHeap->m_uIndex)
				{
					pBlock = Kind.oAllocPartition(Heap.pInfo->m_pHeap, nBytes, uAlign, fFlags);

					if (pBlock)
						break;
				}
			}

			if (!pBlock)
			{
				OverflowHeap_t* pHeap = CreateOverflowHeap(pFullHeap, (unsigned __int64)nBytes + uAlign + sizeof(BlockHeader_t));

				if (pHeap && pHeap->pKind == &Kind)
					pBlock = Kind.oAllocPartition(pHeap->pInfo->m_pHeap, nBytes, uAlign, fFlags);
			}

			if (pBlock)
				++s_Stats.nAllocations;
			else
				++s_Stats.nFailed;
		}

		g_pMemoryDevice->m_Lock.Unlock();

		return pBlock;
	}

	static CHeapAllocInfo* AllocPartition(HeapKind_t& Kind, CHeapInstance* pThis, __int64 nBytes, unsigned __int64 uAlign, int fFlags)
	{
		CHeapAllocInfo* pBlock = Kind.oAllocPartition(pThis, nBytes, uAlign, fFlags);

		// The device heap holds the heap instances themselves, CreateHeap's included
		if (pBlock || !Vars.Misc.bHeapOverflow || pThis == &g_pMemoryDevice->m_DeviceHeap)
			return pBlock;

		// A heap with a fallback heap of its own fails as it always did, so ReserveMemory tries the
		// fallback next, which overflows in turn when it is full too
		CHeapInfo* pFallback = FallbackOf(pThis);

		if (pFallback && pFallback->m_pHeap && pFallback->m_pHeap != pThis)
			return nullptr;

		return AllocOverflow(Kind, pThis, nBytes, uAlign, fFlags);
	}

	static CHeapAllocInfo* hkAllocPartition(CHeapInstance* pThis, __int64 nBytes, unsigned __int64 uAlign, int fFlags)
	{
		return AllocPartition(s_Kinds[0], pThis, nBytes, uAlign, fFlags);
	}

	static CHeapAllocInfo* hkVramAllocPartition(CHeapInstance* pThis, __int64 nBytes, unsigned __int64 uAlign, int fFlags)
	{
		return AllocPartition(s_Kinds[1], pThis, nBytes, uAlign, fFlags);
	}

	// Frees what the overflow heaps still hold for pHeap, which is being released whole. Each overflow
	// heap is walked block by block under its lock, and the blocks are freed after, with
	// FreePartition, which takes the lock itself. A block whose m_pAllocationHeap isn't set yet is
	// one AllocPartition just returned, and not pHeap's.
	static void FreeOverflowBlocks(CHeapInstance* pHeap)
	{
		g_pMemoryDevice->m_Lock.Lock();

		std::vector<BlockHeader_t*> Blocks;

		for (OverflowHeap_t& Heap : s_Heaps)
		{
			CHeapInstance* pOverflow = Heap.pInfo->m_pHeap;

			Blocks.clear();

			pOverflow->m_Lock.Lock();

			for (BYTE* pByte = (BYTE*)pOverflow->m_pPrevPartition; pByte && pByte < (BYTE*)pOverflow->m_pNextPartition;
				pByte += sizeof(BlockHeader_t) + ((BlockHeader_t*)pByte)->m_cbAlignedSize)
			{
				BlockHeader_t* pBlock = (BlockHeader_t*)pByte;

				if (pBlock->m_bAllocated && pBlock->m_pAllocationHeap == pHeap)
					Blocks.push_back(pBlock);
			}

			pOverflow->m_Lock.Unlock();

			for (BlockHeader_t* pBlock : Blocks)
				Heap.pKind->oFreePartition(pOverflow, pBlock);
		}

		g_pMemoryDevice->m_Lock.Unlock();
	}

	static CHeapInstance* hkDestroy(CHeapInstance* pThis, char uFlags)
	{
		if (!s_Heaps.empty())
			FreeOverflowBlocks(pThis);

		return s_Kinds[0].oDestroy(pThis, uFlags);
	}

	static CHeapInstance* hkVramDestroy(CHeapInstance* pThis, char uFlags)
	{
		if (!s_Heaps.empty())
			FreeOverflowBlocks(pThis);

		return s_Kinds[1].oDestroy(pThis, uFlags);
	}

	// Swaps one slot of a vtable in the game's read-only data; returns what was there, or 0. A page
	// that is also code keeps its execute right while it is writable, for any thread running there.
	static ULONG_PTR SwapSlot(ULONG_PTR* pVirtualTable, UINT uSlot, ULONG_PTR uFunction)
	{
		ULONG_PTR* pSlot = &pVirtualTable[uSlot];
		MEMORY_BASIC_INFORMATION Info;
		DWORD dwProtect;

		if (!VirtualQuery(pSlot, &Info, sizeof(Info)))
			return 0;

		const DWORD dwWritable = (Info.Protect & (PAGE_EXECUTE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY))
			? PAGE_EXECUTE_READWRITE : PAGE_READWRITE;

		if (!VirtualProtect(pSlot, sizeof(*pSlot), dwWritable, &dwProtect))
			return 0;

		const ULONG_PTR uPrevious = (ULONG_PTR)InterlockedExchangePointer((PVOID*)pSlot, (PVOID)uFunction);

		VirtualProtect(pSlot, sizeof(*pSlot), dwProtect, &dwProtect);

		return uPrevious;
	}

	void Install(void)
	{
		if (s_Status == HEAPOVERFLOW_INSTALLED)
			return;

		const ULONG_PTR uCreateHeap = FindPattern(NULL, s_szCreateHeapPattern);

		s_Status = HEAPOVERFLOW_NOT_FOUND;

		if (!g_pMemoryDevice || !uCreateHeap || uCreateHeap == (ULONG_PTR)-1)
			return;

		s_pCreateHeap = (HeapScale::CreateHeapFn)uCreateHeap;

		// The vtables from the root heaps Mem::Startup made: four main ones and the VRAM one
		g_pMemoryDevice->m_Lock.Lock();

		for (CHeapInfo* pInfo = g_pMemoryDevice->m_pRootInfo; pInfo; pInfo = pInfo->m_pNext)
		{
			const TypeDescriptor* pType = (pInfo->m_pHeap) ? RTTI::GetTypeInfo(pInfo->m_pHeap) : nullptr;

			if (!pType)
				continue;

			for (HeapKind_t& Kind : s_Kinds)
				if (!Kind.pVirtualTable && !strcmp(pType->name, Kind.szTypeName))
					Kind.pVirtualTable = *(ULONG_PTR**)pInfo->m_pHeap;
		}

		g_pMemoryDevice->m_Lock.Unlock();

		if (!s_Kinds[0].pVirtualTable || !s_Kinds[1].pVirtualTable)
			return;

		for (HeapKind_t& Kind : s_Kinds)
		{
			Kind.oFreePartition = (FreePartitionFn)Kind.pVirtualTable[SLOT_FREE_PARTITION];
			Kind.oDestroy = (DestroyFn)Kind.pVirtualTable[SLOT_DESTROY];
			Kind.oAllocPartition = (AllocPartitionFn)Kind.pVirtualTable[SLOT_ALLOC_PARTITION];
		}

		// Destroy first, so a block can't reach an overflow heap before its heap's release frees it
		for (HeapKind_t& Kind : s_Kinds)
			SwapSlot(Kind.pVirtualTable, SLOT_DESTROY, Kind.uDestroyHook);

		for (HeapKind_t& Kind : s_Kinds)
			SwapSlot(Kind.pVirtualTable, SLOT_ALLOC_PARTITION, Kind.uAllocPartitionHook);

		s_Status = HEAPOVERFLOW_INSTALLED;
	}

	void Uninstall(void)
	{
		if (s_Status != HEAPOVERFLOW_INSTALLED)
			return;

		// The overflow heaps stay: blocks in them are still in use, and they are the game's own heaps,
		// on its root list. Released heaps stop freeing their blocks in them, which only leaks.
		for (HeapKind_t& Kind : s_Kinds)
		{
			SwapSlot(Kind.pVirtualTable, SLOT_ALLOC_PARTITION, (ULONG_PTR)Kind.oAllocPartition);
			SwapSlot(Kind.pVirtualTable, SLOT_DESTROY, (ULONG_PTR)Kind.oDestroy);
		}

		s_Status = HEAPOVERFLOW_OFF;
	}

	Status_t GetStatus(void)
	{
		return s_Status;
	}

	Stats_t GetStats(void)
	{
		return s_Stats;
	}
}
