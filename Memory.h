#pragma once
#include <Windows.h>
#include <Psapi.h>
#include <string>

#ifndef __cplusplus 
#define class struct
#endif

#define RandomInt(_min, _max) (rand() % ((_max) - (_min) + 1) + (_min))

#define INRANGE(x, a, b) ((x) >= (a) && (x) <= (b))
#define GetBits(x) (INRANGE(((x) & (~0x20)), 'A', 'F') ? (((x) & (~0x20)) - 'A' + 0xA) : (INRANGE((x), '0', '9') ? (x) - '0' : 0))
#define GetByte(x) ((GetBits((x)[0]) << 4) | GetBits((x)[1]))

#define MakePtr32(cast, ptr, rva) ((cast)((BYTE* __ptr32)(ptr) + (DWORD)(rva)))
#define MakePtr64(cast, ptr, rva) ((cast)((BYTE* __ptr64)(ptr) + (DWORD)(rva)))

#define FOURCC(a, b, c, d) ( (DWORD)(((DWORD)((BYTE)a)) | (((DWORD)((BYTE)b)) << 8) | (((DWORD)((BYTE)c)) << 16) | (((DWORD)((BYTE)d)) << 24)) )
#define EIGHTCC(a, b, c, d, e, f, g, h) ( (QWORD)(((QWORD)((BYTE)a)) | (((QWORD)((BYTE)b)) << 8) | (((QWORD)((BYTE)c)) << 16) | (((QWORD)((BYTE)d)) << 24) | (((QWORD)((BYTE)e)) << 32) | (((QWORD)((BYTE)f)) << 40) | (((QWORD)((BYTE)g)) << 48) | (((QWORD)((BYTE)h)) << 56)) )

#define NOP_MEMORY_MAGIC (INT)(0x706F6E) // "nop"
#define BYTE_PATCH_MEMORY_MAGIC (INT)(0x6863) // "bp"

#define MINIMUM_HOOK_LENGTH32 5
#define MINIMUM_HOOK_LENGTH64 14


#ifdef _WIN64
#define MakePtr MakePtr64
#define HookFunc HookFunc64
#define MINIMUM_HOOK_LENGTH MINIMUM_HOOK_LENGTH64
#else
#define MakePtr MakePtr32
#define HookFunc HookFunc32
#define MINIMUM_HOOK_LENGTH MINIMUM_HOOK_LENGTH32
#endif

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

#define InitalizeNopMemoryDefault(pNop) memset((PBYTE)(pNop) + FIELD_OFFSET(NOP_MEMORY, Patched), 0, sizeof(NOP_MEMORY) - sizeof(INT));
#define InitalizeNopMemory(pNop, address, pOldOpcodes, nBytes) (pNop)->Magic = NOP_MEMORY_MAGIC;\
															   (pNop)->Patched = FALSE;\
															   (pNop)->Address = address;\
															   (pNop)->pOldOpcodes = pOldOpcodes;\
															   (pNop)->nBytes = nBytes;

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

#define InitalizeBytePatchMemoryDefault(pBytePatch) memset((PBYTE)(pBytePatch) + FIELD_OFFSET(BYTE_PATCH_MEMORY, Patched), 0, sizeof(BYTE_PATCH_MEMORY) - sizeof(INT));
#define InitalizeBytePatchMemory(pBytePatch, address, pNewOpcodes, pOldOpcodes, nBytes) (pBytePatch)->Magic = BYTE_PATCH_MEMORY_MAGIC;\
																						(pBytePatch)->Patched = FALSE;\
																						(pBytePatch)->Address = address;\
																						(pBytePatch)->pNewOpcodes = pNewOpcodes;\
																						(pBytePatch)->pOldOpcodes = pOldOpcodes;\
																						(pBytePatch)->nBytes = nBytes;

typedef struct _HOOK_FUNC
{
	void* m_pSrcFunc;
	BYTE* m_pOldInstructions;
	SIZE_T m_length;
	bool m_hooked;
} HOOK_FUNC, *PHOOK_FUNC;

