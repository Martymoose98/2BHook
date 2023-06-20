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

		if (!pDosHeader)
			return NULL;

		PIMAGE_DATA_DIRECTORY pExportDataDirectory = &((PIMAGE_NT_HEADERS)((ULONG_PTR)pDosHeader + pDosHeader->e_lfanew))->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
		PIMAGE_EXPORT_DIRECTORY pExportDirectory = (PIMAGE_EXPORT_DIRECTORY)((ULONG_PTR)pDosHeader + pExportDataDirectory->VirtualAddress);
		DWORD dwExportDescriptorCount = pExportDataDirectory->Size / sizeof(IMAGE_EXPORT_DIRECTORY);
		PDWORD pAddressOfFunctions = (PDWORD)((ULONG_PTR)pDosHeader + pExportDirectory->AddressOfFunctions);
		PWORD pAddressOfOrdinals = (PWORD)((ULONG_PTR)pDosHeader + pExportDirectory->AddressOfNameOrdinals);
		PDWORD pAddressOfNames;

		for (DWORD dwIndex = 0; dwIndex < dwExportDescriptorCount; ++dwIndex)
		{
			if ((ULONG_PTR)szFunction > MAXWORD)
			{
				pAddressOfNames = (PDWORD)((ULONG_PTR)pDosHeader + pExportDirectory->AddressOfNames);

				for (DWORD dwHint = 0; dwHint < pExportDirectory->NumberOfNames; ++dwHint)
				{
					LPSTR szFunctionName = (LPSTR)((ULONG_PTR)pDosHeader + pAddressOfNames[dwHint]);

					if (!strcmp(szFunctionName, szFunction))
						return InitHook(pDosHeader, pAddressOfFunctions, pAddressOfOrdinals, dwHint);
				}
			}
			else
			{
				for (DWORD dwHint = 0; dwHint < pExportDirectory->NumberOfNames; ++dwHint)
				{
					WORD ordinal = (WORD)(pAddressOfOrdinals[dwHint] + pExportDirectory->Base);

					if (IMAGE_ORDINAL((ULONG_PTR)szFunction) == ordinal)
						return InitHook(pDosHeader, pAddressOfFunctions, pAddressOfOrdinals, dwHint);
				}
			}
		}
		return NULL;
	}

	void Rehook()
	{
		VirtualProtect(m_pdwFuncRVA, sizeof(DWORD), PAGE_READWRITE, &m_dwOldProtect);
		*m_pdwFuncRVA = m_dwNewFunctionRVA;
		VirtualProtect(m_pdwFuncRVA, sizeof(DWORD), m_dwOldProtect, &m_dwOldProtect);
	}

	void Unhook()
	{
		VirtualProtect(m_pdwFuncRVA, sizeof(DWORD), PAGE_READWRITE, &m_dwOldProtect);
		*m_pdwFuncRVA = m_dwOrginalFunctionRVA;
		VirtualProtect(m_pdwFuncRVA, sizeof(DWORD), m_dwOldProtect, &m_dwOldProtect);
	}

	inline LPCVOID GetOriginalFunction() const
	{
		return this->m_pOriginalFunction;
	}

private:

	inline LPCVOID InitHook(PIMAGE_DOS_HEADER pDosHeader, PDWORD pAddressOfFunctions, PWORD pAddressOfOrdinals, DWORD dwHint)
	{
		m_pdwFuncRVA = &pAddressOfFunctions[pAddressOfOrdinals[dwHint]];
		m_dwNewFunctionRVA = (ULONG_PTR)m_pNewFunction - (ULONG_PTR)pDosHeader;
		m_dwOrginalFunctionRVA = pAddressOfFunctions[pAddressOfOrdinals[dwHint]];
		m_pOriginalFunction = (LPCVOID)((ULONG_PTR)pDosHeader + m_dwOrginalFunctionRVA);
		VirtualProtect(m_pdwFuncRVA, sizeof(DWORD), PAGE_READWRITE, &m_dwOldProtect);
		pAddressOfFunctions[pAddressOfOrdinals[dwHint]] = m_dwNewFunctionRVA;
		VirtualProtect(m_pdwFuncRVA, sizeof(DWORD), m_dwOldProtect, &m_dwOldProtect);
		return m_pOriginalFunction;
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