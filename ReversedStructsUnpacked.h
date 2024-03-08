#ifndef __REVERSEDSTRUCTSUNPACKED_H__
#define __REVERSEDSTRUCTSUNPACKED_H__

#include <d3d11.h>
#include <dxgi1_2.h>
#include <dinput.h>
#include "SteamApi.h"
#include "Math.h"


#include "SDK\Common.h"

#include "SDK\CCamera.h"

#include "SDK\Entity\CBehavior.h"
#include "SDK\Entity\Pl0000.h"
#include "SDK\Entity\Pl0013.h"
#include "SDK\Entity\Pl1010.h"


#define _MM_RSHUFFLE(mask, x, y, z, w) \
     (x) = ((mask & 0xC) >> 2);		   \
     (y) = ((mask & 0x30) >> 4);	   \
     (z) = ((mask & 0xC0) >> 6);	   \
     (w) = (mask & 0x3)

#define ALIGN(size, align) (((size) + (align) - 1) & ~(align))

typedef ULONGLONG QWORD;


// nier uses this https://github.com/rmind/tlsf
// debug build has strings


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

class Pl0000;

struct SoundEmitter
{
	Pl0000* m_pEntity; // this is some lower interface like behaviour
	int m_iBoneId;
};

class CUnknown_t
{
	DWORD id;			//0x0000 idk?
	char aligment[4];	//0x0004
	void* punk;			//0x0008
	void* punk2;		//0x0010
	void* pFunction;	//0x0018
};

struct CHeapInstance;

/*
	CXML is a pure virtual interface with all 82 vfuncs pure virtuals

	Size of struct is ?? (??) bytes
*/
//struct CXmlBinary //: CXML
//{
//	void* lpVtbl;
//	QWORD m_pHeap;	// don't exactly know what heap struct this is
//	QWORD* m_pData;
//	void* m_pBXM;
//	BXMEntry* m_pEntries;  // m_pBXM + 16
//	BXMNameOridinal* m_pNameOridinals; // m_dwFlags & 1 ? 8 * nQWords : 4 * nQWords;
//	const char** m_pszNames;
//	DWORD m_dwFlags; // 1u = m_uQWordCount >= 0xFFFF		
//};

struct CObjReadSystem
{
	// Size of struct  0x58
	struct Work
	{
		//  FileRead::Listener, Hw::cRBTreeNodeTemp<ObjReadSystem::Work>
		// Size of struct 0xB8
		struct CObjReadSystem::Work::Desc
		{
			DWORD dword0;
			CObjReadSystem::Work::Desc* m_pParent;
			CObjReadSystem::Work::Desc* m_pRight;
			CObjReadSystem::Work::Desc* m_pLeft;
			int m_Crc32;
			INT m_0x20;
			int gap2c;
			DWORD m_dw0x30;
			DWORD m_dwType;
			char m_szFilename[64];
			int m_objectId;
			DWORD m_dw0x78;
			QWORD m_qw0x80;
			SIZE_T m_nBytes;
			CHeapInstance* m_pHeap;
			DWORD m_dwFlags98;
			DWORD m_dw0x9C;
			DWORD m_dw0xA0;
			DWORD m_dw0xA4;
			CObjReadSystem::Work* m_pWork;
			char pad[8];
		};

		// this this is shit
		struct Info
		{
			BYTE gap0[32];
			DWORD m_crc32;
			CObjReadSystem::Work::Desc* m_pDescription;
		};

		void* m_pVtbl;							//0x0000
		char gap8[16];							//0x0008
		struct CObjReadSystem::Work* m_pPrev;	//0x0018
		struct CObjReadSystem::Work* m_pNext;	//0x0020
		DWORD m_flags;							//0x0028
		int m_ObjectId;
		int m_objectid2;
		int m_objectid3;
		CObjReadSystem::Work::Desc* m_pUnknown;	//0x0038
		HANDLE m_hSemaphore2;					//0x0040
		void* m_pDatPtr;						//0x0048
		void* m_pDatPtr2;						//0x0050
		void* m_unk1;							//0x0058
		void* m_unk2;
	};

	CReadWriteLock m_Lock;
	CReadWriteLock m_Lock2;
};


struct CTextureBatch;

struct TextureFile;


// they might be Vector4's
struct CAxisAlignedBoundingBox
{
	Vector3Aligned m_vMax;
	Vector3Aligned m_vMin;
};

struct CollisionInfo;

struct CCollisionUnk
{
	char gap00[8];
	CReadWriteLock m_Lock;
	BYTE gap34[60];
	QWORD qword70;
	BYTE gap78[16];
	DWORD dword88;
};

struct CCollisionUnknown
{
	CCollisionUnk* m_pUnknownArray;
	CCollisionUnk** m_ppUnknownArray;
	int m_nAllocatedEntries;
	int m_nFreeEntries;
};

struct CCollisionDataSegment
{
	void* m_pVtbl;
	BYTE gap8[8];
	Vector3Aligned m_vMin;
	Vector3Aligned m_vMax;
	QWORD qword30;
	QWORD qword38;
	QWORD qword40;
	QWORD qword48;
	QWORD qword50;
	QWORD qword58;
	QWORD qword60;
	QWORD qword68;
	BYTE gap70[4];
	QWORD qword74;
	DWORD dword7C;
	QWORD qword80;
};

struct CCollisionObjectName
{
	char* m_szName;
	int m_bInitalized;
};

struct CCollisionTreeNode;

// TODO:
class CCollisionDataObjectBase
{
public:
	virtual __int64 function0(char a1);
	virtual __int64 function1();
	virtual __int64 function2();
	virtual void function3(CollisionInfo* a2, __int64 a3, CAxisAlignedBoundingBox* a4);
	virtual void CheckSphere(CollisionInfo* a2, __int64 a3, CAxisAlignedBoundingBox* a4);
	virtual void function5(CollisionInfo* a2, __int64 a3, CAxisAlignedBoundingBox* a4);
	virtual void CheckBox(CollisionInfo* a2, __int64 a3, CAxisAlignedBoundingBox* a4);
	virtual void function7(float* a2, __int64 a3, CAxisAlignedBoundingBox* a4);
	virtual void CheckCapsule(CollisionInfo* a2, __int64 a3, CAxisAlignedBoundingBox* a4);

};

/*
* CCollisionDataObjectImpl : public CCollisionDataObjectBase
*
* Size of struct is 0x120 (288) bytes
*/
class CCollisionDataObject
{
public:
	virtual __int64 function0(char a1);
	virtual __int64 function1();
	virtual __int64 function2();
	virtual void function3(CollisionInfo* a2, __int64 a3, CAxisAlignedBoundingBox* a4);
	virtual void CheckSphere(CollisionInfo* a2, __int64 a3, CAxisAlignedBoundingBox* a4);
	virtual void function5(CollisionInfo* a2, __int64 a3, CAxisAlignedBoundingBox* a4);
	virtual void CheckBox(CollisionInfo* a2, __int64 a3, CAxisAlignedBoundingBox* a4);
	virtual void function7(float* a2, __int64 a3, CAxisAlignedBoundingBox* a4);
	virtual void CheckCapsule(CollisionInfo* a2, __int64 a3, CAxisAlignedBoundingBox* a4);

