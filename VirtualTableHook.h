#pragma once
#include <Windows.h>
#include "Memory.h"

typedef ULONGLONG QWORD;

class VirtualTableHook
{
public:
	VirtualTableHook();
	VirtualTableHook(QWORD** ppqwClassBase);
	VirtualTableHook(QWORD*** pppqwClassBase);
	~VirtualTableHook();

	bool Initialize(QWORD** ppqwClassBase);
	bool Initialize(QWORD*** pppqwClassBase);

	void Unhook();
	void Rehook();

	QWORD GetFunctionCount() const;
	QWORD GetFunctionAddress(UINT uIndex) const;

	QWORD* GetOldVirtualTable()	const;
	QWORD  HookFunction(QWORD qwNewFunc, UINT uIndex);

private:
	QWORD GetVirtualTableCount(QWORD* pqwVirtualTable) const;

	QWORD** m_ClassBase;
	QWORD* m_NewVirtualTable;
	QWORD* m_OldVirtualTable;
	QWORD m_VirtualTableSize;
};