#ifndef __CMODEL_H__
#define __CMODEL_H__

#include "..\Common.h"
#include "..\wmb.h"
#include "..\Graphics\CMaterial.h"

// defines for bone ids
#define BONE_ROOT		4094
#define BONE_ROOT_CHILD	4095
#define BONE_SPINE0		0
#define BONE_SPINE1		1
#define BONE_SPINE2		2
#define BONE_SPINE3		3
#define BONE_NECK		4
#define BONE_HEAD		5
#define BONE_R_SHOULDER	6
#define BONE_R_ARM		7
#define BONE_R_FOREARM	8
#define BONE_R_WRIST	9
#define BONE_L_SHOULDER	10
#define BONE_L_ARM		11
#define BONE_L_FOREARM	12
#define BONE_L_WRIST	13
#define BONE_HIP		14
#define BONE_R_UPPERLEG	15
#define BONE_R_LEG		16
#define BONE_R_ANKLE	17
#define BONE_R_TOES		18
#define BONE_L_UPPERLEG	19
#define BONE_L_LEG		20
#define BONE_L_ANKLE	21
#define BONE_L_TOES		22

#define BONE_BOOBS		3136

#define MAX_MATERIAL_TEXTURES 16

// used in the update func
#define MODELSHADERMODULE_DONT_UPDATE 0x00080002

// Forward Definitions
class CModelShader;
class CModelWork;
class CModelData;
class CModel;

/*
	Size of struct 0xB0 (176) bytes

	m_wFlags - 0x00 - NORMAL, 0x01 - ?, 0x02 - NO_ROTATION_Y, 0x04 - FLOAT_UP, 0x08 - ?, 0x10 - ? (wont set), 0x20 - ? (wont set)
	0x40 - ?, 0x80 - ?, 0x100 - ?, 0x200 - ?, 0x400 - ?, 0x800 - ?, 0x1000 - ?, 0x2000 - ?, 0x4000 - NO_ROTATION_Y, 0x8000 - ?
*/
class CParts
{
public:
	Matrix4x4 m_matTransform;		//0x0000
	Vector3Aligned m_vPosition;		//0x0040
	Matrix4x4 m_matModelToWorld;	//0x0050 | 1st row: ? 2nd row: scale(x,y,z) 3rd: none: 4th rotation(p,y,r)
	void* m_pUnk0x0090;				//0x0090 | set to 0 on construction
	int16_t m_wFlags;				//0x0098 | set to 0 on construction
	float m_flTickBase;				//0x009C |
	void* m_pUnk0x00A0;				//0x00A0 | set to 0 on construction
	int16_t m_wUnk0x00A8;			//0x00A8 | set to -1 on construction
};
VALIDATE_OFFSET(CParts, m_flTickBase, 0x9C);
VALIDATE_SIZE(CParts, 0xB0);

struct CShaderSetting
{
	void* m_pVtbl;
	BYTE gap0[12];
	DWORD dword14;
	DWORD dword18;
	BYTE gap1C[28];
	DWORD dword38;
	BYTE gap3C[16];
	DWORD dword4C;
	DWORD dword50;
	DWORD dword54;
	DWORD dword58;
	DWORD dword5C;
	DWORD dword60;
	DWORD dword64;
	QWORD dword68;
};

struct CBoneSets
{
	int16_t* m_pSets;
	int32_t m_nSets;
};

struct CBatchInfo
{
	int32_t m_iVertexGroupIndex;
	int32_t m_iMeshIndex;
	int32_t m_iMaterialIndex;
	int32_t m_iUnk;
	int32_t m_iMeshMaterialIndex;
	int32_t m_iUnk2;
};

struct CDrawGroupInfo
{
	DWORD dword0;
	int32_t m_iLODLevel;
	DWORD m_uBatchStart;
	BYTE gapC[4];
	CBatchInfo* m_pBatchInfos;
	uint32_t m_nBatchInfos;
};

/*
Size of struct 0x38 (56) bytes
*/
struct CModelInstanceParam
{
	void* m_pVtbl;				//0x00
	DWORD m_dw0x08;				//0x08
	char padC[0x1C];			//0x0C
	CConstantBuffer* m_pBuffer;	//0x28
	DWORD m_dw0x30;				//0x30
};

