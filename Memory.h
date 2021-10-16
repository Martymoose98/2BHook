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

#define MakePtr32(cast, ptr, rva) ((cast)((BYTE* __ptr32)(ptr) + (LONG)(rva)))
#define MakePtr64(cast, ptr, rva) ((cast)((BYTE* __ptr64)(ptr) + (LONG)(rva)))

#define FOURCC(a, b, c, d) ( (DWORD)(((DWORD)((BYTE)a)) | (((DWORD)((BYTE)b)) << 8) | (((DWORD)((BYTE)c)) << 16) | (((DWORD)((BYTE)d)) << 24)) )
#define EIGHTCC(a, b, c, d, e, f, g, h) ( (QWORD)(((QWORD)((BYTE)a)) | (((QWORD)((BYTE)b)) << 8) | (((QWORD)((BYTE)c)) << 16) | (((QWORD)((BYTE)d)) << 24) | (((QWORD)((BYTE)e)) << 32) | (((QWORD)((BYTE)f)) << 40) | (((QWORD)((BYTE)g)) << 48) | (((QWORD)((BYTE)h)) << 56)) )

#define NOP_MEMORY_MAGIC (INT)(0x706F6E) // "nop"
#define BYTE_PATCH_MEMORY_MAGIC (INT)(0x6863) // "bp"

#define MINIMUM_HOOK_LENGTH32 5
#define MINIMUM_HOOK_LENGTH64 22 //14 //15


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
} NOP_MEMORY, * PNOP_MEMORY;

#define InitalizeNopMemoryDefault(pNop) memset((PBYTE)(pNop) + FIELD_OFFSET(NOP_MEMORY, Patched), 0, sizeof(NOP_MEMORY) - sizeof(INT));
#define InitalizeNopMemory(pNop, address, cb) (pNop)->Magic = NOP_MEMORY_MAGIC;\
												  (pNop)->Patched = FALSE;\
												  (pNop)->Address = (VOID*)(address);\
												  (pNop)->pOldOpcodes = NULL;\
												  (pNop)->nBytes = (cb);

