#include "MemoryDeviceHook.h"

CHeapAllocInfo* hkCHeapInstanceAlloc(CHeapInstance* pThis, __int64 nBytes, unsigned __int64 align, int flags)
{
	typedef CHeapAllocInfo* (*OriginalFn)(CHeapInstance*, __int64, unsigned __int64, int);

	CHeapAllocInfo* pAlloc = ((OriginalFn)(g_pMemoryDeviceHook->GetHeapHook().GetFunctionAddress(4)))(pThis, nBytes, align, flags);

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
		g_pMemoryDeviceHook->ExtendHeap(pThis, nBytes);
	}

	return pAlloc;
}