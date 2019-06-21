#include "VirtualTableHook.h"

VirtualTableHook::VirtualTableHook()
{
	ZeroMemory(this, sizeof(VirtualTableHook));
}

VirtualTableHook::VirtualTableHook(ULONG_PTR** ppqwClassBase)
{
	Initialize(ppqwClassBase);
}

VirtualTableHook::VirtualTableHook(ULONG_PTR*** pppqwClassBase)
{
	Initialize(*pppqwClassBase);
}

VirtualTableHook::~VirtualTableHook()
{
	Unhook();
	free(m_NewVirtualTable);
}

bool VirtualTableHook::Initialize(ULONG_PTR** ppqwClassBase)
{
	m_ClassBase = ppqwClassBase;
	m_OldVirtualTable = *ppqwClassBase;
	m_VirtualTableSize = GetVirtualTableCount(*ppqwClassBase);
	m_NewVirtualTable = (ULONG_PTR*)malloc(m_VirtualTableSize * sizeof(ULONG_PTR));
	memcpy(m_NewVirtualTable, m_OldVirtualTable, m_VirtualTableSize * sizeof(ULONG_PTR));
	*ppqwClassBase = m_NewVirtualTable;
	return true;
}

bool VirtualTableHook::Initialize(ULONG_PTR*** pppqwClassBase)
{
	return Initialize(*pppqwClassBase);
}

void VirtualTableHook::Relocate(ULONG_PTR** ppqwClassBase)
{
	Unhook();
	m_ClassBase = ppqwClassBase;
	m_OldVirtualTable = *ppqwClassBase;
	*ppqwClassBase = m_NewVirtualTable;
}

void VirtualTableHook::Unhook()
{
	if (m_ClassBase)
		*m_ClassBase = m_OldVirtualTable;
}

void VirtualTableHook::Rehook()
{
	if (m_ClassBase)
		*m_ClassBase = m_NewVirtualTable;
}

ULONG_PTR VirtualTableHook::GetFunctionCount() const
{
	return m_VirtualTableSize;
}

ULONG_PTR VirtualTableHook::GetFunctionAddress(UINT uIndex) const
{
	if (uIndex >= 0 && uIndex <= m_VirtualTableSize && m_OldVirtualTable)
		return m_OldVirtualTable[uIndex];

	return NULL;
}

ULONG_PTR VirtualTableHook::HookFunction(ULONG_PTR qwNewFunction, UINT uIndex)
{
	if (m_NewVirtualTable && m_OldVirtualTable && uIndex >= 0 && uIndex <= m_VirtualTableSize)
	{
		m_NewVirtualTable[uIndex] = qwNewFunction;
		return m_OldVirtualTable[uIndex];
	}
	
	return NULL;
}

ULONG_PTR VirtualTableHook::GetVirtualTableCount(ULONG_PTR* pqwVirtualTable) const
{
	QWORD qwIndex = 0;

	for (; pqwVirtualTable[qwIndex]; ++qwIndex)
		if (CMemory::IsBadCodePtr((const void*)pqwVirtualTable[qwIndex]))
			break;

	return qwIndex;
}

ULONG_PTR* VirtualTableHook::GetOldVirtualTable() const
{
	return m_OldVirtualTable;
}