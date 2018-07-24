#pragma once
#include <Windows.h>
#include <Psapi.h>
#include <string>

#ifndef __cplusplus 
#define class struct
#endif

#define NOP_MEMORY_MAGIC (INT)(0x706F6E) // "nop"
#define BYTE_PATCH_MEMORY_MAGIC (INT)(0x6863) // "bp"

#define RandomInt(_min, _max) (rand() % ((_max) - (_min)) + (_min))

#define INRANGE(x, a, b) ((x) >= (a) && (x) <= (b))
#define GetBits(x) (INRANGE(((x) & (~0x20)), 'A', 'F') ? (((x) & (~0x20)) - 'A' + 0xA) : (INRANGE((x), '0', '9') ? (x) - '0' : 0))
#define GetByte(x) (GetBits((x)[0]) << 4 | GetBits((x)[1]))

#define MakePtr(cast, ptr, addValue) (cast)((DWORD)(ptr) + (DWORD)(addValue))

#define NOPMemory(dst, size) memset((VOID*)(dst), 0x90, size)

typedef ULONGLONG QWORD;

typedef struct _NOP_MEMORY
{
	INT Magic = NOP_MEMORY_MAGIC;
	BOOL Patched;
	VOID* Address;
	BYTE* pOldOpcodes;
	SIZE_T nBytes;
} NOP_MEMORY, *PNOP_MEMORY;

/*
	If your going to call ZeroMemory(aka. memset(dst, 0, length);) you need to restore the Magic field to BYTE_PATCH_MEMORY_MAGIC,
	if you are planning to free the memory with RestoreMemory.Also, if you want PatchBytes to allocate the OldOpcode buffer for
	you pOldOpcodes needs to equal null.
*/
typedef struct _BYTE_PATCH_MEMORY
{
	INT Magic = BYTE_PATCH_MEMORY_MAGIC;
	BOOL Patched;
	VOID* Address;
	BYTE* pNewOpcodes;
	BYTE* pOldOpcodes;
	SIZE_T nBytes;
} BYTE_PATCH_MEMORY, *PBYTE_PATCH_MEMORY;

enum _ISBADCODEPTR_STATUS : DWORD
{
	VALID_PTR = 0x0,
	VIRTUAL_QUERY_FAILED = 0x1,
	MEMORY_FREED = 0x2,
	GUARD_PAGE = 0x4,
	NO_ACCESS_PAGE = 0x8,
	EXECUTE_ONLY_PAGE = 0x10,
};
typedef DWORD ISBADCODEPTR_STATUS;

class CMemory
{
public:
	DWORD_PTR FindPattern(const char* moduleName, const char* pattern)
	{
		const char* pat = pattern;
		DWORD_PTR firstMatch = 0;
		DWORD_PTR rangeStart = (DWORD_PTR)GetModuleHandle(moduleName);
		MODULEINFO miModInfo;

		if (!GetModuleInformation(GetCurrentProcess(), (HMODULE)rangeStart, &miModInfo, sizeof(MODULEINFO)))
			return -1;

		DWORD_PTR rangeEnd = rangeStart + miModInfo.SizeOfImage;

		for (DWORD_PTR pCur = rangeStart; pCur < rangeEnd; pCur++)
		{
			if (!*pat)
				return firstMatch;

			if (*(BYTE*)pat == '\?' || *(BYTE*)pCur == GetByte(pat))
			{
				if (!firstMatch)
					firstMatch = pCur;

				if (!pat[2])
					return firstMatch;

				if (*(WORD*)pat == '\?\?' || *(BYTE*)pat != '\?')
					pat += 3;
				else
					pat += 2;    //one ?
			}
			else
			{
				pat = pattern;
				firstMatch = 0;
			}
		}
		return NULL;
	}

	inline DWORD CalculateJump(QWORD rip, QWORD dest, SIZE_T uOffset)
	{
		return (DWORD)(dest - rip - (uOffset + sizeof(DWORD)));
	}

