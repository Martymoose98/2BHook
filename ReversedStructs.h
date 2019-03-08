#pragma once
#include <d3d11.h>
#include <d3d11.h>
#include "StaticAssert.h"
#include "SteamApi.h"
#include "Math.h"

#define PURE = 0

#define SAVE_FLAGS_READ_SLOTS	0x00000001
#define SAVE_FLAGS_READ			0x00000002
#define SAVE_FLAGS_WRITE		0x00000003
#define SAVE_FLAGS_DELETE		0x00000004

#define SAVE_SYSTEMDATA -2
#define SAVE_GAMEDATA -1
#define SAVE_SLOT0DATA 0
#define SAVE_SLOT1DATA 1
#define SAVE_SLOT2DATA 2

#define FLAGS_FREE 1

#define FILTER_UI_NOISE_MAX 0x20000
#define FILTER_UI_NOISE_LV3 0x1000000
#define FILTER_UI_NOISE_LV2 0x2000000
#define FILTER_UI_NOISE_LV1 0x4000000	
#define FILTER_UI_NOISE_LV0 0x8000000

#define FILTER_2B_DIE_LV0 FILTER_UI_NOISE_LV0
#define FILTER_2B_DIE_LV1 FILTER_UI_NOISE_LV1
#define FILTER_2B_DIE_LV2 (FILTER_UI_NOISE_LV2 | 0x400)
#define FILTER_2B_DIE_LV3 (FILTER_UI_NOISE_LV3 | 0x400)

#define FILTER_2B_DIE_CLEAR 	// 0xF07DFFFF

#define _MM_RSHUFFLE(mask, x, y, z, w) \
	 (x) = ((mask & 0xC) >> 2);		   \
	 (y) = ((mask & 0x30) >> 4);	   \
	 (z) = ((mask & 0xC0) >> 6);	   \
	 (w) = (mask & 0x3);			   \

#define IS_SAVE_SLOTDATA(s) ((s) > -1 && (s) < 3)

// visual studio says correct offsets are wrong before compalation on interfaces rifk
#define IS_OFFSET_CORRECT(s, m, offset) CASSERT(offsetof(s, m) == (offset), ReversedStructs)
#define IS_SIZE_CORRECT(s, size) CASSERT(sizeof(s) == (size), ReversedStructs)

#define ENTITY_REAL_VTABLE

typedef ULONGLONG QWORD;

typedef DWORD EntityHandle;

enum TypeId {};

class Pl0000;

/*
Nier Automata's lib::Array<T>
*/
template<typename T>
class Array
{
public:
	virtual Array<T>* Clear(BYTE flags);
	virtual QWORD GetSize();
	virtual bool GetItem(OUT T* pItem);
	virtual T* Find(IN T* pSearchItem, OUT T* pFound);

	T* m_pItems;	//0x0008 
	QWORD m_count;	//0x0010
	QWORD m_size;	//0x0018
};

/*
Nier Automata's lib::StaticArray<T, size, element_size>
*/
template<typename T, size_t size, size_t element_size = sizeof(T)>
class StaticArray : public Array<T>
{
public:
	T m_items[size];	//0x0020
};

/*
Nier Automata's lib::AllocatedArray<T>
*/
template<typename T>
class AllocatedArray
{
public:
	T* m_pItems;	//0x0008 
	QWORD m_count;	//0x0010
	QWORD m_size;	//0x0018
};


/*
Used to convert from object id to object name
Can be also used to convert from object name to object id
*/
struct ObjectIdConvert
{
	int m_ObjectIdBase;
	const char* m_szPrefix;
};

/*
Nier Automata's BannedWord::cChecker
*/
class BannedWordChecker
{
public:
	struct BannedWordBinaryHeader
	{
		DWORD dwWordCount;
	};

	struct WordEntry
	{
		DWORD dwLength;
		LPWSTR lpszBannedWord;
	};

	virtual void* function0(BYTE flags);

	void* m_pBuffer;	
	QWORD m_qwBufferSize;
	WordEntry* m_pEntries;
	DWORD m_dwWordCount;
};	

struct Sound
{
	const char* m_szName;
	DWORD m_dwFNV1Hash;
	DWORD m_Flags;		 //probably a bitflag - (values I've seen, 0x05000000, 0x7000000, 0x00000000, 0x800000 [mute probably])
};

struct ItemHashEntry
{
  int groupID;
  int idx;
  DWORD crc;
};	

struct SoundEmitter
{
	Pl0000* m_pEntity; // this is some lower interface like behaviour
	int m_index;		 // have no fucking clue what this is checks if its -1 so it's an int
};

class CUnknown_t
{
public:
	DWORD id;			//0x0000 idk?
	char aligment[4];	//0x0004
	void* punk;			//0x0008
	void* punk2;		//0x0010
	void* pFunction;	//0x0018
};

struct Unknown_t;

struct DatafileDesc
{
	void* pDatafiles[2];	//0x00
	char pad[8];
	int m_ObjectId2;
	int m_ObjectId;
	char m_szObjectName2[16];
	char m_szObjectName[16]; //0x30
};

// Struct at least 0x1A8 bytes
struct EntityInfo
{
	Unknown_t* m_pUnknown;						//0x0000 | i don't really know what this is (confirmed a struct pointer) maybe a void*
	char m_szEntityType[32];					//0x0008
	unsigned int m_ObjectId;					//0x0028
	BYTE m_Flags;								//0x002C  | An Entity cannot have this flag or'd with 3 game crashes (possibly a destroyed flag)
	char alignment[3];							//0x002D
	EntityHandle m_hEntity;						//0x0030
	char _0x0038[4];							//0x0034
	char* m_pszData[2];							//0x0038
	Pl0000* m_pEntity;							//0x0048
	DatafileDesc* m_pDatDesc;					//0x0050  | struct pointer
	DWORD* m_pUnk;								//0x0058  | dword array 2 members (0x1415F6B50)
	Pl0000* m_pParent;							//0x0060
	BOOL bSetInfo;								//0x0068
	DWORD _0x006C;								//0x006C
	DWORD _0x0070;								//0x0070
	char _0x0x0074[68];							//0x0074
	EntityHandle m_hUnk;						//0x00B8
};
IS_OFFSET_CORRECT(EntityInfo, m_pParent, 0x60)
IS_OFFSET_CORRECT(EntityInfo, m_hUnk, 0xB8)

struct EntityInfoListEntry
{
	EntityHandle m_hEntity;
	EntityInfo* m_pInfo;
};

/*
Address = 14160DF88
*/
class EntityInfoList
{
public:
	DWORD m_dwItems;								//0x0000
	DWORD m_dwSize;									//0x0004
	DWORD m_dwBase;									//0x0008
	DWORD m_dwShift;								//0x000C
	std::pair<EntityHandle, EntityInfo*>* m_pItems; //0x0010
	CRITICAL_SECTION m_CriticalSection;				//0x0018
	BOOL m_bCriticalSectionInitalized;				//0x0040
};

/*
Model Part
size = 0x70 (112) bytes
*/
struct ModelPart
{
	Vector4 m_vColor;			//0x0000
	Vector4 m_vColorReadOnly;	//0x0010
	const char* m_pszUnk;		//0x0020
	QWORD m_qwUnknown28;		//0x0028
	Vector4 m_vUnknown30;		//0x0030
	const char* m_szModelPart;	//0x0040 
	void* m_pUnknown;			//0x0048 | important pointer 
	float m_flUnknown50;		//0x0050
	void* m_pUnknown58;			//0x0058
	DWORD m_dwUnknown60;		//0x0060
	BOOL m_bShow;				//0x0064 | ? need to be synced with the other modifing threads
	BOOL m_bUpdate;				//0x0068 | ? fucks with color vec
	float m_flUnknown6C;		//0x006C
};

struct MappedModel
{
	void* m_vtbl;				//0x0000
	char _0x0008[176];			//0x0008
	ModelPart* m_pModelParts;	//0x00B8
	int m_nModelParts;			//0x00C0
};	

class BehaviorAppBase
{
public:
	typedef DWORD TimeLimitedFlag;
};

class BehaviorExtension
{
public:
	//void* m_vtable;	//0x0000
};

class ExWaypoint : public BehaviorExtension
{

};

class ExBaseInfo : public BehaviorExtension
{
public:
	virtual void function0();
	virtual void function1();
	virtual void function2();
	virtual void function3();
	virtual void function4();
	virtual void function5();
};

/*
Level structure used for level and when you level up
size = 28 bytes
*/
struct Level_t
{
	int m_iMinimumExperience;		//0x0000 
	int m_iHealth;					//0x0004 new max health 
	int m_iBaseAttack;				//0x0008 idk about this one for sure
	int m_iDefense;					//0x000C
	int m_iLevel;					//0x0010 new level
	int unk0x14;					//0x0014
	int unk0x18;					//0x0018
};

/*
Size [8 byte aligned] = 2808 (0xAF8) bytes
Size [4 byte aligned] = 2804 (0xAF4) bytes
*/
struct ExExpInfo
{
	void* m_vtable;			//0x0000
	void** unk;				//0x0008
	QWORD unk0x10;			//0x0010
	QWORD unk0x18;			//0x0018
	Level_t m_levels[99];	//0x0020 ordered 1 - 99 (99 is max level)
};

class ExCollision : public BehaviorExtension
{
public:
	void* m_vtbl;
	StaticArray<std::pair<TypeId, BehaviorExtension*>, 16, 8>* m_pBehaviourExtensions;
	EntityHandle m_hOwner;
	BOOL m_bEnabled;
	BOOL m_bUnknown;
	char pad[4];
	Vector3Aligned m_vecs[2];
};

class ExLockOn : public BehaviorExtension
{
	void* m_vtable;			//0x0000
	void* m_pEntity;		//0x0008 
};

class ExCatch : public BehaviorExtension
{
	void* m_vtable;			//0x0000
};

class ExAttack : public BehaviorExtension
{
	void* m_vtable;			//0x0000
};

class ExDamage : public BehaviorExtension
{
	void* m_vtable;			//0x0000
};

class ExAttackCombo : public BehaviorExtension
{
	void* m_vtable;			//0x0000
};

class ExActionState : public BehaviorExtension
{
	void* m_vtable;			//0x0000
};

class CXmlBinary
{
	void* m_vTable;
};

