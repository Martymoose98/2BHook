#include "EspRenderer.h"

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

	// The mask: the colour and, in alpha, the flags (out of 255). No inputs, so it links with any
	// of the game's vertex shaders.
	static const char s_szMaskPS[] =
		"cbuffer Params : register(b0) { float4 g_Color; float4 g_Unused; };\n"
		"float4 main() : SV_Target { return g_Color; }\n";

	// The parameters of the passes below: g_UV maps the back buffer's uv onto the mask (xy scale,
	// zw offset: the part of it the game's viewport covered); g_Misc.x is 1 for a reversed depth
	// buffer, y the depth bias, z the glow's strength. The blur takes its step in g_UV.xy.
	static const char s_szCommon[] =
		"cbuffer Params : register(b0) { float4 g_UV; float4 g_Misc; };\n"
		"SamplerState Point : register(s0);\n"
		"SamplerState Linear : register(s1);\n"
		"uint GetFlags(float4 Mask) { return (uint)(Mask.a * 255.0 + 0.5); }\n";

	// The hidden colour: where a chams target is behind what the game drew
	static const char s_szChamsPS[] =
		"Texture2D Mask : register(t0);\n"
		"Texture2D<float> MaskDepth : register(t1);\n"
		"Texture2D<float> SceneDepth : register(t2);\n"
		"float4 main(float4 pos : SV_Position, float2 uv : TEXCOORD0) : SV_Target\n"
		"{\n"
		"	float2 t = uv * g_UV.xy + g_UV.zw;\n"
		"	float4 m = Mask.SampleLevel(Point, t, 0);\n"
		"	if ((GetFlags(m) & 2) == 0) discard;\n"
		"	float d = MaskDepth.SampleLevel(Point, t, 0);\n"
		"	float s = SceneDepth.SampleLevel(Point, t, 0);\n"
		"	bool bHidden = (g_Misc.x > 0.5) ? (d < s - g_Misc.y) : (d > s + g_Misc.y);\n"
		"	if (!bHidden) discard;\n"
		"	return float4(m.rgb, 1.0);\n"
		"}\n";

	// The glow's source, at half size: the glow targets' colours, from four texels of the mask
	static const char s_szGlowPrepPS[] =
		"Texture2D Mask : register(t0);\n"
		"float4 Tap(float2 t) { float4 m = Mask.SampleLevel(Point, t, 0); return (GetFlags(m) & 4) ? float4(m.rgb, 1.0) : float4(0, 0, 0, 0); }\n"
		"float4 main(float4 pos : SV_Position, float2 uv : TEXCOORD0) : SV_Target\n"
		"{\n"
		"	float2 h = g_Misc.zw * 0.5;\n"
		"	return (Tap(uv + float2(-h.x, -h.y)) + Tap(uv + float2(h.x, -h.y)) + Tap(uv + float2(-h.x, h.y)) + Tap(uv + float2(h.x, h.y))) * 0.25;\n"
		"}\n";

	// One direction of a 9-tap gaussian, in five bilinear fetches
	static const char s_szBlurPS[] =
		"Texture2D Source : register(t0);\n"
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
		"Texture2D Glow : register(t0);\n"
		"Texture2D Mask : register(t1);\n"
		"float4 main(float4 pos : SV_Position, float2 uv : TEXCOORD0) : SV_Target\n"
		"{\n"
		"	float2 t = uv * g_UV.xy + g_UV.zw;\n"
		"	float4 g = Glow.SampleLevel(Linear, t, 0);\n"
		"	float fInside = (GetFlags(Mask.SampleLevel(Point, t, 0)) & 4) ? 1.0 : 0.0;\n"
		"	return float4(g.rgb * g_Misc.z * (1.0 - fInside), 0.0);\n"
		"}\n";

	static ID3D11Device* s_pDevice;
	static ID3D11VertexShader* s_pFullscreenVS;
	static ID3D11PixelShader* s_pMaskPS;
	static ID3D11PixelShader* s_pChamsPS;
	static ID3D11PixelShader* s_pGlowPrepPS;
	static ID3D11PixelShader* s_pBlurPS;
	static ID3D11PixelShader* s_pGlowPS;
	static ID3D11Buffer* s_pParams;						// two float4s, rewritten for each pass
	static ID3D11SamplerState* s_pSamplers[2];			// point, linear (clamped)
	static ID3D11BlendState* s_pAdditive;
	static ID3D11DepthStencilState* s_pMaskDepth[2];	// LESS, GREATER (reversed depth): the nearest target wins
	static ID3D11DepthStencilState* s_pNoDepth;
	static ID3D11RasterizerState* s_pFullscreenRS;

	// ---------------------------------------------------------------- targets

	// The game's depth buffer the mask follows, held so its address can't be reused
	static ID3D11Texture2D* s_pSceneDepth;
	static ID3D11Texture2D* s_pSceneDepthCopy;			// when the game's can't be read by a shader
	static ID3D11ShaderResourceView* s_pSceneDepthSRV;
	static UINT s_uWidth, s_uHeight;

	static ID3D11Texture2D* s_pMask;
	static ID3D11RenderTargetView* s_pMaskRTV;
	static ID3D11ShaderResourceView* s_pMaskSRV;
	static ID3D11Texture2D* s_pMaskDepthTex;
	static ID3D11DepthStencilView* s_pMaskDSV;
	static ID3D11ShaderResourceView* s_pMaskDepthSRV;
	static ID3D11Texture2D* s_pGlow[2];					// half size, ping-ponged by the blur
	static ID3D11RenderTargetView* s_pGlowRTV[2];
	static ID3D11ShaderResourceView* s_pGlowSRV[2];

	// This frame
	static bool s_bMaskStarted;							// cleared and drawn into
	static bool s_bReversedDepth;
	static D3D11_VIEWPORT s_MaskViewport;				// the game's, at the first mask draw
	static uint32_t s_uFlagsDrawn;						// every flag drawn into the mask
	static bool s_bSkippedOther;						// a target was drawn into another depth buffer
	static int s_nMissedFrames;							// frames that only saw other depth buffers

	// What BeginMaskDraw replaced
	static ID3D11RenderTargetView* s_pSavedRTVs[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT];
	static ID3D11DepthStencilView* s_pSavedDSV;
	static ID3D11DepthStencilState* s_pSavedDepth;
	static UINT s_uSavedStencilRef;
	static ID3D11BlendState* s_pSavedBlend;
	static FLOAT s_SavedBlendFactor[4];
	static UINT s_uSavedSampleMask;
	static ID3D11PixelShader* s_pSavedPS;
	static ID3D11Buffer* s_pSavedPSParams;

	template<typename T>
	static void SafeRelease(T*& p)
	{
		if (p)
		{
			p->Release();
			p = NULL;
		}
	}

	static ID3DBlob* Compile(const char* szCommon, const char* szSource, const char* szProfile)
	{
		char szFull[4096];
		ID3DBlob* pBlob = NULL;
		ID3DBlob* pErrors = NULL;

		if (sprintf_s(szFull, "%s%s", szCommon ? szCommon : "", szSource) < 0)
			return NULL;

		HRESULT hr = D3DCompile(szFull, strlen(szFull), "EspRenderer", NULL, NULL, "main", szProfile, D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, &pBlob, &pErrors);

		if (FAILED(hr))
		{
			LOG("EspRenderer: %s failed to compile (%08X): %s\n", szProfile, hr, pErrors ? (const char*)pErrors->GetBufferPointer() : "");
			SafeRelease(pBlob);
		}

		SafeRelease(pErrors);
		return pBlob;
	}

	static bool CreatePS(const char* szCommon, const char* szSource, ID3D11PixelShader** ppShader)
	{
		ID3DBlob* pBlob = Compile(szCommon, szSource, "ps_4_0");

		if (!pBlob)
			return false;

		HRESULT hr = s_pDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, ppShader);

		pBlob->Release();
		return SUCCEEDED(hr);
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

	static void ReleaseTargets(void)
	{
		SafeRelease(s_pSceneDepthSRV);
		SafeRelease(s_pSceneDepthCopy);
		SafeRelease(s_pSceneDepth);
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
		s_bMaskStarted = false;
	}

	static bool CreateColorTarget(UINT uWidth, UINT uHeight, ID3D11Texture2D** ppTexture, ID3D11RenderTargetView** ppRTV, ID3D11ShaderResourceView** ppSRV)
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

	// Makes the targets for the game's depth buffer pTexture. Only the biggest one it draws a
	// target into is followed, the main one: reflections and the like get smaller ones.
	static bool FollowSceneDepth(ID3D11Texture2D* pTexture)
	{
		if (pTexture == s_pSceneDepth)
			return true;

		D3D11_TEXTURE2D_DESC Desc;
		DXGI_FORMAT Typeless, ShaderFormat;

		pTexture->GetDesc(&Desc);

		if (Desc.SampleDesc.Count > 1 || Desc.ArraySize != 1 || !GetDepthFormats(Desc.Format, &Typeless, &ShaderFormat))
			return false;

		if (s_pSceneDepth && (UINT64)Desc.Width * Desc.Height <= (UINT64)s_uWidth * s_uHeight)
			return false;

		ReleaseTargets();

		D3D11_TEXTURE2D_DESC DepthDesc = {};

		DepthDesc.Width = Desc.Width;
		DepthDesc.Height = Desc.Height;
		DepthDesc.MipLevels = 1;
		DepthDesc.ArraySize = 1;
		DepthDesc.Format = DXGI_FORMAT_R32_TYPELESS;
		DepthDesc.SampleDesc.Count = 1;
		DepthDesc.Usage = D3D11_USAGE_DEFAULT;
		DepthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;

		D3D11_DEPTH_STENCIL_VIEW_DESC DSVDesc = {};

		DSVDesc.Format = DXGI_FORMAT_D32_FLOAT;
		DSVDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

		D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};

		SRVDesc.Format = DXGI_FORMAT_R32_FLOAT;
		SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		SRVDesc.Texture2D.MipLevels = 1;

		bool bOk = CreateColorTarget(Desc.Width, Desc.Height, &s_pMask, &s_pMaskRTV, &s_pMaskSRV) &&
			SUCCEEDED(s_pDevice->CreateTexture2D(&DepthDesc, NULL, &s_pMaskDepthTex)) &&
			SUCCEEDED(s_pDevice->CreateDepthStencilView(s_pMaskDepthTex, &DSVDesc, &s_pMaskDSV)) &&
			SUCCEEDED(s_pDevice->CreateShaderResourceView(s_pMaskDepthTex, &SRVDesc, &s_pMaskDepthSRV));

		for (int i = 0; i < 2 && bOk; ++i)
			bOk = CreateColorTarget(max(Desc.Width / 2, 1u), max(Desc.Height / 2, 1u), &s_pGlow[i], &s_pGlowRTV[i], &s_pGlowSRV[i]);

		// The game's depth itself when a shader can read it, else a copy made each frame
		SRVDesc.Format = ShaderFormat;

		if (bOk && (Desc.BindFlags & D3D11_BIND_SHADER_RESOURCE))
		{
			bOk = SUCCEEDED(s_pDevice->CreateShaderResourceView(pTexture, &SRVDesc, &s_pSceneDepthSRV));
		}
		else if (bOk)
		{
			D3D11_TEXTURE2D_DESC CopyDesc = Desc;

			CopyDesc.Format = Typeless;
			CopyDesc.Usage = D3D11_USAGE_DEFAULT;
			CopyDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			CopyDesc.CPUAccessFlags = 0;
			CopyDesc.MiscFlags = 0;

			bOk = SUCCEEDED(s_pDevice->CreateTexture2D(&CopyDesc, NULL, &s_pSceneDepthCopy)) &&
				SUCCEEDED(s_pDevice->CreateShaderResourceView(s_pSceneDepthCopy, &SRVDesc, &s_pSceneDepthSRV));
		}

		if (!bOk)
		{
			LOG("EspRenderer: couldn't make the targets for a %ux%u depth buffer (format %d)\n", Desc.Width, Desc.Height, Desc.Format);
			ReleaseTargets();
			return false;
		}

		s_pSceneDepth = pTexture;
		s_pSceneDepth->AddRef();
		s_uWidth = Desc.Width;
		s_uHeight = Desc.Height;

		LOG("EspRenderer: following the %ux%u depth buffer %p (format %d, %s)\n", Desc.Width, Desc.Height, pTexture, Desc.Format,
			s_pSceneDepthCopy ? "copied each frame" : "read directly");

		return true;
	}

	bool Initialize(ID3D11Device* pDevice)
	{
		if (!pDevice)
			return false;

		s_pDevice = pDevice;

		ID3DBlob* pVS = Compile(NULL, s_szFullscreenVS, "vs_4_0");

		if (!pVS)
			return false;

		HRESULT hr = s_pDevice->CreateVertexShader(pVS->GetBufferPointer(), pVS->GetBufferSize(), NULL, &s_pFullscreenVS);

		pVS->Release();

		if (FAILED(hr) ||
			!CreatePS(NULL, s_szMaskPS, &s_pMaskPS) ||
			!CreatePS(s_szCommon, s_szChamsPS, &s_pChamsPS) ||
			!CreatePS(s_szCommon, s_szGlowPrepPS, &s_pGlowPrepPS) ||
			!CreatePS(s_szCommon, s_szBlurPS, &s_pBlurPS) ||
			!CreatePS(s_szCommon, s_szGlowPS, &s_pGlowPS))
		{
			Shutdown();
			return false;
		}

		D3D11_BUFFER_DESC BufferDesc = {};

		BufferDesc.ByteWidth = 2 * 4 * sizeof(float);
		BufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		BufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		D3D11_SAMPLER_DESC SamplerDesc = {};

		SamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		SamplerDesc.AddressU = SamplerDesc.AddressV = SamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		SamplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		SamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

		bool bOk = SUCCEEDED(s_pDevice->CreateBuffer(&BufferDesc, NULL, &s_pParams)) &&
			SUCCEEDED(s_pDevice->CreateSamplerState(&SamplerDesc, &s_pSamplers[0]));

		SamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		bOk = bOk && SUCCEEDED(s_pDevice->CreateSamplerState(&SamplerDesc, &s_pSamplers[1]));

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

		SafeRelease(s_pFullscreenVS);
		SafeRelease(s_pMaskPS);
		SafeRelease(s_pChamsPS);
		SafeRelease(s_pGlowPrepPS);
		SafeRelease(s_pBlurPS);
		SafeRelease(s_pGlowPS);
		SafeRelease(s_pParams);
		SafeRelease(s_pSamplers[0]);
		SafeRelease(s_pSamplers[1]);
		SafeRelease(s_pAdditive);
		SafeRelease(s_pMaskDepth[0]);
		SafeRelease(s_pMaskDepth[1]);
		SafeRelease(s_pNoDepth);
		SafeRelease(s_pFullscreenRS);

		s_pDevice = NULL;
	}

	static void SetParams(ID3D11DeviceContext* pContext, const float A[4], const float B[4])
	{
		D3D11_MAPPED_SUBRESOURCE Mapped;

		if (FAILED(pContext->Map(s_pParams, 0, D3D11_MAP_WRITE_DISCARD, 0, &Mapped)))
			return;

		memcpy(Mapped.pData, A, 4 * sizeof(float));
		memcpy((float*)Mapped.pData + 4, B, 4 * sizeof(float));
		pContext->Unmap(s_pParams, 0);
	}

	bool BeginMaskDraw(ID3D11DeviceContext* pContext, const float Color[3], uint32_t uFlags)
	{
		if (!s_pMaskPS)
			return false;

		// The mask follows the depth buffer the game draws its targets into
		ID3D11RenderTargetView* pTarget = NULL;
		ID3D11DepthStencilView* pDSV = NULL;

		pContext->OMGetRenderTargets(1, &pTarget, &pDSV);
		SafeRelease(pTarget);

		if (!pDSV)
			return false;

		ID3D11Resource* pResource = NULL;
		ID3D11Texture2D* pTexture = NULL;

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

		pTexture->Release();

		if (!bFollowed)
		{
			s_bSkippedOther = true;
			return false;
		}

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
		pContext->PSGetConstantBuffers(0, 1, &s_pSavedPSParams);

		const float MaskColor[4] = { Color[0], Color[1], Color[2], (float)(uFlags | ESPMASK_PRESENT) / 255.0f };
		const float Unused[4] = {};

		SetParams(pContext, MaskColor, Unused);

		pContext->OMSetRenderTargets(1, &s_pMaskRTV, s_pMaskDSV);
		pContext->OMSetDepthStencilState(s_pMaskDepth[s_bReversedDepth ? 1 : 0], 0);
		pContext->OMSetBlendState(NULL, NULL, 0xFFFFFFFF);
		pContext->PSSetShader(s_pMaskPS, NULL, 0);
		pContext->PSSetConstantBuffers(0, 1, &s_pParams);

		s_uFlagsDrawn |= uFlags;
		return true;
	}

	void EndMaskDraw(ID3D11DeviceContext* pContext)
	{
		pContext->OMSetRenderTargets(D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT, s_pSavedRTVs, s_pSavedDSV);
		pContext->OMSetDepthStencilState(s_pSavedDepth, s_uSavedStencilRef);
		pContext->OMSetBlendState(s_pSavedBlend, s_SavedBlendFactor, s_uSavedSampleMask);
		pContext->PSSetShader(s_pSavedPS, NULL, 0);
		pContext->PSSetConstantBuffers(0, 1, &s_pSavedPSParams);

		for (ID3D11RenderTargetView*& pRTV : s_pSavedRTVs)
			SafeRelease(pRTV);

		SafeRelease(s_pSavedDSV);
		SafeRelease(s_pSavedDepth);
		SafeRelease(s_pSavedBlend);
		SafeRelease(s_pSavedPS);
		SafeRelease(s_pSavedPSParams);
	}

	// Everything Composite changes, put back afterwards: the game's state cache only binds what it
	// thinks has changed, so anything left bound would carry into its next frame
	struct StateBackup_t
	{
		D3D11_PRIMITIVE_TOPOLOGY Topology;
		ID3D11InputLayout* pLayout;
		ID3D11VertexShader* pVS;
		ID3D11HullShader* pHS;
		ID3D11DomainShader* pDS;
		ID3D11GeometryShader* pGS;
		ID3D11PixelShader* pPS;
		ID3D11ShaderResourceView* pSRVs[3];
		ID3D11SamplerState* pSamplers[2];
		ID3D11Buffer* pPSParams;
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
			pContext->PSGetShaderResources(0, 3, pSRVs);
			pContext->PSGetSamplers(0, 2, pSamplers);
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
			pContext->PSSetShaderResources(0, 3, pSRVs);
			pContext->PSSetSamplers(0, 2, pSamplers);
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

			for (ID3D11SamplerState*& pSampler : pSamplers)
				SafeRelease(pSampler);

			SafeRelease(pPSParams);
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
		ID3D11ShaderResourceView* const NoSRVs[3] = {};

		// A texture can't be read and drawn into at once
		pContext->PSSetShaderResources(0, 3, NoSRVs);
		pContext->OMSetRenderTargets(1, &pRTV, NULL);
		pContext->RSSetViewports(1, &Viewport);
	}

	static void EndFrame(void)
	{
		// A resolution change gives the game a smaller depth buffer, which FollowSceneDepth won't
		// take while the old one is held: after a second of only that, let go
		if (s_bSkippedOther && !s_bMaskStarted)
		{
			if (++s_nMissedFrames > 60)
			{
				ReleaseTargets();
				s_nMissedFrames = 0;
			}
		}
		else
		{
			s_nMissedFrames = 0;
		}

		s_bSkippedOther = false;
		s_bMaskStarted = false;
		s_uFlagsDrawn = 0;
	}

	void Composite(ID3D11DeviceContext* pContext, ID3D11RenderTargetView* pBackBuffer)
	{
		if (!s_bMaskStarted || !pBackBuffer || !(s_uFlagsDrawn & (ESPMASK_CHAMS | ESPMASK_GLOW)))
		{
			EndFrame();
			return;
		}

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

		StateBackup_t Backup = {};

		Backup.Save(pContext);

		if (s_pSceneDepthCopy)
			pContext->CopyResource(s_pSceneDepthCopy, s_pSceneDepth);

		pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		pContext->IASetInputLayout(NULL);
		pContext->VSSetShader(s_pFullscreenVS, NULL, 0);
		pContext->HSSetShader(NULL, NULL, 0);
		pContext->DSSetShader(NULL, NULL, 0);
		pContext->GSSetShader(NULL, NULL, 0);
		pContext->RSSetState(s_pFullscreenRS);
		pContext->OMSetDepthStencilState(s_pNoDepth, 0);
		pContext->PSSetSamplers(0, 2, s_pSamplers);
		pContext->PSSetConstantBuffers(0, 1, &s_pParams);

		// The back buffer's uv onto the part of the mask the game's viewport covered
		const float UV[4] = {
			s_MaskViewport.Width / (float)s_uWidth, s_MaskViewport.Height / (float)s_uHeight,
			s_MaskViewport.TopLeftX / (float)s_uWidth, s_MaskViewport.TopLeftY / (float)s_uHeight
		};

		if ((s_uFlagsDrawn & ESPMASK_CHAMS) && s_pSceneDepthSRV)
		{
			const float Misc[4] = { s_bReversedDepth ? 1.0f : 0.0f, 1e-6f, 0.0f, 0.0f };
			ID3D11ShaderResourceView* const SRVs[3] = { s_pMaskSRV, s_pMaskDepthSRV, s_pSceneDepthSRV };

			SetTarget(pContext, pBackBuffer, BackBufferDesc.Width, BackBufferDesc.Height);
			SetParams(pContext, UV, Misc);
			pContext->OMSetBlendState(NULL, NULL, 0xFFFFFFFF);
			pContext->PSSetShader(s_pChamsPS, NULL, 0);
			pContext->PSSetShaderResources(0, 3, SRVs);
			pContext->Draw(3, 0);
		}

		if (s_uFlagsDrawn & ESPMASK_GLOW)
		{
			const UINT uGlowWidth = max(s_uWidth / 2, 1u);
			const UINT uGlowHeight = max(s_uHeight / 2, 1u);
			const float NoUV[4] = {};
			const float PrepMisc[4] = { 0.0f, 0.0f, 1.0f / (float)s_uWidth, 1.0f / (float)s_uHeight };

			pContext->OMSetBlendState(NULL, NULL, 0xFFFFFFFF);

			// The glow targets' silhouettes at half size
			SetTarget(pContext, s_pGlowRTV[0], uGlowWidth, uGlowHeight);
			SetParams(pContext, NoUV, PrepMisc);
			pContext->PSSetShader(s_pGlowPrepPS, NULL, 0);
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
				SetParams(pContext, Step, NoUV);
				pContext->PSSetShaderResources(0, 1, &s_pGlowSRV[iSource]);
				pContext->Draw(3, 0);
			}

			// Added to the back buffer around the silhouettes
			const float GlowMisc[4] = { 0.0f, 0.0f, 2.5f, 0.0f };
			ID3D11ShaderResourceView* const SRVs[2] = { s_pGlowSRV[0], s_pMaskSRV };

			SetTarget(pContext, pBackBuffer, BackBufferDesc.Width, BackBufferDesc.Height);
			SetParams(pContext, UV, GlowMisc);
			pContext->OMSetBlendState(s_pAdditive, NULL, 0xFFFFFFFF);
			pContext->PSSetShader(s_pGlowPS, NULL, 0);
			pContext->PSSetShaderResources(0, 2, SRVs);
			pContext->Draw(3, 0);
		}

		ID3D11ShaderResourceView* const NoSRVs[3] = {};

		pContext->PSSetShaderResources(0, 3, NoSRVs);

		Backup.Restore(pContext);
		EndFrame();
	}
}
