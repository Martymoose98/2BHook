#pragma once
#include <Windows.h>
#include <intrin.h>
#include <Ntsecapi.h>

typedef struct _RTL_DRIVE_LETTER_CURDIR
{
	USHORT                  Flags;
	USHORT                  Length;
	ULONG                   TimeStamp;
	UNICODE_STRING          DosPath;
} RTL_DRIVE_LETTER_CURDIR, *PRTL_DRIVE_LETTER_CURDIR;

typedef struct _LDR_MODULE
{
	LIST_ENTRY              InLoadOrderModuleList;
	LIST_ENTRY              InMemoryOrderModuleList;
	LIST_ENTRY              InInitializationOrderModuleList;
	PVOID                   BaseAddress;
	PVOID                   EntryPoint;
	ULONG                   SizeOfImage;
	UNICODE_STRING          FullDllName;
	UNICODE_STRING          BaseDllName;
	ULONG                   Flags;
	SHORT                   LoadCount;
	SHORT                   TlsIndex;
	LIST_ENTRY              HashTableEntry;
	ULONG                   TimeDateStamp;
} LDR_MODULE, *PLDR_MODULE;

typedef struct _RTL_USER_PROCESS_PARAMETERS
{
	ULONG                   MaximumLength;
	ULONG                   Length;
	ULONG                   Flags;
	ULONG                   DebugFlags;
	PVOID                   ConsoleHandle;
	ULONG                   ConsoleFlags;
	HANDLE                  StdInputHandle;
	HANDLE                  StdOutputHandle;
	HANDLE                  StdErrorHandle;
	UNICODE_STRING          CurrentDirectoryPath;
	HANDLE                  CurrentDirectoryHandle;
	UNICODE_STRING          DllPath;
	UNICODE_STRING          ImagePathName;
	UNICODE_STRING          CommandLine;
	PVOID                   Environment;
	ULONG                   StartingPositionLeft;
	ULONG                   StartingPositionTop;
	ULONG                   Width;
	ULONG                   Height;
	ULONG                   CharWidth;
	ULONG                   CharHeight;
	ULONG                   ConsoleTextAttributes;
	ULONG                   WindowFlags;
	ULONG                   ShowWindowFlags;
	UNICODE_STRING          WindowTitle;
	UNICODE_STRING          DesktopName;
	UNICODE_STRING          ShellInfo;
	UNICODE_STRING          RuntimeData;
	RTL_DRIVE_LETTER_CURDIR DLCurrentDirectory[32];
} RTL_USER_PROCESS_PARAMETERS, *PRTL_USER_PROCESS_PARAMETERS;

typedef struct _PEB_LDR_DATA
{
	ULONG                   Length;
	BOOLEAN                 Initialized;
	PVOID                   SsHandle;
	LIST_ENTRY              InLoadOrderModuleList;
	LIST_ENTRY              InMemoryOrderModuleList;
	LIST_ENTRY              InInitializationOrderModuleList;
} PEB_LDR_DATA, *PPEB_LDR_DATA;

typedef struct _PEB_FREE_BLOCK
{
	_PEB_FREE_BLOCK        *Next;
	ULONG                   Size;
} PEB_FREE_BLOCK, *PPEB_FREE_BLOCK;

typedef void(*PPEBLOCKROUTINE)(PVOID PebLock);

