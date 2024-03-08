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