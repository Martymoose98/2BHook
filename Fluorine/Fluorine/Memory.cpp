#include "pch.h"
#include "Memory.h"


// WARNING: I think this is supposed to return LONG instead of DWORD aka. ULONG
// rel32 jmp instructions are signed rva's.
// It's not a big deal currently 'cause the func is unused and therefore untested
// but, if used unchanged, I presume will cause issues/bugs/crashes.
/// <summary>
/// Calculates a relative 32 jump delta
/// </summary>
/// <param name="src">Instruction pointer (eip/rip)</param>
/// <param name="dest">Destination pointer</param>
/// <param name="uOffset">Offset in bytes to the rva</param>
/// <returns>A relative 32 jump delta</returns>
static inline LONG CalculateJump(ULONG_PTR src, ULONG_PTR dest, SIZE_T uOffset)
{
	return (LONG)(dest - src - (uOffset + sizeof(LONG)));
}

/// <summary>
/// Reads a relative 32 jump instruction and returns the destination
/// </summary>
/// <param name="src">Instruction pointer (eip/rip)</param>
/// <param name="uOffset">Offset in bytes to the rva</param>
/// <returns>Destination pointer</returns>
static inline ULONG_PTR ReadPtr(ULONG_PTR src, SIZE_T uOffset)
{
	return src + uOffset + sizeof(LONG) + *(LONG*)(src + uOffset);
}


ULONG_PTR FindPattern(const char* szModuleName, const char* szPattern)
{
	return FindPatternEx(GetModuleHandleA(szModuleName), szPattern);
}

ULONG_PTR FindPatternEx(HMODULE hModule, const char* szPattern)
{
	const char* szPat = szPattern;
	ULONG_PTR uFirstMatch = 0;
	ULONG_PTR uStart = (ULONG_PTR)hModule;
	MODULEINFO ModuleInfo;

	if (!GetModuleInformation(GetCurrentProcess(), (HMODULE)uStart, &ModuleInfo, sizeof(MODULEINFO)))
		return -1;

	ULONG_PTR uRangeEnd = uStart + ModuleInfo.SizeOfImage;

	for (ULONG_PTR pCur = uStart; pCur < uRangeEnd; ++pCur)
	{
		if (!*szPat)
			return uFirstMatch;

		if (*(BYTE*)szPat == '\?' || *(BYTE*)pCur == GetByte(szPat))
		{
			if (!uFirstMatch)
				uFirstMatch = pCur;

			if (!szPat[2] || !szPat[1])
				return uFirstMatch;

			if (*(WORD*)szPat == '\?\?' || *(BYTE*)szPat != '\?')
				szPat += 3;
			else
				szPat += 2;
		}
		else
		{
			szPat = szPattern;
			uFirstMatch = 0;
		}
	}
	return 0;
}

/*
	NOTE: When using this version of this function, the pointer has to be the first unknown
	@params
	szModuleName - name of the module to scan (IDA or PEiD Style)
	szPattern - the byte pattern to search the module for
*/
static ULONG_PTR FindPatternPtr(const char* szModuleName, const char* szPattern)
{
	ULONG_PTR rip = FindPattern(szModuleName, szPattern);
	ULONG_PTR uOffset = 0;

	while (*(szPattern + uOffset * 3) != '?' && *(WORD*)(szPattern + uOffset * 3) != '??')
		++uOffset;

	SIZE_T OpcodeSize = uOffset + sizeof(LONG);
	return (ULONG_PTR)((*(LONG*)(rip + uOffset)) + OpcodeSize) + rip;
}

/*
	NOTE: When using this version of this function, the pointer has to be the first unknown.
	Also, this is the fastest verison of this function since the uOffset is compile-time known.
	@params
	szModuleName - name of the module to scan (IDA or PEiD Style)
	szPattern - the byte pattern to search the module for
	uOffset - the byte offset to the pointer from the beginning of szPattern
*/
ULONG_PTR FindPatternPtr(const char* szModuleName, const char* szPattern, UINT uOffset)
{
	ULONG_PTR rip = FindPattern(szModuleName, szPattern);
	SIZE_T OpcodeSize = uOffset + sizeof(LONG);
	return (ULONG_PTR)((*(LONG*)(rip + uOffset)) + OpcodeSize) + rip;
}

