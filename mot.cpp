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
		switch (pRecords[i].flags) // // 0: constant value for each frame. The value is in records[i].value.flt. -1: only found on terminator (bone index 0x7fff).
		{
		case -1:
		case 0:
			continue;
		case 1:
			pInterp = MakePtr(MotInterpolation*, pMot, pRecords[i].offset);
			pInterpInternal = (MotInterpolationInternal*)malloc(sizeof(MotInterpolationInternal));
			pInterpInternal->m_sValuesCount = pRecords[i].elemNumber;
			memcpy(pInterpInternal->m_pflValues, pInterp->flValues, sizeof(float) * pInterpInternal->m_sValuesCount);
			entry.type = MotInterpListEntry::INTERP;
			entry.pInterp = pInterpInternal;
			Interps.push_back(entry);
			break;
		case 2:
			pInterp2 = MakePtr(MotInterpolation2*, pMot, pRecords[i].offset);
			pInterp2Internal = (MotInterpolation2Internal*)malloc(sizeof(MotInterpolation2Internal));
			pInterp2Internal->m_scpCount = pRecords[i].elemNumber;
			pInterp2Internal->m_pcp = (PUSHORT)malloc(sizeof(SHORT) * pInterp2Internal->m_scpCount);
			pInterp2Internal->m_Values = pInterp2->values;
			entry.type = MotInterpListEntry::INTERP2;
			entry.pInterp2 = pInterp2Internal;
			Interps.push_back(entry);
			break;
		case 3: // _mm_cvtneeph_ps 
			pInterp3 = MakePtr(MotInterpolation3*, pMot, pRecords[i].offset);
			pInterp3Internal = (MotInterpolation3Internal*)malloc(sizeof(MotInterpolation3Internal));
			pInterp3Internal->m_scpCount = pRecords[i].elemNumber;
			pInterp3Internal->m_pcp = (PBYTE)malloc(sizeof(BYTE) * pInterp3Internal->m_scpCount);
			pInterp3Internal->m_Values = pInterp3->values;
			entry.type = MotInterpListEntry::INTERP3;
			entry.pInterp3 = pInterp3Internal;
			Interps.push_back(entry);
			break;
		case 4:
			pInterp4 = MakePtr(MotInterpolation4*, pMot, pRecords[i].offset);
			pInterp4Internal = (MotInterpolation4Internal*)malloc(sizeof(MotInterpolation4Internal));
			pInterp4Internal->m_sKeyCount = pRecords[i].elemNumber;
			memcpy(pInterp4Internal->m_pKeys, pInterp4->keys, sizeof(MotInterpolation4Key) * pInterp4Internal->m_sKeyCount);
			entry.type = MotInterpListEntry::INTERP4;
			entry.pInterp4 = pInterp4Internal;
			Interps.push_back(entry);
			break;
		case 5:
			pInterp5 = MakePtr(MotInterpolation5*, pMot, pRecords[i].offset);
			pInterp5Internal = (MotInterpolation5Internal*)malloc(sizeof(MotInterpolation5Internal));
			pInterp5Internal->m_values = pInterp5->values;
			pInterp5Internal->m_sKeyCount = pRecords[i].elemNumber;
			memcpy(pInterp5Internal->m_pKeys, pInterp5->keys, sizeof(MotInterpolation5Key) * pInterp5Internal->m_sKeyCount);
			entry.type = MotInterpListEntry::INTERP5;
			entry.pInterp5 = pInterp5Internal;
			Interps.push_back(entry);
			break;
		case 6: // _mm_cvtneeph_ps 
			pInterp6 = MakePtr(MotInterpolation6*, pMot, pRecords[i].offset);
			pInterp6Internal = (MotInterpolation6Internal*)malloc(sizeof(MotInterpolation6Internal));
			pInterp6Internal->m_values = pInterp6->values;
			pInterp6Internal->m_sKeyCount = pRecords[i].elemNumber;
			memcpy(pInterp6Internal->m_pKeys, pInterp6->keys, sizeof(MotInterpolation6Key) * pInterp6Internal->m_sKeyCount);
			entry.type = MotInterpListEntry::INTERP6;
			entry.pInterp6 = pInterp6Internal;
			Interps.push_back(entry);
			break;
		case 7: //_mm_cvtneeph_ps 
			pInterp7 = MakePtr(MotInterpolation7*, pMot, pRecords[i].offset);
			pInterp7Internal = (MotInterpolation7Internal*)malloc(sizeof(MotInterpolation7Internal));
			pInterp7Internal->m_values = pInterp7->values;
			pInterp7Internal->m_sKeyCount = pRecords[i].elemNumber;
			memcpy(pInterp7Internal->m_pKeys, pInterp7->keys, sizeof(MotInterpolation7Key) * pInterp7Internal->m_sKeyCount);
			entry.type = MotInterpListEntry::INTERP7;
			entry.pInterp7 = pInterp7Internal;
			Interps.push_back(entry);
			break;
		case 8: //_mm_cvtneeph_ps 
			pInterp8 = MakePtr(MotInterpolation8*, pMot, pRecords[i].offset);
			pInterp8Internal = (MotInterpolation8Internal*)malloc(sizeof(MotInterpolation8Internal));
			pInterp8Internal->m_values = pInterp8->values;
			pInterp8Internal->m_sKeyCount = pRecords[i].elemNumber;
			memcpy(pInterp8Internal->m_pKeys, pInterp8->keys, sizeof(MotInterpolation8Key) * pInterp8Internal->m_sKeyCount);
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
	Vector3* pvCuMulScale = (Vector3*)calloc(sFrameCount * (pData->m_nBones + 1), sizeof(Vector3));

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

}

//https://github.com/Kerilk/noesis_bayonetta_pc/blob/master/bayonetta_pc/MotionBayo.h
void ProcessMot(std::vector<MotInterpListEntry>&Interps, CModelWork * pWork, short sFrameCount)
{

	for (;;)
	{

	}

	for (auto& it : Interps)
	{
		switch (it.type)
		{
		case MotInterpListEntry::NONE:
			break;
		case MotInterpListEntry::INTERP3:
		{
			float dp = ConvertFP16ToFP32(it.pInterp3->m_Values.dp);
			float p = ConvertFP16ToFP32(it.pInterp3->m_Values.p);
		}
			
		default:
			break;
		}
	}
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

static double Readpghalf(hfloat value)
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

static float half_to_float(hfloat h)
{
	return (float)halfbits_to_floatbits(h);
}

static hfloat make_half_float(float f)
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

static void Model_Bayo_HermitInterpolate(float p, float deltap, float m0, float deltam0, float m1, float deltam1)
{


	//HermiteInterpolate()
}

//HermitInterpolate(float p, float deltap, float m0, float deltam0, float m1, float deltam1)
// Result = (2 * t^3 - 3 * t^2 + 1) * Position0 +
//          (t^3 - 2 * t^2 + t) * Tangent0 +
//          (-2 * t^3 + 3 * t^2) * Position1 +
//          (t^3 - t^2) * Tangent1
static Vector4 HermiteInterpolate(Vector4& vPosition0, Vector4& vTangent0, Vector4& vPosition1, Vector4& vTangent1, float t)
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