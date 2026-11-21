#include "EspRenderer.h"

#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <stdio.h>
#include <string.h>

#include "Log.h"

namespace EspRenderer
{
	// ---------------------------------------------------------------- shaders

	// One triangle over the whole target, uv 0,0 at its top left
	static const char s_szFullscreenVS[] =
		"struct VSOut { float4 pos : SV_Position; float2 uv : TEXCOORD0; };\n"
		"VSOut main(uint id : SV_VertexID)\n"
		"{\n"
		"	VSOut o;\n"
		"	o.uv = float2((id << 1) & 2, id & 2);\n"
		"	o.pos = float4(o.uv * float2(2.0, -2.0) + float2(-1.0, 1.0), 0.0, 1.0);\n"
		"	return o;\n"
		"}\n";

	// The mask: the colour and, in alpha, the flags (out of 255), as constants. No inputs, so it
	// links with any of the game's vertex shaders, and no constant buffer, so the game's pixel
	// shader constants are never rebound around its draws (their offsets wouldn't survive that).
	static const char s_szMaskPSFormat[] = "float4 main() : SV_Target { return float4(%.9g, %.9g, %.9g, %.9g); }\n";

	// What the passes below share. g_UV maps the back buffer's uv onto the mask (xy scale, zw
	// offset: the part of it the game's viewport covered); g_Misc.x is 1 for a reversed depth
	// buffer, y the depth bias, z the glow's strength; g_Size is the mask's size (and its inverse).
	// The blur takes its step in g_UV.xy. The game's depth buffer can be multisampled (the game's
	// MSAA setting); the mask then is too, with the same samples, and sample 0 of each is compared,
	// where both were rasterized alike. MS picks the textures' kind.
	static const char s_szCommon[] =
		"cbuffer Params : register(b0) { float4 g_UV; float4 g_Misc; float4 g_Size; };\n"
		"SamplerState Linear : register(s0);\n"
		"#if MS\n"
		"Texture2DMS<float4> Mask : register(t0);\n"
		"Texture2DMS<float> MaskDepth : register(t1);\n"
		"Texture2DMS<float> SceneDepth : register(t2);\n"
		"float4 LoadMask(int2 p) { return Mask.Load(p, 0); }\n"
		"float LoadMaskDepth(int2 p) { return MaskDepth.Load(p, 0); }\n"
		"float LoadSceneDepth(int2 p) { return SceneDepth.Load(p, 0); }\n"
		"#else\n"
		"Texture2D<float4> Mask : register(t0);\n"
		"Texture2D<float> MaskDepth : register(t1);\n"
		"Texture2D<float> SceneDepth : register(t2);\n"
		"float4 LoadMask(int2 p) { return Mask.Load(int3(p, 0)); }\n"
		"float LoadMaskDepth(int2 p) { return MaskDepth.Load(int3(p, 0)); }\n"
		"float LoadSceneDepth(int2 p) { return SceneDepth.Load(int3(p, 0)); }\n"
		"#endif\n"
		"uint GetFlags(float4 Mask) { return (uint)(Mask.a * 255.0 + 0.5); }\n"
		"int2 MaskTexel(float2 uv) { return min(int2((uv * g_UV.xy + g_UV.zw) * g_Size.xy), int2(g_Size.xy) - 1); }\n";

	// The hidden colour: where a chams target is behind what the game drew
	static const char s_szChamsPS[] =
		"float4 main(float4 pos : SV_Position, float2 uv : TEXCOORD0) : SV_Target\n"
		"{\n"
		"	int2 p = MaskTexel(uv);\n"
		"	float4 m = LoadMask(p);\n"
		"	if ((GetFlags(m) & 2) == 0) discard;\n"
		"	float d = LoadMaskDepth(p);\n"
		"	float s = LoadSceneDepth(p);\n"
		"	bool bHidden = (g_Misc.x > 0.5) ? (d < s - g_Misc.y) : (d > s + g_Misc.y);\n"
		"	if (!bHidden) discard;\n"
		"	return float4(m.rgb, 1.0);\n"
		"}\n";

	// The glow's source, at half size: the glow targets' colours, from the four mask texels under
	// each of its texels
	static const char s_szGlowPrepPS[] =
		"float4 Tap(int2 p) { float4 m = LoadMask(min(p, int2(g_Size.xy) - 1)); return (GetFlags(m) & 4) ? float4(m.rgb, 1.0) : float4(0, 0, 0, 0); }\n"
		"float4 main(float4 pos : SV_Position, float2 uv : TEXCOORD0) : SV_Target\n"
		"{\n"
		"	int2 p = int2(pos.xy) * 2;\n"
		"	return (Tap(p) + Tap(p + int2(1, 0)) + Tap(p + int2(0, 1)) + Tap(p + int2(1, 1))) * 0.25;\n"
		"}\n";

	// One direction of a 9-tap gaussian, in five bilinear fetches
	static const char s_szBlurPS[] =
		"Texture2D Source : register(t3);\n"
		"float4 main(float4 pos : SV_Position, float2 uv : TEXCOORD0) : SV_Target\n"
		"{\n"
		"	float2 s = g_UV.xy;\n"
		"	float4 c = Source.SampleLevel(Linear, uv, 0) * 0.2270270270;\n"
		"	c += (Source.SampleLevel(Linear, uv + s * 1.3846153846, 0) + Source.SampleLevel(Linear, uv - s * 1.3846153846, 0)) * 0.3162162162;\n"
		"	c += (Source.SampleLevel(Linear, uv + s * 3.2307692308, 0) + Source.SampleLevel(Linear, uv - s * 3.2307692308, 0)) * 0.0702702703;\n"
		"	return c;\n"
		"}\n";

