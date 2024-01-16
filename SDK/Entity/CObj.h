#pragma once

#include "..\Common.h"

#include "..\FileSystem\dat.h"

#include "CModel.h"

// Forward Definitions
class CBehaviorAppBase;

// Struct at least 0x1A8 bytes
struct CEntityInfo
{
	Unknown_t* m_pUnknown;						//0x0000 | i don't really know what this is (confirmed a struct pointer) maybe a void*
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

	virtual void function0();
	virtual void function1(void); // from CModel
	virtual void function2(void); // CModel stub function
	virtual void* function3(void);
	virtual float function4(void); // return 1.0f;

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
	char _0x0658[24];						//0x00658
};
VALIDATE_OFFSET(CObj, m_pTextureData, 0x5B0);
VALIDATE_OFFSET(CObj, m_ObjectId, 0x5B8);
VALIDATE_SIZE(CObj, 0x670);