/*
	NOTE: When using this version of this function, the pointer has to be the first unknown.
	Also, this is the fastest verison of this function since the uOffset is compile-time known.
	@params
	hModule - pointer to the image base of the module to scan (IDA or PEiD Style)
	szPattern - the byte pattern to search the module for
	uOffset - the byte offset to the pointer from the beginning of szPattern
*/
ULONG_PTR FindPatternPtrEx(HMODULE hModule, const char* szPattern, UINT uOffset)
{
	ULONG_PTR rip = FindPatternEx(hModule, szPattern);
	SIZE_T OpcodeSize = uOffset + sizeof(LONG);
	return (ULONG_PTR)((*(LONG*)(rip + uOffset)) + OpcodeSize) + rip;
}

static VOID NopMemory(VOID* address, SIZE_T nBytes, BYTE** ppOldOpcodes)
{
	if (!address)
		return;

	if (!nBytes)
		return;

	if (!*ppOldOpcodes)
	{
		*ppOldOpcodes = (BYTE*)malloc(nBytes);

		if (!*ppOldOpcodes)
			return;
	}

	memcpy(*ppOldOpcodes, address, nBytes);
	NOPMemory(address, nBytes);
}

VOID NopMemory(PNOP_MEMORY pNop)
{
	if (!pNop)
		return;

	if (!pNop->Address)
		return;

	if (!pNop->nBytes)
		return;

	if (!pNop->pOldOpcodes)
	{
		pNop->pOldOpcodes = (BYTE*)malloc(pNop->nBytes);

		if (!pNop->pOldOpcodes)
			return;
	}

	if (pNop->Patched)
		return;

	DWORD dwOldProtect;

	EnterCriticalSection(&pNop->CriticalSection);

	VirtualProtect(pNop->Address, pNop->nBytes, PAGE_EXECUTE_READWRITE, &dwOldProtect);

	memcpy(pNop->pOldOpcodes, pNop->Address, pNop->nBytes);
	NOPMemory(pNop->Address, pNop->nBytes);

	FlushInstructionCache(GetCurrentProcess(), pNop->Address, pNop->nBytes);

	VirtualProtect(pNop->Address, pNop->nBytes, dwOldProtect, &dwOldProtect);

	pNop->Patched = TRUE;

	LeaveCriticalSection(&pNop->CriticalSection);
}

static VOID RestoreMemory(VOID* address, SIZE_T nBytes, BYTE* pOldOpcodes)
{
	if (!address)
		return;

	if (!nBytes)
		return;

	if (!pOldOpcodes)
		return;

	memcpy(address, pOldOpcodes, nBytes);
}

static VOID RestoreMemoryNop(CONST PNOP_MEMORY pNop)
{
	if (!pNop)
		return;

	if (!pNop->Address)
		return;

	if (!pNop->nBytes)
		return;

	if (!pNop->pOldOpcodes)
		return;

	if (!pNop->Patched)
		return;

	DWORD dwOldProtect;

	EnterCriticalSection(&pNop->CriticalSection);

	VirtualProtect(pNop->Address, pNop->nBytes, PAGE_EXECUTE_READWRITE, &dwOldProtect);

	memcpy(pNop->Address, pNop->pOldOpcodes, pNop->nBytes);

	FlushInstructionCache(GetCurrentProcess(), pNop->Address, pNop->nBytes);

	VirtualProtect(pNop->Address, pNop->nBytes, dwOldProtect, &dwOldProtect);

	pNop->Patched = FALSE;

	LeaveCriticalSection(&pNop->CriticalSection);
}

static VOID PatchBytes(VOID* address, SIZE_T nBytes, BYTE* pNewOpcodes, BYTE** ppOldOpcodes)
{
	if (!address)
		return;

	if (!nBytes)
		return;

	if (!pNewOpcodes)
		return;

	if (!*ppOldOpcodes)
	{
		*ppOldOpcodes = (BYTE*)malloc(nBytes);

		if (!*ppOldOpcodes)
			return;
	}

	memcpy(*ppOldOpcodes, address, nBytes);
	memcpy(address, pNewOpcodes, nBytes);
}

