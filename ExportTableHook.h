#ifndef __EXPORTTABLEHOOK_H__
#define __EXPORTTABLEHOOK_H__
#include <Windows.h>

class ExportTableHook
{
public:
	ExportTableHook(const char* szModule, const char* szFunction, LPCVOID pHookFunction)
		: m_pNewFunction(pHookFunction)
	{
		Hook(szModule, szFunction);
	}

	~ExportTableHook() { Unhook(); }

	const void* Hook(const char* szModule, const char* szFunction)
	{
		PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)GetModuleHandleA(szModule);
		PIMAGE_DATA_DIRECTORY pExportDataDirectory = &((PIMAGE_NT_HEADERS)((ULONG_PTR)pDosHeader + pDosHeader->e_lfanew))->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
		PIMAGE_EXPORT_DIRECTORY pExportDirectory = (PIMAGE_EXPORT_DIRECTORY)((ULONG_PTR)pDosHeader + pExportDataDirectory->VirtualAddress);
		DWORD dwExportDescriptorCount = pExportDataDirectory->Size / sizeof(IMAGE_EXPORT_DIRECTORY);
		PDWORD pAddressOfFunctions = (PDWORD)((ULONG_PTR)pDosHeader + pExportDirectory->AddressOfFunctions);
		PWORD pAddressOfOrdinals = (PWORD)((ULONG_PTR)pDosHeader + pExportDirectory->AddressOfNameOrdinals);
		PDWORD pAddressOfNames;

		for (DWORD index = 0; index < dwExportDescriptorCount; ++index)
		{
			if ((ULONG_PTR)szFunction > MAXWORD)
			{
				pAddressOfNames = (PDWORD)((ULONG_PTR)pDosHeader + pExportDirectory->AddressOfNames);

				for (DWORD hint = 0; hint < pExportDirectory->NumberOfNames; ++hint)
				{
					LPSTR szFunctionName = (LPSTR)((ULONG_PTR)pDosHeader + pAddressOfNames[hint]);

					if (!strcmp(szFunctionName, szFunction))
					{
						m_pdwFuncRVA = &pAddressOfFunctions[pAddressOfOrdinals[hint]];
						m_dwNewFunctionRVA = (ULONG_PTR)m_pNewFunction - (ULONG_PTR)pDosHeader;
						m_dwOrginalFunctionRVA = pAddressOfFunctions[pAddressOfOrdinals[hint]];
						m_pOriginalFunction = (LPCVOID)((ULONG_PTR)pDosHeader + m_dwOrginalFunctionRVA);
						VirtualProtect(m_pdwFuncRVA, sizeof(DWORD), PAGE_READWRITE, &m_dwOldProtect);
						pAddressOfFunctions[pAddressOfOrdinals[hint]] = m_dwNewFunctionRVA;
						VirtualProtect(m_pdwFuncRVA, sizeof(DWORD), m_dwOldProtect, NULL);
						return m_pOriginalFunction;
					}
				}
			}
			else 
			{
				for (DWORD hint = 0; hint < pExportDirectory->NumberOfNames; ++hint)
				{
					WORD ordinal = (WORD)(pAddressOfOrdinals[hint] + pExportDirectory->Base);
				
					if (IMAGE_ORDINAL((ULONG_PTR)szFunction) == ordinal)
					{
						m_pdwFuncRVA = &pAddressOfFunctions[pAddressOfOrdinals[hint]];
						m_dwNewFunctionRVA = (ULONG_PTR)m_pNewFunction - (ULONG_PTR)pDosHeader;
						m_dwOrginalFunctionRVA = pAddressOfFunctions[pAddressOfOrdinals[hint]];
						m_pOriginalFunction = (LPCVOID)((ULONG_PTR)pDosHeader + m_dwOrginalFunctionRVA);
						VirtualProtect(m_pdwFuncRVA, sizeof(DWORD), PAGE_READWRITE, &m_dwOldProtect);
						pAddressOfFunctions[pAddressOfOrdinals[hint]] = m_dwNewFunctionRVA;
						VirtualProtect(m_pdwFuncRVA, sizeof(DWORD), m_dwOldProtect, NULL);
						return m_pOriginalFunction;
					}
				}
			}
		}
		return NULL;
	}

	void Rehook()
	{
		VirtualProtect(m_pdwFuncRVA, sizeof(DWORD), PAGE_READWRITE, &m_dwOldProtect);
		*m_pdwFuncRVA = m_dwNewFunctionRVA;
		VirtualProtect(m_pdwFuncRVA, sizeof(DWORD), m_dwOldProtect, NULL);
	}

	void Unhook()
	{
		VirtualProtect(m_pdwFuncRVA, sizeof(DWORD), PAGE_READWRITE, &m_dwOldProtect);
		*m_pdwFuncRVA = m_dwOrginalFunctionRVA;
		VirtualProtect(m_pdwFuncRVA, sizeof(DWORD), m_dwOldProtect, NULL);
	}

	inline LPCVOID GetOriginalFunction() const
	{
		return this->m_pOriginalFunction;
	}

private:
	PDWORD m_pdwFuncRVA;
	LPCVOID m_pOriginalFunction;
	DWORD m_dwOrginalFunctionRVA;
	LPCVOID m_pNewFunction;
	DWORD m_dwNewFunctionRVA;
	DWORD m_dwOldProtect;
};
#endif // !__EXPORTTABLEHOOK_H__