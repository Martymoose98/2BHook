#include "pch.h"
#include "VirtualTableHook.h"

_RTTICompleteObjectLocator* RTTI::GetLocator(const void* pObject)
{
	if (pObject == NULL || (*(const void**)pObject) == NULL)
		return NULL;
	
	return *(_RTTICompleteObjectLocator**)((*(ULONG_PTR*)pObject) - sizeof(void*));
	// return *MakePtr(_RTTICompleteObjectLocator**, *(void**)pObject, -sizeof(void*));
}

const std::type_info* RTTI::GetTypeInfo(const void* pObject)
{
	HMODULE hModule = NULL;

	_RTTICompleteObjectLocator* pLocator = GetLocator(pObject);

	if (!pLocator)
		return NULL;

	// GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT
	// GetModuleHandleEx uses this internally if called with an address so just use it
	if (!RtlPcToFileHeader((PVOID)pLocator, (PVOID*)&hModule))
		return NULL;

	return MakePtr(std::type_info*, hModule, pLocator->pTypeDescriptor);
}

bool RTTI::DerivesFrom(const void* pObject, const std::type_info& Info)
{
	return DerivesFrom(pObject, Info.name());
}

bool RTTI::DerivesFrom(const void* pObject, const char* szTypename) 
{
	HMODULE hModule = NULL;

	_RTTICompleteObjectLocator* pLocator = GetLocator(pObject);

	if (!pLocator)
		return false;
	
	// GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT
	// GetModuleHandleEx uses this internally if called with an address so just use it
	if (!RtlPcToFileHeader((PVOID)pLocator, (PVOID*)&hModule))
		return false;

	_RTTIClassHierarchyDescriptor* pHierarchyDescriptor = MakePtr(_RTTIClassHierarchyDescriptor*, hModule, pLocator->pClassDescriptor);

	if (!pHierarchyDescriptor)
		return false;

	_RTTIBaseClassArray* pBaseClassArr = MakePtr(_RTTIBaseClassArray*, hModule, pHierarchyDescriptor->pBaseClassArray);

	if (!pBaseClassArr)
		return false;

	for (ULONG i = 0; i < pHierarchyDescriptor->numBaseClasses; ++i)
	{
		INT iBaseClassDescriptorRVA = pBaseClassArr->arrayOfBaseClassDescriptors[i];

		if (iBaseClassDescriptorRVA == 0)
			continue;

		_RTTIBaseClassDescriptor* pBaseClassDescriptor = MakePtr(_RTTIBaseClassDescriptor*, hModule, iBaseClassDescriptorRVA);

		if (!pBaseClassDescriptor)
			continue;

		//TypeDescriptor* pTypeDesc = MakePtr(TypeDescriptor*, hModule, pBaseClassDescriptor->pTypeDescriptor);
		std::type_info* pTypeInfo = MakePtr(std::type_info*, hModule, pBaseClassDescriptor->pTypeDescriptor);

		if (!pTypeInfo)
			continue;
		
		// could also used ==
		if (!strcmp(pTypeInfo->name(), szTypename))
			return true;

		//if (pti->name() == szTypename)
		//	return true;

		//if (pTypeDesc->name == szTypename || pTypeDesc->spare == szTypename)
		//	return true;
	}
	
	return false;
}

VirtualTableHook::VirtualTableHook(ULONG_PTR** ppClassBase)
{
	Initialize(ppClassBase);
}

VirtualTableHook::VirtualTableHook(ULONG_PTR*** pppClassBase)
{
	Initialize(*pppClassBase);
}

VirtualTableHook::~VirtualTableHook(void)
{
	Unhook();
	free(m_NewVirtualTable);
}

//TODO: Should VirtualTableHook be hooking on creation??
bool VirtualTableHook::Initialize(ULONG_PTR** ppClassBase)
{
	m_ClassBase = ppClassBase;
	m_OldVirtualTable = *ppClassBase;
	m_VirtualTableSize = GetVirtualTableCount(*ppClassBase);
	m_NewVirtualTable = (ULONG_PTR*)malloc(m_VirtualTableSize * sizeof(ULONG_PTR));

	if (!m_NewVirtualTable)
		return false;

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

void VirtualTableHook::Unhook(void)
{
	if (m_ClassBase)
		*m_ClassBase = m_OldVirtualTable;
}

void VirtualTableHook::Rehook(void)
{
	if (m_ClassBase)
		*m_ClassBase = m_NewVirtualTable;
}

ULONG_PTR VirtualTableHook::GetFunctionCount(void) const
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

ULONG_PTR* VirtualTableHook::GetOldVirtualTable(void) const
{
	return m_OldVirtualTable;
}

ULONG_PTR VirtualTableHook::GetVirtualTableCount(ULONG_PTR* pVirtualTable) const
{
	ULONG_PTR uIndex = 0;

	for (; pVirtualTable[uIndex]; ++uIndex)
		if (IsBadCodePtr((const void*)pVirtualTable[uIndex]))
			break;

	return uIndex;
}