struct __declspec(align(8)) CConstantBufferInfo
{
	int32_t m_iStartIndex;				//0x00
	DWORD m_iCount;
	DWORD m_nInstances;
	DWORD dwordC;
	BYTE gap10[8];
	CModelInstanceParam* m_pParams;
	CConstantBufferContext* m_pContext;
};

// had to define virtuals not right tho!
class CModelExtendWorkBase
{
public:
	virtual void function0() {};
	virtual void function1() {};
};

/*
Size of struct 0xA8 (168) bytes
*/
struct CModelMatrixTable
{
	void* m_pVtbl;					//0x0000
	char pad08[8];					//0x0008
	CSamplerState* m_pSampler;		//0x0010
	CBoneSets* m_pSets;				//0x0018
	int32_t m_nMatrixCount;			//0x001C
	char pad20[12];					//0x0020
	int32_t m_nBoneSets;
	CConstantBufferContext m_ConstBufferCtx;
	char pad30[12];
	CModelMatrix* m_pMatrices;
	CModelMatrix* m_pMatrices2;
	int32_t m_nMatrices;
	int32_t m_nMatrices2;
};

class CModelExtendWork : public CModelExtendWorkBase
{
public:

	float flUnk;				//0x0008
	DWORD _0x0010;				//0x000C
	uint32_t m_ObjectId;		//0x0010
	char _0x0014[12];			//0x0014
	CModel* m_pParent;			//0x0020 | maybe? might need CObj or CBehavior
	DWORD m_dw28;
	QWORD qword30;
	DWORD dword38;
	QWORD qword40;
	QWORD qword48;
	QWORD qword50;
	QWORD qword58;
	QWORD qword60;
	QWORD qword68;
	QWORD qword70;
	BYTE gap78[56];
	QWORD qwordB0;
	BYTE gapB8[28];
	DWORD dwordD4;
	BYTE gapD8[24];
	Vector3Aligned m_vF0;
	DWORD dword100;
	float dword104;
	BYTE gap108[12 +32];
	/*__unaligned __declspec(align(1)) QWORD qword114;
	__unaligned __declspec(align(1)) QWORD qword11C;
	__unaligned __declspec(align(1)) QWORD qword124;
	__unaligned __declspec(align(1)) QWORD qword12C;*/
	DWORD dword134;
	QWORD qword138;
	QWORD qword140;
	QWORD qword148;
	QWORD qword150;
	QWORD qword158;
	QWORD qword160;
	QWORD qword168;
	QWORD qword170;
	BYTE m_Wetness;		// 0x0178
	DWORD dword17C;
	BYTE gap180[80];
	CConstantBuffer** qword1D0;
	QWORD qword1D8;
	QWORD qword1E0;
	QWORD qword1E8;
	QWORD qword1F0;
	BYTE gap1F8[72];
	DWORD dword240;
	DWORD dword244;
	CModelData* m_pModelData;
};
VALIDATE_OFFSET(CModelExtendWork, m_ObjectId, 0x010);
VALIDATE_OFFSET(CModelExtendWork, m_pParent, 0x020);
VALIDATE_OFFSET(CModelExtendWork, m_vF0, 0x0F0);
VALIDATE_OFFSET(CModelExtendWork, dword134, 0x134);
VALIDATE_OFFSET(CModelExtendWork, m_Wetness, 0x178);


struct CVertexGroup
{
	struct CVertexBuffer* m_pVertexBuffer;	//0x00
	struct CVertexBuffer* m_pVertexBuffer2;	//0x08
	struct CIndexBuffer* m_pIndexBuffer;	//0x10
};
VALIDATE_SIZE(CVertexGroup, 24);

struct __declspec(align(8)) CModelDrawContext
{
	CVertexGroup* m_pVertexGroups;		// 0x0000
	BYTE gap8[8];						// 0x0008
	CConstantBufferInfo* m_pContexts;	// 0x0010
	BYTE gap18[4];						// 0x0018
	Topology m_Topology;				// 0x001C
};

