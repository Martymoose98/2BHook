#include "mot.h"


void ReadMot(void* pMot, std::vector<MotInterpListEntry>& Interps)
{
	MotHeader* pHdr = (MotHeader*)pMot;

	if (!pHdr || pHdr->magic != MOT_MAGIC)
		return;

	MotRecord* pRecords = MakePtr(MotRecord*, pMot, pHdr->uRecordOffset);

	union
	{
		MotInterpolation* pInterp;
		MotInterpolation2* pInterp2;
		MotInterpolation3* pInterp3;
		MotInterpolation4* pInterp4;
		MotInterpolation5* pInterp5;
		MotInterpolation6* pInterp6;
		MotInterpolation7* pInterp7;
		MotInterpolation8* pInterp8;
	};

	union
	{
		MotInterpolationInternal* pInterpInternal;
		MotInterpolation2Internal* pInterp2Internal;
		MotInterpolation3Internal* pInterp3Internal;
		MotInterpolation4Internal* pInterp4Internal;
		MotInterpolation5Internal* pInterp5Internal;
		MotInterpolation6Internal* pInterp6Internal;
		MotInterpolation7Internal* pInterp7Internal;
		MotInterpolation8Internal* pInterp8Internal;
	};

	MotInterpListEntry entry;

	for (UINT i = 0; i < pHdr->uRecordCount; ++i)
	{
		// Every curve keeps its record: ProcessMot samples it through MotRecord_Sample
		entry.pRecord = &pRecords[i];

		switch (pRecords[i].flags) // // 0: constant value for each frame. The value is in records[i].value.flt. -1: only found on terminator (bone index 0x7fff).
		{
		case -1:
			continue;
		case 0:	// the value is inline in the record
		case 9:	// always 0 (the game's ConstantZeroFloat)
			entry.type = MotInterpListEntry::NONE;
			entry.pInterp = NULL;
			Interps.push_back(entry);
			break;
		case 1:
			pInterp = MakePtr(MotInterpolation*, &pRecords[i], pRecords[i].offset);
			pInterpInternal = (MotInterpolationInternal*)malloc(sizeof(MotInterpolationInternal));
			pInterpInternal->m_sValuesCount = pRecords[i].elemNumber;
			pInterpInternal->m_pflValues = pInterp->flValues;
			entry.type = MotInterpListEntry::INTERP;
			entry.pInterp = pInterpInternal;
			Interps.push_back(entry);
			break;
		case 2:
			pInterp2 = MakePtr(MotInterpolation2*, &pRecords[i], pRecords[i].offset);
			pInterp2Internal = (MotInterpolation2Internal*)malloc(sizeof(MotInterpolation2Internal));
			pInterp2Internal->m_scpCount = pRecords[i].elemNumber;
			pInterp2Internal->m_pcp = pInterp2->cp;
			pInterp2Internal->m_Values = pInterp2->values;
			entry.type = MotInterpListEntry::INTERP2;
			entry.pInterp2 = pInterp2Internal;
			Interps.push_back(entry);
			break;
		case 3: // _mm_cvtneeph_ps 
			pInterp3 = MakePtr(MotInterpolation3*, &pRecords[i], pRecords[i].offset);
			pInterp3Internal = (MotInterpolation3Internal*)malloc(sizeof(MotInterpolation3Internal));
			pInterp3Internal->m_scpCount = pRecords[i].elemNumber;
			pInterp3Internal->m_pcp = pInterp3->cp;
			pInterp3Internal->m_Values = pInterp3->values;
			entry.type = MotInterpListEntry::INTERP3;
			entry.pInterp3 = pInterp3Internal;
			Interps.push_back(entry);
			break;
		case 4:
			pInterp4 = MakePtr(MotInterpolation4*, &pRecords[i], pRecords[i].offset);
			pInterp4Internal = (MotInterpolation4Internal*)malloc(sizeof(MotInterpolation4Internal));
			pInterp4Internal->m_sKeyCount = pRecords[i].elemNumber;
			pInterp4Internal->m_pKeys = pInterp4->keys;
			entry.type = MotInterpListEntry::INTERP4;
			entry.pInterp4 = pInterp4Internal;
			Interps.push_back(entry);
			break;
		case 5:
			pInterp5 = MakePtr(MotInterpolation5*, &pRecords[i], pRecords[i].offset);
			pInterp5Internal = (MotInterpolation5Internal*)malloc(sizeof(MotInterpolation5Internal));
			pInterp5Internal->m_values = pInterp5->values;
			pInterp5Internal->m_sKeyCount = pRecords[i].elemNumber;
			pInterp5Internal->m_pKeys = pInterp5->keys;
			entry.type = MotInterpListEntry::INTERP5;
			entry.pInterp5 = pInterp5Internal;
			Interps.push_back(entry);
			break;
		case 6: // _mm_cvtneeph_ps 
			pInterp6 = MakePtr(MotInterpolation6*, &pRecords[i], pRecords[i].offset);
			pInterp6Internal = (MotInterpolation6Internal*)malloc(sizeof(MotInterpolation6Internal));
			pInterp6Internal->m_values = pInterp6->values;
			pInterp6Internal->m_sKeyCount = pRecords[i].elemNumber;
			pInterp6Internal->m_pKeys = pInterp6->keys;
			entry.type = MotInterpListEntry::INTERP6;
			entry.pInterp6 = pInterp6Internal;
			Interps.push_back(entry);
			break;
		case 7: //_mm_cvtneeph_ps 
			pInterp7 = MakePtr(MotInterpolation7*, &pRecords[i], pRecords[i].offset);
			pInterp7Internal = (MotInterpolation7Internal*)malloc(sizeof(MotInterpolation7Internal));
			pInterp7Internal->m_values = pInterp7->values;
			pInterp7Internal->m_sKeyCount = pRecords[i].elemNumber;
			pInterp7Internal->m_pKeys = pInterp7->keys;
			entry.type = MotInterpListEntry::INTERP7;
			entry.pInterp7 = pInterp7Internal;
			Interps.push_back(entry);
			break;
		case 8: //_mm_cvtneeph_ps 
			pInterp8 = MakePtr(MotInterpolation8*, &pRecords[i], pRecords[i].offset);
			pInterp8Internal = (MotInterpolation8Internal*)malloc(sizeof(MotInterpolation8Internal));
			pInterp8Internal->m_values = pInterp8->values;
			pInterp8Internal->m_sKeyCount = pRecords[i].elemNumber;
			pInterp8Internal->m_pKeys = pInterp8->keys;
			entry.type = MotInterpListEntry::INTERP8;
			entry.pInterp8 = pInterp8Internal;
			Interps.push_back(entry);
			break;
		default:
			__debugbreak();
			break;
		}
	}
}