class CEnt // idk if this exists
{
	void* m_vtbl;					//0x00000
	Matrix4x4 m_matTransform;		//0x00010
	Vector3Aligned m_vPosition;		//0x00050
	Matrix4x4 m_matModelToWorld;	//0x00060 | 1st row: ? 2nd row: scale(x,y,z) 3rd: none: 4th rotation(p,y,r)
	void* m_pUnk0x00A0;				//0x000A0
	short m_wUnk0x00A8;				//0x000A8
	char  m_pad0x000AA[6];			//0x000AA
	void* m_pUnk0x00B0;				//0x000B0
	short m_wUnk0x00B8;				//0x000B8 | set to -1 on construction
	char  m_pad0x000BA[6];			//0x000BA
	Matrix4x4 m_identies[2];		//0x000C0 | end ? CEnt ?
};

struct DATHeader
{
	union {
		char Signature[4];	//0x00
		DWORD dwMagic;		//0x00
	};
	DWORD dwFileCount;		//0x04
	DWORD dwFileTableOffset;//0x08
	DWORD dwExtensionOffset;//0x0C
	DWORD dwNameTableOffset;//0x10
	DWORD dwSizeTableOffset;
	DWORD dwUnknownOffset;
	DWORD dwNull;
};

typedef struct WMBHdr
{
	union {
		char			id[4];			// 0x00
		unsigned int	magic;
	};
	unsigned int		version;			// 4
	int					unknownA;			// 8
	short				unknownB;			// C Seems related to vertex index size
	short				unknownC;			// E
	float				boundingBox[6];		//10
	unsigned int		ofsBones;			//28
	int					numBones;			//2C
	unsigned int		ofsBoneIndexTT;		//30
	int					sizeBoneIndexTT;	//34
	unsigned int		ofsVertexGroups;	//38
	int					numVertexGroups;	//3C
	unsigned int		ofsBatches;			//40
	int					numBatches;			//44
	unsigned int		ofsLods;			//48
	int					numLods;			//4C
	unsigned int		ofsUnknownD;		//50
	int					numUnknownE;		//54
	unsigned int		ofsBoneMap;			//58
	int					sizeBoneMap;		//5C
	unsigned int		ofsBoneSets;		//60
	int					numBoneSets;		//64
	unsigned int		ofsMaterials;		//68
	int					numMaterials;		//6C
	unsigned int		ofsMeshes;			//70
	int					numMeshes;			//74
	unsigned int		ofsMeshMaterial;	//78
	int					numMeshMaterial;	//7C
	unsigned int		ofsUnknownF;		//80
	int					numUnknownG;		//84
} WMBHdr;

// Size of struct 0x58 (88) bytes
typedef struct WMBBone
{
	short	m_sId;				//0x00
	short	m_sParentIndex;		//0x02
	Vector3	m_vLocalPosition;	//0x04
	Vector3	m_vLocalRotation;	//0x10
	Vector3	m_vLocalScale;		//0x1C
	Vector3	m_vPosition;		//0x28
	Vector3	m_vRotation;		//0x34
	Vector3	m_vScale;			//0x40
	Vector3	m_vTPosition;		//0x4C
} WMBBone;

// Size of struct 0xB0 (176) bytes
typedef struct Bone
{
	Vector3Aligned	m_vLocalPosition;	//0x00
	Vector3Aligned	m_vLocalRotation;	//0x10
	Vector3Aligned	m_vLocalScale;		//0x20
	Vector3Aligned	m_vPosition;		//0x30
	Vector3Aligned	m_vRotation;		//0x40
	Vector3Aligned	m_vScale;			//0x50
	Vector3Aligned	m_vTPosition;		//0x60
	Vector3Aligned	m_vUnk;				//0x70
	Vector3Aligned	m_vUnk2;			//0x80
	Bone* m_pUnk;						//0x80
	int m_iFlags;						//0x88 | bones in head and feathers are m_iFlags > 512 (0x200) { 0x208 (origin bone), 0x220 }
	char _0x8C[4];						//0x8C
	Bone* m_pParent;					//0x90
	short m_Id;							//0x98
	char _0x9A[6];						//0x9A
} Bone;
IS_SIZE_CORRECT(Bone, 0xB0)

class CModelData
{
	WMBHdr* m_pHdr;
	void* m_pUnk;
	Bone* m_pBones;
	DWORD m_nBones;
};

struct ModelInfo
{
	Vector4 m_vTint;			//0x0000
	CModelData* m_pData;		//0x0008

	Vector3Aligned m_vPosition; //0x0070
	UINT m_Flags;				//0x00D0
};

// had to define virtuals not right tho!
class CModelExtendWorkBase
{
public:
	virtual void function0() {};
	virtual void function1() {};
};

// had to define virtuals not right tho!
class CModelExtendWork : public CModelExtendWorkBase
{
public:
	virtual void function2() {};
	virtual void function3() {};

	float flUnk;				//0x0008
	DWORD _0x0010;				//0x000C
	unsigned int m_ObjectId;	//0x0010
	char _0x0018[12];			//0x0014
	Pl0000* m_pParent;			//0x0020	
};

/*
Size of struct 0x584 (1412) bytes
*/
class CModel
{
public:
#if 0
	virtual void function0();
	virtual void function1();
	virtual void function2();
#else
	void* vtbl;
#endif

	Matrix4x4 m_matRotation;				//0x0010
	Vector3Aligned m_vPosition;				//0x0050
	Matrix4x4 m_matModelToWorld;			//0x0060 | 1st row: ? 2nd row: scale(x,y,z) 3rd: none: 4th rotation(p,y,r)
	void* m_pUnk0x00A0;						//0x00A0
	short m_wUnk0x00A8;						//0x00A8
	char  m_pad0x000AA[6];					//0x00AA
	void* m_pUnk0x00B0;						//0x00B0
	short m_wUnk0x00B8;						//0x00B8 | set to -1 on construction
	char  m_pad0x000BA[6];					//0x00BA
	Matrix4x4 m_identies[2];				//0x00C0 | end ? CEnt ?
	CModelExtendWork m_pModelExtendWork;	//0x0140
	char _0x0170[556];						//0x0168
	MappedModel* m_pMappedModel;			//0x0398
	ModelPart* m_pModelParts;				//0x03A0 
	DWORD m_nModelParts;					//0x03A8
	char _0x03AC[364];						//0x03AC
	CModelExtendWork* m_pModelExtend;		//0x0518
	char _0x0520[16];						//0x0520
	void* m_pScene;							//0x0530 | when null scene is transparent
	QWORD qw0x00538;						//0x0538
	ModelInfo* m_pModelInfo;				//0x0540
	Unknown_t* m_pUnknown0x548;				//0x0548
	void* m_pWMB_Buffer;					//0x0550
	void* m_p0x00558;						//0x0558
	Bone* m_pBones;							//0x0560
	int m_nBones;							//0x0568 | changes the amount of vertices to get updated each frame (can't be more than 198 for 2B)
	char _0x056C[20];						//0x056C
	DWORD dwFinal;							//0x0580
};
IS_OFFSET_CORRECT(CModel, m_pad0x000AA, 0xAA)
IS_OFFSET_CORRECT(CModel, m_identies, 0xC0)
IS_OFFSET_CORRECT(CModel, m_pModelExtendWork, 0x140)
IS_OFFSET_CORRECT(CModel, m_pMappedModel, 0x398)
IS_OFFSET_CORRECT(CModel, m_pModelInfo, 0x540)
IS_OFFSET_CORRECT(CModel, m_nBones, 0x568)
IS_OFFSET_CORRECT(CModel, dwFinal, 0x580)

/*
Size of struct 0x90 (144) bytes
*/
class CModelShaderModule
{
public:
	void* lpVtbl;			//0x0000
	char _0x0008[8];		//0x0008
	const char* m_szName;	//0x0010
	char _0x0018[108];		//0x0018
	float m_flWetness;		//0x0084
};
IS_OFFSET_CORRECT(CModelShaderModule, m_flWetness, 0x84)

/*
Size of struct 0x670 (1648) bytes 
*/
class CObj
{
	void* m_pVtbl;	//0x0000
	char pad[1640];	//0x0008
};

class CObjHit
{
	void* m_pVtbl;		//0x0000
	char _0x0008[136];	//0x0008
	Vector4 m_vec[2];	//0x0090

};

/*
NieR:Automata's Entity class idk if it's the right name (actual Pl0000). Just started to
reverse this and this structure is fucking huge!

Offsets are -0x50 from the debug build

Size of struct 0x178F0 (96496) bytes
*/
class Pl0000
{
public:
	typedef int PassiveSkill;

