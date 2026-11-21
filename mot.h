#pragma once


#include "WinInclude.h"

#include <immintrin.h>
#include <vector> //tmp

#include <Fluorine/Memory.h>

#include "ReversedStructs.h"
#include "Vector4.h"


#define MOT_MAGIC FOURCC('m', 'o', 't', '\0')

typedef unsigned short uint16;
typedef unsigned short hfloat;
typedef unsigned int uint32;
typedef int int32;

// https://github.com/Kerilk/noesis_bayonetta_pc/blob/master/bayonetta_pc/MotionBayo.h
/*
Found: https://forum.xentax.com/viewtopic.php?f=10&t=16011&sid=110c6732b529049ca7dd0769afd12688&start=15

typedef struct BoneArray {
	SHORT	BoneID;
	UBYTE	Channel; 	// "0-POSX 1-POSY 2-POSZ 3-ROTX 4-ROTY 5-ROTZ
	ULONG	FrameThing; 	// "When this is filled Float changes and 8 bytes are added to the bottom of the file"
	UBYTE	Blank0;
	float	Float;
} BoneArray;

struct MOT
{
	char		fourCC[4]; 	// "MOT"
	ULONG		UNK0;
	SHORT		Blank;
	SHORT		UNK1;
	ULONG		Offset1; 	//"Bone List offset"
	ULONG		ListCount;
	ULONG		UNK1;
	char		Name[20];
	BoneArray	BoneStuff[ListCount];

};
*/

struct MotHeader
{
	union {
		char id[4]; // "mot\0"
		int magic;
	};
	UINT   uHash;
	USHORT flag;
	SHORT  sFrameCount;
	UINT   uRecordOffset;
	UINT   uRecordCount;
	BYTE   bUnknown;
	BYTE   bFrameRate30;
	WORD   pad;
	char   szAnimName[12]; // found at most 12 bytes with terminating 0
};


// MotItem_t on https://github.com/Kerilk/noesis_bayonetta_pc/blob/master/bayonetta_pc/MotionBayo.h
struct MotRecord
{
	SHORT    sBoneIndex;
	char     index;
	char     flags;
	SHORT	 elemNumber;
	SHORT    unknown; //always -1
	union {	// bayoMotField_t		value;
		float    flt;
		uint32   offset;
	};
};

struct MotInterpolation
{
	float flValues[ANYSIZE_ARRAY]; // MotRecord::elemNumber
};

struct MotInterpolationInternal
{
	float* m_pflValues;
	short m_sValuesCount;
};

struct MotInterpolation2Values
{
	float p;
	float dp;
};

struct MotInterpolation2
{
	MotInterpolation2Values values;
	uint16 cp[ANYSIZE_ARRAY]; // MotRecord::elemNumber
};

struct MotInterpolation2Internal
{
	MotInterpolation2Values m_Values;
	PUSHORT m_pcp;
	short m_scpCount;
};

struct MotInterpolation3Values
{
	hfloat p;
	hfloat dp;
};

struct MotInterpolation3
{
	MotInterpolation3Values values;
	byte cp[ANYSIZE_ARRAY]; // MotRecord::elemNumber
};

struct MotInterpolation3Internal
{
	MotInterpolation3Values m_Values;
	PBYTE m_pcp;
	short m_scpCount;
};

struct MotInterpolation4Key
{
	uint16 index; // absolute frame index
	uint16 dummy;
	float p;
	float m0;
	float m1;
};

struct MotInterpolation4
{
	MotInterpolation4Key keys[ANYSIZE_ARRAY]; // MotRecord::elemNumber
};

struct MotInterpolation4Internal
{
	MotInterpolation4Key* m_pKeys;
	short m_sKeyCount;
};

struct MotInterpolation5Values
{
	float p;
	float dp;
	float m0;
	float dm0;
	float m1;
	float dm1;
};

struct MotInterpolation5Key
{
	uint16 index; // absolute frame index
	uint16 cp;
	uint16 cm0;
	uint16 cm1;
};

struct MotInterpolation5
{
	MotInterpolation5Values values;
	MotInterpolation5Key keys[ANYSIZE_ARRAY]; // MotRecord::elemNumber
};

struct MotInterpolation5Internal
{
	MotInterpolation5Values m_values;
	MotInterpolation5Key* m_pKeys; 
	short m_sKeyCount;
};