// FIXME: All of these functions should return a value
VOID PatchBytes(PBYTE_PATCH_MEMORY pBytePatch)
{
	if (!pBytePatch)
		return;

	if (!pBytePatch->Address)
		return;

	if (!pBytePatch->nBytes)
		return;

	if (!pBytePatch->pNewOpcodes)
		return;

	if (!pBytePatch->pOldOpcodes)
	{
		pBytePatch->pOldOpcodes = (BYTE*)malloc(pBytePatch->nBytes);

		if (!pBytePatch->pOldOpcodes)
			return;
	}

	if (pBytePatch->Patched)
		return;

	DWORD dwOldProtect;

	EnterCriticalSection(&pBytePatch->CriticalSection);

	VirtualProtect(pBytePatch->Address, pBytePatch->nBytes, PAGE_EXECUTE_READWRITE, &dwOldProtect);

	memcpy(pBytePatch->pOldOpcodes, pBytePatch->Address, pBytePatch->nBytes);
	memcpy(pBytePatch->Address, pBytePatch->pNewOpcodes, pBytePatch->nBytes);

	FlushInstructionCache(GetCurrentProcess(), pBytePatch->Address, pBytePatch->nBytes);

	VirtualProtect(pBytePatch->Address, pBytePatch->nBytes, dwOldProtect, &dwOldProtect);

	pBytePatch->Patched = TRUE;

	LeaveCriticalSection(&pBytePatch->CriticalSection);
}

static VOID RestoreMemoryBytePatch(CONST PBYTE_PATCH_MEMORY pBytePatch)
{
	if (!pBytePatch)
		return;

	if (!pBytePatch->Address)
		return;

	if (!pBytePatch->nBytes)
		return;

	if (!pBytePatch->pOldOpcodes)
		return;

	if (!pBytePatch->Patched)
		return;

	DWORD dwOldProtect;

	EnterCriticalSection(&pBytePatch->CriticalSection);

	VirtualProtect(pBytePatch->Address, pBytePatch->nBytes, PAGE_EXECUTE_READWRITE, &dwOldProtect);

	memcpy(pBytePatch->Address, pBytePatch->pOldOpcodes, pBytePatch->nBytes);

	FlushInstructionCache(GetCurrentProcess(), pBytePatch->Address, pBytePatch->nBytes);

	VirtualProtect(pBytePatch->Address, pBytePatch->nBytes, dwOldProtect, &dwOldProtect);

	pBytePatch->Patched = FALSE;

	LeaveCriticalSection(&pBytePatch->CriticalSection);
}

VOID RestoreMemory(CONST VOID* pMem)
{
	switch (*(INT*)pMem)
	{
	case NOP_MEMORY_MAGIC:
		RestoreMemoryNop((PNOP_MEMORY)pMem);
		return;
	case BYTE_PATCH_MEMORY_MAGIC:
		RestoreMemoryBytePatch((PBYTE_PATCH_MEMORY)pMem);
		return;
	default:
		__debugbreak();
		return;
	}
}