typedef struct _PEB
{
	BOOLEAN                 InheritedAddressSpace;
	BOOLEAN                 ReadImageFileExecOptions;
	BOOLEAN                 BeingDebugged;
	BOOLEAN                 Spare;
	HANDLE                  Mutant;
	PVOID                   ImageBaseAddress;
	PPEB_LDR_DATA           LoaderData;
	PRTL_USER_PROCESS_PARAMETERS ProcessParameters;
	PVOID                   SubSystemData;
	PVOID                   ProcessHeap;
	PVOID                   FastPebLock;
	PPEBLOCKROUTINE         FastPebLockRoutine;
	PPEBLOCKROUTINE         FastPebUnlockRoutine;
	ULONG                   EnvironmentUpdateCount;
	PVOID*                  KernelCallbackTable;
	PVOID                   EventLogSection;
	PVOID                   EventLog;
	PPEB_FREE_BLOCK         FreeList;
	ULONG                   TlsExpansionCounter;
	PVOID                   TlsBitmap;
	ULONG                   TlsBitmapBits[2];
	PVOID                   ReadOnlySharedMemoryBase;
	PVOID                   ReadOnlySharedMemoryHeap;
	PVOID*                  ReadOnlyStaticServerData;
	PVOID                   AnsiCodePageData;
	PVOID                   OemCodePageData;
	PVOID                   UnicodeCaseTableData;
	ULONG                   NumberOfProcessors;
	ULONG                   NtGlobalFlag;
	BYTE                    Spare2[0x4];
	LARGE_INTEGER           CriticalSectionTimeout;
	ULONG                   HeapSegmentReserve;
	ULONG                   HeapSegmentCommit;
	ULONG                   HeapDeCommitTotalFreeThreshold;
	ULONG                   HeapDeCommitFreeBlockThreshold;
	ULONG                   NumberOfHeaps;
	ULONG                   MaximumNumberOfHeaps;
	PVOID*                  *ProcessHeaps;
	PVOID                   GdiSharedHandleTable;
	PVOID                   ProcessStarterHelper;
	PVOID                   GdiDCAttributeList;
	PVOID                   LoaderLock;
	ULONG                   OSMajorVersion;
	ULONG                   OSMinorVersion;
	ULONG                   OSBuildNumber;
	ULONG                   OSPlatformId;
	ULONG                   ImageSubSystem;
	ULONG                   ImageSubSystemMajorVersion;
	ULONG                   ImageSubSystemMinorVersion;
	ULONG                   GdiHandleBuffer[34];
	ULONG                   PostProcessInitRoutine;
	ULONG                   TlsExpansionBitmap;
	BYTE                    TlsExpansionBitmapBits[128];
	ULONG                   SessionId;
} PEB, *PPEB;

#define GetPEB32 ((PPEB)__readfsdword(0x30))
#define GetPEB64 ((PPEB)__readgsqword(0x60))

#if _WIN64
#define GetProcessEnvironmentBlock() GetPEB64
#define ProcessEnvironmentBlock GetPEB64
#else
#define GetProcessEnvironmentBlock() GetPEB32
#define ProcessEnvironmentBlock GetPEB32
#endif

//still debating wheter to make this a module based hook instead of function based hook
class ImportTableHook
{
public:

	ImportTableHook(const char* szModule, const char* szFunction, LPCVOID lpFunction)
		: m_pNewFunction(lpFunction)
	{
		Hook(szModule, szFunction);
	}

	~ImportTableHook()
	{
		Unhook();
		VirtualProtect(this->m_pIAT, sizeof(IMAGE_THUNK_DATA), this->m_dwOldProtect, NULL);
	}

