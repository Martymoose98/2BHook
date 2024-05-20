#ifndef __IMPORTTABLEHOOK_H__
#define __IMPORTTABLEHOOK_H__

#include <Windows.h>
#include "WindowsOpaqueStructures.h"

//TODO: "Revise. Implement binary search for greater performance & create C implementation"
class ImportTableHook
{
public:

	ImportTableHook(const char* szModule, const char* szFunction, LPCVOID lpFunction)
		: m_pNewFunction(lpFunction)
	{
		Hook(szModule, szFunction);
	}

	~ImportTableHook(void)
	{
		Unhook();
		VirtualProtect(this->m_pIAT, sizeof(IMAGE_THUNK_DATA), this->m_dwOldProtect, &this->m_dwOldProtect);
	}

	const void* Hook(const char* szModule, const char* szFunction)
	{
		if (!szModule || !szFunction)
			return NULL;

		PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)ProcessEnvironmentBlock->ImageBaseAddress;
		PIMAGE_DATA_DIRECTORY pImportDirectory = &((PIMAGE_NT_HEADERS)((ULONG_PTR)pDosHeader + pDosHeader->e_lfanew))->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
		PIMAGE_IMPORT_DESCRIPTOR pImportDescriptors = (PIMAGE_IMPORT_DESCRIPTOR)((ULONG_PTR)pDosHeader + pImportDirectory->VirtualAddress);
		DWORD dwImportDescriptorCount = pImportDirectory->Size / sizeof(IMAGE_IMPORT_DESCRIPTOR);
		BOOL bResolveOrdinal = (ULONG_PTR)szFunction > MAXWORD;
		HMODULE hModule = NULL;
		PIMAGE_DOS_HEADER pModuleDosHeader = NULL;
		PIMAGE_DATA_DIRECTORY pExportDataDirectory = NULL;
		PIMAGE_EXPORT_DIRECTORY pExportDirectory = NULL;
		PDWORD pAddressOfNames = NULL;

		if (bResolveOrdinal)
		{
			hModule = GetModuleHandleA(szModule);

			if (!hModule)
				return NULL;

			pModuleDosHeader = (PIMAGE_DOS_HEADER)hModule;
			pExportDataDirectory = &((PIMAGE_NT_HEADERS)((ULONG_PTR)pModuleDosHeader + pModuleDosHeader->e_lfanew))->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
			pExportDirectory = (PIMAGE_EXPORT_DIRECTORY)((ULONG_PTR)hModule + pExportDataDirectory->VirtualAddress);
			pAddressOfNames = (PDWORD)((ULONG_PTR)hModule + pExportDirectory->AddressOfNames);
		}

		for (DWORD index = 0; index < dwImportDescriptorCount; ++index)
		{
			if (!pImportDescriptors[index].Characteristics)
				return NULL;

			PIMAGE_THUNK_DATA pImportNameTable = (PIMAGE_THUNK_DATA)((ULONG_PTR)pDosHeader + pImportDescriptors[index].OriginalFirstThunk);
			PIMAGE_THUNK_DATA pImportAddressTable = (PIMAGE_THUNK_DATA)((ULONG_PTR)pDosHeader + pImportDescriptors[index].FirstThunk);
			LPCSTR szModuleName = (LPCSTR)((ULONG_PTR)pDosHeader + pImportDescriptors[index].Name);

			if (!_stricmp(szModule, szModuleName))
			{
				for (DWORD i = 0; pImportNameTable[i].u1.AddressOfData; ++i)
				{
					// Check if the function is imported by ordinal or name
					if (IMAGE_SNAP_BY_ORDINAL(pImportNameTable[i].u1.Ordinal))
					{
						if (bResolveOrdinal)
						{
							// Find the function name in the name pointer table and return the hint
							DWORD dwHint = FindNptProc(hModule, pAddressOfNames, pExportDirectory->NumberOfNames, szFunction);

							// If the function name was found compare use the hint to find the function ordinal and compare
							if (dwHint != 0xFFFFFFFF)
							{
								WORD wOrdinal = (WORD)(((PWORD)((ULONG_PTR)hModule + pExportDirectory->AddressOfNameOrdinals))[dwHint] + pExportDirectory->Base);

								if (wOrdinal == IMAGE_ORDINAL(pImportNameTable[i].u1.Ordinal))
									return InitHook(&pImportAddressTable[i]);
							}
						}
						else
						{
							// Assume szFunction is actually an ordinal (WORD), and compare
							if (IMAGE_ORDINAL((ULONG_PTR)szFunction) == IMAGE_ORDINAL(pImportNameTable[i].u1.Ordinal))
								return InitHook(&pImportAddressTable[i]);
						}
					}
					else
					{
						LPCSTR szFunctionName = (LPCSTR)((PIMAGE_IMPORT_BY_NAME)((ULONG_PTR)pDosHeader + pImportNameTable[i].u1.AddressOfData))->Name;

						// Compare function names
						if (szFunctionName && !strcmp(szFunctionName, szFunction))
							return InitHook(&pImportAddressTable[i]);
					}
				}
			}
		}
		return NULL;
	}

	void Rehook(void)
	{
		VirtualProtect(this->m_pIAT, sizeof(IMAGE_THUNK_DATA), PAGE_READWRITE, &this->m_dwOldProtect);
		m_pIAT->u1.Function = (ULONG_PTR)m_pNewFunction;
		VirtualProtect(this->m_pIAT, sizeof(IMAGE_THUNK_DATA), this->m_dwOldProtect, &this->m_dwOldProtect);
	}

	void Unhook(void)
	{
		VirtualProtect(this->m_pIAT, sizeof(IMAGE_THUNK_DATA), PAGE_READWRITE, &this->m_dwOldProtect);
		m_pIAT->u1.Function = (ULONG_PTR)m_pOriginalFunction;
		VirtualProtect(this->m_pIAT, sizeof(IMAGE_THUNK_DATA), this->m_dwOldProtect, &this->m_dwOldProtect);
	}

	inline LPCVOID GetOriginalFunction(void) const
	{
		return this->m_pOriginalFunction;
	}