void InitMotion(CModelData* pData, short sFrameCount)
{
	Matrix4x4* pBoneModelMatrices = (Matrix4x4*)calloc(sFrameCount * (pData->m_nBones + 1), sizeof(Matrix4x4));
	[[maybe_unused]] Vector3* pvCuMulScale = (Vector3*)calloc(sFrameCount * (pData->m_nBones + 1), sizeof(Vector3));

	for (int i = 0; i < pData->m_nBones; ++i)
	{
		CBone* pBone = &pData->m_pBones[i];

		for (int j = 0; j < sFrameCount; ++j)
		{
			// NOTE: Make sure the matrices are indentity matrices if not set
			// Bone id -1 is sent to indentity
			pBoneModelMatrices[j * pData->m_nBones + i].InitTransform(pBone->m_vLocalRotation, pBone->m_vLocalScale, pBone->m_vLocalPosition);
		}
	}
}

void ApplyMotion(CModelData* pData)
{
	UNREFERENCED_PARAMETER(pData);

}

//https://github.com/Kerilk/noesis_bayonetta_pc/blob/master/bayonetta_pc/MotionBayo.h
// Bakes every curve ReadMot found: its value at each of frames 0 .. sFrameCount - 1, sampled the
// way the game samples it (MotRecord_Sample). The format-3 guess that was here read the head as
// IEEE halves; the game packs them its own way (Mot_DecodePacked16).
void ProcessMot(std::vector<MotInterpListEntry>&Interps, CModelWork * pWork, short sFrameCount)
{
	UNREFERENCED_PARAMETER(pWork);

	const int nFrames = (sFrameCount > 0) ? sFrameCount : 0;

	for (auto& it : Interps)
	{
		it.flBaked.resize(nFrames);

		for (int iFrame = 0; iFrame < nFrames; ++iFrame)
			it.flBaked[iFrame] = it.pRecord ? MotRecord_Sample(it.pRecord, (float)iFrame) : 0.0f;
	}
}

void FreeMot(std::vector<MotInterpListEntry>& Interps)
{
	// The internals ReadMot allocated; what they point at is the file's
	for (auto& it : Interps)
		free(it.pInterp);

	Interps.clear();
}