	enum eBones
	{
		BONE_ABDOMEN = 0,
		BONE_ORIGIN = 1,
		BONE_HEAD = 11
	};

#ifdef ENTITY_REAL_VTABLE
	virtual void function0() PURE;
	virtual void function1() PURE;
	virtual void function2() PURE;
	virtual void function3() PURE;
	virtual void function4() PURE;
	virtual void function5() PURE;
	virtual void function6() PURE;
	virtual void function7() PURE;
	virtual void function8() PURE;
	virtual void function9() PURE;
	virtual void function10() PURE;
	virtual void function11() PURE;
	virtual void function12() PURE;
	virtual void function13() PURE;
	virtual void function14() PURE;
	virtual void function15() PURE;
	virtual void function16() PURE;
	virtual void function17() PURE;
	virtual void Animate(unsigned int id, int mode, int a4, int a5) PURE;
	virtual void function19() PURE;
	virtual void function20() PURE;
	virtual void function21() PURE;
	virtual void function22() PURE;
	virtual void function23() PURE;
	virtual void function24() PURE;
	virtual void function25() PURE;
	virtual void function26() PURE;
	virtual void function27() PURE;
	virtual EntityInfo* function28(__int64 a1, unsigned int objectId) PURE;
	virtual bool IsAliveAndEntityInfoFlag() PURE;
	virtual int GetMaxHealth() PURE; //index: 30
	virtual int GetHealth() PURE;
	virtual void function32() PURE;
	virtual bool IsDead() PURE;
	virtual void function34() PURE;
	virtual void function35() PURE;
	virtual void function36() PURE;
	virtual void function37_off() PURE;
	virtual void function38_on() PURE;
	virtual void function39() PURE;
	virtual __int64 EmitSound(const char *szName, Pl0000* pSourceEntity, __int64 a4, __int64 a5, unsigned int a6) PURE;
	virtual void function41() PURE;
	virtual void function42() PURE;
	virtual void function43() PURE;
	virtual void function44() PURE;
	virtual void function45() PURE;
	virtual void function46() PURE;
	virtual void function47() PURE;
	virtual void function48() PURE;
	virtual void function49() PURE;
	virtual void function50() PURE;
	virtual void function51() PURE;
	virtual void function52() PURE;
	virtual void function53() PURE;
	virtual void function54() PURE;
	virtual void function55() PURE;
	virtual void function56() PURE;
	virtual void function57() PURE;
	virtual void function58() PURE;
	virtual void function59() PURE;
	virtual void function60() PURE;
	virtual void function61() PURE;
	virtual void function62() PURE;
	virtual void function63() PURE;
	virtual void function64() PURE;
	virtual void function65() PURE;
	virtual void function66() PURE;
	virtual void function67() PURE;
	virtual void function68() PURE;
	virtual void function69() PURE;
	virtual void function70() PURE;
	virtual void function71() PURE;
	virtual void function72() PURE;
	virtual void function73() PURE;
	virtual void function74() PURE;
	virtual void function75() PURE;
	virtual void function76() PURE;
	virtual void function77() PURE;
	virtual void function78() PURE;
	virtual void function79() PURE;
	virtual void function80() PURE;
	virtual void function81() PURE;
	virtual void function82() PURE;
	virtual void function83() PURE;
	virtual void Hurt(int iDamage, BOOL bKeepAlive) PURE;
	virtual void Heal(int iHealAmount) PURE;
	virtual int GetLevel() PURE;
	virtual bool IsValid() PURE;
	virtual void function88() PURE;
	virtual void function89() PURE;
	virtual void function90() PURE;
	virtual void function91() PURE;
	virtual void function92() PURE;
	virtual void function93() PURE;
	virtual void function94() PURE;
	virtual void function95() PURE;
	virtual void function96() PURE;
	virtual void function97() PURE;
	virtual void function98() PURE;
	virtual void function99() PURE;
	virtual void function100() PURE;
	virtual void function101() PURE;
	virtual void function102() PURE;
	virtual void function103() PURE;
	virtual void SetLevel(int iLevel) PURE;
	virtual void function105() PURE;
	virtual void function106() PURE;

#else
	void* m_pVtable;						//0x00000	 
#endif // ENTITY_REAL_VTABLE
	//char _0x0008[8];						//0x00008  CMODEL STARTS AT 0x0000
	/**  
	* 	rotation z axis matrix:
	* 	----    		     ----
	* 	|  cos(z), sin(z), 0, 0 |
	* 	| -sin(z), cos(z), 0, 0 |
	* 	|     0, 	    0, 1, 0 |
	* 	|     0, 		0, 0, 1 |
	* 	----                 ----
	*
	*  rotation y axis matrix:
	* 	----    		     ----
	* 	| cos(y), 0, -sin(y), 0 |
	* 	| 	   0, 1,       0, 0 |
	* 	| sin(y), 0,  cos(y), 0 |
	* 	|      0, 0,       0, 1 |
	* 	----                 ----
	*
	* 	rotation x axis matrix:
	*  ----	  	   		     ----
	*  | 1, 	 0, 	  0,  0 |
	*  | 0,  cos(x), sin(x),  0 |
	*  | 0, -sin(x), cos(x),  0	|
	*  | 0,      0,       0,  1 |
	*  ----				     ----
	*	transform axis matrix: = mRX * mRY * mRZ + mTranslation
	* 	----    											 ----
	* 	| cos(y) * cos(z),			 sin(z),         -sin(y), 0 | Right
	* 	| 		  -sin(z),	cos(z) * cos(x),          sin(x), 0 | Up
	* 	|		   sin(y),			-sin(x), cos(x) * cos(y), 0 | Forward
	* 	|			   tx,				 ty,			  tz, 1 | Pos
	* 	----												 ----
	*/
	Matrix4x4 m_matTransform;				//0x00010
	Vector3Aligned m_vPosition;				//0x00050
	Matrix4x4 m_matModelToWorld;			//0x00060 | 1st row: ? 2nd row: scale(x,y,z) 3rd: none: 4th rotation(p,y,r) (probably not a mat4x4 now that i think about it lmao)
	void* m_pUnk0x00A0;						//0x000A0
	short m_wBitFlags;						//0x000A8 | 0x00 - NORMAL, 0x01 - ?, 0x02 - NO_ROTATION_Y, 0x04 - FLOAT_UP, 0x08 - ?, 0x10 - ? (wont set), 0x20 - ? (wont set)
											//			0x40 - ?, 0x80 - ?, 0x100 - ?, 0x200 - ?, 0x400 - ?, 0x800 - ?, 0x1000 - ?, 0x2000 - ?, 0x4000 - NO_ROTATION_Y, 0x8000 - ?
	char  m_pad0x000AA[6];					//0x000AA
	void* m_pUnk0x00B0;						//0x000B0
	short m_wUnk0x00B8;						//0x000B8 | set to -1 on construction
	char  m_pad0x000BA[6];					//0x000BA
	Matrix4x4 m_identies[2];				//0x000C0 | end ? CEnt ?
	CModelExtendWork m_pModelExtendWork;	//0x00140
	char _0x0168[336];						//0x00168
	BYTE m_bWetness;						//0x002B8
	char _0x02B9[220];						//0x002B9
	MappedModel* m_pMappedModel;			//0x00398
	ModelPart* m_pModelParts;				//0x003A0 
	int m_nModelParts;						//0x003A8
	char _0x03AC[4];						//0x003AC
	void* m_pBufferShadowsidk;				//0x003B0 | model shaders
	int m_iShadowLevel;						//0x003B8 | 0 = no shadow
	char _0x03BC[4];						//0x003BC
	void* m_p0x03C0;						//0x003C0
	BOOL m_b0x03C8;							//0x003C8
	char _0x03CC[332];						//0x003CC
	CModelExtendWork* m_pModelExtend;		//0x00518
	DWORD m_dwModelRenderFlags;				//0x00520
	char _0x0524[12];						//0x00524
	void* m_pScene;							//0x00530 | when null scene is transparent
	QWORD qw0x00538;						//0x00538
	ModelInfo* m_pModelInfo;				//0x00540
	Unknown_t* m_pUnknown0x548;				//0x00548
	CModelData* m_pModelData;				//0x00550
	void* m_p0x00558;						//0x00558
	Bone* m_pBones;							//0x00560
	int m_nBones;							//0x00568
	char _0x056C[12];						//0x0056C
	Vector3 m_vUnknown;						//0x00578
	BOOL m_bExtraDetail;					//0x00584
	char _0x0588[8];						//0x00588
	DWORD dw0x590;							//0x00590
	char _0x594[4];							//0x00594
	DWORD m_Flags;							//0x00598
	DWORD dw0x0059C;						//0x0059C
	void* m_pMappedFiles[2];				//0x005A0
	void* m_p0x05B0;						//0x005B0 | CObj Start?
	int m_ObjectId;							//0x005B8 | 10000 = 2B | 10200 = 9S | 10203 = A2? (2B only accepts regualar, mech suit, and static -1)
	unsigned int unk0x005BC;				//0x005BC
	unsigned int flag0x005C0;				//0x005C0 | one-way invisible 2B
	BYTE unk0x05C4;							//0x005C4 | disable dynamic skirt?
	BYTE _0x05C5;							//0x005C5
	char _0x05C6[2];						//0x005C6
	DWORD dw0x5C8;							//0x005C8
	float fl0x5CC[7];						//0x005CC
	Vector3 m_vInnerBrightness;				//0x005E8 (x = brightness [paired with z, not modifyable], y = unknown, z = brightness [paired with z, modifyable])
	void* _0x5F4;							//0x005F4
	void* _0x600;							//0x00600
	float fl0x60C;							//0x0060C
	EntityInfo* m_pInfo;					//0x00610 
	CXmlBinary m_xmlBinary;					//0x00618
	char _0x0620[80];						//0x00620	
	int m_iAnimationId;						//0x00670
	int m_iAnimationMode;					//0x00674 | not 100%
	int m_iUnk0x0678;						//0x00678
	int m_iUnk0x067C;						//0x0067C
	char _0x0680[32];						//0x00680
	void* m_pUnk0x6A0;						//0x006A0
	char _0x06A8[8];						//0x006A8
	StaticArray<std::pair<TypeId, BehaviorExtension*>, 16, 8> m_BehaviourExtensions;//0x006B0 | lib::StaticArray<std::pair<enum  lib::TypeId,BehaviorExtension *>,16,8>  BehaviorExtension is an interface
	char _0x07D0[136];						//0x007D0
	//2b skirt								//0x00800
	//ExActionState							//0x00830  ExActionState (a BehaviorExtension)
	int m_iHealth;							//0x00858
	int m_iMaxHealth;						//0x0085C
	char _0x0860[36];						//0x00860
	BYTE m_flags;							//0x00884
	char _0x0885[91];						//0x00885
	void* m_waypointVtbl;					//0x008E0
	char _0x08E8[168];						//0x008E8
	int m_iAnimationId2;					//0x00990
	int m_iAnimationMode2;					//0x00994 | not 100%
	char _0x0998[16];						//0x00998
	StaticArray<BehaviorAppBase::TimeLimitedFlag, 4> m_LimitedTimeFlags;//0x009A8 class lib::StaticArray<BehaviorAppBase::TimeLimitedFlag,4,4>
	char _0x09D8[2788];						//0x009D8
	//ExNpc m_npc;							//0x00A98
	//BOOL m_bTalkDisable;					//0x00BCC
	//EntityHandle m_hUnk;					//0x00C50

	//CRITICAL_SECTION m_CriticalSection;		//0x00C60
	
	//EntityHandle m_hUnk;					//0x00C80
	//EntityHandle m_hUnk;					//0x00DA0

