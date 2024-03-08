#pragma once

#include <d3d11.h>
#include <dxgi1_2.h>

#include "Common.h"

#include "FileSystem\dat.h"

#define MAX_MATERIAL_TEXTURES 16


// Forward Definitions
struct CHeapInfo; // temp move to actual file


struct CConstantBufferContext
{
	int m_iConstantBufferGroup;
	int m_iSize;					//0x04 idk not sure  set to 40 in COtManager::DrawModel
	int m_iConstantBufferIndex;
	BYTE gapC[8];
};

/*
 Cannot have size of zero
 Must be 16 byte aligned
 Size cannot exceed 0x10000 (65536) bytes
*/
class CConstantBuffer
{
public:
	void* m_vtbl;
	BYTE gap8[8];
	volatile signed __int32 volatile10;
	BYTE gap14[20];
	void* m_pResources;
	SIZE_T m_size;	//aligned to 16 bytes
	SIZE_T m_size2; //aligned to 16 bytes
	__int64 m_aligned_size;
	BYTE gap48[16];
	signed int m_index;
	BYTE gap5C[12];
	ID3D11Buffer** m_ppBuffer;
};

struct CBufferInfo
{
	ID3D11Buffer* m_pBuffer;
	DXGI_FORMAT m_Format;
	ID3D11ShaderResourceView* m_pShaderResourceView;
	ID3D11UnorderedAccessView* m_pUnorderedAccessView;
};
VALIDATE_SIZE(CBufferInfo, 0x20);

// Size of struct is 56 (0x38) bytes
struct CVertexBuffer
{
	void* m_pVtbl;
	QWORD qword8;						//0x08
	BOOL m_bInitalized;					//0x10
	UINT m_uVertexSize;					//0x14
	INT m_nVertices;					//0x18
	char pad1c[12];						//0x1C
	CBufferInfo* m_pInfo;				//0x28
	DWORD dword30;
};
VALIDATE_SIZE(CVertexBuffer, 0x38);

// Size of struct is 48 (0x30) bytes
struct CIndexBuffer
{
	void* m_pVtbl;						//0x00
	BOOL m_bInitalized;					//0x0C
	UINT m_uIndiceSize;					//0x08	
	INT m_nIndices;						//0x10
	char pad14[12];						//0x14
	CBufferInfo* m_pInfo;				//0x20
	DWORD dword28;
};
VALIDATE_SIZE(CIndexBuffer, 0x30);

enum TextureFlags
{
	USE_ID_ZERO = 0x20,	// SINGLE TEXTURE IN BATCH???
	LAST_TEXTURE = 0x40,	// 1<<6
	HAS_DX10_HEADER = (1 << 25),
	FORCE_SRGB = (1 << 26),
	TEXTURE3D = (1 << 27),
	TEXTURE2D = (1 << 28),
	TEXTUREBUFFER = (1 << 29),
};

enum TextureDimension
{
	TEXTURE_DIMENSION_CUBEMAP = 2
};

struct CTextureD3D
{
	union {
		ID3D11Texture1D* m_pTexture1D;
		ID3D11Texture2D* m_pTexture2D;
		ID3D11Texture3D* m_pTexture3D;
	};
	ID3D11ShaderResourceView* m_pDefaultShaderResourceView;
	QWORD qword10;
	ID3D11ShaderResourceView* m_pShaderResourceViews[16];
};

/*
Size of struct is 0xB0 (176)
*/
struct CTexture
{
	void* m_vtbl;					//0x00
	int m_iWidth;					//0x08
	int m_iHeight;					//0x0C
	int m_Depth;					//0x10
	BYTE gap14[8];					//0x14
	BOOL m_bForceSRGB;				//0x1C
	UINT m_uMipMapCount;			//0x20
	DXGI_FORMAT m_Format;			//0x24
	BYTE gap28[8];					//0x28
	TextureDimension m_Dimension;	//0x30
	BYTE gap34[60];					//0x34
	CTextureD3D* m_pTexture;		//0x70
	char gap78[56];					//0x78
};
VALIDATE_OFFSET(CTexture, m_Depth, 0x10);
VALIDATE_OFFSET(CTexture, m_Dimension, 0x30);