struct MotInterpolation6Values
{
	hfloat p;
	hfloat dp;
	hfloat m0;
	hfloat dm0;
	hfloat m1;
	hfloat dm1;
};
typedef MotInterpolation6Values MotInterpolation7Values;

struct MotInterpolation6Key
{
	byte index; // absolute frame index
	byte cp;
	byte cm0;
	byte cm1;
};

struct MotInterpolation6
{
	MotInterpolation6Values values;
	MotInterpolation6Key keys[ANYSIZE_ARRAY];  // MotRecord::elemNumber
};

struct MotInterpolation6Internal
{
	MotInterpolation6Values m_values;
	MotInterpolation6Key* m_pKeys;
	short m_sKeyCount;
};

struct MotInterpolation7Key
{
	byte index; // frame index relative to previous key
	byte cp;
	byte cm0;
	byte cm1;
};

struct MotInterpolation7
{
	MotInterpolation6Values values;
	MotInterpolation7Key keys[ANYSIZE_ARRAY]; // MotRecord::elemNumber
};

struct MotInterpolation7Internal
{
	MotInterpolation6Values m_values;
	MotInterpolation7Key* m_pKeys;
	short m_sKeyCount;
};

struct MotInterpolation8Values
{
	hfloat p;
	hfloat dp;
	hfloat m0;
	hfloat dm0;
	hfloat m1;
	hfloat dm1;
};

struct MotInterpolation8Key
{
	uint16 index; // absolute frame index (in big endian order!!!!) Big Endian
	byte cp;
	byte cm0;
	byte cm1;
};

struct MotInterpolation8
{
	MotInterpolation8Values values;
	MotInterpolation8Key keys[ANYSIZE_ARRAY]; // MotRecord::elemNumber
};

struct MotInterpolation8Internal
{
	MotInterpolation8Values m_values;
	MotInterpolation8Key* m_pKeys;
	short m_sKeyCount;
};

// ---------------------------------------------------------------------------------------------
// Sampling a curve, the way the game does: its ten record samplers (g_CMotRecordDispatchTable,
// 0x140D80450, indexed by MotRecord::flags), as ExcitedDelirium reconstructs them
// (PlayerBehavior.cpp). Every record's data is at the RECORD plus its offset, not at the file
// plus it. The formats with hfloat heads (3, 6, 7, 8) use the game's own 16-bit packed float,
// not an IEEE half: see Mot_DecodePacked16.
//
//   0  the value, inline in the record
//   1  a float per frame, linear between frames
//   2  {offset, scale} floats, then a uint16 per frame
//   3  {offset, scale} packed, then a byte per frame
//   4  hermite keys of floats {frame, value, in tangent, out tangent}
//   5  {value, in, out} x {offset, scale} floats, then uint16 keys
//   6  the same, packed, then byte keys with an absolute frame
//   7  the same, with each key's frame a delta from the one before
//   8  the same, with a 16-bit big-endian frame
//   9  always 0
//
// Past the last frame or key, a curve holds its last value.
// ---------------------------------------------------------------------------------------------

#define MOT_FORMAT_COUNT		10
#define MOT_RECORD_TERMINATOR	0x7FFF	// MotRecord::sBoneIndex of the record that ends the array

inline const BYTE* MotRecord_GetData(const MotRecord* pRecord)
{
	return (const BYTE*)pRecord + pRecord->offset;
}

// The game's 16-bit packed float (Motion_DecodePackedSample, 0x1401A2780): bits 0-8 a mantissa
// read as (m + 512) / 512, bits 9-14 an exponent e for 2^(e - 47), bit 15 the sign. An exponent
// of 0 is zero.
inline float Mot_DecodePacked16(uint16 uPacked)
{
	const int iExponent = (uPacked >> 9) & 0x3F;

	if (!iExponent)
		return 0.0f;

	const float flValue = ldexpf((float)((uPacked & 0x1FF) + 0x200) * (1.0f / 512.0f), iExponent - 47);

	return (uPacked & 0x8000) ? -flValue : flValue;
}

// The hermite span all the keyed formats share, summed in the game's order
inline float Mot_Hermite(float flFrame, float flLeftFrame, float flRightFrame, float flLeft, float flRight,
	float flLeftOutTangent, float flRightInTangent)
{
	const float t = (flFrame - flLeftFrame) / (flRightFrame - flLeftFrame);
	const float t2 = t * t;
	const float t2x3 = t2 * 3.0f;
	const float t3 = t2 * t;
	const float t3x2 = t3 + t3;
	const float t3MinusT2 = t3 - t2;

	return ((t3x2 - t2x3) + 1.0f) * flLeft + (t2x3 - t3x2) * flRight
		+ ((t3MinusT2 - t2) + t) * flLeftOutTangent + t3MinusT2 * flRightInTangent;
}