	int m_iLevel;							//0x014BC
	BOOL m_bLevelUp;						//0x014C0 | pretty sure
	char _0x14C4[44];						//0x014C4
	void* m_pCObjHitVtable;					//0x014F0 | start cObjHit
	char _0x014F8[664];						//0x014F8
	ExCollision m_VerticalCollision;		//0x01790
	char _0x17D0[240];						//0x017D0
	ExCollision m_HorizontalCollision;		//0x018C0
	char _0x01900[400];						//0x01900
	float m_flSprintSpeed;					//0x01A90
	char _0x01A94[1500];					//0x01A94
	float fl0x02070;						//0x02070
	char _0x02074[4];						//0x02074
	ExExpInfo m_LevelsContainer;			//0x02078

//	EntityHandle m_hCaughtFish;				//0x03ED8 | on the pod

	char _0x02B70[56056];					//0x02B70
	int m_iHealth2;							//0x10668									
	char _0x1066C[420];						//0x1066C
											//0x10680
	EntityHandle m_hPod;					//0x10810
	EntityHandle m_hUnk;					//0x10814
	char _0x10818[9896];					//0x10818
	StaticArray<CObj*, 64> m_pObjects;		//0x12EC0
	char _0x130E0[10864];					//0x130E0
	BOOL m_bFlightSuit;						//0x15B50
	EntityHandle m_hFlightSuit;				//0x15B54
	char _0x15B58[880];						//0x15B58
	StaticArray<EntityHandle, 64> m_Handles;//0x15EC8
	char _0x15FE8[1156];					//0x15FE8
	EntityHandle m_hBuddy;					//0x1646C | Localplayer = 0x1000C00, Buddy = 0x1020400
	EntityHandle m_hPascal;					//0x16470
	char _0x16474[268];						//0x16474
	StaticArray<EntityHandle, 4> m_Handles2;//0x16580
	char _0x165B0[72];						//0x165B0 | const hap::hap_action::PuidMixin
	StaticArray<Pl0000::PassiveSkill, 100> m_PassiveSkills;	//0x165F8
	char _0x167A8[1320];					//0x167A8
	BOOL b0x16CD0;							//0x16CD0
	BOOL m_bRenderSkirt;					//0x16CD4
	BOOL b0x16CD8;							//0x16CD8
	BOOL m_bRenderEyemask;					//0x16CDC
	BOOL b0x16CE0;							//0x16CE0
	BOOL m_bNotRenderFeather;				//0x16CE4
	BOOL b0x16CE8;							//0x16CE8
	EntityHandle m_hUnknown2;				//0x16CEC
	char _0x16CF0[868];						//0x16CF0
	EntityHandle m_hWig;					//0x17054
	char _0x17058[52];						//0x17058
	DWORD m_dwAccessory;					//0x1708C
	int unk0x17090;							//0x17090
	int m_iBodyType;						//0x17094
	char _0x17090[980];						//0x17098
	EntityHandle m_hUnknown3;				//0x1746C
	char _0x17470[896];						//0x17470
	Vector3Aligned m_vDesinationTarget;		//0x177F0
	BOOL m_bWalkToTarget;					//0x17800
	BOOL m_bRunToTarget;					//0x17804
	char _0x17808[224];						//0x17808
};
typedef Pl0000 Entity2B;
IS_SIZE_CORRECT(Pl0000, 0x178F0)
IS_OFFSET_CORRECT(Pl0000, m_vPosition, 0x50)
IS_OFFSET_CORRECT(Pl0000, m_pModelExtendWork, 0x140)
IS_OFFSET_CORRECT(Pl0000, m_bWetness, 0x2B8)
IS_OFFSET_CORRECT(Pl0000, m_pMappedModel, 0x398)
IS_OFFSET_CORRECT(Pl0000, m_pModelExtend, 0x518)
IS_OFFSET_CORRECT(Pl0000, m_pModelInfo, 0x540)
IS_OFFSET_CORRECT(Pl0000, m_Flags, 0x598)
IS_OFFSET_CORRECT(Pl0000, m_ObjectId, 0x5B8)
IS_OFFSET_CORRECT(Pl0000, m_pInfo, 0x610)
IS_OFFSET_CORRECT(Pl0000, m_BehaviourExtensions, 0x6B0)
IS_OFFSET_CORRECT(Pl0000, m_iHealth, 0x858)
IS_OFFSET_CORRECT(Pl0000, m_pCObjHitVtable, 0x14F0)
IS_OFFSET_CORRECT(Pl0000, m_LevelsContainer, 0x2078)
IS_OFFSET_CORRECT(Pl0000, m_VerticalCollision, 0x1790)
IS_OFFSET_CORRECT(Pl0000, m_HorizontalCollision, 0x18C0)
IS_OFFSET_CORRECT(Pl0000, m_pObjects, 0x12EC0)
IS_OFFSET_CORRECT(Pl0000, m_hFlightSuit, 0x15B54)
IS_OFFSET_CORRECT(Pl0000, m_hBuddy, 0x1646C)
IS_OFFSET_CORRECT(Pl0000, m_hUnknown2, 0x16CEC)
IS_OFFSET_CORRECT(Pl0000, m_dwAccessory, 0x1708C)
IS_OFFSET_CORRECT(Pl0000, m_hUnknown3, 0x1746C)

class CCameraInstance;

/*

141605310
*/
class CCameraGame
{
public:
	void* m_vtable;					//0x0000
	char align8[8];					//0x0008 | align 16 padding tbh
	Matrix4x4 m_matTransform;		//0x0010
	float m_flZoom;					//0x0050
	char align54[12];				//0x0054 | alignment (16)
	CCameraInstance* m_pInstance;	//0x0060
	char align68[8];				//0x0068
	Vector3Aligned m_v0x70;			//0x0070
	Vector3Aligned m_v0x80;			//0x0080
	Vector3Aligned m_v0x90;			//0x0090
	float m_fl0xA0;					//0x00A0
	float m_fl0xA4;					//0x00A4
	Pl0000* m_pLocalPlayer;			//0x00A8 | probs wrong
	DWORD dwUnk;					//0x00B0
	char _0x00B4[28];				//0x00B4
	EntityHandle m_hEntity;			//0x00D0
	DWORD alignment;				//0x00D4
	Pl0000* m_pEntity;				//0x00D8
	Pl0000* m_pEntity2;				//0x00E0
	QWORD align;					//0x00E8
	Vector3Aligned m_vPosition;		//0x00F0
	Vector3Aligned m_vTargetPos;	//0x0100
	Vector3Aligned m_vUnk2;			//0x0110
	Vector3Aligned m_vUnk3;			//0x0120
	char _0x0130[208];				//0x0130
	Matrix4x4 m_matWorldToScreen;	//0x0200
	void* m_pUnk;					//0x0240
	char _0x0240[296];				//0x0248
	Pl0000* m_pCamEntity;			//0x0370
	char _0x0378[120];				//0x0378
	Vector3 m_viewangles;			//0x03F0 | radians (p, ?, ?)
};
IS_OFFSET_CORRECT(CCameraGame, m_matTransform, 0x10)
IS_OFFSET_CORRECT(CCameraGame, m_pLocalPlayer, 0xA8)
IS_OFFSET_CORRECT(CCameraGame, m_hEntity, 0xD0)
IS_OFFSET_CORRECT(CCameraGame, m_vPosition, 0xF0)
IS_OFFSET_CORRECT(CCameraGame, m_matWorldToScreen, 0x200)
IS_OFFSET_CORRECT(CCameraGame, m_viewangles, 0x3F0)

// Found by Dennis
// Address = 0x14160EB40
struct Sun
{
	Vector2	m_vPosition;	 //0x0000
	char _0x0008[56];		 //0x0008
	Vector4 m_vColor;		 //0x0040
	float m_flBloom;		 //0x0044
};

/*
SceneStateHead
size = 0x20 (32) bytes
*/
struct SceneStateHead
{

};

// this has a doubly linked list for sure prbably a different struct tho
// that is 32 bytes that is the head
struct SceneState
{
	SceneStateHead* m_pHead;	//0x0000
	SceneState* m_pNext;		//0x0008
	SceneState* m_pLast;		//0x0010
	char _0x0018;				//0x0018
	bool m_bLast;				//0x0019  not 100%
	char _0x0020[6];			//0x0020
	DWORD m_DescriptionHash;	//crc32
	char alignment[4];
	char* m_szDescription;
	size_t m_ReferenceCount;	//not sure
};

class CScenePosSystem
{
	void* m_vtbl;							//0x0000
	void* m_p0x0008;						//0x0008
	void* m_TokenStateSystem;				//0x0010 | hap::scene_state::SceneStateSystem::`vftable'{for `hap::TokenCategory'};
};

class CSceneStateSystem //const hap::scene_state::SceneStateSystem
{
public:
	void* m_vtable;							//0x0000
	void* m_p0x08;							//0x0008
	void* m_p0x10;							//0x0010
	void* m_p0x18;							//0x0018
	CScenePosSystem m_pPosSystem;			//0x0028
	char _0x0038[40];						//0x0038
	Array<SceneState> m_oldstates;			//0x0060 //const lib::DynamicArray<hap::scene_state::SceneState,hap::configure::Allocator>
	char _0x00[16];							//0x0080
	Array<SceneState> m_states;				//0x0090 //const lib::DynamicArray<hap::scene_state::SceneState,hap::configure::Allocator>
};
IS_OFFSET_CORRECT(CSceneStateSystem, m_states, 0x90)

class CDialogWindow
{
	
	void* lpVtbl;

	float fl0x28;
	float fl0x2C;
	float fl0x30;
	float fl0x34;
};

struct Keyboard_t
{
	IDirectInputDevice8A* pKeyboard;
	BOOL bAcquired;
	HKL Layout;
};

struct Mouse_t
{
	HWND hWnd;
	IDirectInputDevice8A* pMouse;
	BOOL bAcquired;
	BOOL bShowCursor;
	BOOL bShowCursorOld;
	int field_1C;
	int field_20;
	BOOL bClipCursor;
	int field_28;
	float fl_field_2C;
	float fl_field_30;
};

enum eControllerButtons
{
	DPAD_LEFT = 0x1,
	DPAD_RIGHT = 0x2,
	DPAD_DOWN = 0x4,
	DPAD_UP = 0x8,
	A = 0x10,
	B = 0x20,
	X = 0x40,
	Y = 0x80,
	START = 0x100,
	BACK = 0x200,
	LEFT_SHOULDER = 0x400,
	LEFT_THUMB = 0x1000,
	RIGHT_SHOULDER = 0x2000,
	RIGHT_THUMB = 0x8000
};
	