	inline QWORD ReadPtr64(QWORD rip, SIZE_T uOffset)
	{
		return rip + uOffset + sizeof(DWORD) + *(LONG*)(rip + uOffset);
	}

	/*
		Note: When using this version of this function, the pointer has to be the first unknown.
		Also, this is the fastest verison of this function since the uOffset is compile-time
		known.
		@params
		szModulename - name of the module to scan (IDA or PEiD Style)
		szPattern - the byte pattern to search the module for
		uOffset - the byte offset to the pointer from the beginning of szPattern
	*/
	QWORD FindPatternPtr64(const char* szModulename, const char* szPattern, UINT uOffset)
	{
		QWORD rip = FindPattern(szModulename, szPattern);
		SIZE_T OpcodeSize = uOffset + sizeof(DWORD);
		return (QWORD)((*(LONG*)(rip + uOffset)) + OpcodeSize) + rip;
	}

	/*
		Note: When using this version of this function, the pointer has to be the first unknown
		@params
		szModulename - name of the module to scan (IDA or PEiD Style)
		szPattern - the byte pattern to search the module for
	*/
	QWORD FindPatternPtr64(const char* szModulename, const char* szPattern)
	{
		QWORD rip = FindPattern(szModulename, szPattern);
		UINT uOffset = 0;

		while (*(szPattern + uOffset * 3) != '?' && *(WORD*)(szPattern + uOffset * 3) != '??')
			++uOffset;

		SIZE_T OpcodeSize = uOffset + sizeof(DWORD);
		return (QWORD)((*(LONG*)(rip + uOffset)) + OpcodeSize) + rip;
	}

	/*
		Note: When using this version of this function, the pointer can be anywhere in the pattern
		@params
		szModulename - name of the module to scan
		szPattern - the byte pattern to search the module for (IDA or PEiD Style)
		uOpcodeStartOffset - offset from szPattern to the pointer opcode start
	*/
	QWORD FindPatternPtr64Ex(const char* szModulename, const char* szPattern, UINT uOpcodeStartOffset)
	{
		QWORD rip = FindPattern(szModulename, szPattern) + uOpcodeStartOffset;
		UINT uOffset = 0;

		while (*(szPattern + uOpcodeStartOffset + uOffset * 3) != '?' && *(WORD*)(szPattern + uOpcodeStartOffset + uOffset * 3) != '??')
			++uOffset;

		SIZE_T OpcodeSize = uOffset + sizeof(DWORD);
		return (QWORD)((*(LONG*)(rip + uOffset)) + OpcodeSize) + rip;
	}

	VOID NopMemory(VOID* address, SIZE_T nBytes, BYTE** ppOldOpcodes)
	{
		if (!address)
			return;

		if (!nBytes)
			return;

		if (!*ppOldOpcodes)
			*ppOldOpcodes = (BYTE*)malloc(nBytes);

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
			pNop->pOldOpcodes = (BYTE*)malloc(pNop->nBytes);

		if (pNop->Patched)
			return;

		CRITICAL_SECTION cs;
		DWORD dwOldProtect;

		InitializeCriticalSection(&cs);
		EnterCriticalSection(&cs);

		VirtualProtect(pNop->Address, pNop->nBytes, PAGE_EXECUTE_READWRITE, &dwOldProtect);

		memcpy(pNop->pOldOpcodes, pNop->Address, pNop->nBytes);
		NOPMemory(pNop->Address, pNop->nBytes);

		VirtualProtect(pNop->Address, pNop->nBytes, dwOldProtect, NULL);

		LeaveCriticalSection(&cs);
		pNop->Patched = TRUE;
	}

	VOID RestoreMemory(VOID* address, SIZE_T nBytes, BYTE* pOldOpcodes)
	{
		if (!address)
			return;

		if (!nBytes)
			return;

		if (!pOldOpcodes)
			return;

		memcpy(address, pOldOpcodes, nBytes);
	}