float HalfFloatToFloat(hfloat value)
{
	//_mm_cvtsbh_ss(value); // idk if this will work bh is not hfloat
	//_cvtsh_ss();
	//_mm_cvtsh_ss(); // avx512
	//_mm_set1_ph();
	__m128h mmhfloat = _mm_set1_epi16(value);
	__m128 mmfloat = _mm_cvtph_ps(mmhfloat);
	return _mm_cvtss_f32(mmfloat);
}

template<typename T>
static void read_big_endian(T* pLE, T* pBE)
{
	for (size_t i = 0; i < sizeof(T); ++i)
		((byte*)pBE)[sizeof(T) - i - 1] = ((byte*)pLE)[i];
}

template<typename T>
static T read_big_endian(T v)
{
	T bev;

	for (size_t i = 0; i < sizeof(T); ++i)
		((byte*)&bev)[sizeof(T) - i - 1] = ((byte*)&v)[i];

	return bev;
}

[[maybe_unused]] static double Readpghalf(hfloat value)
{
	int32 sexponent;
	double f = 0.0;
	uint32 sign = value & 0x8000;
	uint32 ui = value ^ sign;
	uint32 exponent = ui & 0x7E00;
	uint32 significand = ui ^ exponent;
	int bit = 1 << 8;

	for (int i = 1; i <= 9; i++)
	{
		if (bit & significand)
			f += pow(2.0, -i);

		bit >>= 1;
	}

	if (exponent == 0x7E00)
	{
		if (significand)
		{
			// SPrintf( s, "NaN" );
			return NAN;
		}
		else if (sign)
		{
			//SPrintf( s, "-Infinity" );
			return -INFINITY;
		}
		else
		{
			// SPrintf( s, "+Infinity" );
			return INFINITY;
		}
	}
	else if (exponent != 0)
	{
		exponent >>= 9;
		sexponent = exponent;
		sexponent -= 47;
		f += 1.0;
		f *= pow(2.0, sexponent);

		if (sign)
			f *= -1.0;

		//SPrintf( s, "%e", f );
	}
	else
	{
		if (significand) //denorm
		{
			if (sign)
				f *= -1;

			f *= pow(2.0, -46);
			//SPrintf( s, "%e", f );
		}
		else if (sign)
		{
			// SPrintf( s, "-0.0" );
			f = -0.0;
		}
		else
		{
			//  SPrintf( s, "0.0" );
			f = 0.0;
		}
	}
	return f;
}

static uint32_t halfbits_to_floatbits(hfloat h)
{
	uint16_t h_exp, h_sig;
	uint32_t f_sgn, f_exp, f_sig;

	h_exp = (h & 0x7c00u);
	f_sgn = ((uint32_t)h & 0x8000u) << 16;
	switch (h_exp) {
	case 0x0000u: /* 0 or subnormal */
		h_sig = (h & 0x03ffu);
		/* Signed zero */
		if (h_sig == 0) {
			return f_sgn;
		}
		/* Subnormal */
		h_sig <<= 1;
		while ((h_sig & 0x0400u) == 0) {
			h_sig <<= 1;
			h_exp++;
		}
		f_exp = ((uint32_t)(127 - 15 - h_exp)) << 23;
		f_sig = ((uint32_t)(h_sig & 0x03ffu)) << 13;
		return f_sgn + f_exp + f_sig;
	case 0x7c00u: /* inf or NaN */
		/* All-ones exponent and a copy of the significand */
		return f_sgn + 0x7f800000u + (((uint32_t)(h & 0x03ffu)) << 13);
	default: /* normalized */
		/* Just need to adjust the exponent and shift */
		return f_sgn + (((uint32_t)(h & 0x7fffu) + 0x1c000u) << 13);
	}
}

[[maybe_unused]] static float half_to_float(hfloat h)
{
	return (float)halfbits_to_floatbits(h);
}

