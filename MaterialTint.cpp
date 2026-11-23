#include "MaterialTint.h"

#include <string.h>

#include <Fluorine/Memory.h>

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

	// CConstantBuffer, from CConstantBufferHolder_BeginUpdate (0x1402469D0) and its commit
	// (0x140246830) in build 26534214: a ring of CPU-side slots that the render thread
	// uploads from. The commit records the slot it published, and the engine compares the
	// next update against that slot to skip unchanged ones, so it keeps the last colour.
	constexpr size_t CB_SLOTS = 0x28;			// BYTE*, the first slot
	constexpr size_t CB_SLOT_STRIDE = 0x30;		// size_t
	constexpr size_t CB_SLOT_COUNT = 0x48;		// int
	constexpr size_t CB_CREATED = 0x4C;			// int, 0 until the buffer exists
	constexpr size_t CB_LAST_SLOT = 0x58;		// int, the slot committed last, -1 before the first

	// Saved material tints (Appearance.h) are applied to every model of that name, so a
	// tinted material on a model with several copies around takes several slots
	constexpr int MAX_TINTS = 64;

	// The cb4 buffers one material is drawn with. CModelWork_PublishLodWeights hands out one
	// per run of meshes with the same colour, per draw slot (twice over when the work is
	// double-buffered), so a buffer can stand for several meshes. Draws past this many fall
	// back to the plain tint.
	constexpr int MAX_SOURCES = 16;

	struct Source
	{
		// The engine's cb4 buffer these draws had; claimed by the detour, which also keeps
		// m_vMeshColor up to date. m_Buffer holds m_vMeshColor x the tint once m_bReady.
		const CSamplerParam* volatile m_pMeshBuffer;
		ImVec4 m_vMeshColor;
		volatile LONG m_bHaveMeshColor;	// set after the first m_vMeshColor, so Update never uploads a claim's empty colour
		CSamplerParam m_Buffer;			// ours; kept for the session, since queued draws may still use it
		ImVec4 m_vUploaded;
		volatile LONG m_bReady;
	};

	struct Tint
	{
		// Read by the draw threads through the detour; written only by Update/Set/Clear.
		// m_pShader is the key and is published last, after the plain buffer holds a colour.
		const CModelShader* volatile m_pShader;
		CSamplerParam m_Buffer;			// the plain tint, for draws past MAX_SOURCES
		const CModelShader* m_pPending;	// the material this slot is being set up for
		ImVec4 m_vColor;
		float m_flIntensity;
		bool m_bDirty;					// m_Buffer needs the new colour
		Source m_Sources[MAX_SOURCES];
	};

	// The colour the engine last uploaded into a cb4 buffer: the mesh's CMeshPart::m_vColor,
	// which CModelWork_Prepare copies to m_vColorReadOnly when m_bUpdate is set and
	// CModelWork_PublishLodWeights uploads with its alpha times the LOD fade - so Rainbow Hair,
	// the mesh colour picker and first person's alpha. Only called while the engine is
	// building a draw with this buffer, so it is alive.
	ImVec4 ReadMeshColor(const CSamplerParam* pHandle)
	{
		const ImVec4 vWhite(1.0f, 1.0f, 1.0f, 1.0f);
		const BYTE* pBuffer = pHandle ? (const BYTE*)pHandle->m_pBuffer : NULL;

		if (!pBuffer || !*(const int*)(pBuffer + CB_CREATED))
			return vWhite;

		const BYTE* pSlots = *(const BYTE* const*)(pBuffer + CB_SLOTS);
		const int iSlot = *(const int*)(pBuffer + CB_LAST_SLOT);

		if (!pSlots || iSlot < 0 || iSlot >= *(const int*)(pBuffer + CB_SLOT_COUNT))
			return vWhite;

		ImVec4 vColor;
		memcpy(&vColor, pSlots + iSlot * *(const size_t*)(pBuffer + CB_SLOT_STRIDE), sizeof(vColor));

		return vColor;
	}

	// The source standing in for pMeshBuffer, claiming a free one the first time. Several
	// draw threads may get here at once; at worst one buffer ends up with two sources.
	Source* FindSource(Tint& Entry, const CSamplerParam* pMeshBuffer)
	{
		for (Source& Src : Entry.m_Sources)
		{
			if (Src.m_pMeshBuffer == pMeshBuffer)
				return &Src;
		}

		for (Source& Src : Entry.m_Sources)
		{
			if (!Src.m_pMeshBuffer &&
				!InterlockedCompareExchangePointer((PVOID volatile*)&Src.m_pMeshBuffer, (PVOID)pMeshBuffer, NULL))
				return &Src;
		}

		return NULL;
	}

	// Uploads vColor into a buffer of ours, creating it on first use. False if the engine
	// refused this frame (an earlier colour is still waiting to be uploaded).
	bool Upload(CSamplerParam& Buffer, const ImVec4& vColor)
	{
		if (!Buffer.m_pBuffer && !s_Acquire(NULL, &Buffer, sizeof(ImVec4)))
			return false;

		void* pData = NULL;

		if (!s_BeginUpdate(&Buffer, &pData, sizeof(ImVec4)) || !pData)
			return false;

		memcpy(pData, &vColor, sizeof(vColor));
		s_EndUpdate(&Buffer, 0);

		return true;
	}

	Tint s_Tints[MAX_TINTS];
	volatile LONG s_nActive;

	// Fluorine's HookFunc64 in trampoline mode: a jump over the prologue, which it only patches
	// when the bytes match FIND_MODEL_ENTRY_DATA_PROLOGUE, and a trampoline to the original.
	HOOK_FUNC s_Hook;
	FindModelEntryDataFn s_oFindModelEntryData;

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
				// Stack with the mesh colour rather than replace it: swap in a buffer holding
				// this draw's mesh colour x the tint. Until Update has filled one for this
				// mesh (a frame), the draw keeps the engine's buffer, untinted.
				CSamplerParam** ppBuffer = (CSamplerParam**)(pEntry + ENTRY_MESH_COLOR);
				Source* pSource = FindSource(s_Tints[i], *ppBuffer);

				if (!pSource)
				{
					*ppBuffer = &s_Tints[i].m_Buffer;
					break;
				}

				pSource->m_vMeshColor = ReadMeshColor(*ppBuffer);
				MemoryBarrier();
				pSource->m_bHaveMeshColor = 1;

				if (pSource->m_bReady)
					*ppBuffer = &pSource->m_Buffer;

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

	if (!HookFunc64(pFindModelEntryData, (LPVOID)&hkFindModelEntryData, sizeof(FIND_MODEL_ENTRY_DATA_PROLOGUE),
		&s_Hook, FIND_MODEL_ENTRY_DATA_PROLOGUE, (LPVOID*)&s_oFindModelEntryData))
	{
		WARN("MaterialTint: FindModelEntryData doesn't start with the expected bytes, material tint is disabled\n");
		return false;
	}

	return true;
}