struct CGamePadDevice
{
	void *m_vtbl;
	DWORD m_dwUnk8;
	DWORD m_dwUnkC;
	DWORD m_dw10;
	DWORD m_dw14;
	DWORD m_dw18;
	DWORD m_dwUserIndexs[4];
};

struct controller_input_state2
{
	DWORD dwunk;				//0x0000
	BOOL bUpdated;				//0x0004
	float m_flThumbLX;			//0x0008
	float m_flThumbLY;			//0x000C`
	float m_flThumbRX;			//0x0010
	float m_flThumbRY;			//0x0014
	float m_flLeftTrigger;		//0x0018
	float m_flRightTrigger;		//0x001C
	DWORD m_dwButtons;			//0x0020 | eConrollerButtons
};

/*
Size of struct is 0x80 (128) bytes
*/
struct controller_input
{
	controller_input_state2 state;
	BYTE gap20[24];
	char char38[64];
	BOOL m_bVibrate;
};

/*
Size of struct is 0x18 (24) bytes
*/
struct controller_vibration
{
	BOOL m_bVibrate;			//0x00
	float m_flLeftVibration;	//0x04
	float m_flRightVibration;	//0x08
	DWORD dw0x0C;				//0x0C
	DWORD dw0x10;				//0x10
	BOOL m_bOldVibrate;			//0x14
};

// apperantly 128 bytes according to stack memset, but asm says 32 bytes
struct controller_input_state
{
	BOOL bUpdated;				//0x0000
	float m_flThumbLX;			//0x0004
	float m_flThumbLY;			//0x0008
	float m_flThumbRX;			//0x000C
	float m_flThumbRY;			//0x0010
	float m_flLeftTrigger;		//0x0014
	float m_flRightTrigger;		//0x0018
	DWORD m_dwButtons;			//0x001C | eConrollerButtons
};

//implement these when possible
class CDepthSurface;

class CRenderTarget
{
	void* vtbl;
	int m_iWidth;
	int m_iHeight;

	//const Hw::cRenderTargetImpl::`vftable'{for `Hw::cGraphicObjectBase' }
};

class CVertexBuffer;
class CConstantBuffer;


// maybe more to this
struct CVertexLayout
{
	void* m_vtbl;
	BYTE gap0[304];
	ID3D11InputLayout **m_ppInputlayout;
};

// maybe more to this
struct CPixelShader
{
	BYTE pad[16];
	ID3D11PixelShader** m_ppShader;
};

// maybe more to this
struct CVertexShader
{
	BYTE pad[16];
	ID3D11VertexShader** m_ppShader;
};

struct CIndexBuffer
{
	ID3D11Buffer* m_pBuffer;
	DXGI_FORMAT m_format;
};

struct CConstantBuffers
{
	BOOL m_bSetBuffers;
	ID3D11Buffer* m_pBuffers[14];
};

struct RenderInfo
{
	enum Topology
	{
		TOPOLOGY_POINTLIST,
		TOPOLOGY_LINELIST,
		TOPOLOGY_LINESTRIP,
		TOPOLOGY_TRIANGLELIST,
		TOPOLOGY_TRIANGLESTRIP,
		TOPOLOGY_UNDEFINED
	};

	Topology m_Topology;
	INT m_iBaseVertexLocation;
	UINT m_uVertexCount;
	DWORD m_dwUnk;
	UINT m_uIndexCountPerInstance;
};

/*
Size of struct 504 (0x1F8) bytes
*/
struct CGraphicShader
{
	ID3D11VertexShader** m_pVertexShader;	//0x00 | probs a struct w/ the ID3D11VertexShader* at offset 0
	ID3D11PixelShader** m_pPixelShader;		//0x08 | probs a struct w/ the ID3D11PixelShader* at offset 0
	BOOL m_bSetShaders;						//0x10
	BYTE gap14[4];							//0x14
	BOOL m_bSetVertexBuffer;				//0x18
	BYTE gap1C[4];							//0x1C
	ID3D11Buffer *const m_pBuffers[8];		//0x20
	UINT m_uOffsets[8];						//0x60
	UINT m_uStrides[8];						//0x80
	ID3D11InputLayout** m_ppInputLayout;	//0xA0 | probs a struct w/ the ID3D11InputLayout* at offset 0
	CIndexBuffer* m_pIndexBuffer;			//0xA8
	DWORD m_dw0xB0;							//0xB0
	BOOL m_bSetInputLayout;					//0xB4
	BOOL m_bSetIndexBuffer;					//0xB8
	UINT m_uTopologyIndex;					//0xBC
	CConstantBuffers m_ConstantBuffers[2];	//0xC0
};

/*
this structure is used to create d3d and is used in many of NieR: Automata's
direct3d functions. It might have more to it that I haven't explored so, this is just
the basic layout of the stucture. Strangley enough, it has device pointers and factory
pointers but not a swapchain pointer nor an adapter pointer. Perhaps, I just have not
disovered them. In addition I haven't found a global pointer that directly points to
this structure. This is a virtual class after further investigation. Correctly 
renamed the class to it's proper name. It seems there might be a nested struct too.

 Size of struct 0x110 (272) bytes  probably bigger	
 */
class CGraphicDeviceDx11
{
	//virtual CGraphicDeviceDx11* function0();
	//virtual void function1();
	//virtual unsigned int function2(__int64 a1, CDisplay* pDisplay, HWND hWnd, __int64 a4);
	//virtual bool function3();

public:
	void* pVtable;							//0x0000
	__int32 iScreenWidth;					//0x0008 
	__int32 iScreenHeight;					//0x000C 
	float flScreenHz;						//0x0010 / fps / 60hz capped
	BOOL isFullscreen;						//0x0014
	BOOL bUnk0x018;							//0x0018
	BOOL bUnk0x01C;							//0x001C
	BOOL bUnk0x020;							//0x0020
	char _0x0024[4];						//0x0024
	ID3D11Device* pDevice;					//0x0028
	IDXGIFactory* pFactory;					//0x0030
	HWND hWindow;							//0x0038
	CVertexBuffer* m_pVertexBuffer;			//0x0040 | Hw::cVertexBufferImpl
	void* unk0x048[2];						//0x0048
	CVertexLayout* m_pVertexLayout;			//0x0058 | Hw::cVertexLayoutImpl
	CConstantBuffer* m_pConstantBuffer;		//0x0060 | Hw::cConstantBufferImpl
	char _0x0048[8];						//0x0068
	ID3D11Device* pD3D11Device2;			//0x0070 | same pointer as first | struct
	IDXGIFactory* pFactory2;				//0x0078 | same pointer as first
	char _0x0080[96];						//0x0080 | pointers in here
	BOOL isWindowed;						//0x00E0
	float flFrameRate;						//0x00E4
	char _0x00E8[8];						//0x00E8
	D3D11_QUERY_DESC QueryDescs[2];			//0x00F0
	wchar_t* pszGraphicsAdapterName;		//0x0100 (array?) maybe DXGI_OUTPUT_DESC*
	QWORD qwCurrentAdapterNameIndex;		//0x0108
};
IS_SIZE_CORRECT(CGraphicDeviceDx11, 272)

/*Size of struct 0x580 (1408) bytes*/
class CGraphicContextDx11
{
public:
	enum Topology
	{
		TOPOLOGY_UNDEFINED,
		TOPOLOGY_1_CONTROL_POINT_PATCHLIST,
		TOPOLOGY_2_CONTROL_POINT_PATCHLIST, 
		TOPOLOGY_3_CONTROL_POINT_PATCHLIST,
		TOPOLOGY_4_CONTROL_POINT_PATCHLIST, 
		TOPOLOGY_5_CONTROL_POINT_PATCHLIST,
		TOPOLOGY_6_CONTROL_POINT_PATCHLIST,
		TOPOLOGY_7_CONTROL_POINT_PATCHLIST,
		TOPOLOGY_8_CONTROL_POINT_PATCHLIST, 
		TOPOLOGY_9_CONTROL_POINT_PATCHLIST,
		TOPOLOGY_10_CONTROL_POINT_PATCHLIST,
		TOPOLOGY_11_CONTROL_POINT_PATCHLIST,
		TOPOLOGY_12_CONTROL_POINT_PATCHLIST,
		TOPOLOGY_13_CONTROL_POINT_PATCHLIST,
		TOPOLOGY_14_CONTROL_POINT_PATCHLIST,
		TOPOLOGY_15_CONTROL_POINT_PATCHLIST,
		TOPOLOGY_16_CONTROL_POINT_PATCHLIST,
		TOPOLOGY_17_CONTROL_POINT_PATCHLIST,
		TOPOLOGY_18_CONTROL_POINT_PATCHLIST,
		TOPOLOGY_19_CONTROL_POINT_PATCHLIST,
		TOPOLOGY_20_CONTROL_POINT_PATCHLIST,
		TOPOLOGY_21_CONTROL_POINT_PATCHLIST,
		TOPOLOGY_22_CONTROL_POINT_PATCHLIST,
		TOPOLOGY_23_CONTROL_POINT_PATCHLIST,
		TOPOLOGY_24_CONTROL_POINT_PATCHLIST,
		TOPOLOGY_25_CONTROL_POINT_PATCHLIST,
		TOPOLOGY_26_CONTROL_POINT_PATCHLIST,
		TOPOLOGY_27_CONTROL_POINT_PATCHLIST,
		TOPOLOGY_28_CONTROL_POINT_PATCHLIST,
		TOPOLOGY_29_CONTROL_POINT_PATCHLIST,
		TOPOLOGY_30_CONTROL_POINT_PATCHLIST,
		TOPOLOGY_31_CONTROL_POINT_PATCHLIST,
		TOPOLOGY_32_CONTROL_POINT_PATCHLIST,
		TOPOLOGY_UNDEFINED2
	};

	struct Clear_t
	{
		DWORD m_dwColor; //argb format
		FLOAT m_flDepth;
		BYTE m_Stencil;
		UINT m_uFlags;
	};

