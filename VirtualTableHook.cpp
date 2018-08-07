#include "VirtualTableHook.h"

VirtualTableHook::VirtualTableHook()
{
	ZeroMemory(this, sizeof(VirtualTableHook));
}

VirtualTableHook::VirtualTableHook(QWORD** ppqwClassBase)
{
	Initialize(ppqwClassBase);
}

VirtualTableHook::VirtualTableHook(QWORD*** pppqwClassBase)
{
	Initialize(*pppqwClassBase);
}

VirtualTableHook::~VirtualTableHook()
{
	Unhook();
	free(m_NewVirtualTable);
}

bool VirtualTableHook::Initialize(QWORD** ppqwClassBase)
{
	m_ClassBase = ppqwClassBase;
	m_OldVirtualTable = *ppqwClassBase;
	m_VirtualTableSize = GetVirtualTableCount(*ppqwClassBase);
	m_NewVirtualTable = (QWORD*)malloc(m_VirtualTableSize * sizeof(QWORD));
	memcpy(m_NewVirtualTable, m_OldVirtualTable, m_VirtualTableSize * sizeof(QWORD));
	*ppqwClassBase = m_NewVirtualTable;
	return true;
}

bool VirtualTableHook::Initialize(QWORD*** pppqwClassBase)
{
	return Initialize(*pppqwClassBase);
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

QWORD VirtualTableHook::GetFunctionCount() const
{
	return m_VirtualTableSize;
}

QWORD VirtualTableHook::GetFunctionAddress(UINT uIndex) const
{
	if (uIndex >= 0 && uIndex <= m_VirtualTableSize && m_OldVirtualTable)
		return m_OldVirtualTable[uIndex];

	return NULL;
}

QWORD VirtualTableHook::HookFunction(QWORD qwNewFunction, UINT uIndex)
{
	if (m_NewVirtualTable && m_OldVirtualTable && uIndex >= 0 && uIndex <= m_VirtualTableSize)
	{
		m_NewVirtualTable[uIndex] = qwNewFunction;
		return m_OldVirtualTable[uIndex];
	}
	
	return NULL;
}

QWORD VirtualTableHook::GetVirtualTableCount(QWORD* pqwVirtualTable) const
{
	QWORD qwIndex = 0;

	for (; pqwVirtualTable[qwIndex]; ++qwIndex)
		if (CMemory::IsBadCodePtr((const void*)pqwVirtualTable[qwIndex]))
			break;

	return qwIndex;
}

QWORD* VirtualTableHook::GetOldVirtualTable() const
{
	return m_OldVirtualTable;
}