enum _ISBADPTR_STATUS
{
	VALID_PTR = 0x0,
	VIRTUAL_QUERY_FAILED = 0x1,
	MEMORY_FREED = 0x2,
	GUARD_PAGE = 0x4,
	NO_ACCESS_PAGE = 0x8,
	EXECUTE_ONLY_PAGE = 0x10,
	READ_ONLY_PAGE = 0x20,
	READ_WRITE_PAGE = 0x40,
	WRITE_COPY_PAGE = 0x80
};
typedef DWORD ISBADPTR_STATUS;

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

	static inline QWORD ReadPtr64(QWORD rip, SIZE_T uOffset)
	{
		return rip + uOffset + sizeof(LONG) + *(LONG*)(rip + uOffset);
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
		SIZE_T OpcodeSize = uOffset + sizeof(LONG);
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

		SIZE_T OpcodeSize = uOffset + sizeof(LONG);
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

		SIZE_T OpcodeSize = uOffset + sizeof(LONG);
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
			__debugbreak();
			return;
		}
	}

	// maybe we should have different shellcodes that use other registers
	BOOL HookFunc64(LPVOID pSrcFunc, LPVOID pDstFunc, SIZE_T length, HOOK_FUNC* pStruct)
	{
		if (length < MINIMUM_HOOK_LENGTH64)
			return false;

		if (pStruct->m_hooked)
			return false;

		DWORD oldProtect;

		if (!VirtualProtect(pSrcFunc, length, PAGE_EXECUTE_READWRITE, &oldProtect))
			return false;

		pStruct->m_hooked = TRUE;
		pStruct->m_pOldInstructions = (BYTE*)malloc(length);
		pStruct->m_pSrcFunc = pSrcFunc;
		memcpy(pStruct->m_pOldInstructions, pSrcFunc, length);
		pStruct->m_length = length;

		NOPMemory((LPBYTE)pSrcFunc + MINIMUM_HOOK_LENGTH64, length - MINIMUM_HOOK_LENGTH64);

		/*
		push rax
		movabs rax, address
		xchg rax, [rsp]
		ret

		{ 0x50, 0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x48, 0x87, 0x04, 0x24, 0xC3 }

		or

		push rax
		movabs rax, addy
		jmp rax
		pop rax

		{ 0x50, 0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xE0, 0x58 };

		or active vvvv

		rsp -= 16

		push rax
		movabs rax, addy
		call rax
		pop rax

		{ 0x50, 0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xD0, 0x58 }
		*/
		static BYTE detour[] = { 0x50, 0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xD0, 0x58 };
		memcpy((BYTE*)pSrcFunc, detour, sizeof(detour));
		*(QWORD*)((BYTE*)pSrcFunc + 3) = (QWORD)pDstFunc;

		if (!VirtualProtect(pSrcFunc, length, oldProtect, NULL))
			return false;

		return true;
	}

	BOOL UnhookFunc(HOOK_FUNC* pStruct)
	{
		if (!pStruct->m_hooked)
			return false;

		DWORD oldProtect;

		if (!VirtualProtect(pStruct->m_pSrcFunc, pStruct->m_length, PAGE_EXECUTE_READWRITE, &oldProtect))
			return false;

		memcpy(pStruct->m_pSrcFunc, pStruct->m_pOldInstructions, pStruct->m_length);
		pStruct->m_hooked = false;

		if (!VirtualProtect(pStruct->m_pSrcFunc, pStruct->m_length, oldProtect, NULL))
			return false;

		return true;
	}

	static ISBADPTR_STATUS IsBadReadPtr(const void* p)
	{
		MEMORY_BASIC_INFORMATION mbi;
		ISBADPTR_STATUS status = VALID_PTR;

		SIZE_T nBytes = VirtualQuery(p, &mbi, sizeof(MEMORY_BASIC_INFORMATION));

		if (!nBytes || nBytes == ERROR_INVALID_PARAMETER)
			return VIRTUAL_QUERY_FAILED;

		if (mbi.State == MEM_FREE)
			status |= MEMORY_FREED;

		if (mbi.Protect & PAGE_GUARD)
			status |= GUARD_PAGE;

		if (mbi.Protect & PAGE_NOACCESS)
			status |= NO_ACCESS_PAGE;

		if (mbi.Protect & PAGE_EXECUTE)
			status |= READ_ONLY_PAGE;

		return status;
	}

	static ISBADPTR_STATUS IsBadCodePtr(const void* p)
	{
		MEMORY_BASIC_INFORMATION mbi;
		ISBADPTR_STATUS status = VALID_PTR;

		SIZE_T nBytes = VirtualQuery(p, &mbi, sizeof(MEMORY_BASIC_INFORMATION));

		if (!nBytes || nBytes == ERROR_INVALID_PARAMETER)
			return VIRTUAL_QUERY_FAILED;

		if (mbi.State == MEM_FREE)
			status |= MEMORY_FREED;

		if (mbi.Protect & PAGE_GUARD)
			status |= GUARD_PAGE;

		if (mbi.Protect & PAGE_NOACCESS)
			status |= NO_ACCESS_PAGE;

		if (mbi.Protect & PAGE_READONLY)
			status |= READ_ONLY_PAGE;

		if (mbi.Protect & PAGE_READWRITE)
			status |= READ_WRITE_PAGE;

		if (mbi.Protect & PAGE_WRITECOPY)
			status |= WRITE_COPY_PAGE;

		return status;
	}

private:
};
extern CMemory* g_pMemory;