
#ifndef __MEMORY_H__
#define __MEMORY_H__

//#pragma once


#include <Windows.h>
// can't include ntdef cause it has a stroke
//#include <ntdef.h>
#define UMDF_USING_NTSTATUS
#include <ntstatus.h>

#include <Psapi.h>
#include <malloc.h>
#include <immintrin.h>
#include <stdio.h>

#include "WindowsOpaqueStructures.h"

#ifndef __cplusplus 
#define class struct
#endif

// Everytime I include <winternl.h> i get preprocessor warnings about ntstatus 
// because of include <ntstatus.h>
#pragma region Winternl

//
// Generic test for success on any status value (non-negative numbers
// indicate success).
//

#ifndef NT_SUCCESS
#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)
#endif

//
// Generic test for information on any status value.
//

#ifndef NT_INFORMATION
#define NT_INFORMATION(Status) ((((ULONG)(Status)) >> 30) == 1)
#endif

//
// Generic test for warning on any status value.
//

#ifndef NT_WARNING
#define NT_WARNING(Status) ((((ULONG)(Status)) >> 30) == 2)
#endif

//
// Generic test for error on any status value.
//

#ifndef NT_ERROR
#define NT_ERROR(Status) ((((ULONG)(Status)) >> 30) == 3)
#endif

#pragma endregion Winternl

#define _RandomInt(_min, _max) (rand() % ((_max) - (_min) + 1) + (_min))

#define INRANGE(x, a, b) ((x) >= (a) && (x) <= (b))
#define GetNibble(x) (INRANGE(((x) & (~0x20)), 'A', 'F') ? (((x) & (~0x20)) - 'A' + 0xA) : (INRANGE((x), '0', '9') ? (x) - '0' : 0))
#define GetByte(x) ((GetNibble((x)[0]) << 4) | GetNibble((x)[1]))

#define MakePtr32(type, ptr, rva) ((type)((BYTE* __ptr32)(ptr) + (LONG)(rva)))
#define MakePtr64(type, ptr, rva) ((type)((BYTE* __ptr64)(ptr) + (LONG)(rva)))

#define FOURCC(a, b, c, d) ( (DWORD)(((DWORD)((BYTE)a)) | (((DWORD)((BYTE)b)) << 8) | (((DWORD)((BYTE)c)) << 16) | (((DWORD)((BYTE)d)) << 24)) )
#define EIGHTCC(a, b, c, d, e, f, g, h) ( (QWORD)(((QWORD)((BYTE)a)) | (((QWORD)((BYTE)b)) << 8) | (((QWORD)((BYTE)c)) << 16) | (((QWORD)((BYTE)d)) << 24) | (((QWORD)((BYTE)e)) << 32) | (((QWORD)((BYTE)f)) << 40) | (((QWORD)((BYTE)g)) << 48) | (((QWORD)((BYTE)h)) << 56)) )


#ifndef VALIDATE_OFFSET
#define VALIDATE_OFFSET(s, m, offset) static_assert(offsetof(s, m) == (offset),#s"::"#m" is not at "#offset)
#endif

#ifndef VALIDATE_SIZE
#define VALIDATE_SIZE(s, size) static_assert(sizeof(s) == (size), #s" is not of size "#size)
#endif

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
	INT Magic;
	CRITICAL_SECTION CriticalSection;
	BOOL Patched;
	VOID* Address;
	BYTE* pOldOpcodes;
	SIZE_T nBytes;
} NOP_MEMORY, * PNOP_MEMORY;

static inline VOID InitalizeNopMemoryDefault(PNOP_MEMORY pNop)
{
	pNop->Magic = NOP_MEMORY_MAGIC;
	pNop->Patched = FALSE;
	pNop->Address = NULL;
	pNop->pOldOpcodes = NULL;
	pNop->nBytes = 0;

	InitializeCriticalSection(&pNop->CriticalSection);
}

static VOID InitializeNopMemory(PNOP_MEMORY pNop, VOID* address, SIZE_T cb)
{
	pNop->Magic = NOP_MEMORY_MAGIC;
	pNop->Patched = FALSE;
	pNop->Address = address;
	pNop->pOldOpcodes = NULL;
	pNop->nBytes = cb;

	InitializeCriticalSection(&pNop->CriticalSection);
}

/*
	If your going to call ZeroMemory aka. memset(dst, 0, length); you need to restore the Magic field to BYTE_PATCH_MEMORY_MAGIC,
	if you are planning to free the memory with RestoreMemory. Also, if you want PatchBytes to allocate the OldOpcode buffer for
	you pOldOpcodes needs to equal null.
*/
typedef struct _BYTE_PATCH_MEMORY
{
	INT Magic;
	CRITICAL_SECTION CriticalSection;
	BOOL Patched;
	VOID* Address;
	CONST BYTE* pNewOpcodes;
	BYTE* pOldOpcodes;
	SIZE_T nBytes;
} BYTE_PATCH_MEMORY, * PBYTE_PATCH_MEMORY;

static VOID InitalizeBytePatchMemoryDefault(PBYTE_PATCH_MEMORY pBytePatch)
{
	pBytePatch->Magic = BYTE_PATCH_MEMORY_MAGIC;
	pBytePatch->Patched = FALSE;
	pBytePatch->Address = NULL;
	pBytePatch->pNewOpcodes = NULL;
	pBytePatch->pOldOpcodes = NULL;
	pBytePatch->nBytes = 0;

	InitializeCriticalSection(&pBytePatch->CriticalSection);
}

