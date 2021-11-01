#pragma once
#include <Windows.h>
#include "Memory.h"

typedef ULONGLONG QWORD;

#if defined(__cplusplus) && defined(TEMPLATE_VTABLEHOOK)

/*
	WIP

  Example use:
	VirtualTableHook<Keyboard_t> hook
*/
template<typename T>
class VirtualTableHook
{
public:
	template<typename T>
	struct Interface { ULONG_PTR* vtbl; };

	VirtualTableHook() { ZeroMemory(this, sizeof(VirtualTableHook)); }

	VirtualTableHook(T* pClassBase)
		: m_ClassBase(pClassBase), m_OldVirtualTable(m_ClassBase->vtbl)
	{ 
		m_VirtualTableSize = GetVirtualTableCount(m_ClassBase->vtbl);
		m_NewVirtualTable = (ULONG_PTR*)malloc(m_VirtualTableSize * sizeof(ULONG_PTR));
		memcpy(m_NewVirtualTable, m_OldVirtualTable, m_VirtualTableSize * sizeof(ULONG_PTR));
		m_pClassBase = m_NewVirtualTable;
	}

	~VirtualTableHook();

	void Relocate(T* ppqwClassBase);

	void Unhook();
	void Rehook();

	ULONG_PTR GetFunctionCount() const;

	template<typename Fn>
	Fn GetFunctionAddress(UINT uIndex) const;

	ULONG_PTR* GetOldVirtualTable()	const;

	template<typename Fn>
	Fn HookFunction(Fn NewFunc, UINT uIndex);

private:
	ULONG_PTR GetVirtualTableCount(ULONG_PTR* pqwVirtualTable) const;

	Interface<T>* m_ClassBase;
	ULONG_PTR* m_NewVirtualTable;
	ULONG_PTR* m_OldVirtualTable;
	ULONG_PTR m_VirtualTableSize;
};

#elif defined(__cplusplus)

class VirtualTableHook
{
public:
	VirtualTableHook(void);
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

	ULONG_PTR* GetOldVirtualTable(void)	const;
	ULONG_PTR  HookFunction(ULONG_PTR uNewFunction, UINT uIndex);

private:
	ULONG_PTR GetVirtualTableCount(ULONG_PTR* pqwVirtualTable) const;

	ULONG_PTR** m_ClassBase;
	ULONG_PTR* m_NewVirtualTable;
	ULONG_PTR* m_OldVirtualTable;
	ULONG_PTR m_VirtualTableSize;
};

#else

typedef struct VirtualTableHook_t
{
	ULONG_PTR** m_ClassBase;
	ULONG_PTR* m_NewVirtualTable;
	ULONG_PTR* m_OldVirtualTable;
	ULONG_PTR m_VirtualTableSize;
} VirtualTableHook;

#endif