BOOL HookFunc64(LPVOID pSrcFunc, LPVOID pDstFunc, SIZE_T cbLength, HOOK_FUNC* pHook)
{
	if (cbLength < MINIMUM_HOOK_LENGTH64)
		return FALSE;

	if (pHook->m_bHooked)
		return FALSE;

	DWORD dwOldProtect;

	if (!VirtualProtect(pSrcFunc, cbLength, PAGE_EXECUTE_READWRITE, &dwOldProtect))
		return FALSE;

	pHook->m_pSrcFunc = pSrcFunc;
	pHook->m_pOldInstructions = (BYTE*)malloc(cbLength);

	if (!pHook->m_pOldInstructions)
		return FALSE;

	memcpy(pHook->m_pOldInstructions, pSrcFunc, cbLength);
	pHook->m_length = cbLength;

	NOPMemory((LPBYTE)pSrcFunc + MINIMUM_HOOK_LENGTH64, cbLength - MINIMUM_HOOK_LENGTH64);

	/*
		push rax
		movabs rax, imm64
		xchg rax, [rsp]
		ret

	{ 0x50, 0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x48, 0x87, 0x04, 0x24, 0xC3 }

	or

		push rax
		movabs rax, imm64
		jmp rax
		pop rax

	{ 0x50, 0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xE0, 0x58 };

	or active vvvv

	rsp -= 16

		push rbx
		movabs rbx, imm64
		call rbx
		pop rbx

	{ 0x53, 0x48, 0xBB, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xD3, 0x5B }

	or  active vvvv

	Saves stack pointer

			0:  53                      push   rbx
			1:  55                      push   rbp
			2:  48 89 e5                mov    rbp,rsp
			5:  48 bb 00 00 00 00 00    movabs rbx, imm64
			c:  00 00 00
			f:  ff d3                   call   rbx
			11: 48 89 ec                mov    rsp,rbp
			14: 5d                      pop    rbp
			15: 5b                      pop    rbx

	{ 0x53, 0x55, 0x48, 0x89, 0xE5, 0x48, 0xBB, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xD3, 0x48, 0x89, 0xEC, 0x5D, 0x5B }
	*/
	static BYTE detour[] = { 0x53, 0x55, 0x48, 0x89, 0xE5, 0x48, 0xBB, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xD3, 0x48, 0x89, 0xEC, 0x5D, 0x5B };

	memcpy(pHook->m_Detour, detour, sizeof(detour));
	*(QWORD*)&pHook->m_Detour[7] = (QWORD)pDstFunc;
	memcpy(pSrcFunc, pHook->m_Detour, MINIMUM_HOOK_LENGTH64);

	FlushInstructionCache(GetCurrentProcess(), pSrcFunc, cbLength);
	pHook->m_bHooked = TRUE;

	if (!VirtualProtect(pSrcFunc, cbLength, dwOldProtect, &dwOldProtect))
		return FALSE;

	return TRUE;
}

static BOOL RehookFunc(HOOK_FUNC* pHook)
{
	if (pHook->m_bHooked)
		return FALSE;

	DWORD dwOldProtect;

	if (!VirtualProtect(pHook->m_pSrcFunc, pHook->m_length, PAGE_EXECUTE_READWRITE, &dwOldProtect))
		return FALSE;

	NOPMemory((LPBYTE)pHook->m_pSrcFunc + MINIMUM_HOOK_LENGTH64, pHook->m_length - MINIMUM_HOOK_LENGTH64);

	memcpy(pHook->m_pSrcFunc, pHook->m_Detour, MINIMUM_HOOK_LENGTH64);
	pHook->m_bHooked = TRUE;

	FlushInstructionCache(GetCurrentProcess(), pHook->m_pSrcFunc, pHook->m_length);

	VirtualProtect(pHook->m_pSrcFunc, pHook->m_length, dwOldProtect, &dwOldProtect);

	if (!VirtualProtect(pHook->m_pSrcFunc, pHook->m_length, dwOldProtect, &dwOldProtect))
		return FALSE;

	return TRUE;
}

BOOL UnhookFunc(HOOK_FUNC* pHook)
{
	if (!pHook->m_bHooked)
		return FALSE;

	DWORD dwOldProtect;

	if (!VirtualProtect(pHook->m_pSrcFunc, pHook->m_length, PAGE_EXECUTE_READWRITE, &dwOldProtect))
		return FALSE;

	memcpy(pHook->m_pSrcFunc, pHook->m_pOldInstructions, pHook->m_length);

	FlushInstructionCache(GetCurrentProcess(), pHook->m_pSrcFunc, pHook->m_length);
	pHook->m_bHooked = FALSE;

	if (!VirtualProtect(pHook->m_pSrcFunc, pHook->m_length, dwOldProtect, &dwOldProtect))
		return FALSE;

	return TRUE;
}