private:

	inline LPCVOID InitHook(PIMAGE_THUNK_DATA pImportAddressTable)
	{
		this->m_pIAT = pImportAddressTable;
		VirtualProtect(this->m_pIAT, sizeof(IMAGE_THUNK_DATA), PAGE_READWRITE, &this->m_dwOldProtect);
		this->m_pOriginalFunction = (LPCVOID)this->m_pIAT->u1.Function;
		this->m_pIAT->u1.Function = (ULONG_PTR)m_pNewFunction;
		return this->m_pOriginalFunction;
	}

	DWORD FindNptProc(HMODULE hModule, PDWORD pdwAddressOfNames, DWORD dwSize, LPCSTR szProc)
	{
		INT   iCompare;
		DWORD dwMax;
		DWORD dwMid;
		DWORD dwMin;

		dwMin = 0;
		dwMax = dwSize - 1;

		while (dwMin <= dwMax)
		{
			dwMid = (dwMin + dwMax) >> 1;
			iCompare = strcmp((LPCSTR)((ULONG_PTR)hModule + pdwAddressOfNames[dwMid]), szProc);

			if (iCompare < 0)
				dwMin = dwMid + 1;
			else if (iCompare > 0)
				dwMax = dwMid - 1;
			else
				return dwMid;
		}
		return -1; //	0xFFFFFFFF
	}

#ifdef FAST_HOOKING
	HMODULE m_hModule;
	PIMAGE_EXPORT_DIRECTORY m_pExportDirectory;
	PDWORD m_pAddressOfNames;
#endif
	PIMAGE_THUNK_DATA m_pIAT;
	LPCVOID m_pOriginalFunction;
	LPCVOID m_pNewFunction;
	DWORD m_dwOldProtect;
};

#include <unordered_map>

union ImportOrdinal
{
	ImportOrdinal(WORD wOrdinal) noexcept
	{
		this->wOrdinal = wOrdinal;
	}

	ImportOrdinal(const char* szName) noexcept
	{
		this->szName = szName;
	}

	bool operator==(const ImportOrdinal& Other) const
	{
		return (this->wOrdinal == Other.wOrdinal || !strcmp(this->szName, Other.szName));
	}

	WORD wOrdinal;
	const char* szName;
};

// C++ Standard Library Overrides & Specializations
namespace std
{
	template<>
	struct hash<ImportOrdinal>
	{
		size_t operator()(const ImportOrdinal& Ordinal) const noexcept
		{
			size_t hash = 0;

			for (const char* s = Ordinal.szName; *s; ++s)
			{
				hash ^= std::hash<char>()(*s);
			}

			return hash;
		}
	};
}