struct CModelEntryData
{
	QWORD qword0;						//0x00
	CModelWork* m_pWork;				//0x08
	CModelDrawContext** m_pCtx;			//0x10
	DWORD m_iVertexOffset;				//0x18
	BYTE gap1C[4];						//0x1C
	CModelMatrixTable** m_pMatrices;	//0x20
	CModelInstanceParam** m_pParams;	//0x28
	CConstantBuffer** m_pConstBuffer;	//0x30
	BYTE gap38[44];						//0x38
	DWORD m_iStartIndex;				//0x64 | CConstantBufferInfo
	DWORD m_iCount;						//0x68
	DWORD m_nInstances;					//0x6C
	DWORD dword70;						//0x70
	DWORD dword74;						//0x74
};
VALIDATE_OFFSET(CModelEntryData, m_pParams, 0x28);

class CModelEntry
{
public:
	CModelEntryData* m_pData;
	CModelEntry* m_pNext;
};

class CModelShaderModuleBase
{
public:
	virtual __int64 function0(char flags) = 0;
	virtual void Update(CModelShader* pShader, CModelExtendWork* pWork, char flags) = 0; // flags & 2 UPDATE 1 = idk
	virtual __int64 ApplyExternalForces(CModelShader* pShader, CModelExtendWork* pWork) = 0;
	virtual __int64 Draw(CModelEntryData* pData) = 0; // Submits a job to the render queue (param could be wrong) maybe void

};

/*
Size of struct 0x90 (144) bytes
*/
class CModelShaderModule : public CModelShaderModuleBase
{
public:

	//void* m_pVtbl;						//0x0000
	CShaderSetting* m_pSetting;				//0x0008
	QWORD qword10;							//0x0010
	CConstantBuffer* m_pConstantBuffer;		//0x0018
	int32_t m_nConstantBuffers;				//0x0020
	char _0x0024[4];						//0x0024
	CSamplerParam* m_pSamplerParameters;	//0x0028
	int32_t m_nSamplerParameters;			//0x0030
	char _0x0034[4];						//0x0034
	CConstantBuffer* qword38;				//0x0038 | CConstantBuffer*
	DWORD dword40;							//0x0040
	char _0x0044[4];						//0x0044
	CConstantBuffer* qword48;				//0x0048 | CConstantBuffer*
	int32_t m_nQword48;						//0x0050
	DWORD dword54;							//0x0054
	QWORD qword58;							//0x0058
	Vector4 oword60;						//0x0060
	Vector4 oword70;						//0x0070
	int32_t int80;							//0x0080
	float m_flWetness;						//0x0084
	char pad[4];
};
VALIDATE_OFFSET(CModelShaderModule, m_pSetting, 0x8);
VALIDATE_OFFSET(CModelShaderModule, m_flWetness, 0x84);
VALIDATE_SIZE(CModelShaderModule, 0x90);

/*
Size of struct 0x318 (792) bytes
*/
class CModelShader
{
public:
	CMaterial* m_pMaterial;
	CModelShaderModule* m_pShader;
	CTextureResource m_Resources[MAX_MATERIAL_TEXTURES];
	char pad210[0x104];
	int32_t m_fFlags;	// should be 3?
};
VALIDATE_SIZE(CModelShader, 0x318);

class CModelAnalyzer
{
public:
	virtual void function0(char a2);
	virtual void LOD_sub_143F4B620(__int64 a2);
	virtual int32_t FindTextureIndexByName(const char* szName);
	virtual CModelShaderModule* CreateModelShaderModule(CMaterialDescription* pMaterialDescription, __int64 pModelWorkExtend, __int64 ppHeaps);
	virtual void function4();
	virtual void function5();
};

struct MaterialShaderInfo
{

	int32_t m_iShader;
	CModelShader* m_pShader;
};

