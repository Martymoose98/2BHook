#pragma once

#include "..\Common.h"
#include "..\Graphics.h"


typedef struct CSamplerParam
{
	CConstantBuffer* m_pBuffer;
	BOOL m_bInitalized;
} CModelMatrix;

struct CSamplerParameterGroup
{
	CSamplerParam* m_pParameters;
	int m_nParameters;
	int m_iIndex;
};

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


// Size of struct is 120 (0x78) bytes
struct CMaterial
{
	void* m_pHeap;
	const char* m_szName;
	const char* m_szShaderName;
	const char* m_szTechniqueName;
	unsigned int m_TextureIds[16];
	CSamplerParameterGroup* m_pParameterGroups;
	int m_nParameterGroups;
	int m_nShaderParameters;
	CSamplerParam* m_pParameters;
};
VALIDATE_SIZE(CMaterial, 0x78);

struct CMaterialDescription
{
	CMaterial* m_pMaterial;
	BOOL m_bUnknownsExist;
	BOOL m_bUnkGreZero;
};
