#pragma once

#include <vector>
#include "VirtualTableHook.h"
#include "ReversedStructs.h"

CHeapAllocInfo* hkCHeapInstanceAlloc(CHeapInstance* pThis, __int64 nBytes, unsigned __int64 align, int flags);

class MemoryDeviceHook
{
public:

	MemoryDeviceHook(CMemoryDevice* pDevice) 
		: m_pDevice(pDevice), m_DeviceHook((ULONG_PTR**)pDevice), m_DeviceHeapHook((ULONG_PTR**)&pDevice->m_DeviceHeap)
	{
		m_DeviceHeapHook.HookFunction((ULONG_PTR)hkCHeapInstanceAlloc, 4); // NOTE: can make seperate function to handle this
		//Initalize();
	}

	// Hook all of the heaps
	void Initalize(void)
	{
		for (CHeapAllocInfo* pAllocInfo = m_pDevice->m_DeviceHeap.m_pAllocationInfo; pAllocInfo; pAllocInfo = pAllocInfo->m_pPrevious)
			AddHook(pAllocInfo->m_pHeap);
	}

	void AddHook(CHeapInstance* pHeap)
	{
		if (!pHeap)
			return;

		VirtualTableHook hook((ULONG_PTR**)pHeap);

		hook.HookFunction((ULONG_PTR)hkCHeapInstanceAlloc, 4);

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

		while (pRootInfo->m_pParentInfo) pRootInfo = pRootInfo->m_pParentInfo;

		SIZE_T cbNeededUnaligned = pRootInfo->m_pHeap->m_pAllocationInfo->m_cbSize + nBytes;
		SIZE_T cbNeeded = ALIGN(cbNeededUnaligned, pRootInfo->m_pHeap->m_qwMemoryAlignment);

		m_pDevice->m_Lock.Lock();

		pHeap->m_Lock.Lock();

		CHeapInfo* pInfo = (CHeapInfo*)HeapReAlloc(m_pDevice->m_hHeap, HEAP_NO_SERIALIZE, pHeap->m_pInfo, cbNeeded);

		// fix parent, child, next and prev pointers
		if (pHeap->m_pParentInfo && pHeap->m_pParentInfo->m_pChildInfo)
			pHeap->m_pParentInfo->m_pChildInfo = pInfo;

		if (pHeap->m_pParentInfo && pHeap->m_pParentInfo->m_pChildInfo)
			pHeap->m_pChildren->m_pParentInfo = pInfo;

		if (pHeap->m_pParentInfo && pHeap->m_pParentInfo->m_pChildInfo)
			pHeap->m_pInfo->m_pNext->m_pPrevious = pInfo;

		if (pHeap->m_pParentInfo && pHeap->m_pParentInfo->m_pChildInfo)
			pHeap->m_pInfo->m_pPrevious->m_pNext = pInfo;

		pHeap->m_pInfo = pInfo;

		pHeap->m_Lock.Unlock();

		m_pDevice->m_Lock.Unlock();
	}

	bool IsChildHeapAllocation(CHeapInstance* pHeap) { return pHeap == &m_pDevice->m_DeviceHeap; } // check if it's a call to CMemoryDevice::AllocChildHeap
	VirtualTableHook& GetHeapHook() { return m_DeviceHeapHook; }

private:
	CMemoryDevice* m_pDevice;
	VirtualTableHook m_DeviceHook;
	VirtualTableHook m_DeviceHeapHook;
	std::vector<VirtualTableHook> m_HeapHooks;
};

#include "Globals.h" // cyclic dependancy if on top