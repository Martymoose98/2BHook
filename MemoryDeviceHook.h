#pragma once

#include "WinInclude.h"

#include <vector>

#include <Fluorine/VirtualTableHook.h>

#include "ReversedStructs.h"

BOOL hkCHeapInstanceSetHeapInfo(CHeapInstance* pThis, CHeapInfo* pInfo, unsigned __int64 cbSize, int nChildren, int flags);
CHeapAllocInfo* hkCHeapInstanceAllocPartition(CHeapInstance* pThis, __int64 nBytes, unsigned __int64 align, int flags);

class MemoryDeviceHook
{
public:
	typedef __int64 (*CMemoryDevice_CreateChildHeapInternalFn)(CMemoryDevice* pThis, CHeapInfo* pInfo, int nChildren, __int64 cbSize, __int64 fFlags, CHeapInfo* pParentInfo);
	typedef void (*CMemoryDevice_ReleaseHeapFn)(CMemoryDevice* pThis, CHeapInstance* pInstance, CHeapInfo* pInfo);

	typedef BOOL(*CMemorySystem_CreateHeapFn)(CHeapInfo* pInfo, unsigned __int64 cbSize, unsigned int uHeapIndex);
	typedef BOOL(*CMemorySystem_CreateChildHeapFn)(CHeapInfo* pInfo, unsigned __int64 cbSize, CHeapInfo* pParentInfo);

	MemoryDeviceHook(CMemoryDevice* pDevice) 
		: m_pDevice(pDevice), m_DeviceHook((ULONG_PTR**)pDevice), m_DeviceHeapHook((ULONG_PTR**)&pDevice->m_DeviceHeap)
	{
		m_DeviceHeapHook.HookFunction((ULONG_PTR)&hkCHeapInstanceSetHeapInfo, 1);
		m_DeviceHeapHook.HookFunction((ULONG_PTR)&hkCHeapInstanceAllocPartition, 4); // NOTE: can make seperate function to handle this

		// WTF CAUSES CRASHES
		//ReleaseHeap = (CMemoryDevice_ReleaseHeapFn)FindPattern(NULL, "48 85 D2 0F 84 ? ? ? ? 48 89 5C 24 ? 57 48 83 EC 20 48 8B DA 49 8B F8");
		//CreateHeap = (CMemorySystem_CreateHeapFn)FindPattern(NULL, "40 56 57 41 56 48 83 EC 40 48 C7 44 24 ? ? ? ? ? 48 89 5C 24 ? 48 89 6C 24 ? 41 8B F0");
		//CreateChildHeap = (CMemorySystem_CreateChildHeapFn)FindPattern(NULL, "48 83 EC 48 4C 89 44 24");
		//CreateChildHeapInternal = (CMemoryDevice_CreateChildHeapInternalFn)FindPattern(NULL, "48 89 4C 24 ? 57 41 56 41 57 48 83 EC 40 48 C7 44 24 ? ? ? ? ? 48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 49 8B F1");
	}

	// Hook all of the heaps
	void Initialize(void)
	{
		for (CHeapAllocInfo* pAllocInfo = m_pDevice->m_DeviceHeap.m_pAllocationInfo; pAllocInfo; pAllocInfo = pAllocInfo->m_pPrevious)
			AddHook(pAllocInfo->m_pHeapInfo->m_pHeap);
	}

	void AddHook(CHeapInstance* pHeap)
	{
		if (!pHeap)
			return;

		VirtualTableHook hook((ULONG_PTR**)pHeap);

		hook.HookFunction((ULONG_PTR)&hkCHeapInstanceAllocPartition, 4);

		m_HeapHooks.emplace_back(hook);
	}

	// seems to be only 1 level of heaps root -> child
	void ExtendHeap(CHeapInstance* pHeap, __int64 nBytes)
	{
		if (!pHeap)
			return;

		// Find root heap
		CHeapInfo* pRootInfo = pHeap->m_pInfo;

		if (!pRootInfo)
			return;

		//	while (pRootInfo->m_pParent) pRootInfo-> = pRootInfo->m_pParent;

		SIZE_T cbNeededUnaligned = pRootInfo->m_qwHeapSize + nBytes;
		SIZE_T cbNeeded = ALIGN(cbNeededUnaligned, pRootInfo->m_pHeap->m_qwMemoryAlignment);

		m_pDevice->m_Lock.Lock();

		pHeap->m_Lock.Lock();

		CHeapInfo* pInfo = (CHeapInfo*)HeapReAlloc(m_pDevice->m_hHeap, HEAP_NO_SERIALIZE, pHeap->m_pInfo, cbNeeded);

		if (!pInfo)
		{
			pHeap->m_Lock.Unlock();
			m_pDevice->m_Lock.Unlock();
			return;
		}

		// fix parent, child, next and prev pointers. HeapReAlloc may have moved the block and freed the
		// old one, so the links are read from the new block, which holds a copy (they were read
		// through pHeap->m_pInfo, the freed block)
		if (pHeap->m_pParentInfo && pHeap->m_pParentInfo->m_pChildInfo)
			pHeap->m_pParentInfo->m_pChildInfo = pInfo;

		//if (pHeap->m_pParentInfo && pHeap->m_pParentInfo->m_pChildInfo)
		//	pHeap->m_pNextPartition->= pInfo;

		if (pHeap->m_pParentInfo && pHeap->m_pParentInfo->m_pChildInfo && pInfo->m_pNext)
			pInfo->m_pNext->m_pPrevious = pInfo;

		if (pHeap->m_pParentInfo && pHeap->m_pParentInfo->m_pChildInfo && pInfo->m_pPrevious)
			pInfo->m_pPrevious->m_pNext = pInfo;

		pHeap->m_pInfo = pInfo;

		pHeap->m_Lock.Unlock();

		m_pDevice->m_Lock.Unlock();
	}

