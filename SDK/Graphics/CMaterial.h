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