// The bytes a record's data takes, for checking a file; 0 for an unknown format
inline SIZE_T MotRecord_GetDataSize(const MotRecord* pRecord)
{
	const SIZE_T n = (USHORT)pRecord->elemNumber;

	switch ((BYTE)pRecord->flags)
	{
	case 1: return 4 * n;
	case 2: return 8 + 2 * n;
	case 3: return 4 + n;
	case 4: return 16 * n;
	case 5: return 24 + 8 * n;
	case 6:
	case 7: return 12 + 4 * n;
	case 8: return 12 + 5 * n;
	default: return 0;
	}
}

// One record's value at a frame (0 is the first). The game samples between the frames on each
// side of flFrame, uKey below and uKeyNext above (CMotSampleCtx).
inline float MotRecord_Sample(const MotRecord* pRecord, float flFrame)
{
	if (flFrame < 0.0f)
		flFrame = 0.0f;

	const BYTE uFormat = (BYTE)pRecord->flags;

	if (uFormat == 0)
		return pRecord->flt;

	const uint32 nElements = (USHORT)pRecord->elemNumber;

	if (uFormat >= 9 || !nElements)
		return 0.0f;

	const BYTE* pData = MotRecord_GetData(pRecord);
	const uint32 uKey = (uint32)flFrame;
	const uint32 uKeyNext = (uint32)ceilf(flFrame);

	// 1-3: a sample per frame
	if (uFormat <= 3)
	{
		float flOffset = 0.0f, flScale = 1.0f;
		const BYTE* pSamples = pData;

		if (uFormat == 2)
		{
			flOffset = ((const float*)pData)[0];
			flScale = ((const float*)pData)[1];
			pSamples = pData + 8;
		}
		else if (uFormat == 3)
		{
			flOffset = Mot_DecodePacked16(((const uint16*)pData)[0]);
			flScale = Mot_DecodePacked16(((const uint16*)pData)[1]);
			pSamples = pData + 4;
		}

		auto Sample = [&](uint32 i) -> float
		{
			switch (uFormat)
			{
			case 1: return ((const float*)pSamples)[i];
			case 2: return (float)((const uint16*)pSamples)[i] * flScale + flOffset;
			default: return (float)pSamples[i] * flScale + flOffset;
			}
		};

		if (uKeyNext > nElements - 1)
			return Sample(nElements - 1);

		const float flBlend = (float)uKeyNext - flFrame;	// the weight of the frame below

		return (1.0f - flBlend) * Sample(uKeyNext) + flBlend * Sample(uKey);
	}

	// 4-8: hermite keys. The heads hold {offset, scale} for the value, the in tangent and the
	// out tangent, as floats (5) or packed (6-8); 4's keys carry floats.
	float flHead[6] = { 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f };
	const BYTE* pKeys = pData;
	SIZE_T cbKey = 16;

	if (uFormat == 5)
	{
		memcpy(flHead, pData, sizeof(flHead));
		pKeys = pData + 24;
		cbKey = 8;
	}
	else if (uFormat >= 6)
	{
		for (int i = 0; i < 6; ++i)
			flHead[i] = Mot_DecodePacked16(((const uint16*)pData)[i]);

		pKeys = pData + 12;
		cbKey = (uFormat == 8) ? 5 : 4;
	}

	struct Key_t { uint32 uFrame; float flValue, flIn, flOut; };

	// Key i's frame, value and tangents. Format 7's frames are deltas, so it is read in order.
	uint32 uRunningFrame = 0;

	auto ReadKey = [&](uint32 i) -> Key_t
	{
		const BYTE* p = pKeys + i * cbKey;
		Key_t Key;

		switch (uFormat)
		{
		case 4:
			Key.uFrame = *(const uint16*)p;
			Key.flValue = ((const float*)p)[1];
			Key.flIn = ((const float*)p)[2];
			Key.flOut = ((const float*)p)[3];
			return Key;
		case 5:
			Key.uFrame = ((const uint16*)p)[0];
			Key.flValue = (float)((const uint16*)p)[1] * flHead[1] + flHead[0];
			Key.flIn = (float)((const uint16*)p)[2] * flHead[3] + flHead[2];
			Key.flOut = (float)((const uint16*)p)[3] * flHead[5] + flHead[4];
			return Key;
		case 8:
			Key.uFrame = ((uint32)p[0] << 8) | p[1];
			p += 2;
			break;
		case 7:
			uRunningFrame += p[0];
			Key.uFrame = uRunningFrame;
			p += 1;
			break;
		default:
			Key.uFrame = p[0];
			p += 1;
			break;
		}

		Key.flValue = (float)p[0] * flHead[1] + flHead[0];
		Key.flIn = (float)p[1] * flHead[3] + flHead[2];
		Key.flOut = (float)p[2] * flHead[5] + flHead[4];
		return Key;
	};

	// The first key at or past uKeyNext; the span is from the key before it
	Key_t Left = ReadKey(0);

	if (Left.uFrame >= uKeyNext)
		return Left.flValue;

	for (uint32 i = 1; i < nElements; ++i)
	{
		Key_t Right = ReadKey(i);

		if (Right.uFrame >= uKeyNext)
			return Mot_Hermite(flFrame, (float)Left.uFrame, (float)Right.uFrame, Left.flValue, Right.flValue, Left.flOut, Right.flIn);

		Left = Right;
	}

	return Left.flValue;
}

