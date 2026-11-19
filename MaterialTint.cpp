#include "MaterialTint.h"

#include <string.h>

#include <Fluorine\Memory.h>

#include "Log.h"
#include "SDK/Graphics/CMaterial.h"

namespace
{
	// CModelWork::FindModelEntryData(CModelWork*, const CModelDrawSlotKey*, const CModelDrawBatchKey*).
	// Its first 18 bytes - mov [rsp+18h], rbx / push rbp / push rsi / push rdi / sub rsp, 20h /
	// mov rsi, rcx / mov rdi, r8 - are whole, position-independent instructions, so they can be
	// moved into a trampoline as they are. The same bytes in v1.02 and build 26534214.
	const char* const FIND_MODEL_ENTRY_DATA_SIG =
		"48 89 5C 24 18 55 56 57 48 83 EC 20 48 8B F1 49 8B F8 48 8B 0D ? ? ? ? 48 8B EA 48 85 C9 "
		"0F 84 ? ? ? ? 41 83 78 30 00 0F 8E ? ? ? ? BA 80 00 00 00 44 8D 42 90";

	const BYTE FIND_MODEL_ENTRY_DATA_PROLOGUE[] =
	{
		0x48, 0x89, 0x5C, 0x24, 0x18, 0x55, 0x56, 0x57, 0x48, 0x83, 0xEC, 0x20, 0x48, 0x8B, 0xF1, 0x49, 0x8B, 0xF8
	};

	// CConstantBufferHolder_Acquire(unused, CGraphicObjectHandle*, size): creates a constant
	// buffer the size given and stores it in the handle.
	const char* const ACQUIRE_SIG =
		"48 89 5C 24 08 48 89 74 24 10 57 48 83 EC 30 48 8B DA 49 8B F8 48 8B 12 48 85 D2 74 0C "
		"48 8D 0D ? ? ? ? E8 ? ? ? ? 33 F6 48 89 33 89 73";

	// CConstantBufferHolder_BeginUpdate(handle, void** ppData, size): hands out the buffer's
	// current ring slot. Refuses while an earlier update is waiting to be uploaded.
	const char* const BEGIN_UPDATE_SIG =
		"4C 8B 09 4D 85 C9 74 41 4D 85 C0 74 3C 49 8D 48 0F 48 83 E1 F0 41 83 79 4C 00 74 2D "
		"49 83 79 40 00 77 26 49 83 79 38 00";

	// EndUpdate(handle): jumps to CConstantBuffer's commit with the caller's edx as its
	// "force" flag, so it is called with 0 there.
	const char* const END_UPDATE_SIG = "48 8B 09 48 85 C9 0F 85 04 FE FF FF C3";

	// CModelEntryData fields written by FindModelEntryData
	constexpr size_t ENTRY_MODEL_SHADER = 0x00;	// &pWork->m_pModelShaders[iShaderIndex]
	constexpr size_t ENTRY_MESH_COLOR = 0x30;	// &pWork->m_pSamplerParams[iSamplerIndex], cb4's buffer

	typedef BYTE* (__fastcall* FindModelEntryDataFn)(void* pWork, const void* pSlotKey, const void* pBatchKey);
	typedef BOOL(__fastcall* AcquireFn)(void* pUnused, CSamplerParam* pHandle, size_t cbSize);
	typedef BOOL(__fastcall* BeginUpdateFn)(CSamplerParam* pHandle, void** ppData, size_t cbSize);
	typedef void(__fastcall* EndUpdateFn)(CSamplerParam* pHandle, int bForce);

	AcquireFn s_Acquire;
	BeginUpdateFn s_BeginUpdate;
	EndUpdateFn s_EndUpdate;

	constexpr int MAX_TINTS = 32;

	struct Tint
	{
		// Read by the draw threads through the detour; written only by Update/Set/Clear.
		// m_pShader is the key and is published last, after the buffer holds a colour.
		const CModelShader* volatile m_pShader;
		CSamplerParam m_Buffer;			// ours; kept for the session, since queued draws may still use it
		const CModelShader* m_pPending;	// the material this slot is being set up for
		ImVec4 m_vColor;
		float m_flIntensity;
		bool m_bDirty;
	};

	Tint s_Tints[MAX_TINTS];
	volatile LONG s_nActive;

	// A 14-byte jmp [rip+0] / dq target, written over the first bytes of a function, and a
	// trampoline holding the bytes it replaced followed by a jump back.
	struct InlineDetour
	{
		BYTE* m_pTarget;
		BYTE m_Saved[32];
		size_t m_cbPrologue;
		BYTE* m_pTrampoline;
	};