	// The glow, added around the silhouettes (not over them)
	static const char s_szGlowPS[] =
		"Texture2D Glow : register(t3);\n"
		"float4 main(float4 pos : SV_Position, float2 uv : TEXCOORD0) : SV_Target\n"
		"{\n"
		"	float4 g = Glow.SampleLevel(Linear, uv * g_UV.xy + g_UV.zw, 0);\n"
		"	float fInside = (GetFlags(LoadMask(MaskTexel(uv))) & 4) ? 1.0 : 0.0;\n"
		"	return float4(g.rgb * g_Misc.z * (1.0 - fInside), 0.0);\n"
		"}\n";

	enum
	{
		SAMPLES_ONE,
		SAMPLES_MANY,
		_SAMPLES_KINDS
	};

	static ID3D11Device* s_pDevice;
	static ID3D11VertexShader* s_pFullscreenVS;
	static ID3D11PixelShader* s_pChamsPS[_SAMPLES_KINDS];
	static ID3D11PixelShader* s_pGlowPrepPS[_SAMPLES_KINDS];
	static ID3D11PixelShader* s_pGlowPS[_SAMPLES_KINDS];
	static ID3D11PixelShader* s_pBlurPS;
	static ID3D11Buffer* s_pParams;						// three float4s, rewritten for each pass
	static ID3D11SamplerState* s_pLinear;				// clamped
	static ID3D11BlendState* s_pAdditive;
	static ID3D11DepthStencilState* s_pMaskDepth[2];	// LESS, GREATER (reversed depth): the nearest target wins
	static ID3D11DepthStencilState* s_pNoDepth;
	static ID3D11RasterizerState* s_pFullscreenRS;

	// The mask's pixel shaders, one for each colour and set of flags, made at their first use
	struct MaskShader_t
	{
		float m_Color[3];
		uint32_t m_uFlags;
		ID3D11PixelShader* m_pShader;
	};

	static MaskShader_t s_MaskShaders[16];
	static int s_nMaskShaders;

	// ---------------------------------------------------------------- targets

	// The frame's size, from the back buffer: the game's main depth buffers are this size
	static UINT s_uFrameWidth, s_uFrameHeight;

	// The game's depth buffers the mask has followed, each held (so its address can't be reused)
	// with the view a shader reads it through. The game has two of the frame's size and can draw a
	// frame into either.
	struct SceneDepth_t
	{
		ID3D11Texture2D* m_pTexture;
		ID3D11Texture2D* m_pCopy;						// when the game's can't be read by a shader
		ID3D11ShaderResourceView* m_pSRV;
		UINT64 m_uLastFrame;
	};

	static SceneDepth_t s_SceneDepths[4];
	static SceneDepth_t* s_pCurrent;					// this frame's

	// The mask, the shape of the game's depth buffers (size and samples)
	static UINT s_uWidth, s_uHeight;
	static DXGI_SAMPLE_DESC s_Samples;
	static ID3D11Texture2D* s_pMask;
	static ID3D11RenderTargetView* s_pMaskRTV;
	static ID3D11ShaderResourceView* s_pMaskSRV;
	static ID3D11Texture2D* s_pMaskDepthTex;
	static ID3D11DepthStencilView* s_pMaskDSV;
	static ID3D11ShaderResourceView* s_pMaskDepthSRV;
	static ID3D11Texture2D* s_pGlow[2];					// half size, single-sampled, ping-ponged by the blur
	static ID3D11RenderTargetView* s_pGlowRTV[2];
	static ID3D11ShaderResourceView* s_pGlowSRV[2];

	// The depth buffers targets were drawn into, for the menu's status line
	struct DepthStats_t
	{
		const void* m_pTexture;
		UINT m_uWidth, m_uHeight, m_uSamples;
		int m_nDraws;
		bool m_bFollowed;
	};

	static DepthStats_t s_Stats[8], s_LastStats[8];
	static int s_nStats, s_nLastStats;

	// The depth buffers left out, each logged once
	static const void* s_pRejected[16];
	static int s_nRejected;

	// This frame
	static UINT64 s_uFrame;
	static bool s_bMaskStarted;							// cleared and drawn into
	static bool s_bReversedDepth;
	static D3D11_VIEWPORT s_MaskViewport;				// the game's, at the first mask draw
	static uint32_t s_uFlagsDrawn;						// every flag drawn into the mask

	// What BeginMaskDraw replaced
	static ID3D11RenderTargetView* s_pSavedRTVs[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT];
	static ID3D11DepthStencilView* s_pSavedDSV;
	static ID3D11DepthStencilState* s_pSavedDepth;
	static UINT s_uSavedStencilRef;
	static ID3D11BlendState* s_pSavedBlend;
	static FLOAT s_SavedBlendFactor[4];
	static UINT s_uSavedSampleMask;
	static ID3D11PixelShader* s_pSavedPS;

	template<typename T>
	static void SafeRelease(T*& p)
	{
		if (p)
		{
			p->Release();
			p = NULL;
		}
	}

	static ID3DBlob* Compile(const char* szCommon, const char* szSource, const char* szProfile, bool bMultisampled)
	{
		char szFull[6144];
		ID3DBlob* pBlob = NULL;
		ID3DBlob* pErrors = NULL;
		const D3D_SHADER_MACRO Macros[] = { { "MS", bMultisampled ? "1" : "0" }, { NULL, NULL } };

		if (sprintf_s(szFull, "%s%s", szCommon ? szCommon : "", szSource) < 0)
			return NULL;

		HRESULT hr = D3DCompile(szFull, strlen(szFull), "EspRenderer", Macros, NULL, "main", szProfile, D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, &pBlob, &pErrors);

		if (FAILED(hr))
		{
			LOG("EspRenderer: %s failed to compile (%08X): %s\n", szProfile, hr, pErrors ? (const char*)pErrors->GetBufferPointer() : "");
			SafeRelease(pBlob);
		}

		SafeRelease(pErrors);
		return pBlob;
	}

	static bool CreatePS(const char* szCommon, const char* szSource, bool bMultisampled, ID3D11PixelShader** ppShader)
	{
		ID3DBlob* pBlob = Compile(szCommon, szSource, "ps_5_0", bMultisampled);

		if (!pBlob)
			return false;

		HRESULT hr = s_pDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, ppShader);

