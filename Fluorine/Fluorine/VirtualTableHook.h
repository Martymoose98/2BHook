#pragma once

#include <Windows.h>
#include <crtdefs.h>
#include <vcruntime.h>
#include <rttidata.h>
#include <stdint.h>
#include "Memory.h"

#include <typeinfo> // maybe temp

typedef ULONGLONG QWORD;

#include <typeinfo>
#include <vcruntime.h>
#include <rttidata.h>


#if defined(__cplusplus)
// TODO: https://github.com/praydog/AutomataMP/blob/main/shared/utility/RTTI.hpp
// TODO: https://github.com/praydog/AutomataMP/blob/main/shared/utility/RTTI.cpp

// utility::get_module_within has a bug lol
// TypeDescriptor::spare is undecorated name (const char*)
// after std::type_info::name is called
// 
// https://github.com/strivexjun/MemoryModulePP/blob/master/MemoryModulePP.h
//
// NOTE: need to upgrade to at least C++17 for std::string_view 
// NOTE: std::type_info is equal TypeDescriptor to just with vftable
// NOTE: typeid(type) = const std::type_info&
//
// utility::get_module_within has a bug lol
// Run-time Type Information
namespace RTTI
{
	extern _RTTICompleteObjectLocator* GetLocator(const void* pObject);
	extern const std::type_info* GetTypeInfo(const void* pObject);
	extern bool DerivesFrom(const void* pObject, const char* szTypename);
	extern bool DerivesFrom(const void* pObject, const std::type_info& Info);
}

namespace Virtual
{
	//
	//template<typename Fn>
	//Fn Get(void* pClassBase, uint32_t vidx);
	//template<typename R, typename Base, typename... Args>
	//R(__thiscall* GetT(Base* pClassBase, uint32_t vidx))(Base*, Args...);

	template<typename Fn>
	Fn Get(void* pClassBase, uint32_t vidx)
	{
		return (*reinterpret_cast<Fn**>(pClassBase))[vidx];
	}

	template<typename R, typename Base, typename... Args>
	R(__thiscall* GetT(Base* pClassBase, uint32_t vidx))(Base*, Args...)
	{
		typedef R(__thiscall* Fn)(Base*, Args...);
		return (*reinterpret_cast<Fn**>(pClassBase))[vidx];
	}

	template<typename R, typename Base, typename... Args>
	R Call(Base* pClassBase, uint32_t vidx, Args... Params)
	{
		typedef R(__thiscall Base::* Fn)(Args...);
		Fn** vft = ((Fn**)pClassBase);
		Fn vfunc = (*vft)[vidx];

		return (pClassBase->*vfunc)(Params...);

		//return (pClassBase->_)(Params...);
		//return ((pClassBase->*vft)[vidx])(Params...);
	}
}

/*
	WIP
	
	Could use a template var for vftable size (for interface wrappers)
	TODO: make iface wrapper class probs templated

  Example use:
	VirtualTableHookT<Keyboard_t> hook
*/
template<typename T>
class VirtualTableHookT
{
public:
	template<typename T>
	struct Interface { ULONG_PTR* vtbl; };

	VirtualTableHookT(T* pClassBase)
		: m_pClassBase(pClassBase), m_OldVirtualTable(m_pClassBase.vtbl)
	{ 
		m_VirtualTableSize = GetVirtualTableCount(m_pClassBase.vtbl);
		m_NewVirtualTable = (ULONG_PTR*)malloc(m_VirtualTableSize * sizeof(ULONG_PTR));
		memcpy(m_NewVirtualTable, m_OldVirtualTable, m_VirtualTableSize * sizeof(ULONG_PTR));
		m_pClassBase.vtbl = m_NewVirtualTable;
	}

	~VirtualTableHookT(void) 
	{
		Unhook();
		free(m_NewVirtualTable);
	}

	void Relocate(T* pClassBase)
	{
		Unhook();
		m_pClassBase = pClassBase;
		m_OldVirtualTable = m_pClassBase->vtbl;
		Rehook();
	}

	void Unhook(void)
	{
		m_pClassBase.vtbl = m_OldVirtualTable;
	}

	void Rehook(void)
	{
		m_pClassBase.vtbl = m_NewVirtualTable;
	}