ISBADPTR_STATUS IsBadReadPtr(const void* p)
{
	MEMORY_BASIC_INFORMATION mbi;
	ISBADPTR_STATUS status = VALID_PTR;

	SIZE_T nBytes = VirtualQuery(p, &mbi, sizeof(MEMORY_BASIC_INFORMATION));

	if (!nBytes)
		return VIRTUAL_QUERY_FAILED; // To get extended error information, call GetLastError

	if (mbi.State == MEM_FREE)
		status |= MEMORY_FREED;

	if (mbi.Protect & PAGE_GUARD)
		status |= GUARD_PAGE;

	if (mbi.Protect & PAGE_NOACCESS)
		status |= NO_ACCESS_PAGE;

	if (mbi.Protect & PAGE_EXECUTE)
		status |= EXECUTE_PAGE;

	return status;
}

ISBADPTR_STATUS IsBadCodePtr(const void* p)
{
	MEMORY_BASIC_INFORMATION mbi;
	ISBADPTR_STATUS status = VALID_PTR;

	SIZE_T nBytes = VirtualQuery(p, &mbi, sizeof(MEMORY_BASIC_INFORMATION));

	if (!nBytes)
		return VIRTUAL_QUERY_FAILED; // To get extended error information, call GetLastError

	if (mbi.State == MEM_FREE)
		status |= MEMORY_FREED;

	if (mbi.Protect & PAGE_GUARD)
		status |= GUARD_PAGE;

	if (mbi.Protect & PAGE_NOACCESS)
		status |= NO_ACCESS_PAGE;

	if (mbi.Protect & PAGE_READONLY)
		status |= READ_PAGE;

	if (mbi.Protect & PAGE_READWRITE)
		status |= READ_WRITE_PAGE;

	if (mbi.Protect & PAGE_WRITECOPY)
		status |= WRITE_COPY_PAGE;

	return status;
}

void SHA256_PrintDigest(const SHA256Digest* pDigest, CHAR* szHash, UINT cbSize)
{
	static const char* const s_HexChars = "0123456789ABCDEF";
	
	cbSize >>= 1;

	// FIXME: use assert?
	if (cbSize < sizeof(SHA256Digest)-1)
		return;

	for (UINT i = 0; i < cbSize; ++i)
	{
		UINT8 c = pDigest->Value8[i];
		szHash[i * 2] = s_HexChars[(c >> 4) & 0xF]; // / 16
		szHash[i * 2 + 1] = s_HexChars[c & 0xF];
	}
	szHash[cbSize] = (CHAR)0; // Null terminator
}

void SHA256_PrintDigest(const SHA256Digest* pDigest, SHA256DigestStr* pStorage)
{
	static const char* const s_HexChars = "0123456789ABCDEF";

	for (UINT i = 0; i < sizeof(SHA256Digest) - 1; ++i)
	{
		UINT8 c = pDigest->Value8[i];
		pStorage->Hash[i * 2] = s_HexChars[(c >> 4) & 0xF]; // / 16
		pStorage->Hash[i * 2 + 1] = s_HexChars[c & 0xF];
	}
	pStorage->Hash[sizeof(pStorage->Hash) - 1] = 0;
}

static NTSTATUS QueryBinaryHash(HMODULE hModule, ULONG SizeOfImage, BYTE* pHash, PULONG puHashSize)
{
	ULONG uHashLengthSize;

	// Query the hash length
	NTSTATUS Status = BCryptGetProperty(BCRYPT_SHA256_ALG_HANDLE, BCRYPT_HASH_LENGTH, (PUCHAR)puHashSize, 
		sizeof(ULONG), &uHashLengthSize, 0);

	if (SUCCEEDED(Status))
	{
		// Create the hash
		Status = BCryptHash(BCRYPT_SHA256_ALG_HANDLE, NULL, 0, (PUCHAR)hModule, SizeOfImage, pHash, *puHashSize);
	}

	return Status;
}

#include <stdio.h>