		pBlob->Release();
		return SUCCEEDED(hr);
	}

	static ID3D11PixelShader* GetMaskShader(const float Color[3], uint32_t uFlags)
	{
		for (int i = 0; i < s_nMaskShaders; ++i)
		{
			const MaskShader_t* pEntry = &s_MaskShaders[i];

			if (pEntry->m_uFlags == uFlags && !memcmp(pEntry->m_Color, Color, sizeof(pEntry->m_Color)))
				return pEntry->m_pShader;
		}

		if (s_nMaskShaders >= (int)ARRAYSIZE(s_MaskShaders))
			return NULL;

		char szSource[256];
		MaskShader_t Entry = { { Color[0], Color[1], Color[2] }, uFlags, NULL };

		sprintf_s(szSource, s_szMaskPSFormat, Color[0], Color[1], Color[2], (float)uFlags / 255.0f);

		if (!CreatePS(NULL, szSource, false, &Entry.m_pShader))
			return NULL;

		s_MaskShaders[s_nMaskShaders++] = Entry;
		return Entry.m_pShader;
	}

	// The typeless format of a depth buffer and the format a shader reads its depth through
	static bool GetDepthFormats(DXGI_FORMAT Format, DXGI_FORMAT* pTypeless, DXGI_FORMAT* pShaderFormat)
	{
		switch (Format)
		{
		case DXGI_FORMAT_R24G8_TYPELESS:
		case DXGI_FORMAT_D24_UNORM_S8_UINT:
			*pTypeless = DXGI_FORMAT_R24G8_TYPELESS;
			*pShaderFormat = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
			return true;
		case DXGI_FORMAT_R32_TYPELESS:
		case DXGI_FORMAT_D32_FLOAT:
			*pTypeless = DXGI_FORMAT_R32_TYPELESS;
			*pShaderFormat = DXGI_FORMAT_R32_FLOAT;
			return true;
		case DXGI_FORMAT_R32G8X24_TYPELESS:
		case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
			*pTypeless = DXGI_FORMAT_R32G8X24_TYPELESS;
			*pShaderFormat = DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
			return true;
		case DXGI_FORMAT_R16_TYPELESS:
		case DXGI_FORMAT_D16_UNORM:
			*pTypeless = DXGI_FORMAT_R16_TYPELESS;
			*pShaderFormat = DXGI_FORMAT_R16_UNORM;
			return true;
		default:
			return false;
		}
	}

	static void ReleaseSceneDepth(SceneDepth_t* pEntry)
	{
		if (pEntry == s_pCurrent)
			s_pCurrent = NULL;

		SafeRelease(pEntry->m_pSRV);
		SafeRelease(pEntry->m_pCopy);
		SafeRelease(pEntry->m_pTexture);
		pEntry->m_uLastFrame = 0;
	}

	static void ReleaseMaskTargets(void)
	{
		SafeRelease(s_pMaskSRV);
		SafeRelease(s_pMaskRTV);
		SafeRelease(s_pMask);
		SafeRelease(s_pMaskDepthSRV);
		SafeRelease(s_pMaskDSV);
		SafeRelease(s_pMaskDepthTex);

		for (int i = 0; i < 2; ++i)
		{
			SafeRelease(s_pGlowSRV[i]);
			SafeRelease(s_pGlowRTV[i]);
			SafeRelease(s_pGlow[i]);
		}

		s_uWidth = s_uHeight = 0;
		s_Samples = {};
		s_bMaskStarted = false;
	}

	static void ReleaseTargets(void)
	{
		for (SceneDepth_t& Entry : s_SceneDepths)
			ReleaseSceneDepth(&Entry);

		ReleaseMaskTargets();
	}

	static bool CreateGlowTarget(UINT uWidth, UINT uHeight, ID3D11Texture2D** ppTexture, ID3D11RenderTargetView** ppRTV, ID3D11ShaderResourceView** ppSRV)
	{
		D3D11_TEXTURE2D_DESC Desc = {};

		Desc.Width = uWidth;
		Desc.Height = uHeight;
		Desc.MipLevels = 1;
		Desc.ArraySize = 1;
		Desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		Desc.SampleDesc.Count = 1;
		Desc.Usage = D3D11_USAGE_DEFAULT;
		Desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

		return SUCCEEDED(s_pDevice->CreateTexture2D(&Desc, NULL, ppTexture)) &&
			SUCCEEDED(s_pDevice->CreateRenderTargetView(*ppTexture, NULL, ppRTV)) &&
			SUCCEEDED(s_pDevice->CreateShaderResourceView(*ppTexture, NULL, ppSRV));
	}

	// The mask in the shape of the game's depth buffer: the same size and samples, so a target's
	// draw lands on the same pixels and samples in both
	static bool CreateMaskTargets(UINT uWidth, UINT uHeight, DXGI_SAMPLE_DESC Samples)
	{
		const bool bMultisampled = Samples.Count > 1;
		D3D11_TEXTURE2D_DESC Desc = {};

		Desc.Width = uWidth;
		Desc.Height = uHeight;
		Desc.MipLevels = 1;
		Desc.ArraySize = 1;
		Desc.SampleDesc = Samples;
		Desc.Usage = D3D11_USAGE_DEFAULT;

		D3D11_RENDER_TARGET_VIEW_DESC RTVDesc = {};
		RTVDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		RTVDesc.ViewDimension = bMultisampled ? D3D11_RTV_DIMENSION_TEXTURE2DMS : D3D11_RTV_DIMENSION_TEXTURE2D;

		D3D11_DEPTH_STENCIL_VIEW_DESC DSVDesc = {};
		DSVDesc.Format = DXGI_FORMAT_D32_FLOAT;
		DSVDesc.ViewDimension = bMultisampled ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D;

		D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
		SRVDesc.ViewDimension = bMultisampled ? D3D11_SRV_DIMENSION_TEXTURE2DMS : D3D11_SRV_DIMENSION_TEXTURE2D;
		SRVDesc.Texture2D.MipLevels = 1;

		Desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		Desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		SRVDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

		bool bOk = SUCCEEDED(s_pDevice->CreateTexture2D(&Desc, NULL, &s_pMask)) &&
			SUCCEEDED(s_pDevice->CreateRenderTargetView(s_pMask, &RTVDesc, &s_pMaskRTV)) &&
			SUCCEEDED(s_pDevice->CreateShaderResourceView(s_pMask, &SRVDesc, &s_pMaskSRV));

		Desc.Format = DXGI_FORMAT_R32_TYPELESS;
		Desc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
		SRVDesc.Format = DXGI_FORMAT_R32_FLOAT;

		bOk = bOk && SUCCEEDED(s_pDevice->CreateTexture2D(&Desc, NULL, &s_pMaskDepthTex)) &&
			SUCCEEDED(s_pDevice->CreateDepthStencilView(s_pMaskDepthTex, &DSVDesc, &s_pMaskDSV)) &&
			SUCCEEDED(s_pDevice->CreateShaderResourceView(s_pMaskDepthTex, &SRVDesc, &s_pMaskDepthSRV));

		for (int i = 0; i < 2 && bOk; ++i)
			bOk = CreateGlowTarget(max(uWidth / 2, 1u), max(uHeight / 2, 1u), &s_pGlow[i], &s_pGlowRTV[i], &s_pGlowSRV[i]);

		if (!bOk)
		{
			ReleaseMaskTargets();
			return false;
		}

		s_uWidth = uWidth;
		s_uHeight = uHeight;
		s_Samples = Samples;
		return true;
	}

	// A view of the game's depth itself when a shader can read it, else of a copy made each frame
	static bool CreateSceneDepthView(SceneDepth_t* pEntry, ID3D11Texture2D* pTexture, const D3D11_TEXTURE2D_DESC& Desc, DXGI_FORMAT Typeless, DXGI_FORMAT ShaderFormat)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};

		SRVDesc.Format = ShaderFormat;
		SRVDesc.ViewDimension = (Desc.SampleDesc.Count > 1) ? D3D11_SRV_DIMENSION_TEXTURE2DMS : D3D11_SRV_DIMENSION_TEXTURE2D;
		SRVDesc.Texture2D.MipLevels = 1;

		if (Desc.BindFlags & D3D11_BIND_SHADER_RESOURCE)
			return SUCCEEDED(s_pDevice->CreateShaderResourceView(pTexture, &SRVDesc, &pEntry->m_pSRV));

		D3D11_TEXTURE2D_DESC CopyDesc = Desc;

		CopyDesc.Format = Typeless;
		CopyDesc.Usage = D3D11_USAGE_DEFAULT;
		CopyDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		CopyDesc.CPUAccessFlags = 0;
		CopyDesc.MiscFlags = 0;

		return SUCCEEDED(s_pDevice->CreateTexture2D(&CopyDesc, NULL, &pEntry->m_pCopy)) &&
			SUCCEEDED(s_pDevice->CreateShaderResourceView(pEntry->m_pCopy, &SRVDesc, &pEntry->m_pSRV));
	}

	static void LogRejected(ID3D11Texture2D* pTexture, const D3D11_TEXTURE2D_DESC& Desc, const char* szWhy)
	{
		UNREFERENCED_PARAMETER(Desc);	// LOG is nothing in Release builds
		UNREFERENCED_PARAMETER(szWhy);

		for (int i = 0; i < s_nRejected; ++i)
		{
			if (s_pRejected[i] == pTexture)
				return;
		}

		if (s_nRejected < (int)ARRAYSIZE(s_pRejected))
			s_pRejected[s_nRejected++] = pTexture;

		LOG("EspRenderer: leaving out the %ux%u x%u depth buffer %p (format %d): %s\n", Desc.Width, Desc.Height, Desc.SampleDesc.Count, pTexture,
			Desc.Format, szWhy);
	}

	// This frame's scene depth: the first depth buffer of the frame's size that a target is drawn
	// into. The shadow map (2048x2048) is left out: its silhouettes are from the light's view, and
	// following the biggest depth buffer put them on the screen. The game's own depth buffers are
	// multisampled with MSAA on, which this used to leave out too, so it had only the shadow map.
	static bool FollowSceneDepth(ID3D11Texture2D* pTexture)
	{
		if (s_pCurrent && s_pCurrent->m_pTexture == pTexture)
			return true;

		D3D11_TEXTURE2D_DESC Desc;
		DXGI_FORMAT Typeless, ShaderFormat;

		pTexture->GetDesc(&Desc);

		if (!s_uFrameWidth || Desc.Width != s_uFrameWidth || Desc.Height != s_uFrameHeight)
		{
			LogRejected(pTexture, Desc, "not the frame's size");
			return false;
		}

		if (Desc.ArraySize != 1 || !GetDepthFormats(Desc.Format, &Typeless, &ShaderFormat))
		{
			LogRejected(pTexture, Desc, "an array or an unknown format");
			return false;
		}

		// This frame has its depth buffer already
		if (s_pCurrent)
			return false;

		if (s_uWidth != Desc.Width || s_uHeight != Desc.Height || s_Samples.Count != Desc.SampleDesc.Count || s_Samples.Quality != Desc.SampleDesc.Quality)
		{
			ReleaseMaskTargets();

			if (!CreateMaskTargets(Desc.Width, Desc.Height, Desc.SampleDesc))
			{
				LOG("EspRenderer: couldn't make the %ux%u x%u mask\n", Desc.Width, Desc.Height, Desc.SampleDesc.Count);
				return false;
			}
		}

		SceneDepth_t* pEntry = NULL;
		SceneDepth_t* pOldest = &s_SceneDepths[0];

		for (SceneDepth_t& Entry : s_SceneDepths)
		{
			if (Entry.m_pTexture == pTexture)
				pEntry = &Entry;

			if (Entry.m_uLastFrame < pOldest->m_uLastFrame)
				pOldest = &Entry;
		}

		if (!pEntry)
		{
			pEntry = pOldest;
			ReleaseSceneDepth(pEntry);

			if (!CreateSceneDepthView(pEntry, pTexture, Desc, Typeless, ShaderFormat))
			{
				LOG("EspRenderer: couldn't read the %ux%u x%u depth buffer %p (format %d)\n", Desc.Width, Desc.Height, Desc.SampleDesc.Count, pTexture, Desc.Format);
				ReleaseSceneDepth(pEntry);
				return false;
			}

			pEntry->m_pTexture = pTexture;
			pEntry->m_pTexture->AddRef();

			LOG("EspRenderer: following the %ux%u x%u depth buffer %p (format %d, %s)\n", Desc.Width, Desc.Height, Desc.SampleDesc.Count, pTexture,
				Desc.Format, pEntry->m_pCopy ? "copied each frame" : "read directly");
		}

		pEntry->m_uLastFrame = s_uFrame;
		s_pCurrent = pEntry;
		return true;
	}

	static void RecordDraw(ID3D11Texture2D* pTexture, bool bFollowed)
	{
		for (int i = 0; i < s_nStats; ++i)
		{
			if (s_Stats[i].m_pTexture == pTexture)
			{
				++s_Stats[i].m_nDraws;
				return;
			}
		}

		if (s_nStats >= (int)ARRAYSIZE(s_Stats))
			return;

		D3D11_TEXTURE2D_DESC Desc;

		pTexture->GetDesc(&Desc);
		s_Stats[s_nStats++] = { pTexture, Desc.Width, Desc.Height, Desc.SampleDesc.Count, 1, bFollowed };
	}

	// Whether the bound depth buffer is this frame's scene depth
	static bool CheckMainPass(ID3D11DeviceContext* pContext, bool bRecord)
	{
		ID3D11DepthStencilView* pDSV = NULL;
		ID3D11Resource* pResource = NULL;
		ID3D11Texture2D* pTexture = NULL;

		pContext->OMGetRenderTargets(0, NULL, &pDSV);

		if (!pDSV)
			return false;

		pDSV->GetResource(&pResource);
		pDSV->Release();

		if (pResource)
		{
			pResource->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&pTexture);
			pResource->Release();
		}

		if (!pTexture)
			return false;

		const bool bFollowed = FollowSceneDepth(pTexture);

		if (bRecord)
			RecordDraw(pTexture, bFollowed);

		pTexture->Release();
		return bFollowed;
	}

	bool Initialize(ID3D11Device* pDevice)
	{
		if (!pDevice)
			return false;

		s_pDevice = pDevice;

		ID3DBlob* pVS = Compile(NULL, s_szFullscreenVS, "vs_5_0", false);

		if (!pVS)
			return false;

		HRESULT hr = s_pDevice->CreateVertexShader(pVS->GetBufferPointer(), pVS->GetBufferSize(), NULL, &s_pFullscreenVS);

		pVS->Release();

		bool bOk = SUCCEEDED(hr) && CreatePS(s_szCommon, s_szBlurPS, false, &s_pBlurPS);

		for (int iKind = 0; iKind < _SAMPLES_KINDS && bOk; ++iKind)
		{
			const bool bMultisampled = iKind == SAMPLES_MANY;

			bOk = CreatePS(s_szCommon, s_szChamsPS, bMultisampled, &s_pChamsPS[iKind]) &&
				CreatePS(s_szCommon, s_szGlowPrepPS, bMultisampled, &s_pGlowPrepPS[iKind]) &&
				CreatePS(s_szCommon, s_szGlowPS, bMultisampled, &s_pGlowPS[iKind]);
		}

		if (!bOk)
		{
			Shutdown();
			return false;
		}

		D3D11_BUFFER_DESC BufferDesc = {};

		BufferDesc.ByteWidth = 3 * 4 * sizeof(float);
		BufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		BufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		D3D11_SAMPLER_DESC SamplerDesc = {};

		SamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		SamplerDesc.AddressU = SamplerDesc.AddressV = SamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		SamplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		SamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

		bOk = SUCCEEDED(s_pDevice->CreateBuffer(&BufferDesc, NULL, &s_pParams)) &&
			SUCCEEDED(s_pDevice->CreateSamplerState(&SamplerDesc, &s_pLinear));

		D3D11_BLEND_DESC BlendDesc = {};

		BlendDesc.RenderTarget[0].BlendEnable = TRUE;
		BlendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
		BlendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
		BlendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		BlendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
		BlendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
		BlendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		BlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		bOk = bOk && SUCCEEDED(s_pDevice->CreateBlendState(&BlendDesc, &s_pAdditive));

		D3D11_DEPTH_STENCIL_DESC DepthDesc = {};

		DepthDesc.DepthEnable = TRUE;
		DepthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		DepthDesc.DepthFunc = D3D11_COMPARISON_LESS;
		bOk = bOk && SUCCEEDED(s_pDevice->CreateDepthStencilState(&DepthDesc, &s_pMaskDepth[0]));

		DepthDesc.DepthFunc = D3D11_COMPARISON_GREATER;
		bOk = bOk && SUCCEEDED(s_pDevice->CreateDepthStencilState(&DepthDesc, &s_pMaskDepth[1]));

		DepthDesc.DepthEnable = FALSE;
		DepthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		bOk = bOk && SUCCEEDED(s_pDevice->CreateDepthStencilState(&DepthDesc, &s_pNoDepth));

		D3D11_RASTERIZER_DESC RasterDesc = {};

		RasterDesc.FillMode = D3D11_FILL_SOLID;
		RasterDesc.CullMode = D3D11_CULL_NONE;
		RasterDesc.DepthClipEnable = TRUE;
		bOk = bOk && SUCCEEDED(s_pDevice->CreateRasterizerState(&RasterDesc, &s_pFullscreenRS));

		if (!bOk)
		{
			LOG("EspRenderer: couldn't make its states\n");
			Shutdown();
			return false;
		}

		return true;
	}

	void Shutdown(void)
	{
		ReleaseTargets();

		for (int i = 0; i < s_nMaskShaders; ++i)
			SafeRelease(s_MaskShaders[i].m_pShader);

		s_nMaskShaders = 0;

		for (int iKind = 0; iKind < _SAMPLES_KINDS; ++iKind)
		{
			SafeRelease(s_pChamsPS[iKind]);
			SafeRelease(s_pGlowPrepPS[iKind]);
			SafeRelease(s_pGlowPS[iKind]);
		}

		SafeRelease(s_pFullscreenVS);
		SafeRelease(s_pBlurPS);
		SafeRelease(s_pParams);
		SafeRelease(s_pLinear);
		SafeRelease(s_pAdditive);
		SafeRelease(s_pMaskDepth[0]);
		SafeRelease(s_pMaskDepth[1]);
		SafeRelease(s_pNoDepth);
		SafeRelease(s_pFullscreenRS);

		s_pDevice = NULL;
	}

	static void SetParams(ID3D11DeviceContext* pContext, const float A[4], const float B[4], const float C[4])
	{
		D3D11_MAPPED_SUBRESOURCE Mapped;

		if (FAILED(pContext->Map(s_pParams, 0, D3D11_MAP_WRITE_DISCARD, 0, &Mapped)))
			return;

		memcpy(Mapped.pData, A, 4 * sizeof(float));
		memcpy((float*)Mapped.pData + 4, B, 4 * sizeof(float));
		memcpy((float*)Mapped.pData + 8, C, 4 * sizeof(float));
		pContext->Unmap(s_pParams, 0);
	}

	bool IsMainPass(ID3D11DeviceContext* pContext)
	{
		// Without its shaders there is no mask to keep apart: the seen colour goes everywhere, as
		// before it existed
		if (!s_pFullscreenVS)
			return true;

		return CheckMainPass(pContext, true);
	}

	void GetStatus(char* szStatus, size_t cchStatus)
	{
		if (!szStatus || !cchStatus)
			return;

		szStatus[0] = '\0';

		if (!s_pFullscreenVS)
		{
			strcpy_s(szStatus, cchStatus, "couldn't start (shaders)");
			return;
		}

		if (!s_nLastStats)
		{
			sprintf_s(szStatus, cchStatus, "no target drawn last frame (frame %ux%u)", s_uFrameWidth, s_uFrameHeight);
			return;
		}

		for (int i = 0; i < s_nLastStats; ++i)
		{
			const DepthStats_t* pStats = &s_LastStats[i];
			char szEntry[96];

			sprintf_s(szEntry, "%s%ux%u x%u: %d draws, %s", i ? "; " : "", pStats->m_uWidth, pStats->m_uHeight, pStats->m_uSamples, pStats->m_nDraws,
				pStats->m_bFollowed ? "followed" : "left out");
			strcat_s(szStatus, cchStatus, szEntry);
		}
	}

	bool BeginMaskDraw(ID3D11DeviceContext* pContext, const float Color[3], uint32_t uFlags)
	{
		if (!s_pFullscreenVS || !CheckMainPass(pContext, false) || !s_pMaskRTV)
			return false;

		uFlags |= ESPMASK_PRESENT;

		ID3D11PixelShader* pMaskShader = GetMaskShader(Color, uFlags);

		if (!pMaskShader)
			return false;

		// Which way the game's depth runs, from the test it draws with
		pContext->OMGetDepthStencilState(&s_pSavedDepth, &s_uSavedStencilRef);

		if (s_pSavedDepth)
		{
			D3D11_DEPTH_STENCIL_DESC DepthDesc;

			s_pSavedDepth->GetDesc(&DepthDesc);

			if (DepthDesc.DepthEnable)
				s_bReversedDepth = DepthDesc.DepthFunc == D3D11_COMPARISON_GREATER || DepthDesc.DepthFunc == D3D11_COMPARISON_GREATER_EQUAL;
		}

		if (!s_bMaskStarted)
		{
			const FLOAT Clear[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
			UINT nViewports = 1;

			pContext->ClearRenderTargetView(s_pMaskRTV, Clear);
			pContext->ClearDepthStencilView(s_pMaskDSV, D3D11_CLEAR_DEPTH, s_bReversedDepth ? 0.0f : 1.0f, 0);
			pContext->RSGetViewports(&nViewports, &s_MaskViewport);

			if (!nViewports || s_MaskViewport.Width <= 0.0f || s_MaskViewport.Height <= 0.0f)
				s_MaskViewport = { 0.0f, 0.0f, (FLOAT)s_uWidth, (FLOAT)s_uHeight, 0.0f, 1.0f };

			s_bMaskStarted = true;
		}

		pContext->OMGetRenderTargets(D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT, s_pSavedRTVs, &s_pSavedDSV);
		pContext->OMGetBlendState(&s_pSavedBlend, s_SavedBlendFactor, &s_uSavedSampleMask);
		pContext->PSGetShader(&s_pSavedPS, NULL, NULL);

		pContext->OMSetRenderTargets(1, &s_pMaskRTV, s_pMaskDSV);
		pContext->OMSetDepthStencilState(s_pMaskDepth[s_bReversedDepth ? 1 : 0], 0);
		pContext->OMSetBlendState(NULL, NULL, 0xFFFFFFFF);
		pContext->PSSetShader(pMaskShader, NULL, 0);

		s_uFlagsDrawn |= uFlags;
		return true;
	}

	void EndMaskDraw(ID3D11DeviceContext* pContext)
	{
		pContext->OMSetRenderTargets(D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT, s_pSavedRTVs, s_pSavedDSV);
		pContext->OMSetDepthStencilState(s_pSavedDepth, s_uSavedStencilRef);
		pContext->OMSetBlendState(s_pSavedBlend, s_SavedBlendFactor, s_uSavedSampleMask);
		pContext->PSSetShader(s_pSavedPS, NULL, 0);

		for (ID3D11RenderTargetView*& pRTV : s_pSavedRTVs)
			SafeRelease(pRTV);

		SafeRelease(s_pSavedDSV);
		SafeRelease(s_pSavedDepth);
		SafeRelease(s_pSavedBlend);
		SafeRelease(s_pSavedPS);
	}

	// Everything Composite changes, put back afterwards: the game's state cache only binds what it
	// thinks has changed, so anything left bound would carry into its next frame. Pixel shader
	// constant buffer 0 goes back with its offset and size when the context can say what they were.
	struct StateBackup_t
	{
		D3D11_PRIMITIVE_TOPOLOGY Topology;
		ID3D11InputLayout* pLayout;
		ID3D11VertexShader* pVS;
		ID3D11HullShader* pHS;
		ID3D11DomainShader* pDS;
		ID3D11GeometryShader* pGS;
		ID3D11PixelShader* pPS;
		ID3D11ShaderResourceView* pSRVs[4];
		ID3D11SamplerState* pSampler;
		ID3D11DeviceContext1* pContext1;
		ID3D11Buffer* pPSParams;
		UINT uFirstConstant;
		UINT nConstants;
		ID3D11RasterizerState* pRS;
		UINT nViewports;
		D3D11_VIEWPORT Viewports[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
		ID3D11RenderTargetView* pRTVs[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT];
		ID3D11DepthStencilView* pDSV;
		ID3D11BlendState* pBlend;
		FLOAT BlendFactor[4];
		UINT uSampleMask;
		ID3D11DepthStencilState* pDepth;
		UINT uStencilRef;

		void Save(ID3D11DeviceContext* pContext)
		{
			pContext->IAGetPrimitiveTopology(&Topology);
			pContext->IAGetInputLayout(&pLayout);
			pContext->VSGetShader(&pVS, NULL, NULL);
			pContext->HSGetShader(&pHS, NULL, NULL);
			pContext->DSGetShader(&pDS, NULL, NULL);
			pContext->GSGetShader(&pGS, NULL, NULL);
			pContext->PSGetShader(&pPS, NULL, NULL);
			pContext->PSGetShaderResources(0, 4, pSRVs);
			pContext->PSGetSamplers(0, 1, &pSampler);

			if (SUCCEEDED(pContext->QueryInterface(__uuidof(ID3D11DeviceContext1), (void**)&pContext1)))
				pContext1->PSGetConstantBuffers1(0, 1, &pPSParams, &uFirstConstant, &nConstants);
			else
				pContext->PSGetConstantBuffers(0, 1, &pPSParams);

			pContext->RSGetState(&pRS);
			nViewports = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
			pContext->RSGetViewports(&nViewports, Viewports);
			pContext->OMGetRenderTargets(D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT, pRTVs, &pDSV);
			pContext->OMGetBlendState(&pBlend, BlendFactor, &uSampleMask);
			pContext->OMGetDepthStencilState(&pDepth, &uStencilRef);
		}

		void Restore(ID3D11DeviceContext* pContext)
		{
			pContext->IASetPrimitiveTopology(Topology);
			pContext->IASetInputLayout(pLayout);
			pContext->VSSetShader(pVS, NULL, 0);
			pContext->HSSetShader(pHS, NULL, 0);
			pContext->DSSetShader(pDS, NULL, 0);
			pContext->GSSetShader(pGS, NULL, 0);
			pContext->PSSetShader(pPS, NULL, 0);
			pContext->PSSetShaderResources(0, 4, pSRVs);
			pContext->PSSetSamplers(0, 1, &pSampler);

			if (pContext1)
				pContext1->PSSetConstantBuffers1(0, 1, &pPSParams, &uFirstConstant, &nConstants);
			else
				pContext->PSSetConstantBuffers(0, 1, &pPSParams);

			pContext->RSSetState(pRS);
			pContext->RSSetViewports(nViewports, Viewports);
			pContext->OMSetRenderTargets(D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT, pRTVs, pDSV);
			pContext->OMSetBlendState(pBlend, BlendFactor, uSampleMask);
			pContext->OMSetDepthStencilState(pDepth, uStencilRef);

			SafeRelease(pLayout);
			SafeRelease(pVS);
			SafeRelease(pHS);
			SafeRelease(pDS);
			SafeRelease(pGS);
			SafeRelease(pPS);

			for (ID3D11ShaderResourceView*& pSRV : pSRVs)
				SafeRelease(pSRV);

			SafeRelease(pSampler);
			SafeRelease(pPSParams);
			SafeRelease(pContext1);
			SafeRelease(pRS);

			for (ID3D11RenderTargetView*& pRTV : pRTVs)
				SafeRelease(pRTV);

			SafeRelease(pDSV);
			SafeRelease(pBlend);
			SafeRelease(pDepth);
		}
	};

	static void SetTarget(ID3D11DeviceContext* pContext, ID3D11RenderTargetView* pRTV, UINT uWidth, UINT uHeight)
	{
		const D3D11_VIEWPORT Viewport = { 0.0f, 0.0f, (FLOAT)uWidth, (FLOAT)uHeight, 0.0f, 1.0f };
		ID3D11ShaderResourceView* const NoSRVs[4] = {};

		// A texture can't be read and drawn into at once
		pContext->PSSetShaderResources(0, 4, NoSRVs);
		pContext->OMSetRenderTargets(1, &pRTV, NULL);
		pContext->RSSetViewports(1, &Viewport);
	}

	static void EndFrame(void)
	{
		memcpy(s_LastStats, s_Stats, sizeof(s_Stats));
		s_nLastStats = s_nStats;
		s_nStats = 0;

		s_bMaskStarted = false;
		s_uFlagsDrawn = 0;
		s_pCurrent = NULL;
		++s_uFrame;

		// Depth buffers the game hasn't drawn a target into for a while (a resolution change, a
		// new device) are let go
		for (SceneDepth_t& Entry : s_SceneDepths)
		{
			if (Entry.m_pTexture && Entry.m_uLastFrame + 600 < s_uFrame)
				ReleaseSceneDepth(&Entry);
		}
	}

	void Composite(ID3D11DeviceContext* pContext, ID3D11RenderTargetView* pBackBuffer)
	{
		if (!pBackBuffer)
		{
			EndFrame();
			return;
		}

		// The frame's size, which picks the game's main depth buffers for the next frame
		ID3D11Resource* pBackBufferResource = NULL;
		ID3D11Texture2D* pBackBufferTexture = NULL;
		D3D11_TEXTURE2D_DESC BackBufferDesc = {};

		pBackBuffer->GetResource(&pBackBufferResource);

		if (pBackBufferResource)
		{
			pBackBufferResource->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&pBackBufferTexture);
			pBackBufferResource->Release();
		}

		if (!pBackBufferTexture)
		{
			EndFrame();
			return;
		}

		pBackBufferTexture->GetDesc(&BackBufferDesc);
		pBackBufferTexture->Release();

		s_uFrameWidth = BackBufferDesc.Width;
		s_uFrameHeight = BackBufferDesc.Height;

		if (!s_bMaskStarted || !s_pCurrent || !s_pCurrent->m_pSRV || !(s_uFlagsDrawn & (ESPMASK_CHAMS | ESPMASK_GLOW)))
		{
			EndFrame();
			return;
		}

		const int iKind = (s_Samples.Count > 1) ? SAMPLES_MANY : SAMPLES_ONE;
		StateBackup_t Backup = {};

		Backup.Save(pContext);

		if (s_pCurrent->m_pCopy)
			pContext->CopyResource(s_pCurrent->m_pCopy, s_pCurrent->m_pTexture);

		pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		pContext->IASetInputLayout(NULL);
		pContext->VSSetShader(s_pFullscreenVS, NULL, 0);
		pContext->HSSetShader(NULL, NULL, 0);
		pContext->DSSetShader(NULL, NULL, 0);
		pContext->GSSetShader(NULL, NULL, 0);
		pContext->RSSetState(s_pFullscreenRS);
		pContext->OMSetDepthStencilState(s_pNoDepth, 0);
		pContext->PSSetSamplers(0, 1, &s_pLinear);
		pContext->PSSetConstantBuffers(0, 1, &s_pParams);

		// The back buffer's uv onto the part of the mask the game's viewport covered
		const float UV[4] = {
			s_MaskViewport.Width / (float)s_uWidth, s_MaskViewport.Height / (float)s_uHeight,
			s_MaskViewport.TopLeftX / (float)s_uWidth, s_MaskViewport.TopLeftY / (float)s_uHeight
		};
		const float Size[4] = { (float)s_uWidth, (float)s_uHeight, 1.0f / (float)s_uWidth, 1.0f / (float)s_uHeight };
		const float NoParams[4] = {};

		if (s_uFlagsDrawn & ESPMASK_CHAMS)
		{
			const float Misc[4] = { s_bReversedDepth ? 1.0f : 0.0f, 1e-6f, 0.0f, 0.0f };
			ID3D11ShaderResourceView* const SRVs[3] = { s_pMaskSRV, s_pMaskDepthSRV, s_pCurrent->m_pSRV };

			SetTarget(pContext, pBackBuffer, BackBufferDesc.Width, BackBufferDesc.Height);
			SetParams(pContext, UV, Misc, Size);
			pContext->OMSetBlendState(NULL, NULL, 0xFFFFFFFF);
			pContext->PSSetShader(s_pChamsPS[iKind], NULL, 0);
			pContext->PSSetShaderResources(0, 3, SRVs);
			pContext->Draw(3, 0);
		}

		if (s_uFlagsDrawn & ESPMASK_GLOW)
		{
			const UINT uGlowWidth = max(s_uWidth / 2, 1u);
			const UINT uGlowHeight = max(s_uHeight / 2, 1u);

			pContext->OMSetBlendState(NULL, NULL, 0xFFFFFFFF);

			// The glow targets' silhouettes at half size
			SetTarget(pContext, s_pGlowRTV[0], uGlowWidth, uGlowHeight);
			SetParams(pContext, NoParams, NoParams, Size);
			pContext->PSSetShader(s_pGlowPrepPS[iKind], NULL, 0);
			pContext->PSSetShaderResources(0, 1, &s_pMaskSRV);
			pContext->Draw(3, 0);

			// Blurred twice each way, 0 -> 1 across, 1 -> 0 down
			pContext->PSSetShader(s_pBlurPS, NULL, 0);

			for (int iPass = 0; iPass < 4; ++iPass)
			{
				const int iSource = iPass & 1;
				const float flSpread = (iPass < 2) ? 1.0f : 2.0f;
				const float Step[4] = { (iSource == 0) ? flSpread / (float)uGlowWidth : 0.0f, (iSource == 1) ? flSpread / (float)uGlowHeight : 0.0f, 0.0f, 0.0f };

				SetTarget(pContext, s_pGlowRTV[iSource ^ 1], uGlowWidth, uGlowHeight);
				SetParams(pContext, Step, NoParams, Size);
				pContext->PSSetShaderResources(3, 1, &s_pGlowSRV[iSource]);
				pContext->Draw(3, 0);
			}

			// Added to the back buffer around the silhouettes
			const float GlowMisc[4] = { 0.0f, 0.0f, 2.5f, 0.0f };

			SetTarget(pContext, pBackBuffer, BackBufferDesc.Width, BackBufferDesc.Height);
			SetParams(pContext, UV, GlowMisc, Size);
			pContext->OMSetBlendState(s_pAdditive, NULL, 0xFFFFFFFF);
			pContext->PSSetShader(s_pGlowPS[iKind], NULL, 0);
			pContext->PSSetShaderResources(0, 1, &s_pMaskSRV);
			pContext->PSSetShaderResources(3, 1, &s_pGlowSRV[0]);
			pContext->Draw(3, 0);
		}

		ID3D11ShaderResourceView* const NoSRVs[4] = {};

		pContext->PSSetShaderResources(0, 4, NoSRVs);

		Backup.Restore(pContext);
		EndFrame();
	}
}
