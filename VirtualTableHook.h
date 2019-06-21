#pragma once
#include <Windows.h>
#include "Memory.h"

typedef ULONGLONG QWORD;

class VirtualTableHook
{
public:
	VirtualTableHook();
	VirtualTableHook(ULONG_PTR** ppqwClassBase);
	VirtualTableHook(ULONG_PTR*** pppqwClassBase);
	~VirtualTableHook();

	bool Initialize(ULONG_PTR** ppqwClassBase);
	bool Initialize(ULONG_PTR*** pppqwClassBase);

	void Relocate(ULONG_PTR** ppqwClassBase);

	void Unhook();
	void Rehook();

	ULONG_PTR GetFunctionCount() const;
	ULONG_PTR GetFunctionAddress(UINT uIndex) const;

	ULONG_PTR* GetOldVirtualTable()	const;
	ULONG_PTR  HookFunction(ULONG_PTR qwNewFunc, UINT uIndex);

private:
	ULONG_PTR GetVirtualTableCount(ULONG_PTR* pqwVirtualTable) const;

	ULONG_PTR** m_ClassBase;
	ULONG_PTR* m_NewVirtualTable;
	ULONG_PTR* m_OldVirtualTable;
	ULONG_PTR m_VirtualTableSize;
};