NTSTATUS KsHashModulesCallback64(PLDR_DATA_TABLE_ENTRY pEntry, PVOID pCtx)
{
	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)pEntry->DllBase;
	PIMAGE_NT_HEADERS pNtHdrs = ((PIMAGE_NT_HEADERS)((ULONG_PTR)pDosHeader + pDosHeader->e_lfanew));
	CONST WCHAR* szPlat = TEXT("UNK");

	if (pNtHdrs->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC)
	{
		szPlat = TEXT("x64");
	}
	else if (pNtHdrs->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC)
	{
		szPlat = TEXT("x86");
	}

	ULONG uHashSize;
	SHA256Digest Digest;
	SHA256DigestStr SHA256Hash;

	QueryBinaryHash((HMODULE)pDosHeader, pEntry->SizeOfImage, Digest.Value8, &uHashSize);
	SHA256_PrintDigest(&Digest, &SHA256Hash);

	printf(("%ls Plat: %ls ImageBase: %08llX Size: %08X SHA256: %s\n"), 
		pEntry->FullDllName.Buffer, szPlat, pEntry->DllBase, pEntry->SizeOfImage, SHA256Hash);


	return STATUS_MORE_ENTRIES;
}

NTSTATUS KsTraverseModuleList64(PPEB pProcessEnvironmentBlock, PVOID pCtx,
	NTSTATUS(*pCallback)(PLDR_DATA_TABLE_ENTRY pEntry, PVOID pCtx));

NTSTATUS KsDumpModules(/*PEPROCESS pProcess, PKERNEL_MODULE_REQUEST pRequest*/)
{
	//KAPC_STATE ApcState;
	//VOID* pPEB;

	NTSTATUS Status = STATUS_SUCCESS;
	BOOL bWow64;

	IsWow64Process(GetCurrentProcess(), &bWow64);

	printf("Proc is %s\n", (bWow64) ? "wow64" : "not wow64");

	PPEB pPEB = GetProcessEnvironmentBlock();

	if (/*PEBStatus != STATUS_PEB_UNKNOWN && */pPEB)
	{
		KsTraverseModuleList64(pPEB, NULL, &KsHashModulesCallback64);

	/*	if (PEBStatus == STATUS_PEB32)
			Status = KsTraverseModuleList32((PPEB32)pPEB, (PVOID)pRequest, KsGetModuleCallback32);
		else if (PEBStatus == STATUS_PEB64)
			Status = KsTraverseModuleList64((PPEB)pPEB, (PVOID)pRequest, KsGetModuleCallback64);*/
	}
	else
	{
		Status = STATUS_INVALID_PARAMETER_1;
		//ERROR("pPEB is an invalid ptr! pPEB: %llx PEPROCESS: %llx\n", (QWORD)pPEB, (QWORD)pProcess);
	}

	return Status;
}

NTSTATUS KsTraverseModuleList64(PPEB pProcessEnvironmentBlock, PVOID pCtx,
	NTSTATUS(*pCallback)(PLDR_DATA_TABLE_ENTRY pEntry, PVOID pCtx))
{
	NTSTATUS Status = STATUS_SUCCESS;

	if (!pProcessEnvironmentBlock)
		return STATUS_INVALID_PARAMETER_1;

	if (!pCallback)
		return STATUS_INVALID_PARAMETER_2;

	EnterCriticalSection(pProcessEnvironmentBlock->LoaderLock);

	PLIST_ENTRY pHead = &(pProcessEnvironmentBlock->Ldr)->InMemoryOrderModuleList; // Get the address of the doubly linked-list head
	PLIST_ENTRY pNext = pHead->Flink; // Get the first entry

	// Traverse the doubly linked-list and call the callee supplied callback
	while (pNext && pNext != pHead)
	{
		Status = pCallback(CONTAINING_RECORD(pNext, LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks), pCtx);

		// If the callback is successful; stop traversing
		// If there are more entries; keep traversing
		if (Status != STATUS_MORE_ENTRIES && NT_SUCCESS(Status))
			break;

		pNext = pNext->Flink;
	}

	LeaveCriticalSection(pProcessEnvironmentBlock->LoaderLock);

	return Status;
}