	virtual void function0() PURE;
	virtual unsigned int GetUINT0x4(__int64 a2) PURE; // func = return *(uint*)(a2+4);
	virtual BOOL sub_144FCC1D0(__int64 a2, __int64 a3) PURE;
	virtual BOOL function3() PURE; 
	virtual BOOL MapResource3ReadOnly(__int64 pResource) PURE;
	virtual BOOL MapResource3(__int64 pResource) PURE;
	virtual BOOL UnmapResource3(__int64 pResource) PURE;
	virtual BOOL MapResource2(void** ppMappedData, __int64 pResource) PURE;
	virtual BOOL UnmapResource2(__int64 pResource) PURE;
	virtual BOOL MapResource(void** ppMappedData, __int64 pResource) PURE;
	virtual BOOL UnmapResource(__int64 pResource) PURE;
	virtual BOOL Clear(Clear_t* pClearArgs) PURE;
	virtual BOOL function12() PURE;
	virtual BOOL SetScissorRect(int x, int y, int width, int height) PURE;
	virtual BOOL SetViewport(int x, int y, int width, int height, float min_depth, float max_depth) PURE;
	virtual BOOL function15() PURE;
	virtual BOOL function16() PURE;
	virtual BOOL function17() PURE;
	virtual BOOL function18() PURE;
	virtual BOOL function19() PURE;
	virtual BOOL function20() PURE; 
	virtual BOOL SetInputLayout(CVertexLayout* pVertexLayout) PURE;
	virtual BOOL SetVertexBuffer(UINT index, void* pVertexBuffer) PURE;
	virtual BOOL InvalidateVertexBuffer(UINT index) PURE;
	virtual BOOL SetIndexBuffer(void* pIndexBuffer) PURE;
	virtual BOOL InvalidateIndexBuffer() PURE;
	virtual BOOL SetVertexShader(CVertexShader* pShader) PURE;
	virtual BOOL SetPixelShader(CPixelShader* pShader) PURE;
	virtual BOOL function28() PURE;
	virtual BOOL function29() PURE;
	virtual BOOL function30() PURE;
	virtual BOOL function31() PURE;
	virtual BOOL function32() PURE;
	virtual BOOL function33() PURE; 
	virtual BOOL ClearUnorderedAccessViewUint(__int64 a2, const UINT a3) PURE;
	virtual BOOL function35() PURE;
	virtual BOOL function36() PURE;
	virtual BOOL function37() PURE;
	virtual BOOL SetBlendState(__int64 p) PURE;
	virtual BOOL SetRasterizerState(__int64 p) PURE;
	virtual BOOL SetDepthStencilState(__int64 p) PURE;
	virtual BOOL SetBlendFactor(UINT color) PURE;
	virtual BOOL SetTopology(Topology topology) PURE;
	virtual BOOL Begin(__int64 p) PURE;
	virtual BOOL End(__int64 p) PURE;
	virtual BOOL DrawHud(RenderInfo* pInfo) PURE; // not 100% on the names
	virtual BOOL DrawUnk(RenderInfo* pInfo) PURE;
	virtual BOOL DrawUnk2(RenderInfo* pInfo) PURE;
	virtual BOOL DrawWorldModels(RenderInfo* pInfo) PURE;
	virtual BOOL DrawUnk3(RenderInfo* pInfo) PURE;
	virtual BOOL DrawUnk4(RenderInfo* pInfo, __int64 a3, unsigned int AlignedByteOffsetForArgs) PURE;
	virtual void Flush() PURE;
	virtual int GetWidth() PURE;
	virtual int GetHeight() PURE;

	//void* m_pVtbl;							//0x0000
	ID3D11DeviceContext* m_pContext;				//0x0008
	Vector4 m_vBlendFactor;							//0x0010
	CRenderTarget* m_pRenderTarget;					//0x0020
	void* ptrs[4];									//0x0028
	CDepthSurface* m_pDepthSurface;					//0x0048
	CVertexBuffer* m_pVertexBuffer;					//0x0050
	ID3D11VertexShader* m_pVertexShader;			//0x0058
	ID3D11RenderTargetView* m_pRenderTargetView[4];	//0x0060
	ID3D11DepthStencilView* m_pDepthStencilView;	//0x0080
	CDepthSurface* m_pDepthSurface2;				//0x0088
	CVertexBuffer* m_pVertexBuffer2;				//0x0090
	CGraphicShader m_shader;						//0x0090
	char extra2[816];								//0x0198
	void* m_pIface;									//0x0578
};
IS_SIZE_CORRECT(CGraphicContextDx11, 1408)

class CDisplay
{
public:
	void* m_pVtble;							//0x0000
	DWORD dwUnk0x0008;						//0x0008
	int m_iWidth;							//0x000C
	int m_iHeight;							//0x0010
	DWORD dwUnk0x0014;						//0x0014
	void* m_pConstantBuffer[2];				//0x0018
	char _0x0030[40];						//0x0030
	IDXGISwapChain** m_ppSwapChain;			//0x0058
	char _0x0060[8];						//0x0060
	int m_iUnk;								//0x0068
	char _0x006C[4];						//0x006C
	CGraphicDeviceDx11* m_pGraphicDevice;   //0x0070
};

// Size = 0xF0 (240) bytes
class CGraphics
{
public:
	void* unk;							//0x0000
	DWORD dw0x04;						//0x0004
	char unk0x08[4];					//0x0008
	void* m_pSamplerState;				//0x0010
	DWORD dw0x18;						//0x0018
	char unk0x1C[4];					//0x001C
	CGraphicContextDx11* m_pContext;	//0x0020
	CDisplay m_Display;					//0x0028
	void* m_pUnk98;						//0x0098
	CRITICAL_SECTION m_CriticalSection;	//0x00A0
	BOOL m_bCriticalSectionInitalized;	//0x00C8
	char padCC[36];						//0x00CC
};
IS_OFFSET_CORRECT(CGraphics, m_Display, 0x28)
IS_OFFSET_CORRECT(CGraphics, m_CriticalSection, 0xA0)
IS_SIZE_CORRECT(CGraphics, 240)

struct ReadWriteLock
{
  CRITICAL_SECTION m_CriticalSection;
  BOOL m_bCriticalSectionInitalized;
};

class COsMainWindow
{
public:
	void* m_vtbl;
	HWND m_hWindow;
	union 
	{
		struct { POINT m_WindowPosition; INT m_iWidth; INT m_iHeight; };
		RECT m_windowRect;
	};
	BYTE gap20[4];
	BOOL m_bUpdateRect;
	BYTE gap28[8];
	BOOL m_bMouse[3];
};

struct CSaveSlot
{
	DWORD dwAccountId;
	BOOL InUse;
};

/*
Size of the struct is 0xD0 (208) bytes
*/
struct CSaveDataDevice
{
	void* m_pVtable;				//0x0000
	DWORD dwUnk0x08;				//0x0008
	char _0x000C[4];				//0x000C
	CSaveSlot* pSaveSlots;			//0x0010
	int nMaxSlot;					//0x0018
	DWORD dwError;					//0x001C
	QWORD qw0x0020;					//0x0020
	int i0x0028;					//0x0028
	char _0x002C[4];				//0x002C
	union
	{
		QWORD qwFlags;				//0x0030
		struct
		{
			DWORD dwFlags;			//0x0030
			DWORD dwStatus;			//0x0034
		};
	};
	int nSlot;						//0x0038
	char unk0x003C[4];				//0x003C | Aligment maybe
	HANDLE hFile;					//0x0040
	OVERLAPPED overlapped;			//0x0048
	DWORD nBytesToIO;				//0x0068 | maybe nBytesToIO and nBytesIO are a struct
	DWORD nBytesIO;					//0x006C
	void* pSlotDataBuffer;			//0x0070
	QWORD qwSlotDataBufferSize;		//0x0078
	void* pGameDataBuffer;			//0x0080
	QWORD qwGameDataBufferSize;		//0x0088
	void* pSystemDataBuffer;		//0x0090 | includes screen dimensions
	QWORD qwSystemDataBufferSize;	//0x0098
	void* pBuffer;					//0x00A0
	QWORD nBytesIO_SystemData;		//0x00A8
	QWORD nBytesIO_GameData;		//0x00B0
	QWORD nBytesIO_SaveData;		//0x00B8
	DWORD dwUnk0xC0;				//0x00C0
	DWORD dwUnk0xC4;				//0x00C4
	QWORD qwUnk0xC8;				//0x00C8 | this is an inline struct
	DWORD dwUnk0xD0;				//0x00D0 | this is a member of unk0xc8
	DWORD dwUnk0xD4;				//0x00D4 | this is a member of unk0xc8
};

class CUserInfoBase
{
	virtual ~CUserInfoBase();
};

struct CpkMountInfo
{
	const char* m_szName;
	int m_iLoadOrder;
};

struct CpkLoader
{
	DWORD dwCpkCount;											//0x00
	char alignment[4];											//0x04
	void(*LoadCpks)(unsigned int index, const char* szCpkName); //0x08
	void(*UnloadCpks)(unsigned int index);						//0x10
	QWORD qw0x18;												//0x18
	DWORD dwMaxCpkCount;										
	QWORD qwMaxCpkCount;
};

struct CpkLoad_t
{
	signed int status;
	BYTE gap4[4];
	QWORD ptr8;
	void *gap10;
	BYTE gap18[8];
	PBYTE pBuffer;
	DWORD hBinder;
	BYTE gap28[28];
	QWORD szVersion;
	void *gap50;
	BYTE gap58[368];
	DWORD dword1C8;
	BYTE gap1CC[4];
	QWORD qword1D0;
	QWORD qword1D8;
	QWORD qword1E0;
	const char* szCpkPath;
	DWORD dword1F0;
	DWORD dword1F4;
	void *ptr1F8;
	DWORD dword200[2];
	void *fnptr208;
	void *fnptr210;	
	CpkLoad_t *pNext;
	void *fnptr220;
	void *fnptr228;
	void *fnptr230;
	void *fnptr238;
	DWORD dword240;
	DWORD dword244;
	DWORD dword248;
	DWORD dword24C;
	DWORD dword250;
	DWORD dword254;
	DWORD dword258;
	DWORD dword25C;
	DWORD dword260;
	DWORD dword264;
	DWORD dword268;
	DWORD dword26C;
	DWORD dword270;
	DWORD dword274;
	DWORD dword278;
	DWORD dword27C;
	DWORD dword280;
	DWORD dword284;
	DWORD dword288;
	DWORD dword28C;
	DWORD dword290;
	DWORD dword294;
	DWORD dword298;
	DWORD dword29C;
	DWORD dword2A0;
	DWORD dword2A4;
	DWORD dword2A8;
	DWORD dword2AC;
	const char *sz2B0;
	QWORD dword2B8;
	DWORD dword2C0;
	QWORD dword2C4;
};