struct CTargetTexture
{
	void* m_vtbl;
	CTexture* m_pTexture;
	BOOL m_bInitalized;
};

struct CTextureResource
{
	void* m_punk;	//cTexture::vtble
	CTexture* m_pTexture;
	char pad10[8];
	unsigned int m_TextureId;
	char pad1C[4];
};

struct CTextureBatch;

// size of struct 96 (0x60) bytes
struct CTextureData
{
	DATHeader* m_pHeaders[2];
	struct CTextureData* m_pPrevious;
	struct CTextureData* m_pNext;
	BYTE gap20[8];
	void* m_pWTAFile;
	void* m_pWTPFile;
	CTextureBatch* m_pTextures;
	DWORD m_dwTextureCount;
	BOOL m_bUnk;
	DWORD dword48;
	DWORD dword4C;
	char pad0x50[16];
};
VALIDATE_SIZE(CTextureData, 96);

struct CTextureBatch
{
	CTextureData* m_pData;
	CTargetTexture m_Texture;
	BOOL m_bLast;
	char pad24[4];
};
VALIDATE_SIZE(CTextureBatch, 40);

struct DDS_PIXELFORMAT
{
	uint32_t size;
	uint32_t flags;
	uint32_t fourCC;
	uint32_t RGBBitCount;
	uint32_t RBitMask;
	uint32_t GBitMask;
	uint32_t BBitMask;
	uint32_t ABitMask;
};

struct DDS_HEADER
{
	uint32_t size;
	uint32_t flags;
	uint32_t height;
	uint32_t width;
	uint32_t pitchOrLinearSize;
	uint32_t depth;
	uint32_t mipMapCount;
	uint32_t reserved1[11];
	DDS_PIXELFORMAT ddspf;
	uint32_t caps;
	uint32_t caps2;
	uint32_t caps3;
	uint32_t caps4;
	uint32_t reserved2;
};

/*
	Size of struct 0x14 (20) bytes
*/
struct DDS_HEADER_DXT10
{
	DXGI_FORMAT dxgiFormat;			//0x00
	uint32_t    resourceDimension;	//0x04
	uint32_t    miscFlag;			//0x08 see D3D11_RESOURCE_MISC_FLAG
	uint32_t    arraySize;			//0x0C
	uint32_t    miscFlags2;			//0x10
};

struct TextureFile;

struct CTextureDescription
{
	DDS_HEADER_DXT10* m_pHeaderDX10;
	TextureFile* m_pDDS;
	unsigned int m_uTextureSize;	//size of dds file
	DWORD gap14;					// 0,2
	DWORD dword18;					// 1-4
	BOOL m_b1C;
	BOOL m_b20;
	DWORD dword24;
	BOOL m_bHasDX10Header;
};

struct TextureFile
{
	uint32_t magic;
	DDS_HEADER hdr;
	CTextureDescription m_Desc;	//might be just DDS_HEADER_DXT10
};

// CVertexLayoutImpl is 320 bytes in size
struct CVertexLayout
{
	void* m_vtbl;
	BYTE gap0[304];
	ID3D11InputLayout** m_ppInputlayout;
};
VALIDATE_SIZE(CVertexLayout, 320);

struct CConstantBuffers
{
	BOOL m_bSetBuffers;
	ID3D11Buffer* m_pBuffers[14];
};

enum Topology
{
	TOPOLOGY_POINTLIST,
	TOPOLOGY_LINELIST,
	TOPOLOGY_LINESTRIP,
	TOPOLOGY_TRIANGLELIST,
	TOPOLOGY_TRIANGLESTRIP,
	TOPOLOGY_UNDEFINED
};