	//void* vtbl;						//0x0000
	//char pad[8];						//0x0008
	CAxisAlignedBoundingBox m_AABB;		//0x0010
	DWORD dword30;						//0x0030
	DWORD m_uScenePropIndex;			//0x0034 changes when a scene is loaded therefore appears to be an index
	DWORD dword38;						//0x0038
	BYTE gap3C[20];						//0x003C
	QWORD qword50;
	QWORD qword58;
	CCollisionDataSegment* qword60;
	CCollisionDataSegment* qword68;
	char gap5C[4];
	QWORD qword78;
	QWORD qword80;
	CCollisionObjectName* m_pNames;
	int m_nNames;
	char gap94[4];
	CCollisionDataSegment* m_pSegments;
	int m_nSegments;
	char gapA4[4];
	unsigned __int16* m_pBoneMaps;
	int m_nBoneMaps;
	char gapB4[74];
	QWORD qword100;
	CCollisionTreeNode* m_pCollisionTrees;
	CCollisionTreeNode* m_pCollisionTree;
	int m_nCollisionTreeNodes;
};
VALIDATE_OFFSET(CCollisionDataObject, m_AABB, 0x10);
VALIDATE_OFFSET(CCollisionDataObject, m_uScenePropIndex, 0x34);
VALIDATE_OFFSET(CCollisionDataObject, m_pSegments, 0x98);
VALIDATE_OFFSET(CCollisionDataObject, m_pCollisionTree, 0x110);
VALIDATE_SIZE(CCollisionDataObject, 0x120);

struct CCollisionDataEntry
{
	CCollisionDataObject* m_pObject;
	struct CCollisionDataEntry* m_pNext;
};

struct CCollisionDataObjectManager;

struct CCollisionData
{
	CCollisionDataEntry* m_pEntries;
	CCollisionDataEntry** m_ppEntries;
	int m_nAllocatedEntries;
	int m_nFreeEntries;
	CCollisionDataObjectManager* m_pOwner;
	CCollisionDataEntry* m_pEntries2;
};

struct CCollisionVertex
{
	Vector4 m_vPosition;
	Vector4 m_vBoneWeights;
	unsigned int m_uBones[4];
};

struct CCollisionBatch
{
	CCollisionVertex* m_pVertices;
	unsigned __int16* m_pIndices;
	int m_iBoneIndex;
	int m_iVertexCount;
	int m_iIndiceCount;
	int m_iPrimitiveCount;
};

struct CCollisionMesh
{
	DWORD m_uBatchType;
	int m_iNameIndex;
	DWORD m_uFlags;
	BYTE gapC[4];
	DWORD dword10;
	BYTE gap14[4];
	CCollisionBatch* m_pBatches;
	int m_iBatchCount;
};

struct CCollisionTreeNode
{
	Vector3Aligned m_vP1;
	Vector3Aligned m_vP2;
	unsigned __int16* m_pMeshIndices;
	int m_uMeshIndexCount;
	int m_iLeft;
	int m_iRight;
	int m_iUnk;
};

struct CCollisionContext
{
	unsigned int m_uReferenceCount;
	char** m_pNames;
	int m_nNameCount;
	CCollisionMesh* m_pMeshes;
	int m_iMeshCount;
	QWORD m_pBoneMap;
	DWORD m_uBoneMapCount;
	QWORD m_pBoneMap2;
	DWORD m_uBoneMap2Count;
	QWORD m_pMeshMaps;
	DWORD m_uMeshCount;
	CCollisionTreeNode* m_pColTree;
	int m_iColTreeNodeCount;
	QWORD m_pPrimitives;
	QWORD qword70;
};

struct COLHdr
{
	union {
		char id[4];
		unsigned int magic;
	};
	unsigned int version;
	unsigned int offsetNames;
	int nameCount;
	unsigned int offsetMeshes;
	int meshCount;
	unsigned int offsetBoneMap;
	unsigned int boneMapCount;
	unsigned int offsetBoneMap2;
	unsigned int boneMap2Count;
	unsigned int offsetMeshMap;
	int meshMapCount;
	unsigned int offsetColTreeNodes;
	int colTreeNodesCount;
};

// CRedBlackTreeNode<T>*
struct CCollisionContextNode
{
	DWORD dword0;
	CCollisionContextNode* m_pRoot;
	CCollisionContextNode* m_pRight;
	CCollisionContextNode* m_pLeft;
	CCollisionContext m_Context;
	COLHdr* m_pColFile;
	CCollisionContextNode* m_pNext;
	CCollisionContextNode* m_pPrevious;
};

struct CHeapInfo;

// Size of struct is 0xC8 (200) bytes
struct CCollisionDataObjectManager
{
	CReadWriteLock m_Lock;
	CCollisionData m_Data;
	CCollisionUnknown m_Unknown;
	DWORD dword74;
	CCollisionContextNode* m_pCollisionContext2;
	CCollisionContextNode** m_ppCollisionContexts;
	int m_nAllocatedCollisionTrees;
	int m_nFreeCollisionTrees;
	CCollisionContextNode* m_pCollisionContext;
	CCollisionContextNode* m_pCurrentCollision;
	int m_nUsedCollisionTrees;
	DWORD dwordA4;
	CHeapInfo* m_pHeapInfo;
	CHeapInfo* m_HeapInfo;
	QWORD qwordB0;
	DWORD dwordB8;
	DWORD dwordBC;
};

struct __declspec(align(16)) CollisionInfo
{
	Vector4 m_vPosition;
	Vector3Aligned m_vMax;
	__m128 m_vDirA3A2;
	Vector3Aligned m_vRight;
	float m_flLength_v20;
};



typedef struct WTBHeader
{
	union {
		char			id[4];			// 0x00
		unsigned int	magic;
	};
	int					unknown;
	int					numTex;
	unsigned int		ofsTexOfs;
	unsigned int		ofsTexSizes;
	unsigned int		ofsTexFlags;
	unsigned int		texIdxOffset;
	unsigned int		texInfoOffset;
} WTBHeader, WTBHdr, WTAHeader, WTAHdr;


struct CModelAnalyzer;

struct CMaterialInfo
{
	CModelAnalyzer* m_pModelAnalyzer;
	CMaterialDescription m_Desc;
};


struct CModelEntryData; //forward def maybe temp
class CModelExtendWork;

struct CTextureResourceManager
{
	CReadWriteLock m_Lock;
	BYTE gap2C[52];
	DWORD dword60;
	DWORD dword64;
	CTextureData* m_pTextureData;
	QWORD qword70;
	CTextureResource* m_pTextures;
	QWORD qword80;
};


