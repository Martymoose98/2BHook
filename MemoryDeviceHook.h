#pragma once

#include <vector>
#include "VirtualTableHook.h"
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
	void Initalize(void)
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

		// fix parent, child, next and prev pointers
		if (pHeap->m_pParentInfo && pHeap->m_pParentInfo->m_pChildInfo)
			pHeap->m_pParentInfo->m_pChildInfo = pInfo;

		//if (pHeap->m_pParentInfo && pHeap->m_pParentInfo->m_pChildInfo)
		//	pHeap->m_pNextPartition->= pInfo;

		if (pHeap->m_pParentInfo && pHeap->m_pParentInfo->m_pChildInfo)
			pHeap->m_pInfo->m_pNext->m_pPrevious = pInfo;

		if (pHeap->m_pParentInfo && pHeap->m_pParentInfo->m_pChildInfo)
			pHeap->m_pInfo->m_pPrevious->m_pNext = pInfo;

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

#include "Globals.h" // cyclic dependancy if on top