struct RenderInfo
{
	Topology m_Topology;
	INT m_iBaseVertexLocation;
	UINT m_uPrimitives; //???
	UINT m_uVertexCount;
	UINT m_uIndexCountPerInstance;
};

struct CGraphicObjectFactoryDx11
{
	ID3D11Device* m_pDevice;			//0x0000
	IDXGIFactory* m_pFactory;			//0x0008
	char _0x0010[12];					//0x0010
	DXGI_SAMPLE_DESC m_SampleDescs[5];	//0x001C
	char _0x0050[4];
	QWORD qword48;
	QWORD qword50;
};

struct CShaderMemorizeVS
{
	void* vtbl;
	char m_szName[48];
};

struct CShaderMemorizePS
{
	void* vtbl;
	char m_szName[48];
};

struct CShaderMemorizeCS
{
	void* vtbl;
	char m_szName[48];
};

struct ShaderDescVS
{
	BYTE gap0[8];					//0x00
	const void* m_pShaderBinary;	//0x08
	SIZE_T m_ShaderSize;			//0x10
	CShaderMemorizeVS* m_pShader;	//0x18
	BOOL m_b0x18[2];				//0x20
	char m_szName[48];				//0x28
};

struct ShaderDescPS
{
	BYTE gap0[8];					//0x00
	const void* m_pShaderBinary;	//0x08
	SIZE_T m_ShaderSize;			//0x10
	CShaderMemorizePS* m_pShader;	//0x18
	BOOL m_b0x18[2];				//0x20
	char m_szName[48];				//0x28
};

struct ShaderDescCS
{
	BYTE gap0[8];					//0x00
	const void* m_pShaderBinary;	//0x08
	SIZE_T m_ShaderSize;			//0x10
	CShaderMemorizeCS* m_pShader;	//0x18
	BOOL m_b0x18[2];				//0x20
	char m_szName[48];				//0x28
};

struct PixelShaderInfo
{
	ID3D11PixelShader* m_pShader;	//0x00
	const void* m_pShaderBinary;	//0x08
	SIZE_T m_ShaderSize;			//0x10
};

struct VertexShaderInfo
{
	ID3D11VertexShader* m_pShader;	//0x00
	const void* m_pShaderBinary;	//0x08
	SIZE_T m_ShaderSize;			//0x10
};

struct ComputeShaderInfo
{
	ID3D11ComputeShader* m_pShader;	//0x00
	const void* m_pShaderBinary;	//0x08
	SIZE_T m_ShaderSize;			//0x10
};

struct PixelShaderContext
{
	BYTE gap0[16];
	struct PixelShaderInfo* m_pInfo;
};

struct VertexShaderContext
{
	BYTE gap0[16];
	struct VertexShaderInfo* m_pInfo;
};

struct ComputeShaderContext
{
	BYTE gap0[16];
	struct ComputeShaderInfo* m_pInfo;
};

struct COcclusionQuery
{
	BYTE gap0[40];
	ID3D11Query** m_ppQuery;
};

// size of struct 0xF0 (240) bytes
struct TextureGroup
{
	BOOL m_bUpdateTextures;
	ID3D11ShaderResourceView* m_pTextures[32];
};

/*
Size of struct 264 (0x108) bytes
*/
struct TextureContainer
{
	struct TextureGroup m_Textures;
	char padF0[24];
};

struct CSamplerState
{
	BOOL m_bSetSamplers;
	ID3D11SamplerState* m_pSamplerStates[16];
};

// TODO: reverse
struct CDepthStencilStateInfo;

struct CDepthStencilState
{
	void* m_pVtbl;
	BOOL m_bDepthEnable;
	D3D11_DEPTH_WRITE_MASK m_DepthWriteMask;
	D3D11_COMPARISON_FUNC m_DepthFunc;
	DWORD dword14;
	BOOL m_bStencilEnable;
	UINT m_uStencilRef;
	DWORD dword20;
	DWORD dword24;
	DWORD dword28;
	DWORD dword2C;
	DWORD dword30;
	DWORD dword34;
	DWORD dword38;
	DWORD dword3C;
	BYTE gap40[8];
	ID3D11DepthStencilState** m_ppDepthStencilState;
};