struct CModelWork;
struct CVertexLayout;

class CObjHit
{
	void* m_pVtbl;		//0x0000
	char _0x0008[136];	//0x0008
	Vector4 m_vec[2];	//0x0090

};

struct CSamplerState;

struct CAnimationListEntry
{
	EntityHandle m_hAnimationNode;
	Animation::Motion::NodePlay* m_pNode; //  Animation::Motion::Node
};

struct __declspec(align(8)) CAnimationList
{
	uint32 m_uSize;
	uint32 m_uItems;
	uint32 m_uSlot;
	uint32 m_uShift;
	CAnimationListEntry* m_pEntries;
	CReadWriteLock m_Lock;
};


// Found by Dennis
// Address = 0x14160EB40
struct Sun
{
	Vector2	m_vPosition;	 //0x0000
	char _0x0008[56];		 //0x0008
	Vector4 m_vColor;		 //0x0040
	float m_flBloom;		 //0x0044
};

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
	IDirectInputDevice8A* pDevice;
	BOOL bAcquired;
	HKL Layout;
};

struct Mouse_t
{
	HWND hWnd;
	IDirectInputDevice8A* pDevice;
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
	void* m_vtbl;
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
	float m_flThumbLY;			//0x000C
	float m_flThumbRX;			//0x0010
	float m_flThumbRY;			//0x0014
	float m_flLeftTrigger;		//0x0018
	float m_flRightTrigger;		//0x001C
	DWORD m_dwButtons;			//0x0020 | eControllerButtons
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

// left and right mouse can be swapped
enum eMouseButtons
{
	MOUSE_BUTTON0 = 1,
	MOUSE_BUTTON1 = 2,
	MOUSE_MIDDLE = 4
};

struct CMouseInputContext
{
	DWORD dword0; //possibly bool
	DWORD m_bShowCursorOld;
	DWORD dword8;
	LONG m_lWidth;
	LONG m_lHeight;
	float m_flDeltaX;
	float m_flDeltaY;
	float m_flDeltaZ;	
	DWORD m_fButtons; //  eMouseButtons
};

class CGraphicContextDx11;
class CGraphicDeviceDx11;

struct CConstantBufferInfo;

struct CGraphicCommand
{
	CModelEntryData* m_pModelData;	//0x00
	void* m_p0x008;					//0x08
	void* m_pCallback;				//0x10
	CShaderSetting* m_pSetting;		//0x18
	BYTE gap18[16];					//0x20
	QWORD qword30;					//0x30
	BYTE gap38[88];					//0x38
	BYTE m_SamplerIndex;			//0x90
	BYTE gap91[7];					//0x91
	BYTE m_PrimitiveWorkIndex;		//0x98
	BYTE gap99[23];					//0x99
	CModelEntry* m_pModelEntry;		//0xB0
	BYTE gapB8[8];					//0xB8
	int m_iVertexIndex;				//0xC0
	int m_iInputLayout;				//0xC4
	unsigned int unsignedCC;		//0xCC
};
VALIDATE_OFFSET(CGraphicCommand, qword30, 0x30);
VALIDATE_OFFSET(CGraphicCommand, m_SamplerIndex, 0x90);
VALIDATE_OFFSET(CGraphicCommand, m_pModelEntry, 0xB0);
VALIDATE_OFFSET(CGraphicCommand, m_iVertexIndex, 0xC0);

class CGraphics;

struct CGraphicCommandList
{
	__int64(__fastcall* m_pCallback)(CGraphics*, CGraphics*, CGraphicCommand*); // seems rdx is a garbage pointer
	CGraphicCommand* m_pCommand;
	struct CGraphicCommandList* m_pPrevious;
	struct CGraphicCommandList* m_pNext;
	DWORD m_dwFlags;
	INT m_iCommandIndex;
	CHAR pad28[8];
};
VALIDATE_SIZE(CGraphicCommandList, 0x30);

struct COtManagerPtr98
{
	CGraphicCommand* m_pCommands;
	CGraphicCommandList* m_pCmdLists;
	DWORD m_iCommandIndex;
	BYTE gap14[4];
	LPINT m_pTags;
};

/*
This is from memcpy
This must mean everything after is a different struct

Size of struct is 0xA8 (168) bytes
*/
struct COtManager
{
	void* m_pVtbl;							//0x00
	BYTE gap8[72];							//0x08
	//0x10 | embededd struct
	CGraphicCommandList** m_pCmdLists;		//0x50
	BYTE gap58[8];							//0x58
	INT* qword60;							//0x60
	CGraphicCommandList* m_pActiveList;		//0x68
	INT m_iGraphicListCount;				//0x70
	INT m_nMaxGraphicListCount;				//0x74
	volatile ULONG64 m_uCmdIndex;			//0x78
	ULONG64 m_uCmdCount;					//0x80
	DWORD m_uMaxTagCount;					//0x88
	DWORD dword8C;							//0x8C
	int signed90;							//0x90
	BYTE gap94[4];							//0x94
	COtManagerPtr98* m_ptr98;				//0x98
	BYTE gapA0[4];							//0xA0
	BOOL m_bGraphicListInitalized;			//0xA4
};
VALIDATE_OFFSET(COtManager, m_bGraphicListInitalized, 0xA4);
VALIDATE_SIZE(COtManager, 0xA8);


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
	BOOL bInUse;
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
	void* pBuffer;					
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
	void* gap10;
	BYTE gap18[8];
	PBYTE pBuffer;
	DWORD hBinder;
	BYTE gap28[28];
	QWORD szVersion;
	void* gap50;
	BYTE gap58[368];
	DWORD dword1C8;
	BYTE gap1CC[4];
	QWORD qword1D0;
	QWORD qword1D8;
	QWORD qword1E0;
	const char* szCpkPath;
	DWORD dword1F0;
	DWORD dword1F4;
	void* ptr1F8;
	DWORD dword200[2];
	void* fnptr208;
	void* fnptr210;
	CpkLoad_t* pNext;
	void* fnptr220;
	void* fnptr228;
	void* fnptr230;
	void* fnptr238;
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
	const char* sz2B0;
	QWORD dword2B8;
	DWORD dword2C0;
	QWORD dword2C4;
};

struct CpkBinderHandle
{
	void* ptr0;
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
	CpkBinderHandle* m_pBinderHandle;
	DWORD m_binderid;
	DWORD m_status;
	int m_iLoadOrder;
	DWORD dw7;
};

struct CHeapInstance;

template<typename T>
struct CConstructionInfo
{
	int m_iObjectId;						//0x0000
	char alignment[4];						//0x0004
	T* (*Constructor)(CHeapInfo* pHeapInfo);//0x0008 
	QWORD m_GroupId;						//0x0010
	const char* m_szName;					//0x0018
	void* pUnk;								//0x0020
};

template<typename T>
struct CUIConstructorInfo
{
	DWORD id;								//0x0000
	DWORD dword04;							//0x0004
	DWORD dword08;							//0x0008
	DWORD dword0C;							//0x000C
	T* (*Constructor)(CHeapInfo* pHeapInfo);//0x0010 
};