	InlineDetour s_Detour;
	FindModelEntryDataFn s_oFindModelEntryData;

	void WriteAbsoluteJump(BYTE* pAt, const void* pTo)
	{
		// jmp qword ptr [rip+0]
		pAt[0] = 0xFF;
		pAt[1] = 0x25;
		*(DWORD*)&pAt[2] = 0;
		*(const void**)&pAt[6] = pTo;
	}

	// *ppOriginal gets the trampoline before the target is patched, so the hook can never run
	// with it unset.
	bool InstallDetour(InlineDetour& Detour, BYTE* pTarget, const void* pHook, const BYTE* pPrologue, size_t cbPrologue,
		void** ppOriginal)
	{
		constexpr size_t JUMP_SIZE = 14;

		if (!pTarget || cbPrologue < JUMP_SIZE || cbPrologue > sizeof(Detour.m_Saved))
			return false;

		// Only patch the exact bytes the trampoline is built for
		if (memcmp(pTarget, pPrologue, cbPrologue))
			return false;

		BYTE* pTrampoline = (BYTE*)VirtualAlloc(NULL, 64, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

		if (!pTrampoline)
			return false;

		memcpy(pTrampoline, pTarget, cbPrologue);
		WriteAbsoluteJump(pTrampoline + cbPrologue, pTarget + cbPrologue);
		FlushInstructionCache(GetCurrentProcess(), pTrampoline, 64);

		BYTE Patch[sizeof(Detour.m_Saved)];

		WriteAbsoluteJump(Patch, pHook);
		memset(Patch + JUMP_SIZE, 0xCC, cbPrologue - JUMP_SIZE);

		DWORD dwOldProtect;

		if (!VirtualProtect(pTarget, cbPrologue, PAGE_EXECUTE_READWRITE, &dwOldProtect))
		{
			VirtualFree(pTrampoline, 0, MEM_RELEASE);
			return false;
		}

		memcpy(Detour.m_Saved, pTarget, cbPrologue);

		*ppOriginal = pTrampoline;
		MemoryBarrier();

		// Not atomic, like the other inline hooks here: a thread inside these 18 bytes at this
		// exact moment would fault. The window is a few instructions long.
		memcpy(pTarget, Patch, cbPrologue);

		VirtualProtect(pTarget, cbPrologue, dwOldProtect, &dwOldProtect);
		FlushInstructionCache(GetCurrentProcess(), pTarget, cbPrologue);

		Detour.m_pTarget = pTarget;
		Detour.m_cbPrologue = cbPrologue;
		Detour.m_pTrampoline = pTrampoline;

		return true;
	}

	void RemoveDetour(InlineDetour& Detour)
	{
		if (!Detour.m_pTarget)
			return;

		DWORD dwOldProtect;

		if (VirtualProtect(Detour.m_pTarget, Detour.m_cbPrologue, PAGE_EXECUTE_READWRITE, &dwOldProtect))
		{
			memcpy(Detour.m_pTarget, Detour.m_Saved, Detour.m_cbPrologue);
			VirtualProtect(Detour.m_pTarget, Detour.m_cbPrologue, dwOldProtect, &dwOldProtect);
			FlushInstructionCache(GetCurrentProcess(), Detour.m_pTarget, Detour.m_cbPrologue);
		}

		// The trampoline is left allocated: a draw thread may still be running through it
		Detour.m_pTarget = NULL;
	}

	BYTE* __fastcall hkFindModelEntryData(void* pWork, const void* pSlotKey, const void* pBatchKey)
	{
		BYTE* pEntry = s_oFindModelEntryData(pWork, pSlotKey, pBatchKey);

		if (!pEntry || !s_nActive)
			return pEntry;

		const CModelShader* pShader = *(const CModelShader**)(pEntry + ENTRY_MODEL_SHADER);

		// Free slots hold NULL
		if (!pShader)
			return pEntry;

		for (int i = 0; i < MAX_TINTS; ++i)
		{
			if (s_Tints[i].m_pShader == pShader)
			{
				*(CSamplerParam**)(pEntry + ENTRY_MESH_COLOR) = &s_Tints[i].m_Buffer;
				break;
			}
		}

		return pEntry;
	}

	Tint* FindTint(const CModelShader* pShader)
	{
		for (int i = 0; i < MAX_TINTS; ++i)
		{
			if (s_Tints[i].m_pPending == pShader)
				return &s_Tints[i];
		}

		return NULL;
	}

	ULONG_PTR Find(const char* szSignature)
	{
		ULONG_PTR uAddress = FindPattern(NULL, szSignature);

		return (uAddress == (ULONG_PTR)-1) ? 0 : uAddress;
	}
}

bool MaterialTint::Init(void)
{
	BYTE* pFindModelEntryData = (BYTE*)Find(FIND_MODEL_ENTRY_DATA_SIG);

	s_Acquire = (AcquireFn)Find(ACQUIRE_SIG);
	s_BeginUpdate = (BeginUpdateFn)Find(BEGIN_UPDATE_SIG);
	s_EndUpdate = (EndUpdateFn)Find(END_UPDATE_SIG);

	LOG("MaterialTint: FindModelEntryData %p, Acquire %p, BeginUpdate %p, EndUpdate %p\n",
		pFindModelEntryData, s_Acquire, s_BeginUpdate, s_EndUpdate);

	if (!pFindModelEntryData || !s_Acquire || !s_BeginUpdate || !s_EndUpdate)
	{
		WARN("MaterialTint: a signature is missing on this build, material tint is disabled\n");
		return false;
	}

	InlineDetour Detour = {};

	if (!InstallDetour(Detour, pFindModelEntryData, (const void*)&hkFindModelEntryData,
		FIND_MODEL_ENTRY_DATA_PROLOGUE, sizeof(FIND_MODEL_ENTRY_DATA_PROLOGUE), (void**)&s_oFindModelEntryData))
	{
		WARN("MaterialTint: FindModelEntryData doesn't start with the expected bytes, material tint is disabled\n");
		return false;
	}

	s_Detour = Detour;

	return true;
}

void MaterialTint::Shutdown(void)
{
	s_nActive = 0;
	RemoveDetour(s_Detour);
}

bool MaterialTint::IsAvailable(void)
{
	return s_Detour.m_pTarget != NULL;
}

bool MaterialTint::Get(const CModelShader* pShader, ImVec4& vColor, float& flIntensity)
{
	Tint* pTint = FindTint(pShader);

	if (!pTint)
		return false;

	vColor = pTint->m_vColor;
	flIntensity = pTint->m_flIntensity;

	return true;
}

bool MaterialTint::Set(const CModelShader* pShader, const ImVec4& vColor, float flIntensity)
{
	if (!IsAvailable() || !pShader)
		return false;

	Tint* pTint = FindTint(pShader);

	if (!pTint)
	{
		// A free slot, reusing its buffer if it already has one
		pTint = FindTint(NULL);

		if (!pTint)
			return false;

		pTint->m_pPending = pShader;
		InterlockedIncrement(&s_nActive);
	}

	pTint->m_vColor = vColor;
	pTint->m_flIntensity = flIntensity;
	pTint->m_bDirty = true;

	return true;
}

void MaterialTint::Clear(const CModelShader* pShader)
{
	Tint* pTint = pShader ? FindTint(pShader) : NULL;

	if (!pTint)
		return;

	// Unpublish first, so the detour stops handing out the buffer
	pTint->m_pShader = NULL;
	MemoryBarrier();

	pTint->m_pPending = NULL;
	pTint->m_bDirty = false;
	InterlockedDecrement(&s_nActive);
}

void MaterialTint::ClearAll(void)
{
	for (int i = 0; i < MAX_TINTS; ++i)
	{
		if (s_Tints[i].m_pPending)
			Clear(s_Tints[i].m_pPending);
	}
}

void MaterialTint::Update(void)
{
	if (!IsAvailable() || !s_nActive)
		return;

	for (int i = 0; i < MAX_TINTS; ++i)
	{
		Tint& Entry = s_Tints[i];

		if (!Entry.m_pPending || !Entry.m_bDirty)
			continue;

		if (!Entry.m_Buffer.m_pBuffer && !s_Acquire(NULL, &Entry.m_Buffer, sizeof(ImVec4)))
			continue;

		void* pData = NULL;

		// Refused while the previous colour is still waiting to be uploaded; try next frame
		if (!s_BeginUpdate(&Entry.m_Buffer, &pData, sizeof(ImVec4)) || !pData)
			continue;

		// cb4 is float4 g_MeshColor. Alpha stays 1: the shaders discard (dither out) below it.
		const ImVec4 vColor(Entry.m_vColor.x * Entry.m_flIntensity, Entry.m_vColor.y * Entry.m_flIntensity,
			Entry.m_vColor.z * Entry.m_flIntensity, 1.0f);

		memcpy(pData, &vColor, sizeof(vColor));
		s_EndUpdate(&Entry.m_Buffer, 0);

		Entry.m_bDirty = false;

		// The buffer holds a colour now, so the detour may start using it
		MemoryBarrier();
		Entry.m_pShader = Entry.m_pPending;
	}
}
