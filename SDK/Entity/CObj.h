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
//  32								   0
//   SSSSSSSS IIIIIIIIIIIIIIII RRRRRRRR
//  +--------+----------------+--------+
//  | Sector |      Index     |        |
//  +--------+----------------+--------+
//
//  Where
//
//      S - Sector - 8 bit shift value (0-255) resets every 256 items
//			therefore, grouping entities in 256 bundles
//
//      I - Index - 16 unsigned value
//          Index into the CEntityList::m_pItems
// 
//      R - reserved bits of the handle
//
typedef uint32_t EntityHandle;

#define EHANDLE_INDEX(ehandle) ((ehandle) >> 8) & 0xFFFF)
#define EHANDLE_SECTOR(ehandle) ((ehandle) >> 24) & 0xFF)
#define INVALID_EHANDLE_INDEX (0xFFFFFFFF)

#define MAX_ENTITIES_IN_SECTOR 256

// TODO: idk if my increment operators are correct!
// IDA code is so gay I can't grasp it well
typedef struct EntityHandle_t
{
	union {
		struct {
			uint32_t m_uReserved : 8;
			uint32_t m_uIndex : 16;
			uint32_t m_uSector : 8;
		} m_Parts;

		uint32_t m_uValue;
	};

	constexpr EntityHandle_t(void)
		: m_uValue(INVALID_EHANDLE_INDEX)
	{
	}

	EntityHandle_t(uint16_t uIndex);

	operator EntityHandle (void) { return m_uValue; }

	EntityHandle_t& operator++(void)
	{
		if (!(++m_Parts.m_uIndex & 0xFF)) // % 256
			++m_Parts.m_uSector;

		return *this;
	}

	EntityHandle_t& operator++(int32_t)
	{
		if (!(++m_Parts.m_uIndex & 0xFF)) // % 256
			++m_Parts.m_uSector;

		return *this;
	}

	template<typename T>
	T* GetEntity(const CEntityList* pList)
	{
		CEntityInfo* pEntityInfo = pList->m_pItems[m_Parts.m_uIndex].second;

		// Check if the entry is invalid
		if (!pEntityInfo || pEntityInfo->m_Flags & 3)
			return nullptr;

		return static_cast<T*>(pEntityInfo->m_pEntity);
	}

	inline uint32_t GetIndex(void) const { return m_Parts.m_uIndex; }
	inline uint32_t GetSector(void) const { return m_Parts.m_uSector; }

	inline uint32_t GetEntIndex(void) const
	{
		return m_Parts.m_uIndex + MAX_ENTITIES_IN_SECTOR * (m_Parts.m_uSector - 1);
	}

	inline bool operator!=(EntityHandle_t hOther) const { return (m_uValue != hOther.m_uValue); }
} EHANDLE;

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

// Struct at least 0x20 bytes
struct CEntityInfoUnk_t
{
	int32_t m_0x00;
	int32_t m_0x04;
	int32_t m_0x08;
	volatile int32_t m_nReferenceCount;	// 0x0C
	float m_flUnk0x10;					// 0x10
	float m_flTickBase;					// 0x14 local entity time coefficent
	float m_flUnk0x14;					// 0x18
	float m_flTickCoefficent;			// 0x14 local entity time coefficent
};

class CSceneEntitySystemUnk;

// TODO: figure out better names for these 
// 3 is valid, 0 is free/invalid
#define ENTITYINFO_FLAG_1 0x00000001
#define ENTITYINFO_FLAG_2 0x00000002
#define ENTITYINFO_FLAG_3 ( ENTITYINFO_FLAG_2 | ENTITYINFO_FLAG_1 )

// Struct at least 0x1A8 bytes
struct CEntityInfo
{
	typedef void (*DeleteFn)(CEntityInfo* pThis);

	CEntityInfoUnk_t* m_pUnknown;				//0x0000 | i don't really know what this is (confirmed a struct pointer)
	char m_szEntityType[32];					//0x0008
	uint32_t m_ObjectId;						//0x0028
	uint32_t m_Flags;							//0x002C  | (m_pEntity->flag0x05C4 | 1) An Entity cannot have this flag or'd with 3 game crashes (possibly a destroyed flag)
	EntityHandle_t m_hEntity;					//0x0030
	char _0x0038[4];							//0x0034
	CTextureData* m_pTextureData[2];			//0x0038
	CBehaviorAppBase* m_pEntity;				//0x0048
	DatafileDesc* m_pDatDesc;					//0x0050  | m_pWMB
	CSceneEntitySystemUnk* m_pUnk;				//0x0058  | m_pWTA (debug build) dword array 2 members (0x1415F6B50) CSceneEntitySystem::qword10
	CBehaviorAppBase* m_pParent;				//0x0060  | m_pWTP (debug build)
	BOOL m_bDataExists;							//0x0068
	uint32_t m_uEntityId;						//0x006C
	uint32_t m_uSetType;						//0x0070
	char _0x0x0074[68];							//0x0074
	EntityHandle_t m_hUnk;						//0x00B8
};
VALIDATE_OFFSET(CEntityInfo, m_pParent, 0x60);
VALIDATE_OFFSET(CEntityInfo, m_hUnk, 0xB8);