enum DialogUIWhiteType
{
	DialogUIWhiteType_YesNo = 0,
	DialogUIWhiteType_Ok
};

enum DialogUIBlackType
{
	DialogUIBlackType_YesNo = 0,
	DialogUIBlackType_Message,
	DialogUIBlackType_QuitRetry,
	DialogUIBlackType_Persistent
};


/*
	Event::Work maybe??? probs
	Size of struct 0x60 (96) bytes

*/
struct CPhase
{
	BYTE byte0;
	BYTE gap1[19];
	DWORD dword14;
	DWORD dword18;
	DWORD dword1C;
	VOID* m_pEvent; // 0x20 Event::ReadUnit*
	char pad[0x38];	// 0x28
};

struct CPhaseManager
{
	StaticArray<CPhase*, 8> m_Phases;
};


// Red-Black tree
struct PuidMixin
{
	struct PuidMixin* m_pHead;			// 0x0000
	struct PuidMixin* m_pRight;			// 0x0008
	struct PuidMixin* m_pLeft;			// 0x0010
	char _0x0018;						// 0x0018
	bool m_bLast;						// 0x0019
	char _0x0020[6];					// 0x0020
	unsigned int m_uDescriptionHash;	// 0x0026  FIXME: ???? this might be wrong
	char alignment[4];					// 0x0030
	char* m_szDescription;				// 0x0038
	UINT64 m_uReferenceCount;			// 0x0040
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
// RED BLACK TREE iirc
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

struct CStateObject
{
	void* m_pVtbl;
	DWORD dword8;
	QWORD qword10;
	DWORD dword18;
	char* m_szName;
	QWORD qword28;
	UINT m_uHash;
	QWORD qword38;
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
	Array<SceneState> m_OldStates;			//0x0060 //const lib::DynamicArray<hap::scene_state::SceneState,hap::configure::Allocator>
	char _0x00[16];							//0x0080
	Array<SceneState> m_States;				//0x0090 //const lib::DynamicArray<hap::scene_state::SceneState,hap::configure::Allocator>
};
VALIDATE_OFFSET(CSceneStateSystem, m_States, 0x90);

struct CSceneEntitySystem
{
	int m_nInfoLists;
	int m_nMaxInfoLists;
	void* qword8;
	void* qword10;
	BOOL dword18;
	BOOL dword1C;
	BOOL m_bDataExists;
	CReadWriteLock m_Lock;
	CEntityInfoList* m_pInfoList2;
	CEntityInfoList* qword58;
	BYTE gap68[8];
	SIZE_T m_nListEntriesCount;
	CEntityInfoList* m_pInfoList3;
	CEntityInfoList* m_pInfoList4;
	CEntityInfoList* m_pInfoList;
	//HeapInfo m_heapInfo;
};

//max size 0xC0
struct set_info_t
{
	// pretty sure all these vec 0 - 48 are just a mat4x4 -- it get initalized to an identity 4x4 matrix
	Matrix4x4 m_mat;
	Vector3Aligned m_vPosition;
	Vector3Aligned m_vRotation;
	Vector3Aligned m_vScale;
	DWORD m_dwSetType;
	DWORD m_dw0x74;
	INT m_i0x078;
	INT m_i0x07C;
	INT m_i0x080;
	INT m_i0x084;
	INT m_i0x088;
	INT m_iGroupId;
};

#define CREATE_ENTITY_VFX 0x40000000

struct Create_t
{
	const char* m_szName;			//0x0000
	unsigned int m_ObjectIds[2];	//0x0008
	set_info_t* m_pSetInfo;			//0x0010
	DWORD m_dwSetType;				//0x0018 | this is the same as set_info_t::m_dwSetType
	DWORD m_dwFlags;				//0x001C
	int m_iGenerateMode;			//0x0020 | 0 = no bones & invisible, 1 = normal, 2 = no bones
	char alignment24[4];			//0x0024
	void* m_pWMB;					//0x0028
	CModelData* m_pModelData;		//0x0030
	void* m_pDatBuffers;			//0x0038
	void* m_pWTA;					//0x0040
	void* m_pWTP;					//0x0048
	void* m_pBXM;					//0x0050
};

struct CreateContext
{
	CSceneEntitySystem* m_pSceneEntitySystem;	// 0x0000
	void* ptr8;									// 0x0008
	Create_t* m_pCreate;						// 0x0010
	BOOL m_bDataExists;							// 0x0018 |  m_pSceneEntitySystem->qword18
};

struct HeapAlloc_t
{
	LPVOID Pointer;
	BOOL Succeeded;
};

/*
* Size of struct is 0x40 (64) bytes
*/
struct CHeapInfo
{
	CHeapInstance* m_pHeap;				//0x0000
	struct CHeapInfo* m_pParentInfo;	//0x0008
	struct CHeapInfo* m_pChildInfo;		//0x0010
	struct CHeapInfo* m_pPrevious;		//0x0018 could be next then prev
	struct CHeapInfo* m_pNext;			//0x0020
	QWORD m_uPartitionSize;				//0x0028
	QWORD m_qwHeapSize;					//0x0030
	DWORD m_uIndex;						//0x0038
};

/*
* Size of struct is 0x40 (64) bytes (+8) maybe?
*/
struct CHeapAllocInfo
{
	UINT m_uIndex;					//0x0000
	UINT m_uUnknown;				//0x0004
	CHeapInstance* m_pParent;		//0x0008 might be child idk
	CHeapInstance* m_pChild;		//0x0010
	CHeapAllocInfo* m_pNext;		//0x0018
	CHeapAllocInfo* m_pPrevious;	//0x0020
	CHeapInfo* m_pHeapInfo;			//0x0028
	SIZE_T m_cbAlignedSize;			//0x0030
	SIZE_T m_cbSize;				//0x0038
	__int64* m_pStruct;
};

/*
Size of struct is 0x98 (152) bytes
*/
struct CHeapVramInstance
{
	enum eAllocFlags
	{
		ALLOC_FLAG_PARTITION_INFO = 1,
		ALLOC_FLAG_NO_ALLOC = 2, // modify the struct pointers but don't allocate???
		UNK4 = 4
	};

	virtual void dtor(char a2);
	virtual BOOL SetHeapInfo(CHeapInfo* pInfo, unsigned __int64 cbSize, int nChildren, int flags);
	virtual CHeapInfo* fn2(const void* pMemoryPool);
	virtual QWORD GetPartitionSize(const void* pMemoryPool);
	virtual CHeapAllocInfo* AllocPartition(__int64 nBytes, unsigned __int64 align, int flags);
	virtual void FreePartition(const void* pMemoryPool); // pMemoryPool - 8 = CHeapAllocInfo*