[[maybe_unused]] static hfloat make_half_float(float f)
{

	union {
		float fv;
		uint32_t ui;
	} ci;
	ci.fv = f;

	uint32_t x = ci.ui;
	uint32_t sign = (unsigned short)(x >> 31);
	uint32_t mantissa;
	uint32_t exp;
	uint16_t hf;

	// get mantissa
	mantissa = x & ((1 << 23) - 1);
	// get exponent bits
	exp = x & (0xFF << 23);
	if (exp >= 0x47800000) {
		// check if the original single precision float number is a NaN
		if (mantissa && (exp == (0xFF << 23))) {
			// we have a single precision NaN
			mantissa = (1 << 23) - 1;
		}
		else {
			// 16-bit half-float representation stores number as Inf
			mantissa = 0;
		}
		hf = (((uint16_t)sign) << 15) | (uint16_t)((0x1F << 10)) |
			(uint16_t)(mantissa >> 13);
	}
	// check if exponent is <= -15
	else if (exp <= 0x38000000) {

		/* store a denorm half-float value or zero
			exp = (0x38000000 - exp) >> 23;
			mantissa >>= (14 + exp);
			hf = (((uint16_t)sign) << 15) | (uint16_t)(mantissa);
		*/
		hf = 0; //denormals do not work for 3D, convert to zero
	}
	else {
		hf = (((uint16_t)sign) << 15) |
			(uint16_t)((exp - 0x38000000) >> 13) |
			(uint16_t)(mantissa >> 13);
	}

	return hf;
}

[[maybe_unused]] static void Model_Bayo_HermitInterpolate(float p, float deltap, float m0, float deltam0, float m1, float deltam1)
{
	UNREFERENCED_PARAMETER(p);
	UNREFERENCED_PARAMETER(deltap);
	UNREFERENCED_PARAMETER(m0);
	UNREFERENCED_PARAMETER(deltam0);
	UNREFERENCED_PARAMETER(m1);
	UNREFERENCED_PARAMETER(deltam1);


	//HermiteInterpolate()
}

//HermitInterpolate(float p, float deltap, float m0, float deltam0, float m1, float deltam1)
// Result = (2 * t^3 - 3 * t^2 + 1) * Position0 +
//          (t^3 - 2 * t^2 + t) * Tangent0 +
//          (-2 * t^3 + 3 * t^2) * Position1 +
//          (t^3 - t^2) * Tangent1
[[maybe_unused]] static Vector4 HermiteInterpolate(Vector4& vPosition0, Vector4& vTangent0, Vector4& vPosition1, Vector4& vTangent1, float t)
{
	Vector4 vResult;

	float t2 = t * t;
	float t3 = t2 * t;

	float P0 = 2.0f * t3 - 3.0f * t2 + 1.0f;
	float T0 = t3 - 2.0f * t2 + t;
	float P1 = -2.0f * t3 + 3.0f * t2;
	float T1 = t3 - t2;

	vResult.x = P0 * vPosition0.x + T0 * vTangent0.x + P1 * vPosition1.x + T1 * vTangent1.x;
	vResult.y = P0 * vPosition0.y + T0 * vTangent0.y + P1 * vPosition1.y + T1 * vTangent1.y;
	vResult.z = P0 * vPosition0.z + T0 * vTangent0.z + P1 * vPosition1.z + T1 * vTangent1.z;
	vResult.w = P0 * vPosition0.w + T0 * vTangent0.w + P1 * vPosition1.w + T1 * vTangent1.w;
}

// ---------------------------------------------------------------------------------------------
// MotionPlayer (see mot.h)
// ---------------------------------------------------------------------------------------------

#include <deque>
#include <string>

namespace MotionPlayer
{
	typedef int(*EntityDataWork_PlayMotionFileFn)(CEntityDataWork* pWork, MotHeader* pMot, const char* szName, int hSlot,
		float flBlendIn, float flWeight, unsigned int uFlags, float flStartTime, float flRate);
	typedef void(*BehaviorAppBase_StopMotionFn)(CBehaviorAppBase* pThis, int iSlot, float flBlendOut);
	typedef void(*EntityDataWork_EvaluateMotionFn)(CEntityDataWork* pWork);

	// EntityDataWork_EvaluateMotion's first whole instructions, moved into the trampoline (the same
	// in v1.02 and build 26534214): mov rax, rsp / push rbp / push rbx / lea rbp, [rax-78h] /
	// sub rsp, 168h
	static const BYTE s_EvaluateMotionHead[16] = { 0x48, 0x8B, 0xC4, 0x55, 0x53, 0x48, 0x8D, 0x68, 0x88,
		0x48, 0x81, 0xEC, 0x68, 0x01, 0x00, 0x00 };

	static EntityDataWork_PlayMotionFileFn s_PlayMotionFile;
	static BehaviorAppBase_StopMotionFn s_StopMotion;
	static EntityDataWork_EvaluateMotionFn s_oEvaluateMotion;
	static HOOK_FUNC s_EvaluateMotionHook;
	static bool s_bHooked;