struct CSurface
{
	ID3D11Texture2D* m_pTexture;
	ID3D11DepthStencilView* m_pDepthStencilView;
};

//implement these when possible
struct CDepthSurface
{
	void* vtbl;
	int m_iWidth;
	int m_iHeight;
	char unk[56];
	struct CSurface* m_pSurface;
};

// Size of struct 104 (0x68) bytes
struct CRenderTarget
{
	void* vtbl;
	int m_iWidth;	// could be uint
	int m_iHeight;
	char pad[80];
	CTexture* m_pTexture;
	//const Hw::cRenderTargetImpl::`vftable'{for `Hw::cGraphicObjectBase' }
};

/*
Size of struct 504 (0x1F8) bytes //wrong
*/
struct CGraphicRenderContext
{
	VertexShaderInfo* m_pVertexShader;			//0x000
	PixelShaderInfo* m_pPixelShader;			//0x008
	BOOL m_bSetShaders;							//0x010
	BYTE gap14[4];								//0x014
	BOOL m_bSetVertexBuffer;					//0x018
	BYTE gap1C[4];								//0x01C
	ID3D11Buffer* const m_pVertexBuffers[8];	//0x020
	UINT m_uOffsets[8];							//0x060
	UINT m_uStrides[8];							//0x080
	ID3D11InputLayout** m_ppInputLayout;		//0x0A0 | probs a struct w/ the ID3D11InputLayout* at offset 0
	CIndexBuffer* m_pIndexBuffer;				//0x0A8
	DWORD m_dw0xB0;								//0x0B0
	BOOL m_bSetInputLayout;						//0x0B4
	BOOL m_bSetIndexBuffer;						//0x0B8
	UINT m_uTopologyIndex;						//0x0BC
	CConstantBuffers m_ConstantBuffers[2];		//0x0C0
	TextureContainer m_TextureContainer;		//0x1B0
};
VALIDATE_OFFSET(CGraphicRenderContext, m_TextureContainer, 0x1B0);


struct CSwapChainInfo
{
	unsigned int m_uWidth;
	unsigned int m_uHeight;
	float m_flRefreshRate;
	BOOL m_b0;
	BOOL m_bFullscreen;
	BOOL m_b2;
};


/*
* This struct changed across versions (size increase)
*
* Size of struct 0xC8 (200) bytes
*/
struct CSwapChain
{
	void* pUnknown0x00;				//0x00
	ID3D11DeviceContext* m_pContext;//0x08
	ID3D11Device* m_pDevice;		//0x10
	IDXGISwapChain* m_pSwapChain;	//0x18
	CRenderTarget m_Target;			//0x20
	char pad2[48];					//0x68
	HWND m_hWindow;					//0xB8
	BOOL m_bSyncInterval;			//0xC0
};
VALIDATE_OFFSET(CSwapChain, m_bSyncInterval, 0xC0);
VALIDATE_SIZE(CSwapChain, 200);

struct CGraphicOutputDx11
{
	DXGI_OUTPUT_DESC m_Desc;
	char extra[184];
};

/*
* https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-querydisplayconfig
*
*   Size of struct 0xB8 (184) bytes  old version (denuvo)
*	Size of struct 0xB8 (184) bytes  new version
*/
struct COutput
{
	wchar_t m_szDeviceName[32];
	BYTE gap0[64];
	IDXGIOutput* m_pOutput;
	DXGI_MODE_DESC* m_pDisplayModes;
	int m_nDisplayModes;
	BYTE gap94[4];
	QWORD qword98;
	DWORD dwordA0;
	DWORD dwordA4;
	DWORD dwordA8;
	DWORD dwordAC;
	DWORD dwordB0;
	DWORD dwordB4;
};