	CReadWriteLock m_Lock;					//0x0008
	CHeapInfo* m_pParentInfo;				//0x0038
	CHeapInfo* m_pInfo;						//0x0040
	CHeapAllocInfo* m_pPrevPartition;		//0x0048 | void*
	CHeapAllocInfo* m_pNextPartition;		//0x0050 | void*
	QWORD m_qwHeapFlags;					//0x0058
	void* m_pMemoryPool;					//0x0060
	CHeapInfo** m_pHeapInfos;				//0x0068
	CHeapAllocInfo* m_pAllocationInfo;		//0x0070		
	QWORD m_qwMemoryAlignment;				//0x0078
	DWORD m_uIndex;							//0x0080 | used for CMemoryDevice::GetHeapAligment
	DWORD dword84;							//0x0084
	DWORD m_dwReferenceCount;				//0x0088
	int m_nRootHeaps;						//0x008C
	int m_nHeaps;							//0x0090
};

/*
Size of struct is 0x98 (152) bytes
*/
struct CHeapInstance
{
	enum eAllocFlags
	{
		ALLOC_ALIGN = 1,
		UNK2 = 2,
		UNK4 = 4
	};

	virtual void dtor(char a2);
	virtual BOOL SetHeapInfo(CHeapInfo* pInfo, unsigned __int64 cbSize, int nChildren, int flags);
	virtual CHeapAllocInfo* fn2(const void* pMemoryPool);
	virtual QWORD GetPartitionSize(const void* pMemoryPool);
	virtual CHeapAllocInfo* AllocPartition(__int64 nBytes, unsigned __int64 align, int flags);
	virtual void FreePartition(const void* pMemoryPool); // pMemoryPool - 8 = CHeapAllocInfo*

	CReadWriteLock m_Lock;					//0x0008
	CHeapInfo* m_pParentInfo;				//0x0038
	CHeapInfo* m_pInfo;						//0x0040
	CHeapAllocInfo* m_pPrevPartition;		//0x0048 | void*
	CHeapAllocInfo* m_pNextPartition;		//0x0050 | void*
	QWORD m_qwHeapFlags;					//0x0058
	void* m_pMemoryPool;					//0x0060
	CHeapInfo** m_pHeapInfos;				//0x0068
	CHeapAllocInfo* m_pAllocationInfo;		//0x0070		
	QWORD m_qwMemoryAlignment;				//0x0078
	DWORD m_uIndex;							//0x0080 | used for CMemoryDevice::GetHeapAligment
	DWORD dword84;							//0x0084
	DWORD m_dwReferenceCount;				//0x0088
	int m_nRootHeaps;						//0x008C
	int m_nHeaps;							//0x0090
};
VALIDATE_SIZE(CHeapInstance, 0x98);

//	Mem pool start
//  4 * nRootHeaps
//	8 * nHeaps
//  Heap
//
struct CMemoryDevice
{
	virtual __int64 dtor();
	virtual BOOL Alloc(void** ppMem, SIZE_T nByteSize); // call heap
	virtual BOOL Free(void** ppMem);
	virtual QWORD GetHeapAlignment(int index);

	CReadWriteLock m_Lock;		//0x08
	CHeapInstance m_DeviceHeap;	//0x38
	void* m_pMemoryPool;		//0xC8 start CHeapInfo pretty sure
	CHeapInfo* m_pRootInfo;		//0xD0 | points to end of list (use previous link to traverse)
	CHeapInfo* m_punk;			//0xD8
	BOOL m_bHeapAllocated;		//0xE4
	char align[4];				//0xE8
	HANDLE m_hHeap;				//0xF0 growable heap (main heap that has all the heaps for the device)
};

class HandlerBase;

/*

Size of struct is 0xC (12) bytes
*/
struct CallbackInstalled
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
	virtual void UnloadContent(CGameContentDevice* pGameContentDevice);
	virtual bool LoadContent(CGameContentDevice* pGameContentDevice);
public:
	//void* m_pVtbl;														//0x0000
	CCallback<CGameContentDeviceSteam, DlcInstalled_t, false> m_Callback;	//0x0008 | id is 1005
	CallbackInstalled* m_pItems;											//0x0028
	DWORD m_uItems;															//0x0030
	BOOL m_bInitalized;														//0x0034
};
VALIDATE_SIZE(CGameContentDeviceSteam, 56);

/*
Size of is 0x48 (72) bytes
*/
class CGameContentDevice
{
public:
	void* m_pVtbl;													//0x0000
	HandlerBase* m_pHandler;										//0x0008
	DWORD m_uCpkCount;												//0x0010 
	DWORD unk0x014;													//0x0014 
	DWORD unk0x18;													//0x0018
	void(*LoadCpks)(unsigned int index, const char* szCpkName);		//0x0020
	void(*UnloadCpks)(unsigned int index);							//0x0028
	CHeapInstance** m_ppHeap;										//0x0030 | probably a parent pointer
	CGameContentDeviceSteam* m_pSteamContent;						//0x0038
	DWORD unk0x40;													//0x0040
};
VALIDATE_SIZE(CGameContentDevice, 72);

/*
Size of is 0x20 (32) bytes

Max achievement index = (0x2F) 47
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
VALIDATE_SIZE(CAchievementDeviceSteam, 32);

/*
Size of is 0x68 (104) bytes
*/
class CAchievementDevice
{
public:
	QWORD qw0x00;
	QWORD qw0x08;
	DWORD dw0x10;
	DWORD dw0x14;
	QWORD qw0x18;
	QWORD qw0x20;
	CReadWriteLock m_Lock;
	CAchievementDeviceSteam* m_pAchievementDeviceSteam;
	UINT m_uMaxAchievement;
};
VALIDATE_SIZE(CAchievementDevice, 104);

struct CNetworkDevice;

enum LeaderboardDownloadMode
{
	LeaderboardDownloadMode_ForUser,
	LeaderboardDownloadMode_ForRange
};

union CLeaderboardDetails
{
	struct Layout {
		int32 m_cbSize;
		char szDetails[252];
	} m_Layout;
	int32 m_Raw[64];
};

struct CLeaderboardEntry
{
	CSteamID m_SteamID;
	char m_szName[128];
	int32 m_nGlobalRank;
	BYTE gap8C[4];
	int64 m_nScore;
	BYTE gap98[32];
	char* m_pszDetails;
	int64 m_cbDetails;
	char m_szDetails[252];
};

struct CRankingReadWork
{
	void* m_pVtble;
	CRankingReadWork* m_pNext;
	CRankingReadWork* m_pPrevious;
	DWORD dword18;
	BYTE gap1C[24];
	DWORD dword34;
	DWORD dword38;
	BYTE gap3C[16];
	int32 m_iLeaderboardEntry;
	int32 m_nLeaderboardEntries;
	CLeaderboardEntry* m_pLeaderboardEntries;
	LeaderboardDownloadMode m_DownloadMode;
	BYTE gap64[4];
	SteamLeaderboard_t m_hLeaderboard;
	CCallResult<CRankingReadWork, LeaderboardScoresDownloaded_t> m_LeaderboardDownloaded;
	volatile int32 m_Status;
	__unaligned __declspec(align(1)) CSteamID m_SteamID;
	int32 m_nSteamIDs;
	ELeaderboardDataRequest m_LeaderboardDataRequest;
	int32 m_nRangeStart;
	int32 m_nRangeEnd;
	DWORD dwordB0;
};

