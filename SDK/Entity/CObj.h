#pragma once

#include "..\Common.h"

#include "..\FileSystem\dat.h"

#include "CModel.h"

// Forward Definitions
class CBehaviorAppBase;
struct CEntityInfo;
struct CEntityList;

//
//  EntityHandles are 32 bit values laid out as follows:
//
//   RRRRRRRR IIIIIIIIIIIIIIII SSSSSSSS
//  +--------+----------------+--------+
//  |        |      Index     | Shift  |
//  +--------+----------------+--------+
//
//  where
//
//      S - Shift - 8 bit shift value (unknown use)
//
//      I - Index - 16 unsigned value
//          Index into the CEntityList::m_pItems
// 
//      R - reserved bits of the handle
//
typedef unsigned long EntityHandle;

#define EHANDLE_INDEX(ehandle) ((ehandle) >> 8) & 0xFFFF)
#define EHANDLE_SHIFT(ehandle) ((ehandle) & 0xFF)

struct CEntityInfoList
{
	CEntityInfo* m_pInfo;
	struct CEntityInfoList* m_pNext;
	struct CEntityInfoList* m_pPrevious;
};

struct CEntityListEntry
{
	EntityHandle m_hEntity;
	CEntityInfo* m_pInfo;
};

typedef struct EntityHandle_t
{
	union {
		struct {
			UINT m_uShift : 8;
			UINT m_uIndex : 16;
			UINT m_uReserved : 8;
		} m_Parts;

		UINT m_uValue;
	};

	operator EntityHandle (void) { return m_uValue; }

	template<typename T>
	T* GetEntity(const CEntityList* pList)
	{
		CEntityInfo* pEntityInfo = pList->m_pItems[m_Parts.m_uIndex].second;

		// Check if the entry is invalid
		if (!pEntityInfo || pEntityInfo->m_Flags & 3)
			return nullptr;

		return static_cast<T*>(pEntityInfo->m_pEntity);
	}

	inline UINT GetIndex(void) { return m_Parts.m_uIndex; }
	inline UINT GetShift(void) { return m_Parts.m_uShift; }
} EHANDLE;


/*
Address = 14160DF88
*/
struct CEntityList
{
	DWORD m_dwItems;								//0x0000
	DWORD m_dwSize;									//0x0004
	DWORD m_dwBase;									//0x0008 | What the indices start at for handles
	DWORD m_dwShift;								//0x000C
	std::pair<EntityHandle, CEntityInfo*>* m_pItems;//0x0010
	CReadWriteLock m_Lock;							//0x0018

	template<typename T>
	T* GetEntity(EntityHandle_t hEntity)
	{
		CEntityInfo* pEntityInfo = m_pItems[hEntity.m_Parts.m_uIndex].second;

		// Check if the entry is invalid
		if (!pEntityInfo || pEntityInfo->m_Flags & 3)
			return nullptr;

		return static_cast<T*>(pEntityInfo->m_pEntity);
	}
};

// Struct at least 0x20 bytes
struct CEntityInfoUnk_t
{
	int m_0x00;
	int m_0x04;
	int m_0x08;
	int m_0x0C;
	float m_flUnk0x10;			// 0x10
	float m_flTickBase;			// 0x14 local entity time coefficent
	float m_flUnk0x14;			// 0x18
	float m_flTickCoefficent;	// 0x14 local entity time coefficent
};