void MaterialTint::Shutdown(void)
{
	s_nActive = 0;
	UnhookFunc(&s_Hook);
}

bool MaterialTint::IsAvailable(void)
{
	return s_Hook.m_bHooked != FALSE;
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

		// The slot is unpublished, so the detour no longer touches its sources. Their
		// buffers are kept for reuse.
		for (Source& Src : pTint->m_Sources)
		{
			Src.m_bReady = 0;
			Src.m_bHaveMeshColor = 0;
			Src.m_pMeshBuffer = NULL;
		}

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

		if (!Entry.m_pPending)
			continue;

		// cb4 is float4 g_MeshColor
		const ImVec4 vTint(Entry.m_vColor.x * Entry.m_flIntensity, Entry.m_vColor.y * Entry.m_flIntensity,
			Entry.m_vColor.z * Entry.m_flIntensity, 1.0f);

		// The plain tint, for draws past MAX_SOURCES. Alpha 1: the shaders discard (dither
		// out) below it. Refused while the previous colour is still waiting to be uploaded;
		// try next frame.
		if (Entry.m_bDirty && Upload(Entry.m_Buffer, vTint))
		{
			Entry.m_bDirty = false;

			// The buffer holds a colour now, so the detour may start using it
			MemoryBarrier();
			Entry.m_pShader = Entry.m_pPending;
		}

		// The stacked colours: each mesh's own colour (and alpha, which first person and
		// fades use) times the tint, re-uploaded whenever either changes
		for (Source& Src : Entry.m_Sources)
		{
			if (!Src.m_pMeshBuffer || !Src.m_bHaveMeshColor)
				continue;

			MemoryBarrier();

			const ImVec4 vMesh = Src.m_vMeshColor;
			const ImVec4 vColor(vMesh.x * vTint.x, vMesh.y * vTint.y, vMesh.z * vTint.z, vMesh.w);

			if (Src.m_bReady && !memcmp(&vColor, &Src.m_vUploaded, sizeof(vColor)))
				continue;

			if (!Upload(Src.m_Buffer, vColor))
				continue;

			Src.m_vUploaded = vColor;

			MemoryBarrier();
			Src.m_bReady = 1;
		}
	}
}