struct CRankingWriteWork
{
	void* m_pVtbl;
	CRankingWriteWork* m_pNext;
	CRankingWriteWork* m_pPrevious;
	DWORD dword18;
	BYTE gap1C[24];
	DWORD dword34;
	DWORD dword38;
	BYTE gap3C[4];
	SteamLeaderboard_t m_hLeaderboard;
	ELeaderboardUploadScoreMethod m_LeaderboardUploadScoreMethod;
	int32 m_nScore;
	CLeaderboardDetails m_LeaderboardDetails;
	int32 m_nLeaderboardDetails;
	char m_szFileName[128];
	BYTE gap1D4[4];
	const void* m_pvData;
	int32 m_cbDataSize;
	UGCHandle_t m_hFile;
	DWORD dword1F0;
	DWORD dword1F4;
	BYTE gap1F8[8];
	CCallResult<CRankingWriteWork, LeaderboardScoreUploaded_t> m_LeaderboardScoreUploaded;
	CCallResult<CRankingWriteWork, RemoteStorageFileShareResult_t> m_RemoteStorageFileShare;
	CCallResult<CRankingWriteWork, LeaderboardUGCSet_t> m_UGCLeaderboardSet;
	volatile int32 m_UploadStatus;
	volatile int32 m_WriteStatus;
	volatile int32 m_ShareStatus;
	volatile int32 m_UGCSetStatus;
};
VALIDATE_SIZE(CRankingWriteWork, 0x288);

/*
Size of is 0x38 (56) bytes
*/
struct CLeaderboard
{
	uint32 m_nLeaderboards;
	CCallResult<CRankingReadWork, LeaderboardFindResult_t> m_Leaderboards;
};

struct CServerRealTimes
{
	RTime32 m_uServerTimes[4];
	int32 m_unk1;
	int32 m_unk2;
	int32 m_unk3;
	int32 m_unk4;
	int32 m_unk5;
};

/*
Size of is 0x78 (120) bytes
*/
class CRankingDevice
{
	void* m_pVtable;					//0x00
	CReadWriteLock m_Lock;				//0x08
	CNetworkDevice* m_pNetworkDevice;	//0x38
	CHeapInfo* m_pHeapInfo;             //0x40
	union {
		CRankingReadWork* m_pReadWork;   //0x48
		CRankingWriteWork* m_pWriteWork; //0x48
	};
	DWORD dword50;
	BYTE gap54[4];
	CServerRealTimes* m_pServerRealTimes;
	int32 m_Status;
	CCallResult<CRankingDevice, LeaderboardFindResult_t>* m_pLeaderboards;		//0x68
	DWORD dwLeaderboardCount;			//0x70 | statically set to 32
	DWORD dword74;
};
VALIDATE_SIZE(CRankingDevice, 120);

/*
Size of is 0x?? (?) bytes
*/
struct CNetworkDevice0x16
{
	BYTE gap0[40];
	DWORD dword28;
	BYTE gap2C[60];
	__int64* pint6468;
	BYTE gap70[8];
	QWORD qword78;
};

/*
Size of is 0x48 (72) bytes
*/
struct CNetworkDevice
{
	void* m_pVtbl;
	CRankingDevice* m_pRankingDevice;
	QWORD gap8[2];
	CNetworkDevice0x16* m_pUnknownDevice;
	void* m_pTitleStorageDevice;
	CReadWriteLock* m_Lock;
	void* m_pHeap;
	BOOL m_bHeapCreated;
	DWORD m_dwStatus;
};


/*
Size of struct is 0x40 (64) bytes
*/
class COsSystemDeviceSteam
{
public:
	virtual __int64 sub_144F5DAA0(char a2); //destructor???
	virtual __int64 RunCallbacks();
	virtual __int64 InitContext(void* p);
	virtual BOOL IsOverlayActive();

	//void* m_pVtable;		//0x00
	CCallback<COsSystemDeviceSteam, GameOverlayActivated_t, false> m_Overlay; //0x08 id = 331
	BOOL m_bInitalized;		//0x28
	char pad2[4];			//0x2C
	ISteamUser019* m_pUser;	//0x30
	BOOL m_bOverlayActive;	//0x38
	char pad3[4];			//0x3C
};
VALIDATE_SIZE(COsSystemDeviceSteam, 64);

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
VALIDATE_SIZE(CUserSignInProcess, 32);

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
VALIDATE_SIZE(CGameBootProcess, 24);


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

struct CWetObjManagerDelay
{
	float m_flMaxDelay;
	float m_flDelay;
};

/*
* Denuvo ver. Address = 0x1417BFD48
*/
struct CWetObjManager
{
	CReadWriteLock m_Lock;
	EntityHandle m_LocalHandles[2];
	CWetObjManagerDelay m_WetDelays[2];
	EntityHandle m_EntityHandles[256];
	EntityHandle m_SoundHandles[32];
};

class CYorhaManager
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
class CNPCManager
{
public:
	virtual void function0(); //maybe constructor
	virtual CEntityInfo* sub_1439C5CA0(Pl0000* pEntity);
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

class CAnimalManager
{
	CReadWriteLock m_Lock;
  __declspec(align(8)) DWORD dword30;
  BYTE gap34[84];
  QWORD qword88;
  QWORD qword90;
  QWORD qword98;
  QWORD qwordA0;
};

// Size of struct 0x1520 (5408) bytes
class CEmBaseManager
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
	virtual bool function22(CEntityInfo* pInfo); //calls/returns vfunc 21
	virtual bool function23(EntityHandle* pHandle); //calls/returns vfunc 21
	virtual bool function24(Pl0000* pEntity);
	virtual bool function25(CEntityInfo* pInfo);//calls/returns vfunc 24
	virtual bool function26(EntityHandle* pHandle); //calls/returns vfunc 24
	virtual bool IsEntityValid(); //return GetEntity() != 0;
	virtual EntityHandle function28(CEntityInfo* pInfo); //calls/returns IsEntityValid (vfunc 27)
	virtual void function29(EntityHandle* pHandle);
	virtual bool function30(Pl0000* pEntity);
	virtual bool function31(CEntityInfo* pInfo); //calls/returns vfunc 30
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
	char _0x0840[16];							//0x0848
	StaticArray<EntityHandle, 256> m_handles3;	//0x0858
	StaticArray<EntityHandle, 256> m_handles4;	//0x0C78
	StaticArray<EntityHandle, 256> m_handles5;	//0x1098
	EntityHandle m_hCameraEntity;				//0x14B8
	EntityHandle m_hEntity;						//0x14BC
	EntityHandle m_hEnt[7];						//0x14C0
	CReadWriteLock m_Lock;						//0x14DC
};
VALIDATE_OFFSET(CEmBaseManager, m_handles, 0x8);
VALIDATE_OFFSET(CEmBaseManager, m_handles2, 0x428);
VALIDATE_OFFSET(CEmBaseManager, m_handles3, 0x858);
VALIDATE_OFFSET(CEmBaseManager, m_handles4, 0xC78);
VALIDATE_OFFSET(CEmBaseManager, m_handles5, 0x1098);
VALIDATE_OFFSET(CEmBaseManager, m_hEntity, 0x14BC);
//VALIDATE_OFFSET(EmBaseManager, m_Lock, 0x14D8); // wtf is going on