// Checks a .mot before the game reads it: the header, every record's format, and that each
// record's data lies inside the file. FALSE, with a reason, for one that would make the game read
// out of bounds.
inline BOOL Mot_Validate(const void* pFile, SIZE_T cbFile, char* szError, SIZE_T cchError)
{
	const MotHeader* pHeader = (const MotHeader*)pFile;

	if (cbFile < sizeof(MotHeader) || pHeader->magic != MOT_MAGIC)
	{
		sprintf_s(szError, cchError, "No \"mot\" header.");
		return FALSE;
	}

	if (pHeader->uRecordOffset > cbFile || (cbFile - pHeader->uRecordOffset) / sizeof(MotRecord) < pHeader->uRecordCount)
	{
		sprintf_s(szError, cchError, "Its %u records run past the end of the file.", pHeader->uRecordCount);
		return FALSE;
	}

	const MotRecord* pRecords = (const MotRecord*)((const BYTE*)pFile + pHeader->uRecordOffset);

	for (UINT i = 0; i < pHeader->uRecordCount; ++i)
	{
		const MotRecord* pRecord = &pRecords[i];
		const BYTE uFormat = (BYTE)pRecord->flags;

		if (pRecord->sBoneIndex == MOT_RECORD_TERMINATOR || uFormat == 0 || uFormat == 9)
			continue;

		if (uFormat >= MOT_FORMAT_COUNT)
		{
			sprintf_s(szError, cchError, "Record %u has an unknown format (%u).", i, uFormat);
			return FALSE;
		}

		const SIZE_T cbData = MotRecord_GetDataSize(pRecord);
		const SIZE_T uStart = (SIZE_T)((const BYTE*)pRecord - (const BYTE*)pFile) + pRecord->offset;

		if (!pRecord->elemNumber || uStart > cbFile || cbData > cbFile - uStart)
		{
			sprintf_s(szError, cchError, "Record %u's data runs past the end of the file.", i);
			return FALSE;
		}
	}

	return TRUE;
}

struct MotInterpListEntry
{
	enum Type
	{
		NONE,
		INTERP,
		INTERP2,
		INTERP3,
		INTERP4,
		INTERP5,
		INTERP6,
		INTERP7,
		INTERP8
	};

	Type type;

	union
	{
		MotInterpolationInternal* pInterp;
		MotInterpolation2Internal* pInterp2;
		MotInterpolation3Internal* pInterp3;
		MotInterpolation4Internal* pInterp4;
		MotInterpolation5Internal* pInterp5;
		MotInterpolation6Internal* pInterp6;
		MotInterpolation7Internal* pInterp7;
		MotInterpolation8Internal* pInterp8;
	};

	const MotRecord* pRecord;		// the curve's record in the file: bone, channel, format
	std::vector<float> flBaked;		// its value at each frame, from ProcessMot
};

EXTERN_C float __fastcall ConvertFP16ToFP32(hfloat value);
extern "C" void ProcessMot(std::vector<MotInterpListEntry>&Interps, CModelWork * pWork, short sFrameCount);