struct CpkBinderHandle
{
	void *ptr0;
	BYTE gap0[16];
	QWORD fnptr18;
	BYTE gap20[8];
	DWORD dword28;
	DWORD dword2C;
	DWORD dword30;
	const char* szCpkName;
	BYTE gap40[24];
	QWORD qword58;
	CpkLoad_t* pCpkLoad;
};

struct CpkEntry
{
	BOOL m_bInUse;
	DWORD dw2;
	CpkBinderHandle *m_pBinderHandle;
	DWORD m_binderid;
	DWORD m_status;
	int m_iLoadOrder;
	DWORD dw7;
};

struct CHeapInstance;

template<typename T>
struct ConstructionInfo
{
	int m_iObjectId;					//0x0000
	char alignment[4];					//0x0004
	T* (*Constructor)(CHeapInstance**); //0x0008 | not a CHeapInstance**, some parent struct
	QWORD id;							//0x0010
	const char* szName;					//0x0018
	void* pUnk;							//0x0020
};

template<typename T>
struct UIConstructorInfo
{
	DWORD id;							//0x0000
	char bs[12];						//0x0004
	T* (*Constructor)(CHeapInstance**); //0x0010 | not a CHeapInstance**, some parent struct
};

struct CSceneEntitySystem
{
	char pad0[32];
	CRITICAL_SECTION m_CriticalSection;
	BOOL m_bCriticalSectionInitalized;
	DWORD dw0x54;
	DWORD dw0x58;
	DWORD dw0x5C;
	QWORD dw0x60;
	QWORD dw0x68;
	QWORD dw0x70;
	QWORD dw0x78;
	QWORD sptr0x80;
	QWORD qw0x88;
	QWORD dw0x90;
	QWORD dw0x98;
};

//max size 0xC0
struct set_info_t
{
	// pretty sure all these vec 0 - 48 are just a mat4x4 -- it get initalized to an indenity 4x4 matrix
	Matrix4x4 m_mat;
	Vector3Aligned m_vPosition;
	Vector3Aligned m_vRotation;
	Vector3Aligned m_vScale;
	DWORD m_dw0x70;
	DWORD m_dw0x74;
	INT m_i0x078;
	INT m_i0x07C;
	INT m_i0x080;
	INT m_i0x084;
	INT m_i0x088;
	INT m_i0x08C;
};

struct Create_t
{
	const char* m_szName;			//0x0000
	unsigned int m_ObjectIds[2];	//0x0008
	set_info_t* m_pSetInfo;			//0x0010
	DWORD m_dw0x18;					//0x0018 | this is the same as set_info_t::m_dw0x70
	DWORD m_dwFlags;				//0x001C
	BOOL m_b0x0020;					//0x0020
	char alignment24[4];			//0x0024
	void* m_pWMBBuffer;				//0x0028
	void* m_unk;					//0x0040
	void* m_pDatBuffers;			//0x0048
	Vector3Aligned m_vec;			//0x0050
	QWORD m_0x60;					//0x0060
};

struct Create2_t
{
	CSceneEntitySystem* m_pSceneEntitySystem;
	void* ptr8;
	Create_t* m_pCreate;
	BOOL m_bSetInfo;
};

struct HeapAlloc_t
{
	LPVOID Pointer;
	BOOL Succeeded;
};

/*
Size of struct is 0x98 (152) bytes
*/
struct CHeapInstance
{
	virtual void fn0();
	virtual void fn1();
	virtual void fn2();
	virtual void fn3();
	virtual void* Alloc(__int64 nBytes, unsigned __int64 align, int flags);


	CRITICAL_SECTION m_CriticalSection;		//0x0008
	BOOL m_bCriticalSectionInitalized;		//0x0038
	char align2C[4];						//0x003C
	HeapAlloc_t* m_pAllocation;				//0x0040
	BYTE gap48[4];							//0x0048
	BOOL m_b0x4C;							//0x004C
	BYTE gap50[44];							//0x0050
	QWORD m_qwMemoryAlignment;				//0x0078
	DWORD dword84;							//0x0080
	DWORD dword88;							//0x0084
	DWORD m_dwReferenceCount;				//0x0088
	DWORD dword8c;							//0x008C
	DWORD dword90;							//0x0090
	DWORD dword94;							//0x0094
};
IS_SIZE_CORRECT(CHeapInstance, 0x98)

struct HeapAllocInfo
{
	BYTE gap0[16];
	CHeapInstance *m_pHeap;
	HeapAllocInfo* m_pNext;
	BYTE gap2[32];
	QWORD m_nSize;
	__int64 *m_pStruct;
};

struct CMemoryDevice
{
	virtual __int64 dtor();
	virtual BOOL CreateHeap(void** ppMem, SIZE_T nByteSize);
	virtual BOOL FreeHeap(VOID** ppMem);
	virtual __int64 FindHeap(int index);

	CRITICAL_SECTION m_CriticalSection; //0x00
	BOOL m_bCriticalSectionInitalized;	//0x28
	CHeapInstance m_inst;
	void *m_pMemory;
	void *m_punks[2];
	BOOL m_bHeapAllocated;
	char align[4];
	HANDLE m_hHeap;
};

class HandlerBase;

/*

Size of struct is 0xC (12) bytes
*/
class CallbackInstalled
{
	DlcInstalled_t m_installed;
	BOOL m_bInstalled;
	DWORD dwUnk;
};

class CGameContentDevice;

/*
Size of is 0x38 (56) bytes
*/
class CGameContentDeviceSteam
{
	virtual void* function0();
	virtual void UnloadContent(CGameContentDevice* pCGameContentDevice);
	virtual void LoadContent(CGameContentDevice* pGameContentDevice);
public:
	//void* m_pVtbl;															//0x0000
	CCallback<CGameContentDeviceSteam, DlcInstalled_t, false> m_Callback;	//0x0008 | id is 1005
	CallbackInstalled* m_pItems;											//0x0028
	DWORD m_nItems;															//0x0030
	BOOL m_Unloaded;														//0x0034 | not sure
};
IS_SIZE_CORRECT(CGameContentDeviceSteam, 56)

/*
Size of is 0x48 (72) bytes
*/
class CGameContentDevice
{
public:
	void* m_pVtbl;													//0x0000
	HandlerBase* m_pHandler;										//0x0008
	DWORD m_nCpkCount;												//0x0010 
	DWORD unk0x014;													//0x0014 
	DWORD unk0x18;													//0x0018
	void(* LoadCpks)(unsigned int index, const char* szCpkName);	//0x0020
	void(* UnloadCpks)(unsigned int index);							//0x0028
	CHeapInstance** m_ppHeap;										//0x0030 | probably a parent pointer
	CGameContentDeviceSteam* m_pSteamContent;						//0x0038
	DWORD unk0x40;													//0x0040
};
IS_SIZE_CORRECT(CGameContentDevice, 72)

/*
Size of is 0x20 (32) bytes

Max achievment index = (0x2F) 47
*/
class CAchievementDeviceSteam
{
	virtual ~CAchievementDeviceSteam();

	virtual __int64 sub_1450F7720();
	virtual BOOL SetAchievement(unsigned int index);
	virtual BOOL IsAchievementUnlocked(unsigned int index);
	virtual BOOL IsUserLoggedOn();
	virtual BOOL ReturnTrue();
	virtual BOOL SetVar0x18True(BOOL b);

	//void* m_pVtable;					//0x00
	ISteamUser019* m_pUser;				//0x08
	ISteamUserStats011* m_pUserStats;	//0x10
	void* m_pUnknown;					//0x18 | DWORD
};
IS_SIZE_CORRECT(CAchievementDeviceSteam, 32)

/*
Size of is 0x68 (104) bytes
*/
class CAchievementDevice
{
	QWORD qw0x00;
	QWORD qw0x08;
	DWORD dw0x10;
	DWORD dw0x14;
	QWORD qw0x18;
	QWORD qw0x20;
	CRITICAL_SECTION m_CriticalSection;
	BOOL m_bCriticalSectionValid;
	CAchievementDeviceSteam* m_pAchievementDeviceSteam;
	UINT m_uMaxAchievement;
};
IS_SIZE_CORRECT(CAchievementDevice, 104)

/*
Size of is 0x78 (120) bytes
*/
class CRankingDevice
{
	void* m_pVtable;			//0x00
	char _0x08[112];			//0x08
};
IS_SIZE_CORRECT(CRankingDevice, 120)

/*
Size of is 0x40 (64) bytes
*/
class COsSystemDeviceSteam 
{
public:
	virtual __int64 sub_144F5DAA0(char a2); //destructor???
	virtual __int64 RunCallbacks();
	virtual __int64 InitContext(void* p);
	virtual int GetVar0x38();

	//void* m_pVtable;		//0x00
	CCallback<COsSystemDeviceSteam, GameOverlayActivated_t, false> m_overlay; //0x08 id - 331
	BOOL m_bInitalized;		//0x28
	char pad2[4];			//0x2C
	ISteamUser019* m_pUser;	//0x30
	int _0x038;				//0x38
	char pad3[4];			//0x3C
};
IS_SIZE_CORRECT(COsSystemDeviceSteam, 64)

/*
Size of is 0x20 (32) bytes
*/
class CUserSignInProcess
{
public:
	void* m_pVtbl;			//0x00
	void* pUnk0x08;			//0x08
	char unknown[16];		//0x10
};
IS_SIZE_CORRECT(CUserSignInProcess, 32)

/* 
This struct is an oddball and is quite fucked

Size of is 0x18 (24) bytes
*/
class CGameBootProcess
{
public:
	void* m_pVtbl;			//0x00
	BOOL m_bInitialized;	//0x08
	DWORD dwordC;			//0x0C
	DWORD dword10;
	DWORD dword14;
};
IS_SIZE_CORRECT(CGameBootProcess, 24)

struct Task;

struct TaskInfo
{
	Task* m_pParent;
	void(*m_pCallback)();
	QWORD qword10;
	DWORD dword18;
	DWORD dword1C;
	const char* m_szTaskName;
	DWORD dword28;
	BYTE gap2C[28];
	TaskInfo* m_pPrevious;
	TaskInfo* m_pNext;
};