/*
*
*   Size of struct 0x118 (280) bytes  old version (denuvo)
*	Size of struct 0x118 (280) bytes  new version
*/
struct CAdapter
{
	wchar_t m_szDescription[128];
	IDXGIAdapter* m_pAdapter;
	COutput* m_pOutputs;
	UINT m_uOutputs;
};

class CGraphicDeviceDx11;
class CGraphics;

/*
*	This struct changed across versions (size increase)
*	CGraphics +0x30
*/
class CDisplay
{
public:
	void* m_pVtble;							//0x0000
	DWORD dwUnk0x0008;						//0x0008
	int m_iWidth;							//0x000C
	int m_iHeight;							//0x0010
	DWORD dwUnk0x0014;						//0x0014
	void* m_pConstantBuffer[2];				//0x0018
	char _0x0030[64];						//0x0028
	CSwapChain* m_pWindowedSwapChain;		//0x0068
	char _0x0070[8];						//0x0070
	CSwapChain* m_pSwapChain;				//0x0078
	int m_iUnk;								//0x0080
	char _0x006C[4];						//0x0084
	CGraphicDeviceDx11* m_pGraphicDevice;   //0x0088
};
VALIDATE_OFFSET(CDisplay, m_pWindowedSwapChain, 0x68);

/*
This struct is the same size on both versions

Size of struct 0x580 (1408) bytes	old version (denuvo)
Size of struct 0x580 (1408) bytes	new version
*/
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
		enum ClearFlags
		{
			NONE,
			CLEAR_RENDERTARGETVIEW = 1,
			CLEAR_DEPTH = 2,
			CLEAR_STENCIL = 4
		};

		DWORD m_dwColor; //argb format
		FLOAT m_flDepth;
		BYTE m_Stencil;
		ClearFlags m_Flags;
	};

	struct TextureContext
	{
		int m_iGroup;
		int m_iIndex; //valid range 0 - 31 (clamped)
	};

	virtual void function0() PURE;
	virtual unsigned int GetUINT0x4(__int64 a2) PURE; // func = return *(uint*)(a2+4);
	virtual BOOL SetRenderTarget(CDisplay* pDisplay, CDepthSurface* pSurface) PURE;
	virtual BOOL function3() PURE; // returns 1
	virtual BOOL MapResource3ReadOnly(__int64 pResource) PURE;
	virtual BOOL MapResource3(__int64 pResource) PURE;
	virtual BOOL UnmapResource3(__int64 pResource) PURE;
	virtual BOOL MapResource2(void** ppMappedData, __int64 pResource) PURE;
	virtual BOOL UnmapResource2(__int64 pResource) PURE;
	virtual BOOL MapResource(void** ppMappedData, __int64 pResource) PURE;
	virtual BOOL UnmapResource(__int64 pResource) PURE;
	virtual BOOL Clear(Clear_t* pClearArgs) PURE;
	virtual BOOL function12() PURE; // returns 1
	virtual BOOL SetScissorRect(int x, int y, int width, int height) PURE;
	virtual BOOL SetViewport(int x, int y, int width, int height, float min_depth, float max_depth) PURE;
	virtual BOOL function15() PURE; // returns 1
	virtual BOOL function16() PURE;
	virtual BOOL function17() PURE;
	virtual BOOL Resolve(CRenderTarget* pTarget, unsigned int a3, RECT* pRect) PURE; // used when changing resolutions?
	virtual BOOL CopyDepthSurface(CDepthSurface* pDst) PURE;
	virtual BOOL function20() PURE; // returns 0
	virtual BOOL SetInputLayout(CVertexLayout* pVertexLayout) PURE; //SetVertexLayout
	virtual BOOL SetVertexBuffer(UINT index, CVertexBuffer* pVertexBuffer) PURE;
	virtual BOOL InvalidateVertexBuffer(UINT index) PURE;
	virtual BOOL SetIndexBuffer(void* pIndexBuffer) PURE;
	virtual BOOL InvalidateIndexBuffer() PURE;
	virtual BOOL SetVertexShader(VertexShaderContext* pCtx) PURE;
	virtual BOOL SetPixelShader(PixelShaderContext* pCtx) PURE;
	virtual BOOL SetConstantBuffer(CConstantBufferContext* pCtx, CConstantBuffer* pBuffer) PURE; //model params & matrices etc
	virtual BOOL function29() PURE;
	virtual BOOL function30() PURE;
	virtual BOOL function31() PURE;
	virtual BOOL function32() PURE;
	virtual BOOL function33() PURE;
	virtual BOOL ClearUnorderedAccessViewUint(__int64 a2, const UINT a3) PURE;
	virtual BOOL SetTexture(TextureContext* pCtx, CTexture* pTexture, int iShaderResourceView) PURE;
	virtual BOOL UnsetTexture(CTexture* pTexture) PURE;
	virtual BOOL SetSamplerState() PURE;
	virtual BOOL SetBlendState(__int64 p) PURE;
	virtual BOOL SetRasterizerState(__int64 p) PURE;
	virtual BOOL SetDepthStencilState(CDepthStencilState* pState) PURE;
	virtual BOOL SetBlendFactor(UINT color) PURE;
	virtual BOOL SetTopology(Topology topology) PURE;
	virtual BOOL BeginOcclusionQuery(COcclusionQuery* pQuery) PURE;
	virtual BOOL EndOcclusionQuery(COcclusionQuery* pQuery) PURE;
	virtual BOOL DrawPrimitive(RenderInfo* pInfo) PURE;
	virtual BOOL DrawIndexedPrimitive(RenderInfo* pInfo) PURE;
	virtual BOOL DrawPrimitiveInstanced(RenderInfo* pInfo) PURE;
	virtual BOOL DrawIndexedPrimitiveInstanced(RenderInfo* pInfo) PURE;
	virtual BOOL DrawPrimitiveInstancedIndirect(RenderInfo* pInfo) PURE;
	virtual BOOL DrawIndexedPrimitiveInstancedIndirect(RenderInfo* pInfo, __int64 a3, unsigned int AlignedByteOffsetForArgs) PURE;
	virtual void Flush(void) PURE;
	virtual int GetRenderWidth(void) PURE;
	virtual int GetRenderHeight(void) PURE;

	//void* m_pVtbl;								//0x0000
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
	CGraphicRenderContext m_RenderContext;			//0x0098
	char pad368[512];								//0x0368
	int m_iRenderWidth;								//0x0568
	int m_iRenderHeight;							//0x056C
	char pad570[8];									//0x0570
	ID3D11Device* m_pDevice;						//0x0578
};
VALIDATE_SIZE(CGraphicContextDx11, 1408);
VALIDATE_OFFSET(CGraphicContextDx11, m_RenderContext, 0x98);
VALIDATE_OFFSET(CGraphicContextDx11, m_iRenderWidth, 0x568);