static VOID InitializeBytePatchMemory(PBYTE_PATCH_MEMORY pBytePatch, VOID* address, CONST BYTE* pOpcodes, SIZE_T cb)
{
	pBytePatch->Magic = BYTE_PATCH_MEMORY_MAGIC;
	pBytePatch->Patched = FALSE;
	pBytePatch->Address = address;
	pBytePatch->pNewOpcodes = pOpcodes;
	pBytePatch->pOldOpcodes = NULL;
	pBytePatch->nBytes = cb;

	InitializeCriticalSection(&pBytePatch->CriticalSection);
}

typedef struct _HOOK_FUNC
{
	void* m_pSrcFunc;
	BYTE* m_pOldInstructions;
	SIZE_T m_length;
	BYTE m_Detour[MINIMUM_HOOK_LENGTH];
	BOOLEAN m_bHooked;
} HOOK_FUNC, * PHOOK_FUNC;


extern inline ULONG_PTR ReadPtr(ULONG_PTR src, SIZE_T uOffset);

inline ULONG_PTR ReadPtr(ULONG_PTR src, SIZE_T uOffset);

static SIZE_T PatternSize(const char* szPattern);
static void PatternCvt(const char* szPattern, BYTE* const& pPattern);

// TODO: revise this hooking method
extern ULONG_PTR FindPattern(const char* szModuleName, const char* szPattern);
extern ULONG_PTR FindPatternEx(HMODULE hModule, const char* szPattern);
extern ULONG_PTR FindPatternAVXEx(HMODULE hModule, const char* szPattern);

extern ULONG_PTR Search(const BYTE* pData, ULONG_PTR uSize, const char* szPattern);

extern ULONG_PTR FindPatternPtr(const char* szModuleName, const char* szPattern);
extern ULONG_PTR FindPatternPtr(const char* szModuleName, const char* szPattern, UINT uOffset);
extern ULONG_PTR FindPatternPtrEx(HMODULE hModule, const char* szPattern, UINT uOffset);

extern VOID PatchBytes(PBYTE_PATCH_MEMORY pBytePatch);
extern VOID NopMemory(PNOP_MEMORY pNop);

extern VOID RestoreMemory(CONST VOID* pMem);

// FIXME: This hook works but is digusting
// hmm it seems yo do have to pause the threads to do this
// if you get really unlucky you can crash
// or find someway to syncrhronize it
// crashed on hooking the save io stuff
// maybe we should have different shellcodes that use other registers
extern BOOL HookFunc64(LPVOID pSrcFunc, LPVOID pDstFunc, SIZE_T cbLength, HOOK_FUNC* pHook);
extern BOOL UnhookFunc(HOOK_FUNC* pHook);

enum _ISBADPTR_STATUS
{
	VALID_PTR = 0x0,
	VIRTUAL_QUERY_FAILED = 0x1,
	MEMORY_FREED = 0x2,
	GUARD_PAGE = 0x4,
	NO_ACCESS_PAGE = 0x8,
	EXECUTE_PAGE = 0x10,
	READ_PAGE = 0x20,
	WRITE_PAGE = 0x40,
	READ_WRITE_PAGE = READ_PAGE | WRITE_PAGE,
	WRITE_COPY_PAGE = 0x80,
};
typedef DWORD ISBADPTR_STATUS;

typedef union SHA256Digest_t
{
	__m128i Oword[2];
	BYTE Value8[32];
} SHA256Digest;
VALIDATE_SIZE(SHA256Digest, 0x20);

// Convenient SHA256Digest String Storage
typedef struct SHA256DigestStr_t
{
	CHAR Hash[(sizeof(SHA256Digest) << 1) + 1];
} SHA256DigestStr;
VALIDATE_SIZE(SHA256DigestStr, 0x41);

extern void SHA256_PrintDigest(const SHA256Digest* pDigest, CHAR* szHash, UINT cbSize);
extern void SHA256_PrintDigest(const SHA256Digest* pDigest, SHA256DigestStr* pStorage);

#if defined(__cplusplus)
template<typename T>
static inline T* ReadPtr(ULONG_PTR src, SIZE_T uOffset)
{
	return src + uOffset + sizeof(LONG) + *(LONG*)(src + uOffset);
}
#endif // __cplusplus

extern ISBADPTR_STATUS IsBadReadPtr(const void* p);
extern ISBADPTR_STATUS IsBadCodePtr(const void* p);

extern NTSTATUS DumpModules(/*PEPROCESS pProcess, PKERNEL_MODULE_REQUEST pRequest*/);

//NTSTATUS KsTraverseModuleList32(PPEB32 pProcessEnvironmentBlock, PVOID pCtx,
//	NTSTATUS(*pCallback)(PLDR_DATA_TABLE_ENTRY32 pEntry, PVOID pCtx))
//{
//	NTSTATUS Status = STATUS_SUCCESS;
//
//	if (!pProcessEnvironmentBlock)
//		return STATUS_INVALID_PARAMETER_1;
//
//	if (!pCallback)
//		return STATUS_INVALID_PARAMETER_2;
//
//	PLIST_ENTRY32 pHead = &((PPEB_LDR_DATA32)pProcessEnvironmentBlock->Ldr)->InMemoryOrderModuleList; // Get the address of the doubly linked-list head
//	PLIST_ENTRY32 pNext = (PLIST_ENTRY32)pHead->Flink; // Get the first entry
//
//	// Traverse the doubly linked-list and call the callee supplied callback
//	while (pNext && pNext != pHead)
//	{
//		Status = pCallback(CONTAINING_RECORD(pNext, LDR_DATA_TABLE_ENTRY32, InMemoryOrderLinks), pCtx);
//
//		// If the callback is successful; stop traversing
//		// If there are more entries; keep traversing
//		if (Status != STATUS_MORE_ENTRIES && NT_SUCCESS(Status))
//			break;
//
//		pNext = (PLIST_ENTRY32)pNext->Flink;
//	}
//
//	return Status;
//}

#endif  // !__MEMORY_H__