	// Motion files from disk and the names motions were played under: kept until unload, since the
	// game reads a playing motion's file every frame and may keep the name
	static std::deque<std::vector<BYTE>> s_LoadedFiles;
	static std::deque<std::string> s_Names;

	enum RequestType_t { REQUEST_NONE, REQUEST_PLAY, REQUEST_STOP };

	struct Request_t
	{
		RequestType_t m_Type;
		CBehaviorAppBase* m_pEntity;
		CEntityDataWork* m_pWork;		// the entity's, which the hook matches
		MotHeader* m_pMot;
		const char* m_szName;
		int m_iSlot;
		float m_flBlend;
		float m_flRate;
		bool m_bLoop;
	};

	static Request_t s_Request;
	static volatile LONG s_lPending;
	static volatile LONG s_lLastResult = -1;
	static SRWLOCK s_Lock = SRWLOCK_INIT;

	static void hkEvaluateMotion(CEntityDataWork* pWork)
	{
		if (s_lPending)
		{
			Request_t Request = {};

			AcquireSRWLockExclusive(&s_Lock);

			if (s_lPending && s_Request.m_pWork == pWork)
			{
				Request = s_Request;
				s_lPending = 0;
			}

			ReleaseSRWLockExclusive(&s_Lock);

			if (Request.m_Type == REQUEST_PLAY)
			{
				unsigned int uFlags = MOTIONFLAG_NO_SEQUENCES | (Request.m_bLoop ? 0 : MOTIONFLAG_NO_WRAP);

				// Weight 1, and a start time of -1: from the start (or the end, playing backwards)
				int hMotion = s_PlayMotionFile(pWork, Request.m_pMot, Request.m_szName, Request.m_iSlot,
					Request.m_flBlend, 1.0f, uFlags, -1.0f, Request.m_flRate);

				InterlockedExchange(&s_lLastResult, hMotion);
			}
			else if (Request.m_Type == REQUEST_STOP)
			{
				s_StopMotion(Request.m_pEntity, Request.m_iSlot, Request.m_flBlend);
				InterlockedExchange(&s_lLastResult, -1);
			}
		}

		s_oEvaluateMotion(pWork);
	}

	bool Initialize(void)
	{
		// EntityDataWork_PlayMotionFile (0x14019FFD0, v1.02 0x14019FEE0)
		s_PlayMotionFile = (EntityDataWork_PlayMotionFileFn)FindPattern(NULL,
			"40 55 56 57 41 56 48 83 EC 78 48 8B 05 ? ? ? ? 48 33 C4 48 89 44 24 60 45 8B F1 49 8B E8 48 8B F2 48 8B F9 E8");
		// BehaviorAppBase_StopMotion (0x140415950, v1.02 0x1404140F0)
		s_StopMotion = (BehaviorAppBase_StopMotionFn)FindPattern(NULL,
			"48 89 5C 24 08 57 48 83 EC 30 48 8B 99 60 08 00 00 8B FA 0F 29 74 24 20 0F 28 F2 48 85 DB 74 21 48 8B CB E8 ? ? ? ? 48 8D 93 D0 00 00 00");
		// EntityDataWork_EvaluateMotion (0x14019FA30, v1.02 0x14019F940). All three are unique in both builds.
		ULONG_PTR uEvaluateMotion = FindPattern(NULL,
			"48 8B C4 55 53 48 8D 68 88 48 81 EC 68 01 00 00 F6 81 C0 00 00 00 01 48 8B D9");

		if ((ULONG_PTR)s_PlayMotionFile == (ULONG_PTR)-1)
			s_PlayMotionFile = NULL;

		if ((ULONG_PTR)s_StopMotion == (ULONG_PTR)-1)
			s_StopMotion = NULL;

		if (!s_PlayMotionFile || !s_StopMotion || !uEvaluateMotion || uEvaluateMotion == (ULONG_PTR)-1)
			return false;

		ZeroMemory(&s_EvaluateMotionHook, sizeof(s_EvaluateMotionHook));

		s_bHooked = HookFunc64((LPVOID)uEvaluateMotion, (LPVOID)&hkEvaluateMotion, sizeof(s_EvaluateMotionHead),
			&s_EvaluateMotionHook, s_EvaluateMotionHead, (LPVOID*)&s_oEvaluateMotion) != FALSE;

		return s_bHooked;
	}

	void Shutdown(void)
	{
		if (s_bHooked)
		{
			UnhookFunc(&s_EvaluateMotionHook);
			s_bHooked = false;
		}
	}

	bool IsAvailable(void)
	{
		return s_bHooked;
	}

