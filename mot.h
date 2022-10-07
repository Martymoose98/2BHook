#pragma once


#include <vector> //tmp
#include "Memory.h"
#include "ReversedStructs.h"
#include "Vector4.h"


#define MOT_MAGIC FOURCC('m', 'o', 't', '\0')

typedef unsigned short uint16;
typedef unsigned short pghalf;
typedef unsigned int uint32;
typedef int int32;

// pghalf - https://github.com/Scobalula/Husky/blob/master/src/Husky/Husky/Utility/HalfFloats.cs
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
	pghalf p;
	pghalf dp;
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
	pghalf p;
	pghalf dp;
	pghalf m0;
	pghalf dm0;
	pghalf m1;
	pghalf dm1;
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
	pghalf p;
	pghalf dp;
	pghalf m0;
	pghalf dm0;
	pghalf m1;
	pghalf dm1;
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
};