	VOID RestoreMemoryNop(CONST PNOP_MEMORY pNop)
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

		CRITICAL_SECTION cs;
		DWORD dwOldProtect;

		InitializeCriticalSection(&cs);
		EnterCriticalSection(&cs);

		VirtualProtect(pNop->Address, pNop->nBytes, PAGE_EXECUTE_READWRITE, &dwOldProtect);

		memcpy(pNop->Address, pNop->pOldOpcodes, pNop->nBytes);

		VirtualProtect(pNop->Address, pNop->nBytes, dwOldProtect, NULL);

		LeaveCriticalSection(&cs);
		pNop->Patched = FALSE;
	}

	VOID PatchBytes(VOID* address, SIZE_T nBytes, BYTE* pNewOpcodes, BYTE** ppOldOpcodes)
	{
		if (!address)
			return;

		if (!nBytes)
			return;

		if (!pNewOpcodes)
			return;

		if (!*ppOldOpcodes)
			*ppOldOpcodes = (BYTE*)malloc(nBytes);

		memcpy(*ppOldOpcodes, address, nBytes);
		memcpy(address, pNewOpcodes, nBytes);
	}

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
			pBytePatch->pOldOpcodes = (BYTE*)malloc(pBytePatch->nBytes);

		if (pBytePatch->Patched)
			return;

		CRITICAL_SECTION cs;
		DWORD dwOldProtect;

		InitializeCriticalSection(&cs);
		EnterCriticalSection(&cs);

		VirtualProtect(pBytePatch->Address, pBytePatch->nBytes, PAGE_EXECUTE_READWRITE, &dwOldProtect);
		
		memcpy(pBytePatch->pOldOpcodes, pBytePatch->Address, pBytePatch->nBytes);
		memcpy(pBytePatch->Address, pBytePatch->pNewOpcodes, pBytePatch->nBytes);

		VirtualProtect(pBytePatch->Address, pBytePatch->nBytes, dwOldProtect, NULL);

		LeaveCriticalSection(&cs);
		pBytePatch->Patched = TRUE;
	}

	VOID RestoreMemoryBytePatch(CONST PBYTE_PATCH_MEMORY pBytePatch)
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

		CRITICAL_SECTION cs;
		DWORD dwOldProtect;

		InitializeCriticalSection(&cs);
		EnterCriticalSection(&cs);
		VirtualProtect(pBytePatch->Address, pBytePatch->nBytes, PAGE_EXECUTE_READWRITE, &dwOldProtect);

		memcpy(pBytePatch->Address, pBytePatch->pOldOpcodes, pBytePatch->nBytes);

		VirtualProtect(pBytePatch->Address, pBytePatch->nBytes, dwOldProtect, NULL);

		LeaveCriticalSection(&cs);
		pBytePatch->Patched = FALSE;
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
			return;
		}
	}

	static ISBADCODEPTR_STATUS IsBadCodePtr(const void* p)
	{
		MEMORY_BASIC_INFORMATION mbi;
		ISBADCODEPTR_STATUS ret = VALID_PTR;

		SIZE_T nBytes = VirtualQuery(p, &mbi, sizeof(MEMORY_BASIC_INFORMATION));

		if (!nBytes || nBytes == ERROR_INVALID_PARAMETER)
			return VIRTUAL_QUERY_FAILED;

		if (mbi.State == MEM_FREE)
			ret |= MEMORY_FREED;

		if (mbi.Protect & PAGE_GUARD)
			ret |= GUARD_PAGE;

		if (mbi.Protect & PAGE_NOACCESS)
			ret |= NO_ACCESS_PAGE;

		if (mbi.Protect & PAGE_EXECUTE)
			ret |= EXECUTE_ONLY_PAGE;

		return ret;
	}
private:
};
extern CMemory* g_pMemory;