	static void AddMotions(DATHeader* const pPair[2], std::vector<EntityMotion_t>& Motions)
	{
		for (int iDatFile = DATFILE_DATA; iDatFile <= DATFILE_TEXTURES; ++iDatFile)
		{
			const DATHeader* pHeader = pPair[iDatFile];

			if (!DatFile_IsValid(pHeader))
				continue;

			for (DWORD i = 0; i < DatFile_GetFileCount(pHeader); ++i)
			{
				if (!_strnicmp(DatFile_GetFileExtension(pHeader, i), "mot", 4))
					Motions.push_back({ DatFile_GetFileName(pHeader, i), (MotHeader*)DatFile_GetFileData(pHeader, i) });
			}
		}
	}

	void GetEntityMotions(CBehaviorAppBase* pEntity, std::vector<EntityMotion_t>& Motions)
	{
		Motions.clear();

		CEntityDataWork* pWork = pEntity ? pEntity->m_pDataWork : NULL;

		if (!pWork)
			return;

		AddMotions(pWork->m_pDataFiles, Motions);

		for (int i = 0; i < (int)ARRAYSIZE(pWork->m_Slots); ++i)
		{
			if (pWork->m_Slots[i].m_uObjectId != (uint32_t)-1)
				AddMotions(pWork->m_Slots[i].m_pDataFiles, Motions);
		}
	}

	MotHeader* LoadFromDisk(const char* szPath, char* szError, size_t cchError)
	{
		HANDLE hFile = CreateFileA(szPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

		if (hFile == INVALID_HANDLE_VALUE)
		{
			sprintf_s(szError, cchError, "Couldn't open it (error %lu).", GetLastError());
			return NULL;
		}

		LARGE_INTEGER Size;

		if (!GetFileSizeEx(hFile, &Size) || Size.QuadPart < (LONGLONG)sizeof(MotHeader) || Size.QuadPart > 64 * 1024 * 1024)
		{
			CloseHandle(hFile);
			sprintf_s(szError, cchError, "It's too small or too big to be a .mot.");
			return NULL;
		}

		std::vector<BYTE> File((size_t)Size.QuadPart);
		DWORD cbRead = 0;
		BOOL bRead = ReadFile(hFile, File.data(), (DWORD)File.size(), &cbRead, NULL);

		CloseHandle(hFile);

		if (!bRead || cbRead != File.size())
		{
			sprintf_s(szError, cchError, "Couldn't read it.");
			return NULL;
		}

		// Every record's format and data range: the game would read a bad one out of bounds
		if (!Mot_Validate(File.data(), File.size(), szError, cchError))
			return NULL;

		s_LoadedFiles.push_back(std::move(File));
		return (MotHeader*)s_LoadedFiles.back().data();
	}

	static void SetRequest(const Request_t& Request)
	{
		AcquireSRWLockExclusive(&s_Lock);
		s_Request = Request;
		s_lPending = 1;
		ReleaseSRWLockExclusive(&s_Lock);

		InterlockedExchange(&s_lLastResult, MOTIONPLAYER_PENDING);
	}

	void RequestPlay(CBehaviorAppBase* pEntity, MotHeader* pMot, const char* szName, int iSlot, float flBlendIn, float flRate, bool bLoop)
	{
		if (!s_bHooked || !pEntity || !pEntity->m_pDataWork || !pMot)
			return;

		s_Names.emplace_back(szName ? szName : "2BHook");

		Request_t Request = {};
		Request.m_Type = REQUEST_PLAY;
		Request.m_pEntity = pEntity;
		Request.m_pWork = pEntity->m_pDataWork;
		Request.m_pMot = pMot;
		Request.m_szName = s_Names.back().c_str();
		Request.m_iSlot = iSlot;
		Request.m_flBlend = flBlendIn;
		Request.m_flRate = flRate;
		Request.m_bLoop = bLoop;

		SetRequest(Request);
	}

	void RequestStop(CBehaviorAppBase* pEntity, int iSlot, float flBlendOut)
	{
		if (!s_bHooked || !pEntity || !pEntity->m_pDataWork)
			return;

		Request_t Request = {};
		Request.m_Type = REQUEST_STOP;
		Request.m_pEntity = pEntity;
		Request.m_pWork = pEntity->m_pDataWork;
		Request.m_iSlot = iSlot;
		Request.m_flBlend = flBlendOut;

		SetRequest(Request);
	}

	int GetLastResult(void)
	{
		return s_lLastResult;
	}
}