// Size of struct 0xB0 (176) bytes
typedef struct CBone
{
	Vector3Aligned	m_vLocalPosition;	//0x00
	Vector3Aligned	m_vLocalRotation;	//0x10
	Vector3Aligned	m_vLocalScale;		//0x20
	Vector3Aligned	m_vPosition;		//0x30
	Vector3Aligned	m_vRotation;		//0x40
	Vector3Aligned	m_vScale;			//0x50
	Vector3Aligned	m_vTPosition;		//0x60 
	Vector3Aligned	m_vUnk;				//0x70 | maybe min, max?
	Vector3Aligned	m_vScaledRotation;	//0x80 | definetly some rotation vector (pitch, yaw, roll)
	CBone* m_pUnk;						//0x90
	int32_t m_iFlags;					//0x98 | bones in head and feathers are m_iFlags > 512 (0x200) { 0x208 (origin bone), 0x220 }
	char _0x8C[4];						//0x9C
	CBone* m_pParent;					//0xA0
	int16_t m_Id;						//0xA8
	char _0x9A[6];						//0xAA
} Bone;
VALIDATE_SIZE(CBone, 0xB0);


struct CHitbox
{
	Vector3Aligned m_vMax;
	Vector3Aligned m_vMin;
	int32_t m_iBone1;
	int32_t m_iBone2;
};

/*
* Could be called CModelPart
Mesh struct used by the engine
size = 0x70 (112) bytes
*/
struct CMeshPart
{
	Vector4 m_vColor;					//0x0000
	Vector4 m_vColorReadOnly;			//0x0010
	Vector3Aligned m_vMax;				//0x0020
	Vector3Aligned m_vMin;				//0x0030
	const char* m_szMeshName;			//0x0040 
	MaterialShaderInfo* m_pShaderInfo;	//0x0048 | important pointer 
	int32_t m_nShaderInfo;				//0x0050
	WMBBone* m_pBones;					//0x0058
	int32_t m_nBones;					//0x0060
	BOOL m_bShow;						//0x0064 | ? need to be synced with the other modifing threads
	BOOL m_bUpdate;						//0x0068 | ? fucks with color vec
	float m_flUnknown6C;				//0x006C
};

struct CVertexGroupEx
{
	void* m_pVertexData;		//0x00
	void* m_pVertexData2;		//0x08
	BYTE gap10[40];				//0x10
	void* m_pIndexData;			//0x38
	uint32_t m_uVertexSize;		//0x40
	uint32_t m_nIndices;		//0x44
	uint32_t m_uIndiceSize;		//0x48
};

struct CBatch
{
	int32_t m_iVertexGroupIndex;
	int32_t m_iBoneSetIndex;
	int32_t m_iVertexStart;
	int32_t m_iIndexStart;
	int32_t m_nVertices;
	int32_t m_nIndices;
	int32_t m_nPrimitives;
};
VALIDATE_SIZE(CBatch, 28);

struct CModelDrawData
{
	CVertexGroup* m_pVertexGroups;	//0x0000
	int32_t m_nVertexGroups;		//0x0008
	CBatch* m_pBatches;				//0x0010
	int32_t m_nBatches;				//0x0018
	int32_t m_nPrimitiveVertices;	//0x001C
};
VALIDATE_SIZE(CModelDrawData, 32);

struct CMesh
{
	Vector3Aligned m_vMax;		//0x00
	Vector3Aligned m_vMin;		//0x10
	const char* m_szName;		//0x20
	WMBMaterial* m_pMaterials;	//0x28
	int32_t m_nMaterials;		//0x30
	WMBBone* m_pBones;			//0x38
	int32_t m_nBones;			//0x40
	int32_t m_iLodLevel;		//0x44
};
VALIDATE_SIZE(CMesh, 0x50);