/*
this structure is used to create d3d and is used in many of NieR: Automata's
direct3d functions. It might have more to it that I haven't explored so, this is just
the basic layout of the stucture. Strangley enough, it has device pointers and factory
pointers but not a swapchain pointer nor an adapter pointer. Perhaps, I just have not
disovered them. In addition I haven't found a global pointer that directly points to
this structure. This is a virtual class after further investigation. Correctly
renamed the class to it's proper name. It seems there might be a nested struct too.

 Size of struct 0x110 (272) bytes  old version (denuvo)
 Size of struct 0x140 (320) bytes  new version
 */
class CGraphicDeviceDx11
{
public:
	virtual CGraphicDeviceDx11* function0();
	virtual BOOL Create(CGraphicContextDx11** ppContext, CDisplay* pDisplay, HWND hWnd, void* pInfo);
	virtual BOOL function2();
	virtual BOOL function3();
	virtual BOOL function4();
	virtual BOOL function5();
	virtual BOOL Render(CGraphics* pGraphics, DWORD* a2, CDisplay* pDisplay);
	virtual BOOL function7();
	virtual BOOL CreateComputeContext(__int64 a1, __int64 a2);
	virtual BOOL function9();
	virtual BOOL function10();
	virtual void function11();
	virtual BOOL function12();
	virtual BOOL function13();
	virtual BOOL function14();
	virtual BOOL LoadDirectDrawSurface(CTexture* pTexture, void* pFile); // index: 11 now according to ida
	virtual BOOL CreateIndexBuffer(CVertexBuffer* pBuffer, __int64* a2);
	virtual BOOL CreateImmutableVertexBuffer(CVertexBuffer* pBuffer, const void* pData, int* pcbVertexSizes, int index, int iElement);
	virtual BOOL CreateDynamicVertexBuffer(CVertexBuffer* pBuffer, unsigned int uByteWidth);
	virtual BOOL CreateIndexBuffer(CIndexBuffer* pBuffer, __int64* a2);
	virtual BOOL CreateImmutableIndexBuffer(CIndexBuffer* pBuffer, const void* pData, int cbIndexSize, int nIndices);
	virtual BOOL CreateDynamicIndexBuffer(CIndexBuffer* pBuffer, int iElements, int cbElementSize);
	virtual BOOL CreateInputLayout(__int64 a2, const void* pDescriptions, unsigned int nDescs, VertexShaderContext* pCtx);
	virtual BOOL CreateVertexShader(VertexShaderContext* pCtx, ShaderDescVS* pDesc);
	virtual BOOL CreatePixelShader(PixelShaderContext* pCtx, ShaderDescPS* pDesc);
	virtual BOOL CreateComputeShader(ComputeShaderContext* pCtx, ShaderDescCS* pDesc);
	virtual BOOL function26();
	virtual BOOL function27();
	virtual void function28();
	virtual BOOL function29();
	virtual BOOL function30();
	virtual BOOL function31();
	virtual BOOL function32();
	virtual BOOL function33();
	virtual BOOL CreateDepthStencilState(CDepthStencilState* pState, CDepthStencilStateInfo* pInfo);
	virtual BOOL CreateSamplerState();
	virtual BOOL CreateOcclusionQuery(COcclusionQuery* pQuery);

public:
	//void* pVtable;						//0x0000
	int m_iScreenWidth;						//0x0008 
	int m_iScreenHeight;					//0x000C 
	float flScreenHz;						//0x0010 / fps / 60hz capped
	BOOL m_bFullscreen;						//0x0014 | UINT
	BOOL m_bUnk0x018;						//0x0018
	BOOL m_bUnk0x01C;						//0x001C
	BOOL m_bAcquiredFullscreen;				//0x0020
	BOOL m_bUnk0x024;						//0x0024
	BOOL m_bUnk0x028;						//0x0028
	BOOL m_bUnk0x02C;						//0x002C
	BOOL m_bFullscreen2;					//0x0030
	BOOL m_bUnk0x034;						//0x0034
	IDXGIOutput* m_pOutput;					//0x0038
	UINT m_uFullscreenTimer;				//0x0040
	char _0x0024[4];						//0x0044
	ID3D11Device* m_pDevice;				//0x0048
	IDXGIFactory* m_pFactory;				//0x0050 | IDXGIFactory2*
	HWND m_hWindow;							//0x0058
	CVertexBuffer* m_pVertexBuffer;			//0x0060 | Hw::cVertexBufferImpl
	ID3D11Device* m_pID3D11Device2;			//0x0068
	IDXGIFactory2* m_pFactory2;				//0x0070		
	CVertexLayout* m_pVertexLayout;			//0x0078 | Hw::cVertexLayoutImpl
	CConstantBuffer* m_pConstantBuffer;		//0x0080 | Hw::cConstantBufferImpl
	char _0x0048[8];						//0x0088
	CGraphicObjectFactoryDx11 m_Factory;	//0x0090
	char _0x00A4[32];						//0x00B4