// TODO: reverse engineer this class/struct
class CItemManager;

/*
* SteamInternal_ContextInit(&pSteam_InitInterface_WrapperFn);
*/
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
	ISteamHTMLSurface003* pISteamHTMLSurface;
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
	~CUserManager() = delete;
public:
	virtual CUserManager* Clear(BYTE bFlags);

	CReadWriteLock m_Lock;						//0x0008
	//char alignment[4];						//0x0034
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
VALIDATE_SIZE(CUserManager, 264);
VALIDATE_OFFSET(CUserManager, m_UsersInfo, 0x38);
VALIDATE_OFFSET(CUserManager, m_iActiveUser, 0xB8);
VALIDATE_OFFSET(CUserManager, m_iUserIndices, 0xBC);
VALIDATE_OFFSET(CUserManager, m_pBootProcess, 0xD8);
VALIDATE_OFFSET(CUserManager, m_pSaveDataDevice, 0xF0);

#define MRB_FUNCALL_ARGC_MAX 16
#define MRB_CALL_LEVEL_MAX 128 // could be 512 too

typedef uint32_t mrb_sym;
typedef uint8_t mrb_bool;
typedef void mrb_value;
typedef uint8_t mrb_code;

enum mrb_vtype {
	MRB_TT_FALSE = 0,   /*   0 */
	MRB_TT_FREE,        /*   1 */
	MRB_TT_TRUE,        /*   2 */
	MRB_TT_FIXNUM,      /*   3 */
	MRB_TT_SYMBOL,      /*   4 */
	MRB_TT_UNDEF,       /*   5 */
	MRB_TT_FLOAT,       /*   6 */
	MRB_TT_CPTR,        /*   7 */
	MRB_TT_OBJECT,      /*   8 */
	MRB_TT_CLASS,       /*   9 */
	MRB_TT_MODULE,      /*  10 */
	MRB_TT_ICLASS,      /*  11 */
	MRB_TT_SCLASS,      /*  12 */
	MRB_TT_PROC,        /*  13 */
	MRB_TT_ARRAY,       /*  14 */
	MRB_TT_HASH,        /*  15 */
	MRB_TT_STRING,      /*  16 */
	MRB_TT_RANGE,       /*  17 */
	MRB_TT_EXCEPTION,   /*  18 */
	MRB_TT_FILE,        /*  19 */
	MRB_TT_ENV,         /*  20 */
	MRB_TT_DATA,        /*  21 */
	MRB_TT_FIBER,       /*  22 */
	MRB_TT_ISTRUCT,     /*  23 */
	MRB_TT_BREAK,       /*  24 */
	MRB_TT_MAXDEFINE    /*  25 */
};

typedef enum {

	MRB_GC_STATE_ROOT = 0,
	MRB_GC_STATE_MARK,
	MRB_GC_STATE_SWEEP
} mrb_gc_state;

typedef struct mrb_heap_page {

	struct RBasic* freelist;
	struct mrb_heap_page* prev;
	struct mrb_heap_page* next;
	struct mrb_heap_page* free_next;
	struct mrb_heap_page* free_prev;
	mrb_bool old : 1;
	void* objects[ANYSIZE_ARRAY];
} mrb_heap_page;

typedef struct mrb_gc {

	mrb_heap_page* heaps;                /* heaps for GC */
	mrb_heap_page* sweeps;
	mrb_heap_page* free_heaps;
	size_t live; /* count of live objects */

#ifdef MRB_GC_FIXED_ARENA
	struct RBasic* arena[MRB_GC_ARENA_SIZE]; /* GC protection array */
#else
	struct RBasic** arena;                   /* GC protection array */
	int arena_capa;
#endif

	int arena_idx;
	mrb_gc_state state; /* state of gc */
	int current_white_part; /* make white object by white_part */
	struct RBasic* gray_list; /* list of gray objects to be traversed incrementally */
	struct RBasic* atomic_gray_list; /* list of objects to be traversed atomically */
	size_t live_after_mark;
	size_t threshold;
	int interval_ratio;
	int step_ratio;
	mrb_bool iterating : 1;
	mrb_bool disabled : 1;
	mrb_bool full : 1;
	mrb_bool generational : 1;
	mrb_bool out_of_memory : 1;
	size_t majorgc_old_threshold;

} mrb_gc;

typedef struct {
	mrb_sym mid;
	struct RProc* proc;
	mrb_value* stackent;
	uint16_t ridx;
	uint16_t epos;
	struct REnv* env;	//0x20 (padded)
	mrb_code* pc;       //0x28	  /* return address */
	mrb_code* err;      //0x30    /* error position */
	int argc;			//0x38
	int acc;			//0x40
	struct RClass* target_class;
} mrb_callinfo;

enum mrb_fiber_state {
	MRB_FIBER_CREATED = 0,
	MRB_FIBER_RUNNING,
	MRB_FIBER_RESUMED,
	MRB_FIBER_SUSPENDED,
	MRB_FIBER_TRANSFERRED,
	MRB_FIBER_TERMINATED,
};

struct mrb_context {
	struct mrb_context* prev;
	mrb_value* stack;                       /* stack of virtual machine */
	mrb_value* stbase, * stend;
	mrb_callinfo* ci;
	mrb_callinfo* cibase, * ciend;
	uint16_t* rescue;                       /* exception handler stack */
	uint16_t rsize;
	struct RProc** ensure;                  /* ensure handler stack */
	uint16_t esize, eidx;
	enum mrb_fiber_state status;
	mrb_bool vmexec;
	struct RFiber* fib;
};

struct mrb_parser_state;

typedef struct mrbc_context {
	mrb_sym* syms;
	int slen;
	char* filename;
	uint16_t lineno;
	int (*partial_hook)(struct mrb_parser_state*);
	void* partial_data;
	struct RClass* target_class;
	mrb_bool capture_errors : 1;
	mrb_bool dump_result : 1;
	mrb_bool no_exec : 1;
	mrb_bool keep_lv : 1;
	mrb_bool no_optimize : 1;
	mrb_bool no_ext_ops : 1;
	const struct RProc* upper;

	size_t parser_nerr;
} mrbc_context;

struct segment;