// Size of struct 0x140 (320) bytes
class CModelData
{
public:
	union {
		byte* m_pWMB;						//0x0000
		WMBHdr* m_pHdr;						//0x0000
	};
	short* m_pBoneIndexTranslationTable;	//0x0008
	Vector3Aligned m_vMin;					//0x0010
	Vector3Aligned m_vMax;					//0x0020
	int32_t m_iReferenceCount;				//0x0030
	int32_t m_iUnk;							//0x0034
	CModelDrawData* m_pDrawData;			//0x0038
	int32_t* m_piVertexExFlags;				//0x0040 
	int32_t m_nVertexGroups;				//0x0048
	char pad4C[4];							//0x004C
	CBone* m_pBones;						//0x0050
	void* m_pBoneMaps;						//0x0058
	int32_t m_nBones;						//0x0060
	BOOL m_bVisible;						//0x0064
	short* m_pBoneIndexTranslationTable2;	//0x0068
	CHitbox* m_pHitboxes;					//0x0070
	int32_t m_nHitboxes;					//0x0078
	char pad78[4];							//0x007C
	void* m_pBoneMap;						//0x0080
	int32_t m_nBoneMapEntries;				//0x0088
	char pad8C[4];							//0x008C
	short** m_pBoneSets;					//0x0090
	int32_t m_nBoneSets;					//0x0098
	char pad9C[4];							//0x009C
	void* m_pLODS;							//0x00A0
	int32_t m_nLODS;						//0x00A8
	char padAC[12];							//0x00AC
	CMesh* m_pMeshes;						//0x00B8
	int32_t m_nMeshes;						//0x00C0
	char padC4[4];							//0x00C4
	CMaterial* m_pMaterials;				//0x00C8
	int32_t m_nMaterials;					//0x00CC
	char padCC[4];							//0x00D0
	void* m_pUnks2;							//0x00D4
	int32_t m_nUnks2;						//0x00DC
	int32_t padE0;							//0x00E0
	BYTE gapE4[8];							//0x00E4
	DWORD m_WMB0xE;							//0x00EC
	BYTE gapF0[4];							//0x00F0
	DWORD m_dwIndex;						//0x00F4
	BYTE gapF8[48];
	CModelData* m_pNext;
	char pad9[8];
};
VALIDATE_OFFSET(CModelData, m_pBones, 0x50);
VALIDATE_OFFSET(CModelData, m_pLODS, 0xA0);
VALIDATE_OFFSET(CModelData, m_pMeshes, 0xB8);
VALIDATE_SIZE(CModelData, 0x140);

struct CModelDataList
{
	CModelData* m_pModelData;
	QWORD m_ptr;
	int32_t m_iIndex;
	int32_t m_iCapacity;
	QWORD qword18;
	CModelData* m_pLast;
	DWORD m_iSize;
	BYTE gap2C[12];
	void(__fastcall* pfunc38)(__int64);
};

// FIXME: wtf finish this unacceptable
struct CCameraDevice
{
	char p[32];
};

// FIXME: move to better file
struct __declspec(align(16)) CVertexLayoutEntry
{
	CVertexLayout* m_pVertex;
	BOOL m_bInitalized;
};

//size of struct is 0x110 (272) bytes
struct CModelManager
{
	QWORD qword00;						//0x00 | g_pModelResource
	QWORD qword08;						//0x08 | these are like heaps
	QWORD qword10;						//0x10
	CReadWriteLock m_Lock;				//0x18
	CModelAnalyzer* m_pModelAnalyzer;	//0x48
	CSamplerParam m_Param;				//0x50
	CCameraDevice* m_pCameraDevices;	//0x60
	int32_t m_nCameraDevices;			//0x68
	BYTE gap6C[4];						//0x6C
	QWORD qword70;						//0x70
	DWORD m_nOfQword70;					//0x78
	BYTE gap7C[4];						//0x7C
	CModelWork* m_pModelWorks;			//0x80
	CModelWork* m_pCurrentModelWork;	//0x88
	DWORD dword90;						//0x90
	DWORD dword94;
	DWORD dword98;
	DWORD dword9C;
	DWORD dwordA0;
	DWORD dwordA4;
	CVertexLayoutEntry* m_pVertexLayouts;
	QWORD qwordB0;
	CModelDataList m_ModelDataList;		//0xB8
	BYTE gapF8[24];						//0xF8
};
VALIDATE_SIZE(CModelManager, 0x110);

// name not official
struct CModelDataManager
{
	BYTE gap0[24];
	CReadWriteLock m_Lock;
	BYTE gap44[116];
	CModelDataList m_ModelDataList;
};

struct CModelInfo
{
	Vector4 m_vTint;			//0x0000
	CModelData* m_pData;		//0x0010
	char pad18[88];				//0x0018
	Vector3Aligned m_vPosition; //0x0070
	char pad90[0x50];			//0x0090
	UINT m_Flags;				//0x00D0 | |= 0x10u this happens a lot
};