// Struct at least 0x1A8 bytes
struct CEntityInfo
{
	CEntityInfoUnk_t* m_pUnknown;				//0x0000 | i don't really know what this is (confirmed a struct pointer)
	char m_szEntityType[32];					//0x0008
	unsigned int m_ObjectId;					//0x0028
	BYTE m_Flags;								//0x002C  | An Entity cannot have this flag or'd with 3 game crashes (possibly a destroyed flag)
	char alignment[3];							//0x002D
	EntityHandle m_hEntity;						//0x0030
	char _0x0038[4];							//0x0034
	CTextureData* m_pTextureData[2];			//0x0038
	CBehaviorAppBase* m_pEntity;				//0x0048
	DatafileDesc* m_pDatDesc;					//0x0050  | m_pWMB
	DWORD* m_pUnk;								//0x0058  | m_pWTA (debug build) dword array 2 members (0x1415F6B50) CSceneEntitySystem::qword10
	CBehaviorAppBase* m_pParent;				//0x0060  | m_pWTP (debug build)
	BOOL m_bDataExists;							//0x0068
	UINT m_uEntityId;							//0x006C
	UINT m_uSetType;							//0x0070
	char _0x0x0074[68];							//0x0074
	EntityHandle m_hUnk;						//0x00B8
};
VALIDATE_OFFSET(CEntityInfo, m_pParent, 0x60);
VALIDATE_OFFSET(CEntityInfo, m_hUnk, 0xB8);

struct BXMEntry
{
	BIG_ENDIAN USHORT m_usCatMax;
	BIG_ENDIAN USHORT m_usCatOffset;
	BIG_ENDIAN USHORT m_usMax;
	BIG_ENDIAN USHORT m_usOffset;
};

struct BXMNameOridinal
{
	union
	{
		struct
		{
			BIG_ENDIAN UINT m_uNameIndex;
			BIG_ENDIAN UINT m_u04;
		};
		struct
		{
			BIG_ENDIAN USHORT m_usNameIndex;
			BIG_ENDIAN USHORT m_us04;
		};
	};
};


/*
	CXML is a pure virtual interface with all 82 vfuncs pure virtuals

	Size of struct is ?? (??) bytes
*/
struct CXmlBinary //: CXML
{
	void* lpVtbl;
	QWORD m_pHeap;	// don't exactly know what heap struct this is
	QWORD* m_pData;
	void* m_pBXM;
	BXMEntry* m_pEntries;  // m_pBXM + 16
	BXMNameOridinal* m_pNameOridinals; // m_dwFlags & 1 ? 8 * nQWords : 4 * nQWords;
	const char** m_pszNames;
	DWORD m_dwFlags; // 1u = m_uQWordCount >= 0xFFFF		
};

/*
* Size of struct 0x670 (1648) bytes
*
* Alignment: 32 bytes??
*/
class CObj : public CModel
{
public:

	virtual void function0(void) = 0;
	virtual void UpdateModelPhysics(Vector4* v1, Vector4* v2, Vector4* v3) = 0; // from CModel
	virtual void function2(void) = 0; // CModel stub function
	virtual void* GetTypeInfo(void) = 0;
	virtual float function4(void) = 0; // return 1.0f;

	DATHeader* m_pDataFiles[2];				//0x005A0
	CTextureData* m_pTextureData;			//0x005B0
	int m_ObjectId;							//0x005B8 | 10000 = 2B | 10200 = 9S | 10203 = A2? (2B only accepts regular, mech suit, and static -1)
	unsigned int m_SetType;					//0x005BC
	unsigned int flag0x005C0;				//0x005C0 | one-way invisible 2B
	BYTE _0x05C4;							//0x005C4 | disable dynamic skirt?
	BYTE _0x05C5;							//0x005C5
	char _0x05C6[2];						//0x005C6
	DWORD dw0x5C8;							//0x005C8
	float fl0x5CC[7];						//0x005CC
	Vector3 m_vInnerBrightness;				//0x005E8 (x = brightness [paired with z, not modifyable], y = unknown, z = brightness [paired with z, modifyable])
	void* _0x5F4;							//0x005F4
	void* _0x600;							//0x00600
	float fl0x60C;							//0x0060C
	CEntityInfo* m_pInfo;					//0x00610 
	CXmlBinary m_xmlBinary;					//0x00618
	BOOL m_bObjectRegistered;				//0x00658 | cObj::construct is registered multiple times.
	char _0x065C[20];						//0x0065C
};
VALIDATE_OFFSET(CObj, m_pTextureData, 0x5B0);
VALIDATE_OFFSET(CObj, m_ObjectId, 0x5B8);
VALIDATE_SIZE(CObj, 0x670);

