
#ifndef __WMB_H__
#define __WMB_H__

#include "Common.h"

/*
Size of struct 0x88 (136) bytes

dummy.wmb is just this header structure
*/
typedef struct WMBHeader
{
	union {
		char			id[4];			// 0x00
		unsigned int	magic;
	};
	unsigned int		version;			// 4
	int					unknown8;			// 8
	short				unknownC;			// C Seems related to vertex index size
	short				unknownE;			// E
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
	unsigned int		ofsHitboxes;		//50
	int					numHitboxes;		//54
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
} WMBHeader, WMBHdr;

struct WMBTexture
{
	unsigned int m_uNameOffset;
	int m_TextureId;
};

struct WMBBatch
{
	int m_iVertexGroupIndex;	//0x00
	int m_nPrimitives;			//0x04
	int m_nIndices;				//0x08
	int m_nVertices;			//0x0C
	int m_iIndexStart;			//0x10
	int m_iVertexStart;			//0x14
	int m_iBoneSetIndex;		//0x18
};

struct WMBVertexGroup
{
	unsigned int m_uVertexOffset;		//0x00
	unsigned int m_uVertexExOffset;		//0x04
	int pad8[2];						//0x08
	unsigned int m_uVertexSize;			//0x10
	unsigned int m_uVertexExSize;		//0x14
	int pad18[2];						//0x18
	int m_nVertices;					//0x20
	int m_uVertexExFlags;				//0x24
	unsigned int m_uIndexBufferOffset;	//0x28
	int m_nIndices;						//0x2C
};
VALIDATE_SIZE(WMBVertexGroup, 0x30);

struct WMBMaterial
{
	short m_sVersion[4];					//0x00
	unsigned int m_uNameOffset;				//0x08
	unsigned int m_uShaderNameOffset;		//0x0C
	unsigned int m_uTechniqueOffset;		//0x10
	int m_nShaderParameters;				//0x14			
	unsigned int m_uTexturesOffset;			//0x18
	int m_nTextures;						//0x1C
	unsigned int m_uParameterGroupsOffset;	//0x20
	int m_nParameterGroups;					//0x24
	unsigned int m_uVariablesOffset;		//0x28
	int m_nVariables;						//0x2C
};
VALIDATE_SIZE(WMBMaterial, 0x30);


// Size of struct 0x58 (88) bytes
typedef struct WMBBone
{
	short	m_sId;				//0x00
	short	m_sParentIndex;		//0x02 | parent id
	Vector3	m_vLocalPosition;	//0x04
	Vector3	m_vLocalRotation;	//0x10
	Vector3	m_vLocalScale;		//0x1C
	Vector3	m_vPosition;		//0x28
	Vector3	m_vRotation;		//0x34
	Vector3	m_vScale;			//0x40
	Vector3	m_vTPosition;		//0x4C
} WMBBone;

struct WMBMesh
{
	unsigned int m_uNameOffset;
	Vector3 m_vMax;
	Vector3 m_vMin;
	unsigned int m_uMaterialsOffset;
	int m_nMaterials;
	unsigned int m_uBonesOffset;
	int m_nBones;
};
VALIDATE_SIZE(WMBMesh, 0x2C);

struct WMBHitbox
{
	Vector3 m_vMax;
	Vector3 m_vMin;
	int m_iBone1;
	int m_iBone2;
};

struct WMBSamplerParameterGroup
{
	int m_iIndex;						//0x00
	unsigned int m_uParametersOffset;	//0x04 float*
	int m_nParameters;					//0x08
};

struct WMBVariable
{
	unsigned int m_uNameOffset;		//0x00
	float m_flValue;				//0x04
};

#endif	// !__WMB_H__