	const void* Hook(const char* szModule, const char* szFunction)
	{
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

			if (!strcmp(szModule, szModuleName))
			{
				for (DWORD i = 0; pImportNameTable[i].u1.Function; ++i)
				{
					if (IMAGE_SNAP_BY_ORDINAL(pImportNameTable[i].u1.Ordinal))
					{
						if (hModule && bResolveOrdinal)
						{
							for (DWORD hint = 0; hint < pExportDirectory->NumberOfNames; ++hint)
							{
								LPCSTR szFunctionName = (LPCSTR)((ULONG_PTR)hModule + pAddressOfNames[hint]);

								if (!strcmp(szFunctionName, szFunction))
								{
									WORD ordinal = (WORD)(((PWORD)((ULONG_PTR)hModule + pExportDirectory->AddressOfNameOrdinals))[hint] + pExportDirectory->Base);

									if (ordinal == IMAGE_ORDINAL(pImportNameTable[i].u1.Ordinal))
									{
										this->m_pIAT = &pImportAddressTable[i];
										VirtualProtect(this->m_pIAT, sizeof(IMAGE_THUNK_DATA), PAGE_READWRITE, &this->m_dwOldProtect);
										this->m_pOriginalFunction = (LPCVOID)this->m_pIAT->u1.Function;
										this->m_pIAT->u1.Function = (ULONG_PTR)m_pNewFunction;
										return this->m_pOriginalFunction;
									}
								}
							}
						}
						else
						{
							if (IMAGE_ORDINAL((ULONG_PTR)szFunction) == IMAGE_ORDINAL(pImportNameTable[i].u1.Ordinal))
							{
								this->m_pIAT = &pImportAddressTable[i];
								VirtualProtect(this->m_pIAT, sizeof(IMAGE_THUNK_DATA), PAGE_READWRITE, &this->m_dwOldProtect);
								this->m_pOriginalFunction = (LPCVOID)this->m_pIAT->u1.Function;
								this->m_pIAT->u1.Function = (ULONG_PTR)m_pNewFunction;
								return this->m_pOriginalFunction;
							}
						}
					}
					else
					{
						LPCSTR szFunctionName = (LPCSTR)((PIMAGE_IMPORT_BY_NAME)((ULONG_PTR)pDosHeader + pImportNameTable[i].u1.AddressOfData))->Name;
						
						if (!strcmp(szFunctionName, szFunction))
						{
							this->m_pIAT = &pImportAddressTable[i];
							VirtualProtect(this->m_pIAT, sizeof(IMAGE_THUNK_DATA), PAGE_READWRITE, &this->m_dwOldProtect);
							this->m_pOriginalFunction = (LPCVOID)this->m_pIAT->u1.Function;
							this->m_pIAT->u1.Function = (ULONG_PTR)m_pNewFunction;
							return this->m_pOriginalFunction;
						}
					}
				}
			}
		}
		return NULL;
	}

	void Rehook()
	{
		m_pIAT->u1.Function = (ULONG_PTR)m_pNewFunction;
	}

	void Unhook()
	{
		m_pIAT->u1.Function = (ULONG_PTR)m_pOriginalFunction;
	}

	inline LPCVOID GetOriginalFunction() const
	{
		return this->m_pOriginalFunction;
	}

private:

	/*void Initalize(const char* szModule)
	{
		GetModuleHandleA(szModule);
		pModuleDosHeader = (PIMAGE_DOS_HEADER)hModule;
		pExportDataDirectory = &((PIMAGE_NT_HEADERS)((ULONG_PTR)pModuleDosHeader + pModuleDosHeader->e_lfanew))->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
		pExportDirectory = (PIMAGE_EXPORT_DIRECTORY)((ULONG_PTR)hModule + pExportDataDirectory->VirtualAddress);
		pAddressOfNames = (PDWORD)((ULONG_PTR)hModule + pExportDirectory->AddressOfNames);
	}*/

	DWORD FindNptProc(PDWORD npt, DWORD size, PBYTE base, LPCSTR szProc)
	{
		INT   cmp;
		DWORD max;
		DWORD mid;
		DWORD min;

		min = 0;
		max = size - 1;

		while (min <= max)
		{
			mid = (min + max) >> 1;
			cmp = strcmp((LPCSTR)(npt[mid] + base), szProc);

			if (cmp < 0)
				min = mid + 1;
			else if (cmp > 0)
				max = mid - 1;
			else
				return mid;
		}
		return -1;
	}

#ifdef FAST_HOOKING
	HMODULE m_hModule;
	IMAGE_EXPORT_DIRECTORY m_pExportDirectory;
	PDWORD m_pAddressOfNames;
#endif
	PIMAGE_THUNK_DATA m_pIAT;
	LPCVOID m_pOriginalFunction;
	LPCVOID m_pNewFunction;
	DWORD m_dwOldProtect;
};