/*
	If your going to call ZeroMemory aka. memset(dst, 0, length); you need to restore the Magic field to BYTE_PATCH_MEMORY_MAGIC,
	if you are planning to free the memory with RestoreMemory. Also, if you want PatchBytes to allocate the OldOpcode buffer for
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
} BYTE_PATCH_MEMORY, * PBYTE_PATCH_MEMORY;

#define InitalizeBytePatchMemoryDefault(pBytePatch) memset((PBYTE)(pBytePatch) + FIELD_OFFSET(BYTE_PATCH_MEMORY, Patched), 0, sizeof(BYTE_PATCH_MEMORY) - sizeof(INT));
#define InitalizeBytePatchMemory(pBytePatch, address, pOpcodes, cb) (pBytePatch)->Magic = BYTE_PATCH_MEMORY_MAGIC;\
																		   (pBytePatch)->Patched = FALSE;\
																		   (pBytePatch)->Address = (VOID*)(address);\
																		   (pBytePatch)->pNewOpcodes = (PBYTE)(pOpcodes);\
																		   (pBytePatch)->pOldOpcodes = NULL;\
																		   (pBytePatch)->nBytes = (cb);

typedef struct _HOOK_FUNC
{
	void* m_pSrcFunc;
	BYTE* m_pOldInstructions;
	SIZE_T m_length;
	BYTE m_Detour[MINIMUM_HOOK_LENGTH];
	BOOLEAN m_bHooked;
} HOOK_FUNC, * PHOOK_FUNC;

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
	ULONG_PTR FindPattern(const char* szModuleName, const char* szPattern)
	{
		const char* szPat = szPattern;
		ULONG_PTR uFirstMatch = 0;
		ULONG_PTR uStart = (ULONG_PTR)GetModuleHandleA(szModuleName);
		MODULEINFO ModuleInfo;

		if (!GetModuleInformation(GetCurrentProcess(), (HMODULE)uStart, &ModuleInfo, sizeof(MODULEINFO)))
			return -1;

		ULONG_PTR uRangeEnd = uStart + ModuleInfo.SizeOfImage;

		for (ULONG_PTR pCur = uStart; pCur < uRangeEnd; pCur++)
		{
			if (!*szPat)
				return uFirstMatch;

			if (*(BYTE*)szPat == '\?' || *(BYTE*)pCur == GetByte(szPat))
			{
				if (!uFirstMatch)
					uFirstMatch = pCur;

				if (!szPat[2])
					return uFirstMatch;

				if (*(WORD*)szPat == '\?\?' || *(BYTE*)szPat != '\?')
					szPat += 3;
				else
					szPat += 2;    //one ?
			}
			else
			{
				szPat = szPattern;
				uFirstMatch = 0;
			}
		}
		return 0;
	}

	// WARNING: I think this is supposed to return LONG instead of DWORD aka. ULONG
	// rel32 jmp instructions are signed rva's.
	// It's not a big deal currently 'cause the func is unused and therefore untested
	// but, if used unchanged, I presume will cause issues/bugs/crashes.
	inline DWORD CalculateJump(QWORD rip, QWORD dest, SIZE_T uOffset)
	{
		return (DWORD)(dest - rip - (uOffset + sizeof(DWORD)));
	}

	static inline QWORD ReadPtr64(QWORD rip, SIZE_T uOffset)
	{
		return rip + uOffset + sizeof(LONG) + *(LONG*)(rip + uOffset);
	}

	/*
		NOTE: When using this version of this function, the pointer has to be the first unknown
		@params
		szModulename - name of the module to scan (IDA or PEiD Style)
		szPattern - the byte pattern to search the module for
	*/
	ULONG_PTR FindPatternPtr(const char* szModulename, const char* szPattern)
	{
		ULONG_PTR rip = FindPattern(szModulename, szPattern);
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
		szModulename - name of the module to scan (IDA or PEiD Style)
		szPattern - the byte pattern to search the module for
		uOffset - the byte offset to the pointer from the beginning of szPattern
	*/
	ULONG_PTR FindPatternPtr(const char* szModulename, const char* szPattern, UINT uOffset)
	{
		ULONG_PTR rip = FindPattern(szModulename, szPattern);
		SIZE_T OpcodeSize = uOffset + sizeof(LONG);
		return (ULONG_PTR)((*(LONG*)(rip + uOffset)) + OpcodeSize) + rip;
	}

	VOID NopMemory(VOID* address, SIZE_T nBytes, BYTE** ppOldOpcodes)
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

		CRITICAL_SECTION cs;
		DWORD dwOldProtect;

		InitializeCriticalSection(&cs);
		EnterCriticalSection(&cs);

		VirtualProtect(pNop->Address, pNop->nBytes, PAGE_EXECUTE_READWRITE, &dwOldProtect);

		memcpy(pNop->pOldOpcodes, pNop->Address, pNop->nBytes);
		NOPMemory(pNop->Address, pNop->nBytes);

		VirtualProtect(pNop->Address, pNop->nBytes, dwOldProtect, &dwOldProtect);

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

		VirtualProtect(pNop->Address, pNop->nBytes, dwOldProtect, &dwOldProtect);

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

		CRITICAL_SECTION cs;
		DWORD dwOldProtect;

		InitializeCriticalSection(&cs);
		EnterCriticalSection(&cs);

		VirtualProtect(pBytePatch->Address, pBytePatch->nBytes, PAGE_EXECUTE_READWRITE, &dwOldProtect);

		memcpy(pBytePatch->pOldOpcodes, pBytePatch->Address, pBytePatch->nBytes);
		memcpy(pBytePatch->Address, pBytePatch->pNewOpcodes, pBytePatch->nBytes);

		VirtualProtect(pBytePatch->Address, pBytePatch->nBytes, dwOldProtect, &dwOldProtect);

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

		VirtualProtect(pBytePatch->Address, pBytePatch->nBytes, dwOldProtect, &dwOldProtect);

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
	BOOL HookFunc64(LPVOID pSrcFunc, LPVOID pDstFunc, SIZE_T length, HOOK_FUNC* pHook)
	{
		if (length < MINIMUM_HOOK_LENGTH64)
			return FALSE;

		if (pHook->m_bHooked)
			return FALSE;

		DWORD dwOldProtect;

		if (!VirtualProtect(pSrcFunc, length, PAGE_EXECUTE_READWRITE, &dwOldProtect))
			return FALSE;

		pHook->m_bHooked = TRUE;
		pHook->m_pSrcFunc = pSrcFunc;
		pHook->m_pOldInstructions = (BYTE*)malloc(length);

		if (!pHook->m_pOldInstructions)
			return FALSE;

		memcpy(pHook->m_pOldInstructions, pSrcFunc, length);
		pHook->m_length = length;

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

			push rbx
			movabs rbx, addy
			call rbx
			pop rbx

		{ 0x53, 0x48, 0xBB, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xD3, 0x5B }
		
		or  active vvvv

		Saves stack pointer

				0:  53                      push   rbx
				1:  55                      push   rbp
				2:  48 89 e5                mov    rbp,rsp
				5:  48 bb 00 00 00 00 00    movabs rbx, addy
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

		if (!VirtualProtect(pSrcFunc, length, dwOldProtect, &dwOldProtect))
			return FALSE;

		//FlushInstructionCache(GetCurrentProcess(), pSrcFunc, length);

		return FALSE;
	}

	BOOL RehookFunc(HOOK_FUNC* pHook)
	{
		if (pHook->m_bHooked)
			return FALSE;

		DWORD dwOldProtect;

		if (!VirtualProtect(pHook->m_pSrcFunc, pHook->m_length, PAGE_EXECUTE_READWRITE, &dwOldProtect))
			return FALSE;

		NOPMemory((LPBYTE)pHook->m_pSrcFunc + MINIMUM_HOOK_LENGTH64, pHook->m_length - MINIMUM_HOOK_LENGTH64);

		memcpy(pHook->m_pSrcFunc, pHook->m_Detour, MINIMUM_HOOK_LENGTH64);
		pHook->m_bHooked = TRUE;

		if (!VirtualProtect(pHook->m_pSrcFunc, pHook->m_length, dwOldProtect, &dwOldProtect))
			return FALSE;

		//FlushInstructionCache(GetCurrentProcess(), pHook->m_pSrcFunc, pHook->m_length);

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
		pHook->m_bHooked = FALSE;

		if (!VirtualProtect(pHook->m_pSrcFunc, pHook->m_length, dwOldProtect, &dwOldProtect))
			return FALSE;

		//FlushInstructionCache(GetCurrentProcess(), pHook->m_pSrcFunc, pHook->m_length);

		return TRUE;
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