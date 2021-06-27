#include "VirtualTableHook.h"

VirtualTableHook::VirtualTableHook()
{
	ZeroMemory(this, sizeof(VirtualTableHook));
}

VirtualTableHook::VirtualTableHook(ULONG_PTR** ppClassBase)
{
	Initialize(ppClassBase);
}

VirtualTableHook::VirtualTableHook(ULONG_PTR*** pppClassBase)
{
	Initialize(*pppClassBase);
}

VirtualTableHook::~VirtualTableHook()
{
	Unhook();
	free(m_NewVirtualTable);
}

//TODO("Should VirtualTableHook be hooking on creation??")
bool VirtualTableHook::Initialize(ULONG_PTR** ppClassBase)
{
	m_ClassBase = ppClassBase;
	m_OldVirtualTable = *ppClassBase;
	m_VirtualTableSize = GetVirtualTableCount(*ppClassBase);
	m_NewVirtualTable = (ULONG_PTR*)malloc(m_VirtualTableSize * sizeof(ULONG_PTR));
	memcpy(m_NewVirtualTable, m_OldVirtualTable, m_VirtualTableSize * sizeof(ULONG_PTR));
	*m_ClassBase = m_NewVirtualTable;
	return true;
}

bool VirtualTableHook::Initialize(ULONG_PTR*** pppClassBase)
{
	return Initialize(*pppClassBase);
}

void VirtualTableHook::Relocate(ULONG_PTR** ppClassBase)
{
	Unhook();
	m_ClassBase = ppClassBase;
	m_OldVirtualTable = *ppClassBase;
	Rehook();
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

ULONG_PTR VirtualTableHook::HookFunction(ULONG_PTR uNewFunction, UINT uIndex)
{
	if (m_NewVirtualTable && m_OldVirtualTable && uIndex >= 0 && uIndex <= m_VirtualTableSize)
	{
		m_NewVirtualTable[uIndex] = uNewFunction;
		return m_OldVirtualTable[uIndex];
	}
	
	return NULL;
}

ULONG_PTR VirtualTableHook::GetVirtualTableCount(ULONG_PTR* pVirtualTable) const
{
	ULONG_PTR uIndex = 0;

	for (; pVirtualTable[uIndex]; ++uIndex)
		if (CMemory::IsBadCodePtr((const void*)pVirtualTable[uIndex]))
			break;

	return uIndex;
}

ULONG_PTR* VirtualTableHook::GetOldVirtualTable() const
{
	return m_OldVirtualTable;
}