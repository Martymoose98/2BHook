#pragma once
#include <Windows.h>

class ExportTableHook
{
	ExportTableHook(const char* szModule, const char* szFunction, LPCVOID pHookFunction)
		: m_pNewFunction(pHookFunction)
	{
		Hook(szModule, szFunction);
	}

	const void* Hook(const char* szModule, const char* szFunction)
	{
		PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)GetModuleHandleA(szModule);
		PIMAGE_DATA_DIRECTORY pExportDataDirectory = &((PIMAGE_NT_HEADERS)((ULONG_PTR)pDosHeader + pDosHeader->e_lfanew))->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
		PIMAGE_EXPORT_DIRECTORY pExportDirectory = (PIMAGE_EXPORT_DIRECTORY)((ULONG_PTR)pDosHeader + pExportDataDirectory->VirtualAddress);
		PDWORD pAddressOfNames = (PDWORD)((ULONG_PTR)pDosHeader + pExportDirectory->AddressOfNames);
		PDWORD pAddressOfOrdinals = (PDWORD)((ULONG_PTR)pDosHeader + pExportDirectory->AddressOfNameOrdinals);
		DWORD dwExportDescriptorCount = pExportDataDirectory->Size / sizeof(IMAGE_EXPORT_DIRECTORY);

		for (DWORD index = 0; index < dwExportDescriptorCount; ++index)
		{
			if (!pExportDirectory[index].Characteristics)
				return NULL;

			if ()
		}
	}

	void Rehook()
	{
		m_pIAT->u1.Function = (ULONG_PTR)m_pNewFunction;
	}

	inline LPCVOID GetOriginalFunction() const
	{
		return this->m_pOriginalFunction;
	}

private:
	PIMAGE_THUNK_DATA m_pIAT;
	LPCVOID m_pOriginalFunction;
	LPCVOID m_pNewFunction;
	DWORD m_dwOldProtect;
};