// ReadMot fills Interps with one entry per curve of a .mot in memory, pointing into it (so it
// has to stay loaded); FreeMot frees what ReadMot allocated
void ReadMot(void* pMot, std::vector<MotInterpListEntry>& Interps);
void FreeMot(std::vector<MotInterpListEntry>& Interps);

// ---------------------------------------------------------------------------------------------
// Playing a .mot in game
// ---------------------------------------------------------------------------------------------

// One object's data files inside a CEntityDataWork (ExcitedDelirium, Entity.h)
struct CEntityDatSlot
{
	uint32_t m_uObjectId;			//0x00 | -1 when empty
	uint32_t m_uNameObjectId;		//0x04
	char m_szName[16];				//0x08 | "pl0000"
	DATHeader* m_pDataFiles[2];		//0x18 | {.dat, .dtt}
};
VALIDATE_SIZE(CEntityDatSlot, 0x28);
VALIDATE_OFFSET(CEntityDatSlot, m_pDataFiles, 0x18);

// The per-entity data and motion work (0x640 bytes), BehaviorAppBase::m_pDataWork. Its motions are
// looked up in its own pair and its four slots' (EntityDataWork_FindFile, 0x1401A0F50).
struct CEntityDataWork
{
	DATHeader* m_pDataFiles[2];		//0x000
	uint32_t m_uState;				//0x010
	char _0x014[4];					//0x014
	CEntityDatSlot m_Slots[4];		//0x018
	EntityHandle m_hEntity;			//0x0B8
	uint32_t m_uUnkBC;				//0x0BC
	uint32_t m_uFlags;				//0x0C0 | bit 0: resolved (EntityDataWork_IsResolved)
};
VALIDATE_OFFSET(CEntityDataWork, m_Slots, 0x18);
VALIDATE_OFFSET(CEntityDataWork, m_hEntity, 0xB8);
VALIDATE_OFFSET(CEntityDataWork, m_uFlags, 0xC0);

// Motion node flags, EntityDataWork_PlayMotionFile's uFlags (ExcitedDelirium, Animation.h)
#define MOTIONFLAG_NO_SEQUENCES		0x400		// don't look for "<name>_seq.bxm"
#define MOTIONFLAG_NO_WRAP			0x8000000	// plays once: BehaviorAppBase_PlayMotionNoWrap adds it

// The camera entity plays a motion through the game's own motion system:
// EntityDataWork_PlayMotionFile (0x14019FFD0) takes a motion file already in memory, which is how
// the game plays the ones it finds in an entity's DATs, so a .mot read from disk plays the same way,
// with the game's interpolation, blending and root motion. The play and stop requests are made from
// the menu and carried out in EntityDataWork_EvaluateMotion (0x14019FA30), hooked, on the thread
// that is updating that entity, since the motion tree belongs to its update.
//
// A .mot drives bones by their ids, so it has to be made for the model's skeleton. The entity's own
// state machine keeps playing its motions too, and can take the slot back (walking, idling).
namespace MotionPlayer
{
	struct EntityMotion_t
	{
		const char* m_szName;		// the file's name in its DAT ("pl0000_0001.mot")
		MotHeader* m_pMot;
	};

	bool Initialize(void);			// signatures and the EvaluateMotion hook; false when not found
	void Shutdown(void);
	bool IsAvailable(void);

	// The .mot files in an entity's DATs, in its data work's own pair and its four slots
	void GetEntityMotions(CBehaviorAppBase* pEntity, std::vector<EntityMotion_t>& Motions);

	// Reads a .mot from disk and keeps it for as long as 2BHook is loaded, since a playing motion
	// reads it every frame. NULL, with a reason in szError, when it isn't a .mot.
	MotHeader* LoadFromDisk(const char* szPath, char* szError, size_t cchError);

	// Carried out the next time the entity's motions are evaluated
	void RequestPlay(CBehaviorAppBase* pEntity, MotHeader* pMot, const char* szName, int iSlot, float flBlendIn, float flRate, bool bLoop);
	void RequestStop(CBehaviorAppBase* pEntity, int iSlot, float flBlendOut);

	// What the last request did: the motion's handle (>= 0), -1 when the game refused it, or
	// MOTIONPLAYER_PENDING while it waits for the entity's update
	constexpr int MOTIONPLAYER_PENDING = -2;
	int GetLastResult(void);
}