	//void* pd3dunk;						//0x00A0 

	BOOL m_bWindowed;						//0x0108 | UINT
	float m_flFrameRate;					//0x00E4
	int m_iWidth;							//0x0120
	int m_iHeight;							//0x0124
	D3D11_QUERY_DESC m_OccusionDesc;		//0x00F0
	//D3D11_QUERY_DESC m_TimestampDesc;		//0x00F8
	int m_iAdapter;							//0x0120
	int m_iOutput;							//0x0124
	CAdapter* m_pAdapters;					//0x0128
	UINT m_uAdapters;						//0x0130
	BOOL m_b0x0134;							//0x0134
	void* m_pUnk0x0138;						//0x0138
};
VALIDATE_OFFSET(CGraphicDeviceDx11, m_pOutput, 0x38);
VALIDATE_OFFSET(CGraphicDeviceDx11, m_bWindowed, 0x108);
VALIDATE_OFFSET(CGraphicDeviceDx11, m_pAdapters, 0x128);
VALIDATE_SIZE(CGraphicDeviceDx11, 320);

/*
* Struct is used to contruct CGraphics!
* Stack var size of struct is guessed!
* 
*/
struct CGraphicCreateContext
{
	int m_ScreenType;		//0x0000 | windowed, fullscreen, borderless ????
	int m_Width;			//0x0004
	int m_Height;			//0x0008
	char pad0C[4];			//0x000C
	CHeapInfo* m_pHeapInfo;	//0x0010
	QWORD* m_Ptr18;			//0x0018
	QWORD* m_Ptr20;			//0x0020
	QWORD* m_Ptr28;			//0x0028
	int m_Unk30;			//0x0030 | ida only says 3
	BOOL m_bUnk34;			//0x0034
	int m_UnkFlag38;		//0x0038
	char pad3C[4];			//0x003C
	__int64 m_Unk40;		//0x0040
	Vector4* m_pvUnk48;		//0x0048
	int m_Unk50;			//0x0050
	int m_Unk54[9];			//0x0054
	bool m_bUnk;			//0x0078
	char pad79[3];			//0x0079
	int m_Unk7C[4];			//0x007C
};