/* Instance variable table structure */
typedef struct iv_tbl {
	segment* rootseg;
	size_t size;
	size_t last_len;
} iv_tbl;

#ifdef MRB_METHOD_CACHE_SIZE
# define MRB_METHOD_CACHE
#else
/* default method cache size: 128 */
/* cache size needs to be power of 2 */
# define MRB_METHOD_CACHE_SIZE (1<<7)
#endif

typedef void* (*mrb_allocf)(struct mrb_state* mrb, void* ptr, size_t s, void* ud);
typedef void(*mrb_atexit_func)(struct mrb_state*);

// i think
// size of stuct is 0x188 (392) bytes
typedef struct mrb_state {

	struct mrb_jmpbuf* jmp;		//0x00
	mrb_allocf allocf;			//0x08      /* memory allocation function */
	void* allocf_ud;            //0x18      /* auxiliary data of allocf */
	struct mrb_context* c;		//0x20
	struct mrb_context* root_c; //0x28
	struct iv_tbl* globals;     //0x30      /* global variable table */
	struct RObject* exc;        //0x38      /* exception */
	struct RObject* top_self;
	struct RClass* object_class;            /* Object class */
	struct RClass* class_class;
	struct RClass* module_class;
	struct RClass* proc_class;
	struct RClass* string_class;
	struct RClass* array_class;
	struct RClass* hash_class;
	struct RClass* range_class;

#ifndef MRB_WITHOUT_FLOAT
	struct RClass* float_class;
#endif

	struct RClass* fixnum_class;
	struct RClass* true_class;
	struct RClass* false_class;
	struct RClass* nil_class;
	struct RClass* symbol_class;
	struct RClass* kernel_module;
	struct alloca_header* mems;
	mrb_gc gc;

#ifdef MRB_METHOD_CACHE
	struct mrb_cache_entry cache[MRB_METHOD_CACHE_SIZE];
#endif

	mrb_sym symidx;
	struct symbol_name* symtbl;   /* symbol table */
	mrb_sym symhash[256];
	size_t symcapa;

#ifndef MRB_ENABLE_SYMBOLL_ALL
	char symbuf[8];               /* buffer for small symbol names */
#endif

#ifdef MRB_ENABLE_DEBUG_HOOK
	void(*code_fetch_hook)(struct mrb_state* mrb, struct mrb_irep* irep, mrb_code* pc, mrb_value* regs);
	void(*debug_op_hook)(struct mrb_state* mrb, struct mrb_irep* irep, mrb_code* pc, mrb_value* regs);
#endif

#ifdef MRB_BYTECODE_DECODE_OPTION

	mrb_code(*bytecode_decoder)(struct mrb_state* mrb, mrb_code code);

#endif

	struct RClass* eException_class;
	struct RClass* eStandardError_class;
	struct RObject* nomem_err;              /* pre-allocated NoMemoryError */
	struct RObject* stack_err;              /* pre-allocated SysStackError */

#ifdef MRB_GC_FIXED_ARENA

	struct RObject* arena_err;              /* pre-allocated arena overfow error */

#endif

	void* ud; /* auxiliary data */

#ifdef MRB_FIXED_STATE_ATEXIT_STACK

	mrb_atexit_func atexit_stack[MRB_FIXED_STATE_ATEXIT_STACK_SIZE];

#else
	mrb_atexit_func* atexit_stack;
#endif

	uint16_t atexit_stack_len;
	uint16_t ecall_nest;                    /* prevent infinite recursive ecall() */
} mrb_state;


struct EventFiber;
struct MrubyImpl;

struct EventEntry
{
	void* placeholder;
};

struct ScriptMixin
{
	const char* GetName() { return (const char*)((char*)this->qword60 + 0x28); }

	void* vtbl;
	BYTE gap0[8];
	QWORD qword10;
	const char* m_szJISScript; // the word script in Japanese JIS encoding
	DWORD m_dwHash;
	BYTE gap24[12];
	SceneState* pscenestate30;
	void* ptr38;
	byte* m_pBinary;
	void* ptr48;
	BYTE gap48[16];
	QWORD qword60;
	MrubyImpl* m_pRuby;
	BYTE gap70[8];
	BYTE byte78;
};

/*
Size of struct is 288 bytes
*/
struct ScriptAction
{
	void* vtbl;
	void* m_pSceneMap;
	BYTE pad10[8];
	DWORD m_dwHash;
	DWORD m_fFlags;
	DWORD m_dwHash2;
	BYTE gap24[36];
	BYTE byte48;
	BYTE gap49[7];
	struct ScriptMixin m_Script;
	BYTE gap58[40];
	QWORD qwordF8;
	QWORD index100;
};

struct MrubyScript
{
	byte* m_pBinary;	//0x00 actual file to the .rb mapped in mem
	char unk8[4];		//0x08
	DWORD m_dwHash;		//0x0C
	FILE* m_pFile;		//0x10
	char unk18[8];		//0x18
	void* punk;			//0x20
	char unk30[16];		//0x30
	struct ScriptAction* m_pAction;
};

struct ScriptProxy
{
	INT m_iReferenceCount;
	struct mrb_state* m_pRubyVM;
};

struct MrubyImpl
{
#if 0				
	virtual BOOL function0();
	vitrual BOOL IsVirtualMachineValid();
	virtual BOOL LoadScript(MrubyScript* pScript);
#endif

	void* m_vtbl;													//0x0000 | if vtbl is zero vtbl is [this - 32]
	struct mrb_state* m_pRubyVM;									//0x0008
	QWORD m_pGarbageCollector;										//0x0010 | mrb_value
	StaticArray<EventEntry, 256> m_events;							//0x0018
	StaticArray<EventEntry, 256> m_events2;
	StaticArray<std::pair<unsigned int, EventFiber*>, 16> m_Fibers;
	QWORD qword1180;//StaticArray<std::pair<lib::HashedString<sys::StringSystem::Allocator>,lib::HashedString<sys::StringSystem::Allocator>>,256,8>
	QWORD qword1188;
	QWORD qword1190;
	QWORD qword1198;
	BYTE gap11A0[4112];
	struct MrubyImpl* m_pNext;
};
VALIDATE_OFFSET(MrubyImpl, m_Fibers, 0x1058);
VALIDATE_OFFSET(MrubyImpl, m_pNext, 0x21A8);
VALIDATE_SIZE(MrubyImpl, 8624);


struct CUILayerString
{
	void* m_pVtbl;
	QWORD qword8;
	BYTE gap10[16];
	WORD word20;
	BYTE gap22[14];
	float float30;
	BYTE gap34[12];
	float m_flX;
	float m_flY;
	float float48;
	float float4C;
	char m_szBuffer[32];
	size_t m_uSize;
	float float70;
	float float74;
	float float78;
	float float7C;
	float float80;
	float float84;
	float float88;
	float float8C;
	float float90;
};
#endif // !__REVERSEDSTRUCTSUNPACKED_H__