	ULONG_PTR GetFunctionCount(void) const { return m_VirtualTableSize; }

	template<typename Fn>
	Fn GetFunctionAddress(UINT uIndex) const
	{
		if (uIndex >= 0 && uIndex <= m_VirtualTableSize && m_OldVirtualTable)
			return m_OldVirtualTable[uIndex];

		return NULL;
	}

	ULONG_PTR* GetOldVirtualTable(void)	const { return m_OldVirtualTable; }

	template<typename Fn>
	Fn HookFunction(Fn NewFunction, UINT uIndex)
	{
		if (m_NewVirtualTable && m_OldVirtualTable && uIndex >= 0 && uIndex <= m_VirtualTableSize)
		{
			reinterpret_cast<Fn*>(m_NewVirtualTable)[uIndex] = NewFunction;
			return reinterpret_cast<Fn*>(m_OldVirtualTable)[uIndex];
		}

		return NULL;
	}

	template<typename Ret, typename... Args>
	Ret CallFunction(Args... Params, UINT uIndex)
	{
		reinterpret_cast<Ret(__thiscall*)(T*, Args...)>(m_OldVirtualTable[uIndex])(m_pClassBase, Params...);
	}
	

private:
	ULONG_PTR GetVirtualTableCount(ULONG_PTR* pVirtualTable) const
	{
		ULONG_PTR uIndex = 0;

		for (; pVirtualTable[uIndex]; ++uIndex)
			if (IsBadCodePtr((const void*)pVirtualTable[uIndex]))
				break;

		return uIndex;
	}

	Interface<T>* m_pClassBase;
	ULONG_PTR* m_NewVirtualTable;
	ULONG_PTR* m_OldVirtualTable;
	ULONG_PTR m_VirtualTableSize;
};

class VirtualTableHook
{
public:
	VirtualTableHook(ULONG_PTR** ppqwClassBase);
	VirtualTableHook(ULONG_PTR*** pppqwClassBase);
	~VirtualTableHook(void);

	bool Initialize(ULONG_PTR** ppqwClassBase);
	bool Initialize(ULONG_PTR*** pppqwClassBase);

	void Relocate(ULONG_PTR** ppqwClassBase);

	void Unhook(void);
	void Rehook(void);

	ULONG_PTR GetFunctionCount(void) const;
	ULONG_PTR GetFunctionAddress(UINT uIndex) const;

	template<typename R, typename Base, typename... Args>
	R(__thiscall* GetFunction(UINT uIndex) const)(const Base*, Args...)
	{
		typedef R(__thiscall* OriginalFn)(const Base*, Args...);

		if (uIndex >= 0 && uIndex <= m_VirtualTableSize && m_OldVirtualTable)
			return (OriginalFn)m_OldVirtualTable[uIndex];

		return NULL;
	}

	ULONG_PTR HookFunction(ULONG_PTR uNewFunction, UINT uIndex);

	/*template<typename Fn>
	R Call(uint32_t vidx, Args... Params)
	{
		typedef R(__thiscall* OriginalFn)(Base*, Args...);
		return ((OriginalFn)m_OldVirtualTable[vidx])(m_ClassBase, Params...);
	}*/

	template<typename R, typename Base, typename... Args>
	R Call(uint32_t vidx, Args... Params)
	{
		typedef R(__thiscall* OriginalFn)(const Base*, Args...);
		return ((OriginalFn)m_OldVirtualTable[vidx])((const Base*)m_ClassBase, Params...);
	}

	ULONG_PTR* GetOldVirtualTable(void)	const;
private:
	ULONG_PTR GetVirtualTableCount(ULONG_PTR* pqwVirtualTable) const;

	ULONG_PTR** m_ClassBase;
	ULONG_PTR* m_NewVirtualTable;
	ULONG_PTR* m_OldVirtualTable;
	ULONG_PTR m_VirtualTableSize;
};

#else

typedef struct VirtualTableHook
{
	ULONG_PTR** m_ClassBase;
	ULONG_PTR* m_NewVirtualTable;
	ULONG_PTR* m_OldVirtualTable;
	ULONG_PTR m_VirtualTableSize;
} VirtualTableHook;

#endif // __cplusplus