struct CModelLOD
{
	int32_t int0;
	uint32_t m_uModelIndex;
	uint32_t m_uShadowModelIndex;
	DWORD dwordC;
	DWORD dword10;
	DWORD dword14;
	DWORD dword18;
	DWORD dword1C;
};


// size of struct 0x1A8 (424) bytes
class CModelWork
{
public:
	enum ERenderFlags
	{
		RF_NO_LOD = 0x20,
		RF_DONT_RENDER = 0x20000000,
	};

	CModelManager* m_pModelManager;				//0x0000
	CModelData* m_pModelData;					//0x0008
	CMeshPart* m_pMeshes;						//0x0010
	int32_t m_nMeshes;							//0x0018
	char pad1C[4];								//0x001C	
	CModelShader* m_pModelShaders;				//0x0020 | model shaders +0x20
	int32_t m_nMaterialShaders;					//0x0028 | (also is number of shaders)
	char pad2C[4];								//0x002C
	void* m_p0x030;								//0x0030
	int32_t m_n0x038;							//0x0038
	char _0x03C[12];							//0x003C
	void* m_p0x048;								//0x0048
	int32_t m_n0x48;							//0x0050
	char _0x054[92];							//0x0054
	//void*										//0x0060
	CModelLOD* m_pLODS;							//0x00A0
	int32_t m_nLODS;							//0x00A8
	char _0x0AC[0x24];							//0x00AC
	CModelMatrixTable** m_pMatrixTables;		//0x00E0
	CModelInstanceParam** m_pInstanceParams;	//0x00E8
	int32_t m_nModelParams;						//0x00F0
	char _0x0F4[140];							//0x00F4
	//CHitbox* m_pHitboxes;						//0x0110
	//int32_t m_nHitboxes;						//0x0118
	CModelExtendWork* m_pModelExtend;			//0x0180
	CModelExtendWork* m_pModelExtend2;			//0x0188
	DWORD m_dwRenderFlags;						//0x0190
	char _0x018C[4];							//0x018C
	CModelWork* m_pPrevious;					//0x0198
	CModelWork* m_pNext;						//0x01A0
};
VALIDATE_SIZE(CModelWork, 0x1A8);
VALIDATE_OFFSET(CModelWork, m_pModelShaders, 0x20);
VALIDATE_OFFSET(CModelWork, m_pMatrixTables, 0xE0);
VALIDATE_OFFSET(CModelWork, m_pModelExtend, 0x180);


/*
*
* Size of struct 0x584 (1412) bytes
* could be 0x590 bytes
*
* apprently supposed to inherit cparts??
*/
class CModel : public CParts
{
public:

	virtual void function0(void) = 0;
	virtual void UpdateModelPhysics(Vector4* v1, Vector4* v2, Vector4* v3) = 0;
	virtual void function2(void) = 0;

	Matrix4x4 m_matB0;						//0x00B0 | set to indentity matrix on construction
	Matrix4x4 m_matF0;						//0x00F0 | set to indentity matrix on construction
	CModelExtendWork m_ExtendWork;			//0x0140
	CModelWork m_Work;						//0x0390
	QWORD m_qw0x538;						//0x0538
	CModelInfo* m_pModelInfo;				//0x0540
	Unknown_t* m_pUnknown0x548;				//0x0548
	CModelData* m_pModelData;				//0x0550
	void* m_p0x00558;						//0x0558
	CBone* m_pBones;						//0x0560
	int32_t m_nBones;						//0x0568 | changes the amount of vertices to get updated each frame (can't be more than 198 for 2B)
	void* m_p0x00570;						//0x0570
	float m_fl00578;						//0x0578
	float m_fl0057C;						//0x057C
	DWORD dwFinal;							//0x0580
	char _0x0584[28];						//0x0584
};
VALIDATE_OFFSET(CModel, m_matTransform, 0x10);
VALIDATE_OFFSET(CModel, m_ExtendWork, 0x140);
VALIDATE_OFFSET(CModel, m_Work, 0x390);
VALIDATE_OFFSET(CModel, m_pModelInfo, 0x540);
VALIDATE_OFFSET(CModel, m_nBones, 0x568);
VALIDATE_OFFSET(CModel, dwFinal, 0x580);
VALIDATE_SIZE(CModel, 0x5A0);

#endif // !__CMODEL_H__