/*
* Size of struct 0x0F0 (240) bytes	old version (denuvo)
* Size of struct 0x320 (800) bytes	new version
*/
class CGraphics
{
public:
	BOOL m_b0x00;							//0x0000
	char unk0x04[4];						//0x0004
	void* m_pComputeCtx;					//0x0008 | gets passed to CreateComputeContext
	UINT m_uUAVStartSlot;					//0x0010
	char unk0x0C[4];						//0x0014
	void* m_pSamplerState;					//0x0018
	DWORD dw0x18;							//0x0020
	char unk0x1C[4];						//0x0024
	CGraphicContextDx11* m_pContext;		//0x0028
	CDisplay m_Display;						//0x0030
	char _0x00C0[504];						//0x00C0
	CGraphicDeviceDx11* m_pGraphicalDevice;	//0x02B8
	CHeapInfo* m_pHeapInfo;					//0x02C0 
	CReadWriteLock m_Lock;					//0x02C8
	char padCC[12];							//0x02F8
	int m_iTimeStep;						//0x00DC |  g_flDeltaTime * 3000.0 
	char padE0[20];							//0x00E0
};
VALIDATE_OFFSET(CGraphics, m_pContext, 0x28);
VALIDATE_OFFSET(CGraphics, m_Display, 0x30);
VALIDATE_OFFSET(CGraphics, m_pGraphicalDevice, 0x2B8);
VALIDATE_OFFSET(CGraphics, m_Lock, 0x2C8);
VALIDATE_SIZE(CGraphics, 0x320);