struct ImportFunction
{
	ImportFunction(PIMAGE_THUNK_DATA pIAT)
		: m_pIAT(pIAT), m_pNewFunction(nullptr)
	{
		VirtualProtect(m_pIAT, sizeof(IMAGE_THUNK_DATA), PAGE_READWRITE, &m_dwOldProtect);
		m_pOriginalFunction = (LPCVOID)m_pIAT->u1.Function;
	}

	~ImportFunction(void)
	{
		Unhook();
		VirtualProtect(m_pIAT, sizeof(IMAGE_THUNK_DATA), m_dwOldProtect, &m_dwOldProtect);
	}

	const void* Hook(LPCVOID lpFunction)
	{
		m_pNewFunction = lpFunction;
		m_pIAT->u1.Function = (ULONG_PTR)m_pNewFunction;
		return m_pOriginalFunction;
	}

	void Unhook(void)
	{
		m_pIAT->u1.Function = (ULONG_PTR)m_pOriginalFunction;
	}

	inline LPCVOID GetOriginalFunction(void) const
	{
		return m_pOriginalFunction;
	}

	PIMAGE_THUNK_DATA m_pIAT;
	LPCVOID m_pOriginalFunction;
	LPCVOID m_pNewFunction;
	DWORD m_dwOldProtect;
};

class ImportDLLHook
{
public:

	ImportDLLHook(const char* szModuleName)
		: m_szModule(szModuleName)
	{
		m_hModule = GetModuleHandleA(szModuleName);
		Initialize(szModuleName);
	}

	~ImportDLLHook(void)
	{
	}

	ImportDLLHook(const ImportDLLHook&) = delete;

	ImportDLLHook& operator=(const ImportDLLHook&) = delete;

	const void* Hook(const char* szFunction, LPCVOID lpFunction)
	{
		auto it = m_ExportedFunctions.find(szFunction);

		if (it == m_ExportedFunctions.cend())
			return NULL;

		return it->second.Hook(lpFunction);
	}

	void Unhook(const char* szFunction)
	{
		auto it = m_ExportedFunctions.find(szFunction);

		if (it == m_ExportedFunctions.cend())
			return;

		it->second.Unhook();
	}

private:

	void Initialize(const char* szModule)
	{
		PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)ProcessEnvironmentBlock->ImageBaseAddress;
		PIMAGE_DATA_DIRECTORY pImportDirectory = &((PIMAGE_NT_HEADERS)((ULONG_PTR)pDosHeader + pDosHeader->e_lfanew))->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
		PIMAGE_IMPORT_DESCRIPTOR pImportDescriptors = (PIMAGE_IMPORT_DESCRIPTOR)((ULONG_PTR)pDosHeader + pImportDirectory->VirtualAddress);
		DWORD dwImportDescriptorCount = pImportDirectory->Size / sizeof(IMAGE_IMPORT_DESCRIPTOR);

		for (DWORD index = 0; index < dwImportDescriptorCount; ++index)
		{
			if (!pImportDescriptors[index].Characteristics)
				return;

			PIMAGE_THUNK_DATA pImportNameTable = (PIMAGE_THUNK_DATA)((ULONG_PTR)pDosHeader + pImportDescriptors[index].OriginalFirstThunk);
			PIMAGE_THUNK_DATA pImportAddressTable = (PIMAGE_THUNK_DATA)((ULONG_PTR)pDosHeader + pImportDescriptors[index].FirstThunk);
			LPCSTR szModuleName = (LPCSTR)((ULONG_PTR)pDosHeader + pImportDescriptors[index].Name);

			if (!_stricmp(szModule, szModuleName))
			{
				for (DWORD i = 0; pImportNameTable[i].u1.AddressOfData; ++i)
				{
					// Check if the function is imported by ordinal or name
					if (IMAGE_SNAP_BY_ORDINAL(pImportNameTable[i].u1.Ordinal))
					{
						m_ExportedFunctions.emplace(IMAGE_ORDINAL(pImportNameTable[i].u1.Ordinal), &pImportAddressTable[i]);
					}
					else
					{
						LPCSTR szFunctionName = (LPCSTR)((PIMAGE_IMPORT_BY_NAME)((ULONG_PTR)pDosHeader + pImportNameTable[i].u1.AddressOfData))->Name;

						m_ExportedFunctions.emplace(szFunctionName, &pImportAddressTable[i]);
					}
				}
			}
		}
	}


private:
	const char* m_szModule;
	HMODULE m_hModule;
	std::unordered_map<ImportOrdinal, ImportFunction> m_ExportedFunctions;
};

#endif // !__IMPORTTABLEHOOK_H__