extern struct CEntityList* g_pEntityList;

//extern CEntityList* EntityList(void);

// Need to C++20 to have this inline in class
static constexpr EHANDLE INVALID_EHANDLE = {};

/*
Address = 14160DF88
*/
struct CEntityList
{
	// should do by classname & objectid iterator
	struct Iterator
	{
		Iterator(void)
		{
			First();
		}

		void First(void)
		{
			m_hIterator.m_uValue = (g_pEntityList->m_uBase << 24);
		}

		operator EntityHandle_t(void) { return m_hIterator; }

		EntityHandle_t& operator++(void)
		{
			++m_hIterator;

			if (m_hIterator.GetEntIndex() >= g_pEntityList->m_uSize)
				m_hIterator = INVALID_EHANDLE;

			return m_hIterator;
		}

		EntityHandle_t& operator++(int32_t)
		{
			++m_hIterator;

			if (m_hIterator.GetEntIndex() >= g_pEntityList->m_uSize)
				m_hIterator = INVALID_EHANDLE;
			
			return m_hIterator;
		}

		bool operator!=(const EntityHandle_t& hOther) const { return (m_hIterator != hOther); }

		EntityHandle_t m_hIterator;
	};

	struct IteratorByName : public Iterator
	{
		IteratorByName(const char* szClassname)
		{
			First();
		}

		EntityHandle_t& operator++(void)
		{
			++m_hIterator;

			if (m_hIterator.GetEntIndex() >= g_pEntityList->m_uSize)
			{
				m_hIterator = INVALID_EHANDLE;
				return m_hIterator;
			}

			for (; m_hIterator != INVALID_EHANDLE; ++m_hIterator)
			{

			}

			CEntityInfo* pInfo = g_pEntityList->GetEntityInfo(*this);

			if (pInfo && !strcmp(pInfo->m_szEntityType, m_szClassname))
			{

			}

			return m_hIterator;
		}

		EntityHandle_t& operator++(int32_t)
		{
			++m_hIterator;

			if (m_hIterator.GetEntIndex() >= g_pEntityList->m_uSize)
			{
				m_hIterator = INVALID_EHANDLE;
				return m_hIterator;
			}

			for (; m_hIterator != INVALID_EHANDLE; ++m_hIterator)
			{

			}

			CEntityInfo* pInfo = g_pEntityList->GetEntityInfo(*this);

			if (pInfo && !strcmp(pInfo->m_szEntityType, m_szClassname))
			{

			}

			return m_hIterator;
		}

		const char* m_szClassname;
	};

	uint32_t m_uItems;								//0x0000
	uint32_t m_uSize;								//0x0004
	uint32_t m_uBase;								//0x0008 | What the indices start at for handles
	uint32_t m_uSector;								//0x000C | Which group of 256
	std::pair<EntityHandle_t, CEntityInfo*>* m_pItems;//0x0010
	CReadWriteLock m_Lock;							//0x0018

	CEntityInfo* GetEntityInfo(EntityHandle_t hEntity)
	{
		if (hEntity.m_Parts.m_uIndex >= m_uItems)
			return nullptr;

		CEntityInfo* pEntityInfo = m_pItems[hEntity.m_Parts.m_uIndex].second;

		// Check if the entry is invalid
		if (!pEntityInfo || pEntityInfo->m_Flags & 3)
			return nullptr;

		return pEntityInfo;
	}

	// idk if CModel is proper
	template<typename T, std::enable_if_t<std::is_base_of_v<CModel, T>, bool> = true>
	T* GetEntity(EntityHandle_t hEntity)
	{
		if (hEntity.m_Parts.m_uIndex >= m_uItems)
			return nullptr;

		CEntityInfo* pEntityInfo = m_pItems[hEntity.m_Parts.m_uIndex].second;

		// Check if the entry is invalid
		if (!pEntityInfo || pEntityInfo->m_Flags & 3)
			return nullptr;

		return static_cast<T*>(pEntityInfo->m_pEntity);
	}

	// idk if CModel is proper
	template<typename T, std::enable_if_t<std::is_base_of_v<CModel, T>, bool> = true>
	T* FindEntity(const char* szClassname)
	{
		for (uint32_t i = 0; i < m_uSize; ++i)
		{
			CEntityInfo* pEntityInfo = m_pItems[i].second;

			if (!pEntityInfo)
				continue;

			if (!strcmp(pEntityInfo->m_szEntityType, szClassname))
				return GetEntity<T>(i);
		}
		return nullptr;
	}
};


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
			BIG_ENDIAN uint32_t m_uNameIndex;
			BIG_ENDIAN uint32_t m_u04;
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
	int32_t m_ObjectId;						//0x005B8 | 10000 = 2B | 10200 = 9S | 10203 = A2? (2B only accepts regular, mech suit, and static -1)
	uint32_t m_SetType;						//0x005BC
	uint32_t flag0x005C0;					//0x005C0 | one-way invisible 2B
	uint8_t _flag0x05C4;					//0x005C4 | disable dynamic skirt?
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