	bool IsChildHeapAllocation(CHeapInstance* pHeap) { return pHeap != &m_pDevice->m_DeviceHeap; } // check if it's a call to CMemoryDevice::AllocChildHeap
	VirtualTableHook& GetHeapHook() { return m_DeviceHeapHook; }
	VirtualTableHook& GetDeviceHook() { return m_DeviceHook; }

private:
	CMemoryDevice* m_pDevice;

	// TODO: probs make static and const/constexpr
	static CMemoryDevice_CreateChildHeapInternalFn CreateChildHeapInternal;
	static CMemoryDevice_ReleaseHeapFn ReleaseHeap;
	static CMemorySystem_CreateHeapFn CreateHeap;
	static CMemorySystem_CreateChildHeapFn CreateChildHeap;

	VirtualTableHook m_DeviceHook;
	VirtualTableHook m_DeviceHeapHook;
	std::vector<VirtualTableHook> m_HeapHooks;
};

// Bigger heaps. The game makes all its memory heaps up front: Mem::Startup (0x14086B970) asks
// CMemorySystem::CreateHeap (0x1402623B0) for five roots, about 4.4 GB between them, each from the
// Win32 heap, and every other heap is carved out of one of those by
// CMemoryDevice::CreateChildHeapInternal (0x140261DE0), at startup or when a map block, the UI or a
// movie needs one. Their sizes are fixed, so loading more than a map plans for (Create Entity on
// anything) can run a heap out and stop the game with its out-of-memory box.
//
// This multiplies every heap's size by the "Heap Scale" setting (i_heap_scale in default.xml,
// read here since the menu's config loads too late): the roots, and each child whose parent was
// made bigger, so every child still fits in its parent. It has to be in place before Mem::Startup
// runs, so it only works when the mod loader loads 2BHook from the mods folder at startup;
// injected later, it finds the heaps already made and leaves them alone. ExcitedDelirium does the
// same at a fixed 2x (MemSystem.h, HEAP_SCALE_MAIN / HEAP_SCALE_VRAM).
//
// MemoryDeviceHook above is the vtable approach, still a work in progress (off behind
// ENABLE_MEMORYDEVICE_HOOK in DLLMain.cpp); HeapOverflow below takes the same vtable route.
namespace HeapScale
{
	enum Status_t
	{
		HEAPSCALE_OFF,				// Heap Scale is 1, or the setting couldn't be read
		HEAPSCALE_APPLIED,			// the heaps were made bigger at startup
		HEAPSCALE_TOO_LATE,		// 2BHook was loaded after the game made its heaps
		HEAPSCALE_NOT_FOUND,		// a signature didn't match this build
	};

	constexpr int MAX_SCALE = 4;

	// From DllMain, before anything else: reads the setting and hooks heap creation
	void InstallEarly(HINSTANCE hInstance);
	void Uninstall(void);

	int GetScale(void);			// what the heaps were made with, 1 when off
	Status_t GetStatus(void);
}

// Overflow heaps, for a heap that is full: the vtable way, which needs no early start. Every game heap
// allocates through its vtable's AllocPartition (slot 4), and they all share one of two vtables:
// cHeapInstance, or cHeapVramInstance for the VRAM root (index 4) and its children. With the slot
// patched, an allocation the heap can't make goes to an overflow heap of the same kind and index, the
// game's own root heap made on demand with CMemorySystem::CreateHeap, instead of failing into the
// out-of-memory box.
//
// Nothing moves: the full heap isn't grown, so no pointer into it changes. A block's header names
// the heap it was carved from (+0x08) and the heap it was allocated for (+0x10), and the game's free
// path reads both, so a block from an overflow heap frees back into it. The game's own fallback heap
// (CHeapInfo::m_pFallbackInfo, CHeapInstance::ReserveMemory 0x1402616F0) works the same way.
//
// A heap released whole (a map block, CMemoryDevice::ReleaseHeap 0x140261A90) frees the blocks in its
// own memory by giving that memory back, never one by one, so its blocks in an overflow heap would
// stay. Destroy (slot 0), which ReleaseHeap calls last, frees them first.
namespace HeapOverflow
{
	enum Status_t
	{
		HEAPOVERFLOW_OFF,			// not installed (yet)
		HEAPOVERFLOW_INSTALLED,		// the vtables are patched; Heap Overflow in the menu turns it on
		HEAPOVERFLOW_NOT_FOUND,		// CreateHeap or a heap vtable wasn't found
	};

	// An overflow heap is this big, or bigger for a bigger block, and all of them together no more
	// than MAX_OVERFLOW_TOTAL: a heap the game fills until an allocation fails (a cache) stops there.
	constexpr unsigned __int64 OVERFLOW_HEAP_SIZE = 256ull << 20;
	constexpr unsigned __int64 MAX_OVERFLOW_TOTAL = 4ull << 30;

	struct Stats_t
	{
		unsigned int		nHeaps;			// overflow heaps made
		unsigned __int64	cbHeaps;		// their total size
		unsigned __int64	nAllocations;	// allocations they took for a full heap
		unsigned __int64	nFailed;		// allocations that failed anyway, past MAX_OVERFLOW_TOTAL
	};

	// From Setup, with g_pMemoryDevice found: the game has made its heaps by then
	void Install(void);
	void Uninstall(void);

	Status_t GetStatus(void);
	Stats_t GetStats(void);
}

#include "Globals.h" // cyclic dependancy if on top