/*
Size of struct is 0x60 (96) bytes
*/
struct Task
{
	DWORD dwTaskCount;
	BYTE pad[4];
	TaskInfo* m_pHead;
	TaskInfo* m_pInfo2;
	TaskInfo* m_pInfo;
	QWORD qword20;
	DWORD gap28;
	Task* m_pNext;
	HANDLE hSemaphore;
	HANDLE hSemaphore2;
};

struct WetObjManagerDelay
{
	float m_flMaxDelay;
	float m_flDelay;
};

/*
Address = 0x1417BFD48
*/
struct WetObjManager
{
	CRITICAL_SECTION m_CriticalSection;
	BOOL m_bCriticalSectionInitalized;
	char pad[4];
	EntityHandle m_Localhandles[2];
	WetObjManagerDelay m_WetDelays[2];
	EntityHandle m_EntityHandles[256];
	EntityHandle m_SoundHandles[32];
};

class YorhaManager
{
public:
	void* m_pvtable;				//0x0000
	Array<EntityHandle> m_handles;	//0x0008
};

class FlightSuitManager
{
public:
	virtual void function0(); //maybe constructor
	virtual EntityHandle GetHandle(Pl0000* pEntity);
	virtual Array<EntityHandle>* GetHandles();
	virtual Pl0000* GetClosestEntity(Vector3Aligned* pvPosition, Pl0000* pEntity);
	virtual unsigned int CountEntitiesWithinDistance(Vector3Aligned* pvPosition, float max_distance);
	virtual void function4();

	//void* m_pvtable;				//0x0000
	Array<EntityHandle> m_handles;	//0x0008
};

/* incomplete getters say this exceeds 0x434 */
class NPCManager
{
public:
	virtual void function0(); //maybe constructor
	virtual EntityInfo* sub_1439C5CA0(Pl0000* pEntity);
	virtual Array<EntityHandle>* GetHandles();
	virtual __int64 GetField428();
	virtual DWORD* GetField42C(OUT DWORD* a1);
	virtual BOOL GetField430(); // *(_DWORD *)(pNPCManager + 0x430) = 1;
	virtual EntityHandle* GetEntityHandle(OUT EntityHandle* pHandle); //field - 0x434
	virtual void function7();
	virtual void function8();

	//void* m_pvtable;				//0x0000
	Array<EntityHandle> m_handles;	//0x0008
};


// Size of struct 0x1520 (5408) bytes
class EmBaseManager
{
public:
	virtual void function0(); //maybe constructor
	virtual void function1();
	virtual void function2();
	virtual void function3();
	virtual void function4();
	virtual void function5();
	virtual void InvalidateHandle(); //sets 0x14bc handle to 0 
	virtual void function7();
	virtual void function8();
	virtual Array<EntityHandle>* GetHandles();
	virtual unsigned int GetHandleIndex();
	virtual Array<EntityHandle>* GetHandles2();
	virtual Pl0000* GetEntity();  // from the 0x14bc handle
	virtual Pl0000* GetEntity2(); // from the 0x14bc handle
	virtual Array<EntityHandle>* GetHandles3();
	virtual void SetField14B8(float a1);
	virtual Array<EntityHandle>* GetHandles4();
	virtual Pl0000* function17(Vector3Aligned* pvPosition, int a2);
	virtual Array<EntityHandle>* GetHandles5();
	virtual Pl0000* function19(Vector3Aligned* pvPosition, int a2);
	virtual Pl0000* function20(Vector3Aligned* pvPosition, int a2);
	virtual bool function21(Pl0000* pEntity);
	virtual bool function22(EntityInfo* pInfo); //calls/returns vfunc 21
	virtual bool function23(EntityHandle* pHandle); //calls/returns vfunc 21
	virtual bool function24(Pl0000* pEntity);
	virtual bool function25(EntityInfo* pInfo);//calls/returns vfunc 24
	virtual bool function26(EntityHandle* pHandle); //calls/returns vfunc 24
	virtual bool IsEntityValid(); //return GetEntity() != 0;
	virtual EntityHandle function28(EntityInfo* pInfo); //calls/returns IsEntityValid (vfunc 27)
	virtual void function29(EntityHandle* pHandle);
	virtual bool function30(Pl0000* pEntity);
	virtual bool function31(EntityInfo* pInfo); //calls/returns vfunc 30
	virtual bool function32(EntityHandle* pHandle); //calls/returns vfunc 30
	virtual void function33();
	virtual void function34();
	virtual bool IsFloatField14C8LessThanOrEqualToZero(); //return *(float *)(a1 + 0x14C8) <= 0.0;
	virtual void function36();
	virtual unsigned int GetField14CC();
	virtual unsigned int GetField14D0();
	virtual BOOL function39(); //could be bool tbh but uses eax not al
	virtual bool IsField1508GreaterThanZero(); //return *(_DWORD *)(a1 + 0x1508) > 0;
	virtual unsigned int CountEntitiesFromHandle2ObjectId(); //sub_1434B1740
	virtual unsigned int CountEntitiesFromHandle2();		 //sub_1434B5B10
	virtual void function43(int a2);
	virtual void function44();

	//void* m_pvtable;							//0x0000
	StaticArray<EntityHandle, 256> m_handles;	//0x0008
	StaticArray<EntityHandle, 256> m_handles2;	//0x0428
	char _0x0840[16];							//0x0840
	StaticArray<EntityHandle, 256> m_handles3;	//0x0858
	StaticArray<EntityHandle, 256> m_handles4;	//0x0C78
	StaticArray<EntityHandle, 256> m_handles5;	//0x1098
	char _0x14B0[4];							//0x14B0
	EntityHandle m_hEntity;						//0x14BC
	EntityHandle m_hEnt[7];						//0x14BC
	CRITICAL_SECTION m_CriticalSection;			//0x14D8
	BOOL m_bUnk;								//0x1500
};
IS_OFFSET_CORRECT(EmBaseManager, m_handles, 0x8)
IS_OFFSET_CORRECT(EmBaseManager, m_handles2, 0x428)
IS_OFFSET_CORRECT(EmBaseManager, m_handles3, 0x858)
IS_OFFSET_CORRECT(EmBaseManager, m_handles4, 0xC78)
IS_OFFSET_CORRECT(EmBaseManager, m_handles5, 0x1098)
IS_OFFSET_CORRECT(EmBaseManager, m_hEntity, 0x14BC)

struct SteamContext
{
	ISteamClient017* pISteamClient;
	ISteamUser019* pISteamUser;
	ISteamFriends015* pISteamFriends;
	ISteamUtils008* pISteamUtils;
	ISteamMatchmaking009* pISteamMatchMaking;
	ISteamUserStats011* pISteamUserStats;
	ISteamApps008* pISteamApps;
	ISteamMatchmakingServers002* pISteamMatchMakingServers;
	ISteamNetworking005* pISteamNetworking;
	ISteamRemoteStorage014* pISteamRemoteStorage;
	ISteamScreenshots003* pISteamScreenshots;
	ISteamHTTP002* pISteamHTTP;
	ISteamUnifiedMessages001* pISteamUnifiedMessages;
	ISteamController005* pISteamController;
	ISteamUGC009* pISteamUGC;
	ISteamAppList001* pISteamAppList;
	ISteamMusic001* pISteamMusic;		
	ISteamMusicRemote001* pISteamMusicRemote;
	ISteamHTMLSurface003 *pISteamHTMLSurface;
	ISteamInventory001* pISteamInventory;
	ISteamVideo001* pISteamVideo;
};

class CUserInfo
{
	virtual ~CUserInfo();

	BOOL m_bLoggedOn;	//0x08
	char _0x0C[4];		//0x0C
	int idk;			//0x10
	BOOL m_bInit;		//0x14
	int index;			//0x18
	char _0x1C[4];		//0x1C	
};

/* Size of struct 0x108 (264) bytes	*/
class CUserManager
{
public:
	virtual CUserManager* Clear(BYTE bFlags);

	CRITICAL_SECTION m_CriticalSection;			//0x0008
	BOOL m_bCriticalSectionValid;				//0x0030
	char alignment[4];							//0x0034
	CUserInfo m_UsersInfo[4];					//0x0038
	int m_iActiveUser;							//0x00B8
	int m_iUserIndices[3];						//0x00BC
	DWORD dwUnknown;							//0x00C8
	QWORD qw0x00D0;								//0x00D0	
	CGameBootProcess* m_pBootProcess;			//0x00D8
	CUserSignInProcess* m_pUserSignInProcess;	//0x00E0
	CGameContentDevice* m_pGameContentDevice;	//0x00E8
	CSaveDataDevice* m_pSaveDataDevice;			//0x00F0
	CAchievementDevice* m_pAchivementDevice;	//0x00F8
	DWORD _0x100;								//0x0100
};
IS_SIZE_CORRECT(CUserManager, 264)
IS_OFFSET_CORRECT(CUserManager, m_UsersInfo, 0x38)
IS_OFFSET_CORRECT(CUserManager, m_iActiveUser, 0xB8)
IS_OFFSET_CORRECT(CUserManager, m_iUserIndices, 0xBC)
IS_OFFSET_CORRECT(CUserManager, m_pBootProcess, 0xD8)
IS_OFFSET_CORRECT(CUserManager, m_pSaveDataDevice, 0xF0)

struct EventFiber;

struct EventEntry
{
	void* placeholder;
};

struct MrubyImpl
{
	void* m_vtbl;
	QWORD qw8[2];
	StaticArray<EventEntry, 256> m_events;
	StaticArray<EventEntry, 256> m_events2;
	StaticArray<std::pair<unsigned int, EventFiber*>, 16> m_fibers;
	QWORD qword1180;//StaticArray<std::pair<lib::HashedString<sys::StringSystem::Allocator>,lib::HashedString<sys::StringSystem::Allocator>>,256,8>
	QWORD qword1188;
	QWORD qword1190;
	QWORD qword1198;
	BYTE gap11A0[4104];
	QWORD qword21A8;
	QWORD extra[5];
};
IS_OFFSET_CORRECT(MrubyImpl, m_fibers, 0x1058)
IS_SIZE_CORRECT(MrubyImpl, 8656)