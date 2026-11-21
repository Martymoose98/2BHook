// Console, module dump and logging in every configuration. It has to come before the
// includes: Log.h picks the LOG/WARN/LERROR definitions when it is included, so defining it
// further down left them as no-ops in Release (no SHA256 or failure messages).
#define VERBOSE

#include "WinInclude.h"
#include <d3d11.h>
#include <VersionHelpers.h>

#include <iostream>

#include <Fluorine/ExportUtil.h>
#include <Fluorine/ImportTableHook.h>
#include <Fluorine/Memory.h>
#include <Fluorine/VirtualTableHook.h>

#include "cpk.h"
#include "mot.h"
#include "EspRenderer.h"
#include "Hooks.h"
#include "MaterialTint.h"
#include "Version.h"

// temp
NTSTATUS DumpModules(/*PEPROCESS pProcess, PKERNEL_MODULE_REQUEST pRequest*/);

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "xinput.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dbghelp.lib")
#pragma comment(lib, "bcrypt.lib")

//#define ENABLE_MEMORYDEVICE_HOOK

CONST BYTE LocalJmp[] = { 0xEB };
CONST BYTE JmpFramecap[] = { 0xE9, 0x93, 0x00, 0x00, 0x00, 0x90, 0x90 };

#ifdef DENUVO_STEAM_BUILD
#define FindOffsets FindDenuvoSteamOffsets
#else
#define FindOffsets FindSteamOffsets
#endif


void InitHooks(void)
{
	g_pFactoryHook = new VirtualTableHook((ULONG_PTR**)g_pFactory);
	g_pSwapChainHook = new VirtualTableHook((ULONG_PTR**)g_pSwapChain);
	g_pDeviceContextHook = new VirtualTableHook((ULONG_PTR**)g_pDeviceContext);

	{
		// Hold the internal CDIDev's critical section while the keyboard's vtable hook is
		// built, to stop crashes. Released at the end of this scope, before Rehook().
		CDirectInputDeviceLock KeyboardLock(g_pKeyboard->pDevice);

		g_pKeyboardHook = new VirtualTableHook((ULONG_PTR**)g_pKeyboard->pDevice);

		g_pKeyboardHook->Unhook();
	}

	g_pKeyboardHook->Rehook();

	{
		// Same for the mouse
		CDirectInputDeviceLock MouseLock(g_pMouse->pDevice);

		g_pMouseHook = new VirtualTableHook((ULONG_PTR**)g_pMouse->pDevice);

		g_pMouseHook->Unhook();
	}

	g_pMouseHook->Rehook();

	g_pCameraHook = new VirtualTableHook((ULONG_PTR**)g_pCamera);

	g_pModelAnalyzerHook = new VirtualTableHook((ULONG_PTR**)g_pModelAnalyzer);
	oCreateModelShaderModule = (CModelAnalyzer_CreateModelShaderModuleFn)g_pModelAnalyzerHook->HookFunction((ULONG_PTR)&hkCreateModelShaderModule, 3);

#ifdef ENABLE_MEMORYDEVICE_HOOK
	g_pMemoryDeviceHook = new MemoryDeviceHook(g_pMemoryDevice); // for larger child heap sizes
#endif

	for (MrubyImpl* it : g_pRubyInstances)
	{
		g_pRubyInstancesHooks.emplace_back(new VirtualTableHook((ULONG_PTR**)it));
		oMRubyLoadScript = (MRubyLoadScriptFn)g_pRubyInstancesHooks.back()->HookFunction((ULONG_PTR)&hkMRubyLoadScript, 2);
	}

	g_pQueryPerformanceCounterHook = new ImportTableHook("kernel32.dll", "QueryPerformanceCounter", (LPCVOID)&hkQueryPerformanceCounter);
	oQueryPerformanceCounter = (QueryPerformaceCounterFn)g_pQueryPerformanceCounterHook->GetOriginalFunction();

	g_pClipCursorHook = new ImportTableHook("user32.dll", "SetCursorPos", (LPCVOID)hkSetCursorPos);
	oSetCursorPos = (SetCursorPosFn)g_pClipCursorHook->GetOriginalFunction();

	g_pXInputGetStateHook = new ImportTableHook("xinput1_3.dll", "XInputGetState", (LPCVOID)hkXInputGetState);
	oXInputGetState = (XInputGetStateFn)g_pXInputGetStateHook->GetOriginalFunction();

	g_pSetUnhandledExceptionFilterHook = new ImportTableHook("kernel32.dll", "SetUnhandledExceptionFilter", (LPCVOID)&hkSetUnhandledExceptionFilter);
	oSetUnhandledExceptionFilter = (SetUnhandledExceptionFilterFn)g_pSetUnhandledExceptionFilterHook->GetOriginalFunction();

	oPresent = (PresentFn)g_pSwapChainHook->HookFunction((ULONG_PTR)hkPresent, 8);

#ifdef DENUVO_STEAM_BUILD
	// NOTE: This is only useful/wordks on the old version. New version uses CreateSwapChainForHwnd
	oCreateSwapChain = (CreateSwapChainFn)g_pFactoryHook->HookFunction((ULONG_PTR)hkCreateSwapChain, 10);
#else
	oResizeBuffers = (ResizeBuffersFn)g_pSwapChainHook->HookFunction((ULONG_PTR)hkResizeBuffers, 13);
#endif // DENUVO_STEAM_BUILD
	oPSSetShaderResources = (PSSetShaderResourcesFn)g_pDeviceContextHook->HookFunction((ULONG_PTR)hkPSSetShaderResources, 8);
	oDrawIndexed = (DrawIndexedFn)g_pDeviceContextHook->HookFunction((ULONG_PTR)hkDrawIndexed, 12);
	oDraw = (DrawFn)g_pDeviceContextHook->HookFunction((ULONG_PTR)hkDraw, 13);
	oClearRenderTargetView = (ClearRenderTargetViewFn)g_pDeviceContextHook->HookFunction((ULONG_PTR)hkClearRenderTargetView, 50);
	oKeyboardRelease = (ReleaseFn)g_pKeyboardHook->HookFunction((ULONG_PTR)hkKeyboardRelease, 2);
	oKeyboardAcquire = (AcquireFn)g_pKeyboardHook->HookFunction((ULONG_PTR)hkKeyboardAcquire, 7);
	oKeyboardGetDeviceState = (GetDeviceStateFn)g_pKeyboardHook->HookFunction((ULONG_PTR)hkKeyboardGetDeviceState, 9);
	oMouseRelease = (ReleaseFn)g_pMouseHook->HookFunction((ULONG_PTR)hkMouseRelease, 2);
	oMouseAcquire = (AcquireFn)g_pMouseHook->HookFunction((ULONG_PTR)hkMouseAcquire, 7);
	oMouseGetDeviceState = (GetDeviceStateFn)g_pMouseHook->HookFunction((ULONG_PTR)hkMouseGetDeviceState, 9);
	oCCameraGameSetViewAngles = (CCameraGame_SetViewAnglesFn)g_pCameraHook->HookFunction((ULONG_PTR)hkCCameraGameSetViewAngles, 1);
	oCCameraGameMove = (CCameraGame_MoveFn)g_pCameraHook->HookFunction((ULONG_PTR)hkCCameraGameMove, 2);

	ZeroMemory(&g_UserManagerSaveFileIO, sizeof(HOOK_FUNC));
	ZeroMemory(&g_CreateEntityHook, sizeof(HOOK_FUNC));
	ZeroMemory(&g_LoadWordBlacklist, sizeof(HOOK_FUNC));

#ifdef DENUVO_STEAM_BUILD
	HookFunc64(SaveFileIO, hkSaveFileIO, 45, &g_UserManagerSaveFileIO);

	// FIXME:  this could be moved idk it's debatable  
	oCreateEntity = (CreateEntityFn)FindPattern(NULL, "48 89 5C 24 ? 48 89 4C 24 ? 55 48 83 EC 20");
	QWORD qwCallerFunc = FindPatternPtr(NULL, "E8 ? ? ? ? 85 C0 75 1F 48 8B CD", 1);

	//THERE IS ANOTHER CreateEntity call at: qwCallerFunc + 0x676
	HookFunc64((LPVOID)(qwCallerFunc + 0x132), hkCreateEntityThunk, 16, &g_CreateEntityHook);

	// FIXME: sig this hard coded offset for both version clown
	//HookFunc64((VOID*)0x140607011, hkLoadWordBlacklist, 157, &hf); //caller
	HookFunc64((VOID*)0x140606940, hkLoadWordBlacklistThunk, 20, &g_LoadWordBlacklist);//callee
#else // DENUVO_STEAM_BUILD

	// FIXME: This hook works but is disgusting
	//	hmm it seems yo do have to pause the threads to do this
	//	if you get really unlucky you can crash
	//	or find someway to syncrhronize it
	//	crashed on hooking the save io stuff
	HookFunc64(SaveFileIO, hkSaveFileIOThunk, 46, &g_UserManagerSaveFileIO);

	// FIXME: this could be moved idk it's debatable  
	oCreateEntity = (CreateEntityFn)FindPatternPtr(NULL, "E8 ? ? ? ? 48 89 47 60 48 85 C0 75 9D", 1);
	QWORD qwCallerFunc = FindPatternPtr(NULL, "E8 ? ? ? ? 85 C0 75 25 48 8B CD", 1);

	//THERE IS ANOTHER CreateEntity call at: qwCallerFunc + 0x6A1
	HookFunc64((LPVOID)(qwCallerFunc + 0x237 - 0x16), hkCreateEntityThunk, 27, &g_CreateEntityHook);
#endif // !OLD_DENUVO_STEAM_BUILD

	// Also, forget what this does but it's not critical for the dll's operation.
	// If I do keep I need to resig the new version
#ifdef DENUVO_STEAM_BUILD
	InitializeNopMemory(&nop_HairColor, (void*)FindPattern(NULL, "0F 29 22 49 8B CE"), 12);
#else
	InitializeNopMemory(&nop_HairColor, (void*)FindPattern(NULL, "E8 ? ? ? ? 44 89 6B 68"), 9);
#endif
	NopMemory(&nop_HairColor);

#ifndef DENUVO_STEAM_BUILD
	// Per-material colours (issue #16); disables itself if a signature is missing
	MaterialTint::Init();
#endif

	// Play a .mot (Visuals > Motion); off when its signatures aren't found
	if (!MotionPlayer::Initialize())
		WARN("Motion player: not found in this game build.\n");

	// Show Game Errors. The game never installs a CRI error callback, so CriLogInternal counts every
	// CRI error and warning and drops it. Only into an empty slot, so another mod's stays.
	if (CRIMainCallbackLog)
		InterlockedCompareExchangePointer((PVOID*)CRIMainCallbackLog, (PVOID)&CRILogCallbackConsole, NULL);
}

// A 1x1 texture of one colour (0xAABBGGRR) and its view, for Chams without "Use Shader"
static HRESULT CreateColorTexture(uint32_t uColor, ID3D11ShaderResourceView** ppView)
{
	D3D11_SUBRESOURCE_DATA Data = { &uColor, sizeof(uColor), 0 };
	D3D11_TEXTURE2D_DESC Desc = {};
	Desc.Width = Desc.Height = Desc.MipLevels = Desc.ArraySize = 1;
	Desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	Desc.SampleDesc.Count = 1;
	Desc.Usage = D3D11_USAGE_IMMUTABLE;
	Desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	ID3D11Texture2D* pTexture = NULL;
	HRESULT hr = g_pDevice->CreateTexture2D(&Desc, &Data, &pTexture);

	if (FAILED(hr))
		return hr;

	hr = g_pDevice->CreateShaderResourceView(pTexture, NULL, ppView);
	pTexture->Release();
	return hr;
}

HRESULT CreateRenderTarget(void)
{
	HRESULT hr = S_OK;

	DXGI_SWAP_CHAIN_DESC SwapChainDesc;

	hr = g_pSwapChain->GetDesc(&SwapChainDesc);

	if (FAILED(hr))
	{
		LERROR("Failed to get swapchain description");
		return hr;
	}

	// Create the render target
	ID3D11Texture2D* pBackBuffer;
	D3D11_RENDER_TARGET_VIEW_DESC RenderTargetViewDesc;

	ZeroMemory(&RenderTargetViewDesc, sizeof(RenderTargetViewDesc));

	RenderTargetViewDesc.Format = SwapChainDesc.BufferDesc.Format;
	RenderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

	hr = g_pSwapChain->GetBuffer(0, IID_ID3D11Texture2D, (void**)&pBackBuffer);

	if (FAILED(hr))
	{
		LERROR("Failed to get backbuffer from swapchain");
		return hr;
	}

	if (pBackBuffer)
	{
		hr = g_pDevice->CreateRenderTargetView(pBackBuffer, &RenderTargetViewDesc, &g_pRenderTargetView);

		if (SUCCEEDED(hr))
			g_pDeviceContext->OMSetRenderTargets(1, &g_pRenderTargetView, NULL);
		else
			LERROR("Failed to CreateRenderTargetView for backbuffer");

		pBackBuffer->Release();
	}

	return hr;
}

HRESULT CreateStencilDescription(void)
{
	HRESULT hr = S_OK;

	D3D11_DEPTH_STENCIL_DESC  StencilDesc;
	StencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
	StencilDesc.StencilEnable = true;
	StencilDesc.StencilReadMask = 0xFF;
	StencilDesc.StencilWriteMask = 0xFF;
	StencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	StencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	StencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	StencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	StencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	StencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	StencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	StencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	StencilDesc.DepthEnable = true;
	StencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	hr = g_pDevice->CreateDepthStencilState(&StencilDesc, &g_pDepthStencilStates[ENABLED]);

	if (FAILED(hr))
	{
		LERROR("Failed to CreateDepthStencilState for enabled state");
		return hr;
	}

	StencilDesc.DepthEnable = false;
	StencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	hr = g_pDevice->CreateDepthStencilState(&StencilDesc, &g_pDepthStencilStates[DISABLED]);

	if (FAILED(hr))
	{
		LERROR("Failed to CreateDepthStencilState for disabled state");
		return hr;
	}

	StencilDesc.DepthEnable = false;
	StencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	StencilDesc.StencilEnable = false;
	StencilDesc.StencilReadMask = 0xFF;
	StencilDesc.StencilWriteMask = 0x0;
	hr = g_pDevice->CreateDepthStencilState(&StencilDesc, &g_pDepthStencilStates[NO_READ_NO_WRITE]);

	if (FAILED(hr))
	{
		LERROR("Failed to CreateDepthStencilState for noreadnowrite state");
		return hr;
	}

	StencilDesc.DepthEnable = true;
	StencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	StencilDesc.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL;
	StencilDesc.StencilEnable = false;
	StencilDesc.StencilReadMask = 0xFF;
	StencilDesc.StencilWriteMask = 0x0;

	StencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_ZERO;
	StencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_ZERO;
	StencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	StencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;

	StencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_ZERO;
	StencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_ZERO;
	StencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_ZERO;
	StencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_NEVER;
	hr = g_pDevice->CreateDepthStencilState(&StencilDesc, &g_pDepthStencilStates[READ_NO_WRITE]);

	if (FAILED(hr))
	{
		LERROR("Failed to CreateDepthStencilState for readnowrite state");
		return hr;
	}

	return hr;
}

HRESULT CreateRasterizerStates(void)
{
	HRESULT hr = S_OK;

	// wireframe	
	D3D11_RASTERIZER_DESC RenderWireframeDesc;
	g_pDeviceContext->RSGetState(&g_pRenderWireframeState); // retrieve the current state
	g_pRenderWireframeState->GetDesc(&RenderWireframeDesc); // get the desc of the state
	RenderWireframeDesc.FillMode = D3D11_FILL_WIREFRAME;
	RenderWireframeDesc.CullMode = D3D11_CULL_NONE;
	// create a whole new rasterizer state
	hr = g_pDevice->CreateRasterizerState(&RenderWireframeDesc, &g_pRenderWireframeState);

	if (FAILED(hr))
	{
		LERROR("Failed to CreateRasterizerState for wireframe");
		return hr;
	}

	// solid
	D3D11_RASTERIZER_DESC RenderSolidDesc;
	g_pDeviceContext->RSGetState(&g_pRenderSolidState); // retrieve the current state
	g_pRenderSolidState->GetDesc(&RenderSolidDesc);     // get the desc of the state
	RenderSolidDesc.FillMode = D3D11_FILL_SOLID;
	RenderSolidDesc.CullMode = D3D11_CULL_BACK;
	// create a whole new rasterizer state
	hr = g_pDevice->CreateRasterizerState(&RenderSolidDesc, &g_pRenderSolidState);

	if (FAILED(hr))
	{
		LERROR("Failed to CreateRasterizerState for solid");
		return hr;
	}
	return hr;
}

HRESULT InitD3D11(void)
{
	// i'm using IID_ID3D11Device for c compatiblity alternatively you can use __uuidof
	HRESULT hr = g_pSwapChain->GetDevice(IID_ID3D11Device, (void**)&g_pDevice);

	if (FAILED(hr))
	{
		LERROR("2B Hook Failed Initialization!\nCould not obtain a ID3D11Device pointer! HRESULT %x\n", hr);
		return hr;
	}

	hr = g_pSwapChain->GetParent(IID_IDXGIFactory, (void**)&g_pFactory);

	if (FAILED(hr))
	{
		LERROR("2B Hook Failed Initialization!\nCould not obtain a IDXGIFactory pointer! HRESULT %x\n", hr);
		return hr;
	}

	g_pDevice->GetImmediateContext(&g_pDeviceContext);

	ImGui::CreateContext();

	if (!ImGui_ImplDX11_Init(g_pDevice, g_pDeviceContext) || !ImGui_ImplWin32_Init(g_hWnd))
	{
		LERROR("2B Hook Failed Initialization!\nCould not initialize ImGui!\n");
		return S_FALSE;
	}

	if (!ImGui_ImplDX11_CreateDeviceObjects())
	{
		LERROR("2B Hook Failed Initialization!\nCould not create device objects!\n");
		return S_FALSE;
	}

	hr = CreateStencilDescription();

	if (FAILED(hr))
		return hr;

	hr = CreateRenderTarget();

	if (FAILED(hr))
		return hr;

	hr = CreateRasterizerStates();

	if (FAILED(hr))
		return hr;

	//create sample state
	D3D11_SAMPLER_DESC SamplerDesc;
	ZeroMemory(&SamplerDesc, sizeof(SamplerDesc));
	SamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	SamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	SamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	SamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	SamplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	SamplerDesc.MaxAnisotropy = 1;
	SamplerDesc.MinLOD = 0;
	SamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	hr = g_pDevice->CreateSamplerState(&SamplerDesc, &g_pSamplerState);

	if (FAILED(hr))
	{
		LERROR("Failed to CreateSamplerState");
		return hr;
	}

	//create green texture
	DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; //format
	static const uint32_t s_pixel = 0xff00ff00; //0xffffffff white, 0xff00ff00 green, 0xffff0000 blue, 0xff0000ff red
	D3D11_SUBRESOURCE_DATA initData = { &s_pixel, sizeof(uint32_t), 0 };
	D3D11_TEXTURE2D_DESC desc;
	memset(&desc, 0, sizeof(desc));
	desc.Width = desc.Height = desc.MipLevels = desc.ArraySize = 1;
	desc.Format = format;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_DEFAULT;// D3D11_USAGE_IMMUTABLE;
	desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	hr = g_pDevice->CreateTexture2D(&desc, &initData, &g_pTexGreen);

	if (FAILED(hr))
	{
		LERROR("Failed to CreateTexture2D for green texture");
		return hr;
	}

	//create red texture
	static const uint32_t s_pixelr = 0xff0000ff; //0xffffffff white, 0xff00ff00 green, 0xffff0000 blue, 0xff0000ff red
	D3D11_SUBRESOURCE_DATA initDatar = { &s_pixelr, sizeof(uint32_t), 0 };
	D3D11_TEXTURE2D_DESC descr;
	memset(&descr, 0, sizeof(descr));
	descr.Width = descr.Height = descr.MipLevels = descr.ArraySize = 1;
	descr.Format = format;
	descr.SampleDesc.Count = 1;
	descr.Usage = D3D11_USAGE_DEFAULT;// D3D11_USAGE_IMMUTABLE;
	descr.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	hr = g_pDevice->CreateTexture2D(&descr, &initDatar, &g_pTexRed);

	if (FAILED(hr))
	{
		LERROR("Failed to CreateTexture2D for red texture");
		return hr;
	}

	//create green shaderresourceview
	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
	memset(&SRVDesc, 0, sizeof(SRVDesc));
	SRVDesc.Format = format;
	SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	SRVDesc.Texture2D.MipLevels = 1;
	hr = g_pDevice->CreateShaderResourceView(g_pTexGreen, &SRVDesc, &g_pGreenSRV);

	g_pTexGreen->Release();

	if (FAILED(hr))
	{
		LERROR("Failed to CreateShaderResourceView for green texture");
		return hr;
	}

	//create red shaderresourceview
	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDescr;
	memset(&SRVDescr, 0, sizeof(SRVDescr));
	SRVDescr.Format = format;
	SRVDescr.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	SRVDescr.Texture2D.MipLevels = 1;
	hr = g_pDevice->CreateShaderResourceView(g_pTexRed, &SRVDescr, &g_pRedSRV);

	g_pTexRed->Release();

	if (FAILED(hr)) {
		LERROR("Failed to CreateShaderResourceView for red texture");
		return hr;
	}

	// The chams' hidden colour and the glow
	if (!EspRenderer::Initialize(g_pDevice))
		LERROR("EspRenderer failed to initialize: no hidden chams colour or glow");

	GeneratePixelShader(g_pDevice, &g_pRed, 1.0f, 0.0f, 0.0f);
	GeneratePixelShader(g_pDevice, &g_pGreen, 0.0f, 1.0f, 0.0f);

	// NPC Chams' colours
	GeneratePixelShader(g_pDevice, &g_pBlue, 0.0f, 0.3f, 1.0f);
	GeneratePixelShader(g_pDevice, &g_pCyan, 0.0f, 1.0f, 1.0f);
	CreateColorTexture(0xFFFF4D00, &g_pBlueSRV);
	CreateColorTexture(0xFFFFFF00, &g_pCyanSRV);

	return S_OK;
}

void ExposeHiddenXInputFunctions(void)
{
	HMODULE hXInput910 = GetModuleHandleA("xinput9_1_0.dll");
	HMODULE hXInput13 = GetModuleHandleA("xinput1_3.dll");
	HMODULE hXInput14 = GetModuleHandleA("xinput1_4.dll");
	HMODULE hLatestXInput = (hXInput14) ? hXInput14 : (hXInput13) ? hXInput13 : hXInput910;

	if (hLatestXInput && hLatestXInput != hXInput910)
	{
		XInputGetStateEx = (XInputGetStateExFn)GetProcAddress(hLatestXInput, (LPCSTR)100);
		InputWaitForGuideButton = (XInputWaitForGuideButtonFn)GetProcAddress(hLatestXInput, (LPCSTR)101);

		if (hLatestXInput == hXInput14)
		{
			XInputGetBaseBusInformation = (XInputGetBaseBusInformationFn)GetProcAddress(hLatestXInput, (LPCSTR)104);
			InputGetCapabilitiesEx = (XInputGetCapabilitiesExFn)GetProcAddress(hLatestXInput, (LPCSTR)108);
		}
	}
}

void LogOffsets(void)
{
	CCONSOLE_DEBUG_LOG(ImVec4(0.0f, 0.5f, 0.8f, 1.0f), "[Function Pointers]");
	LOG_OFFSET("CriMainCallbackLog", CRIMainCallbackLog);
	LOG_OFFSET("GetEntityInfoFromHandle", GetEntityInfoFromHandle);
	LOG_OFFSET("GetEntityFromHandle", GetEntityFromHandle);
	LOG_OFFSET("GetItemNameById", ItemManager_GetItemNameById);
	LOG_OFFSET("CalculateLevel", CalculateLevel);
	LOG_OFFSET("PlaySound", ExecuteSound);
	LOG_OFFSET("GetItemIdByName", ItemManager_GetItemIdByName);
	LOG_OFFSET("SetLocalPlayer", SetLocalPlayer);
	LOG_OFFSET("UnlockAchievement", UnlockAchievement);
	LOG_OFFSET("CUserManager::GiveAchievement", UserManager_GiveAchievement);
	LOG_OFFSET("ResetCamera", ResetCamera);
	LOG_OFFSET("ChangePlayer", ChangePlayer);
	LOG_OFFSET("CSceneStateSystem::Insert", SceneStateSystem_Insert);
	LOG_OFFSET("SetSceneEntity", SetSceneEntity);
	LOG_OFFSET("CSceneEntitySystem::RemoveNode", SceneEntitySystem_RemoveNode);
	LOG_OFFSET("SaveFileIO", SaveFileIO);
	LOG_OFFSET("ReadSaveSlots", ReadSaveSlots);
	LOG_OFFSET("ReadSaveData", ReadSaveData);
	LOG_OFFSET("WriteSaveData", WriteSaveData);
	LOG_OFFSET("DeleteSaveData", DeleteSaveData);
	LOG_OFFSET("CSceneEntitySystem::CreateEntity", SceneEntitySystem_CreateEntity);
	LOG_OFFSET("CSceneStateSystem::Set", SceneStateSystem_Set);
	LOG_OFFSET("CSceneStateSystem_FindDefinition", SceneStateSystem_FindDefinition);
	LOG_OFFSET("CSceneStateArray_InsertByName", SceneStateArray_InsertByName);
	LOG_OFFSET("CSceneStateSystem_NotifyChanged", SceneStateSystem_NotifyChanged);
	LOG_OFFSET("CWetObjectManager::SetWet", WetObjectManager_SetWet);
	LOG_OFFSET("CWetObjectManager::SetDry", WetObjectManager_SetDry);
	LOG_OFFSET("CWetObjectManager::AddLocalEntity", WetObjectManager_AddLocalEntity);
	LOG_OFFSET("CreateTexture", CreateTexture);
	LOG_OFFSET("CTextureResourceManager::FindResource", TextureResourceManager_FindResource);
	LOG_OFFSET("CCameraGame::SetLookAt", CameraGame_SetLookAt);
	LOG_OFFSET("CCameraGame::SetLookAtNoDistance", CameraGame_SetLookAtNoDistance);
	LOG_OFFSET("DestroyBuddy", DestroyBuddy);
	LOG_OFFSET("FNV1Hash", FNV1Hash);
	LOG_OFFSET("HashStringCRC32", HashStringCRC32);
	LOG_OFFSET("GetConstructionInfo", GetConstructionInfo);
	LOG_OFFSET("CObjReadSystem::RequestStartAsscessory", ObjReadSystem_RequestStartAsscessory);
	LOG_OFFSET("CObjReadSystem::RequestStartFilter", ObjReadSystem_RequestStartFilter);
	LOG_OFFSET("CObjReadSystem::RequestStart", ObjReadSystem_RequestStart);
	LOG_OFFSET("CObjReadSystem_ReleaseRequest", ObjReadSystem_ReleaseRequest);
	LOG_OFFSET("ExCollision::GetOBBMax", ExCollision_GetOBBMax);
	LOG_OFFSET("ExCollision_ResetOrigin", ExCollision_ResetOrigin);
	LOG_OFFSET("HashLowercaseStringNoLenCRC32", HashLowercaseStringNoLenCRC32);
	LOG_OFFSET("DialogUI::System::GetText", DialogUI_System_GetText);
	LOG_OFFSET("DialogUI::System::CreateBlackBanner", DialogUI_System_CreateBlackBanner);
	LOG_OFFSET("Pl0000::SetFlightUnit", Pl0000_SetFlightUnit);
	LOG_OFFSET("Pl0000_TurnTowardHeading", Pl0000_TurnTowardHeading);
	LOG_OFFSET("CCameraInstance_SetZNear", CameraInstance_SetZNear);

	CCONSOLE_DEBUG_LOG(ImVec4(0.0f, 0.5f, 0.8f, 1.0f), "[Flag Pointers]");
	LOG_OFFSET("DebugFlags", g_pDebugFlags);
	LOG_OFFSET("CameraFlags", g_pCameraFlags);
	LOG_OFFSET("GameFlags", g_pGameFlags);
	LOG_OFFSET("FramerateCap", g_pAntiFramerateCap_Test4);

	CCONSOLE_DEBUG_LOG(ImVec4(0.0f, 0.5f, 0.8f, 1.0f), "[Structure Pointers]");
	LOG_OFFSET("CEntityList", g_pEntityList);
	LOG_OFFSET("LocalPlayerHandle", g_pLocalPlayerHandle);
	LOG_OFFSET("EmilHandle", g_pEmilHandle);
	LOG_OFFSET("CYorhaManager", g_pYorhaManager);
	LOG_OFFSET("CNPCManager", g_pNPCManager);
	LOG_OFFSET("CEnemyManager", g_pEnemyManager);
	LOG_OFFSET("CUserManager", g_pUserManager);
	LOG_OFFSET("CItemManager (database)", g_pItemManager);
	LOG_OFFSET("CItemManager (g_ITEM_MANAGER)", g_pGameItemManager);
	LOG_OFFSET("UiFileSlots", g_pUiFileSlots);
	LOG_OFFSET("DialogUISystem", g_pDialogUISystem);
	LOG_OFFSET("CWetObjectManager", g_pWetObjectManager);
	LOG_OFFSET("CCollisionDataObjectManager", g_pCollisionDataObjectManager);
	LOG_OFFSET("CTextureResourceManager", g_pTextureResourceManager);
	LOG_OFFSET("CSceneEntitySystem", g_pSceneEntitySystem);
	LOG_OFFSET("CObjReadSystem", g_pObjReadSystem);
	LOG_OFFSET("ConstructionInfoList", g_pConstructionInfoList);
	CCONSOLE_DEBUG_LOG(ImVec4(0.0f, 0.5f, 0.8f, 1.0f), "ConstructionInfoList rows: %u", g_uConstructionInfoRows);
	LOG_OFFSET("CSceneStateSystem", g_pSceneStateSystem);
	LOG_OFFSET("SceneStateSystem_Scenes", g_pSceneStateSystem_Scenes);
	LOG_OFFSET("COtManager", g_pOtManager);
	LOG_OFFSET("CModelManager", g_pModelManager);
	LOG_OFFSET("CModelAnalyzer", g_pModelAnalyzer);
	//LOG_OFFSET("MRubyVM", g_pRubyInstances);
	LOG_OFFSET("CGameCamera", g_pCamera);
	LOG_OFFSET("CNetworkDevice", g_pNetworkDevice);
	LOG_OFFSET("CMemoryDevice", g_pMemoryDevice);
	LOG_OFFSET("DataDirectory", g_szDataDirectoryPath);
	LOG_OFFSET("CpkMount", CpkMount);
	LOG_OFFSET("CpkEntries", g_pCpkEntries);
	LOG_OFFSET("Money", g_piMoney);
	LOG_OFFSET("Experience", g_piExperience);
	LOG_OFFSET("IDirectInput8A", g_pDirectInput8);
	LOG_OFFSET("CKeyboard", g_pKeyboard);
	LOG_OFFSET("CMouse", g_pMouse);
	LOG_OFFSET("CGraphics", g_pGraphics);
	LOG_OFFSET("CLightManager", g_pLightManager);
	LOG_OFFSET("ViewMatrix", g_pViewMatrix);
	LOG_OFFSET("DataDirPath", g_szDataDirectoryPath);

	CCONSOLE_DEBUG_LOG(ImVec4(0.0f, 0.5f, 0.8f, 1.0f), "Data Directory Path: %s", g_szDataDirectoryPath);
}

/*
*  ALL OLD SIGS FOR DENUVO PACKED STEAM VERSION
*
*	48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 41 56 48 83 EC 40 F7 05 ? ? ? ? ? ? ? ? - CModelShaderModule::Draw
*	[E9 ? ? ? ? 48 89 5C 24 ? 57 48 83 EC 20 49 8B D0 + 1] - COtManager::DrawModel
*	48 83 EC 08 48 8B 05 ? ? ? ? - COtManager::GetGraphicCommand
*	[E8 ? ? ? ? 83 FF 58 + 1] - COtManager::RegisterGraphicCommand
*/
void FindDenuvoSteamOffsets(void)
{
	CRIMainCallbackLog = (CRILogCallbackFn*)FindPatternPtr(NULL, "48 8B 35 ? ? ? ? 48 8B 1D", 10);

	CalculateLevel = (CalculateLevelFn)FindPattern(NULL, "44 8B 91 ? ? ? ? 45 33 C9 41 8B C1 45 85 D2 7E");
	GetEntityFromHandle = (GetEntityFromHandleFn)FindPattern(NULL, "40 53 48 83 EC ? 8B 11 85 D2 74"); // there is like 50 identical functions kek
	ItemManager_GetItemNameById = (CItemManager_GetItemNameByIdFn)FindPatternPtr(NULL, "E8 ? ? ? ? 33 FF 8B D7", 1);
	ItemManager_GetItemIdByName = (CItemManager_GetItemIdByNameFn)FindPatternPtr(NULL, "E8 ? ? ? ? 85 C0 78 11", 1);
	ItemManager_AddItem = (CItemManager_AddItemFn)0x1405DC410; // TODO: Sig but low priority!
	ItemManager_UseItem = (CItemManager_UseItemFn)0x1405DC5A0; // TODO: Sig but low priority!
	SetLocalPlayer = (SetLocalPlayerFn)FindPatternPtr(NULL, "E8 ? ? ? ? C6 43 48 00 81 7B", 1);
	UnlockAchievement = (UnlockAchievementFn)FindPatternPtr(NULL, "E8 ? ? ? ? 8B D8 83 3D", 1);
	ResetCamera = (CCameraGame::ResetCameraFn)FindPatternPtr(NULL, "E8 ? ? ? ? 41 0F 28 07", 1);
	ChangePlayer = (ChangePlayerFn)FindPattern(NULL, "40 53 48 83 EC 20 8B 05 ? ? ? ? 48 8B D9 48 8D 4C 24 ? 89 44 24 30 E8 ? ? ? ? 48 85 C0 74 31");
	SceneStateSystem_Insert = (CSceneStateNames::InsertFn)FindPatternPtr(NULL, "E8 ? ? ? ? 83 CE 01", 1);
	SetSceneEntity = (SetSceneEntityFn)FindPatternPtr(NULL, "E8 ? ? ? ? 8B 87 ? ? ? ? 41 3B C4", 1);

	SaveFileIO = (CSaveDataDevice::SaveFileIOFn)FindPattern(NULL, "8B 51 30 FF CA");
	ReadSaveSlots = (CSaveDataDevice::ReadSaveSlotsFn)FindPatternPtr(NULL, "E8 ? ? ? ? 48 8B AE ? ? ? ? 48 85 ED 74 38", 1);
	ReadSaveData = (CSaveDataDevice::ReadSaveDataFn)FindPatternPtr(NULL, "E8 ? ? ? ? EB 05 E8 ? ? ? ? 48 8B AE", 1);
	WriteSaveData = (CSaveDataDevice::WriteSaveDataFn)FindPatternPtr(NULL, "E8 ? ? ? ? EB 0C E8 ? ? ? ? EB 05 E8 ? ? ? ? 48 8B AE", 1);
	DeleteSaveData = (CSaveDataDevice::DeleteSaveDataFn)FindPattern(NULL, "40 53 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 84 24 ? ? ? ? 48 8B D9 8B 49 34");

	SceneEntitySystem_CreateEntity = (CSceneEntitySystem::CreateEntityFn)0x1404F9AA0; // TODO: Sig but low priority!
	SceneStateSystem_Set = (CSceneStateSystem_SetFn)0x14001EC80; // TODO: Sig but low priority!

	WetObjectManager_SetWet = (CWetObjManager::SetWetFn)FindPatternPtr(NULL, "E8 ? ? ? ? 4C 8D 05 ? ? ? ? 41 FF C6", 1);
	WetObjectManager_SetDry = (CWetObjManager::SetDryFn)FindPattern(NULL, "48 85 D2 0F 84 ? ? ? ? 53 48 83 EC 20 83 3D ? ? ? ? ? 48 8B DA 0F 84 ? ? ? ? 48 89 74 24 ? 48 8D 35 ? ? ? ? 48 89 7C 24 ? 48 8B CE FF 15 ? ? ? ? 4C 8B 1D ? ? ? ? 44 8B 15 ? ? ? ? 45 33 C9 48 8D 0D ? ? ? ? 48 8D 3D ? ? ? ? 44 8B 01 45 85 C0 74 34 41 8B C0 C1 F8 08 0F B7 D0 41 3B D2 73 26 48 03 D2 41 8B 04 D3 41 33 C0 A9 ? ? ? ? 75 15 49 8B 44 D3 ? 48 85 C0 74 0B F6 40 2C 03 75 05 48 3B C3 74 0E 48 83 C1 04 41 FF C1 48 3B CF 7C B8 EB 1B");
	WetObjectManager_AddLocalEntity = (CWetObjManager::AddLocalEntityFn)FindPatternPtr(NULL, "E8 ? ? ? ? 44 89 AF ? ? ? ? 48 C7 87", 1);
	
	// TODO: Sig low priority!
	ExecuteSound = (PlaySoundFn)0x14081FB90;
	
	CameraGame_SetLookAt = (CCameraGame::SetLookAtFn)FindPatternPtr(NULL, "E8 ? ? ? ? 83 7F 10 01", 1);
	DestroyBuddy = (DestroyBuddyFn)FindPattern(NULL, "40 53 48 83 EC 30 48 C7 44 24 ? ? ? ? ? 48 8B D9 48 8B 01");
	FNV1Hash = (FNV1HashFn)FindPatternPtr(NULL, "E8 ? ? ? ? 85 C0 74 A3", 1);
	HashStringCRC32 = (HashStringCRC32Fn)FindPattern(NULL, "40 57 83 C8 FF");
	GetConstructionInfo = (GetConstructorFn)FindPattern(NULL, "33 D2 44 8B C9");
	ExCollision_GetOBBMax = (ExCollision_GetOBBMaxFn)FindPattern(NULL, "48 8B C4 48 89 68 18 56");

	QueryHeap = (QueryHeapFn)FindPattern(NULL, "48 83 EC 28 4C 8B D9 C7 41");
	// TODO: Sig low priority!
	CreateWork = 0; 
	FindObjectWork = (CObjReadSystem_FindObjectWorkFn)FindPattern(NULL, "40 57 48 83 EC 20 8B F9");
	PreloadFile = (PreloadFileFn)FindPatternPtr(NULL, "E8 ? ? ? ? 8D 5F 01", 1);
	RequestEnd = (ObjReadSystem_RequestEndFn)FindPatternPtr(NULL, "E8 ? ? ? ? 41 8D 46 FF", 1);
	PreloadModel = (ObjReadSystem_PreloadModelFn)FindPatternPtr(NULL, "E8 ? ? ? ? 33 FF 45 33 C0", 1);
	GetGraphicCommand = (COtManager_GetGraphicCommandFn)FindPattern(NULL, "48 83 EC 08 48 8B 05");

	CreateTexture = (CreateTextureFn)FindPattern(NULL, "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8B 02 48 8B CA 49 8B F0 48 8B FA FF 50 08 48 83 3D 95");
	TextureResourceManager_FindResource = (CTextureResourceManager_FindResourceFn)0x140936F60; // TODO: Sig but low priority!

	CpkMount = (CpkMountFn)FindPattern(NULL, "48 83 EC 28 44 8B C9");

	g_pEntityList = (CEntityList*)FindPatternPtr(NULL, "4C 8B 4A 10 48 8D 15 ? ? ? ? 4D 8D 89", 7);
	g_pLocalPlayerHandle = (EntityHandle_t*)FindPatternPtr(NULL, "45 33 F6 4C 8D 25 ? ? ? ? 4C 8D 05", 6);
	g_pEmilHandle = (EntityHandle_t*)FindPatternPtr(NULL, "8B 15 ? ? ? ? 85 D2 74 59", 2);
	g_piMoney = (int*)FindPatternPtr(NULL, "48 8D 3D ? ? ? ? 48 8D 8D", 3);
	g_piExperience = (int*)FindPatternPtr(NULL, "8B 15 ? ? ? ? 75 06", 2);
	g_pItemManager = (CItemManager*)0x14133B510; // TODO: Sig but low priority!
	g_pYorhaManager = *(CYorhaManager**)FindPatternPtr(NULL, "48 8B D1 48 8B 0D ? ? ? ? 48 8B 01", 6);
	g_pNPCManager = *(CNPCManager**)FindPatternPtr(NULL, "75 B9 48 8B 0D", 5);
	g_pEnemyManager = *(CEmBaseManager**)FindPatternPtr(NULL, "4C 89 A3 ? ? ? ? 48 8B 0D", 10);
	g_pUserManager = *(CUserManager**)FindPatternPtr(NULL, "74 1D 48 8B 0D ? ? ? ? 48 85 C9", 5);
	g_pWetObjectManager = (CWetObjManager*)(ReadPtr(FindPatternPtr(NULL, "E8 ? ? ? ? 44 89 AF ? ? ? ? 48 C7 87", 1) + 0x1B, 3));
	g_pCollisionDataObjectManager = *(CCollisionDataObjectManager**)FindPatternPtr(NULL, "0F 45 C7 48 8B 3D", 6);
	g_pTextureResourceManager = *(CTextureResourceManager**)FindPatternPtr(NULL, "48 8B D1 48 8D 0D ? ? ? ? 4C 89 44 24", 6);
	g_pModelAnalyzer = (CModelAnalyzer*)FindPatternPtr(NULL, "48 89 1D ? ? ? ? 48 89 44 24 ? 48 8D 05", 15);
	g_pCamera = (CCameraGame*)FindPatternPtr(NULL, "4C 8D 05 ? ? ? ? 4C 89 D1", 3);
	g_pSceneStateSystem = (CSceneStateSystem*)FindPatternPtr(NULL, "48 8D 0D ? ? ? ? E8 ? ? ? ? 90 EB 52", 3);
	g_pSceneEntitySystem = (CSceneEntitySystem*)FindPatternPtr(NULL, "48 8D 35 ? ? ? ? 4C 8D 35 ? ? ? ? 48 8D 54 24", 3);
	g_pMemoryDevice = (CMemoryDevice*)FindPatternPtr(NULL, "48 8B DA 48 8D 0D ? ? ? ? E8", 6);
	g_pDecreaseHealth[NOP_DAMAGE_ENEMY] = (byte*)FindPattern(NULL, "29 BB ? ? ? ? 8B 83 ? ? ? ? 41 0F 48 C5");
	g_pDecreaseHealth[NOP_DAMAGE_WORLD] = (byte*)FindPattern(NULL, "29 BB ? ? ? ? 8B 83 ? ? ? ? BD ? ? ? ? 0F 48 C5");

	g_szDataDirectoryPath = (CHAR*)FindPatternPtr(NULL, "E8 ? ? ? ? 48 8D 3D ? ? ? ? 48 8B CF", 8);
	g_pDirectInput8 = *(IDirectInput8A**)FindPatternPtr(NULL, "48 8B 0D ? ? ? ? 48 85 C9 74 06 48 8B 01 FF 50 10 48 89 35 ? ? ? ? 48 89 35 ? ? ? ? 48 89 35", 3);
	g_pKeyboard = (Keyboard_t*)FindPatternPtr(NULL, "48 8B 0D ? ? ? ? 48 85 C9 74 06 48 8B 01 FF 50 10 48 89 35 ? ? ? ? 48 89 35 ? ? ? ? 89", 3);
	g_pMouse = (Mouse_t*)FindPatternPtr(NULL, "48 8D 0D ? ? ? ? 44 8B C3 E8", 3);
	g_pGraphics = *(CGraphics**)FindPatternPtr(NULL, "48 8D 05 ? ? ? ? 48 83 C4 ? C3 CC CC CC CC CC CC CC CC 48 89 4C 24 ? 57", 3);
	g_pOtManager = (COtManager*)FindPatternPtr(NULL, "4C 89 78 D8 4C 8D 2D", 7);
	g_pModelManager = (CModelManager*)FindPatternPtr(NULL, "48 8B C7 48 89 05", 6);
	g_pViewMatrix = (VMatrix*)FindPatternPtr(NULL, "0F 29 02 0F 28 2D", 6);
	//g_pSwapChain = *(IDXGISwapChain**)((*(byte**)FindPatternPtr64(NULL, "48 89 35 ? ? ? ? 48 85 C9 74 ? 39 35 ? ? ? ? 74 ? 48 8B 01 BA ? ? ? ? FF 10 48 8B 0D ? ? ? ? 48 85 C9 74 ? 39 35 ? ? ? ? 74 ? 48 8B 01 BA ? ? ? ? FF 10 48 8B 0D ? ? ? ? 48 89 35 ? ? ? ? C7 05 ? ? ? ? ? ? ? ? 48 89 35 ? ? ? ? 48 85 C9 74 ? 39 35 ? ? ? ? 74 ? 48 8B 01 BA ? ? ? ? FF 10 48 8B 0D ? ? ? ? 48 85 C9 74 ? 39 35 ? ? ? ? 74 ? 48 8B 01 BA ? ? ? ? FF 10 48 8B 0D ? ? ? ? 48 89 35 ? ? ? ? C7 05 D8", 3)) + 0xE0);
	// Without CGraphics there's no swapchain to hook, so stop here; Setup() bails out on the nulls
	if (!g_pGraphics || !g_pGraphics->m_Display.m_pWindowedSwapChain)
		return;

	g_pSwapChain = g_pGraphics->m_Display.m_pWindowedSwapChain->m_pSwapChain;
	g_pGraphicDevice = g_pGraphics->m_Display.m_pGraphicDevice;

	if (!IsWindows10OrGreater()) // for some reason it causes a crash on windows 7
		g_pSecondarySwapChain = (IDXGISwapChain*)(*(byte**)((*(byte**)((*(byte**)((*(byte**)((byte*)g_pGraphics + 0x1B8)) + 0x140))) + 0x10)));// I have no idea what this swapchain is for, but it points to the right place

	g_pAntiVSync = (byte*)FindPattern(NULL, "0F 94 D2 45 31 C0 FF 50 40"); //unused
	g_pAntiFramerateCap_Sleep = (byte*)FindPattern(NULL, "8B CA FF 15 ? ? ? ? 48 8D 4C 24") + 2;
	g_pAntiFramerateCap_Spinlock = g_pAntiFramerateCap_Sleep + 0x48;
	g_pAntiFramerateCap_Test4 = (byte*)FindPattern(NULL, "F6 05 ? ? ? ? ? 0F 29 74 24");
	g_hWnd = *(HWND*)FindPatternPtr(NULL, "48 89 05 ? ? ? ? 48 85 C0 0F 84 ? ? ? ? 0F 57 C0", 3);

	for (const MrubyImpl* pVM = *(MrubyImpl**)FindPatternPtr(NULL, "48 89 0D ? ? ? ? 48 89 81", 3); pVM; pVM = pVM->m_pNext)
		g_pRubyInstances.emplace_back((MrubyImpl*)((byte*)pVM - 0x20));
}

// Reads a game global through the address FindPatternPtr resolved. FindPatternPtr returns 0
// when the signature misses (an unknown build), and dereferencing that would crash here
// before Setup() gets to its "unsupported version" check.
template <typename T>
static T ReadGamePtr(ULONG_PTR uAddress)
{
	return uAddress ? *(T*)uAddress : T{};
}

// FindPattern returns 0 when the signature misses and -1 when it couldn't read the module;
// either way there is nothing to use, and the callers check for NULL.
static ULONG_PTR FindPatternOrNull(const char* szPattern)
{
	ULONG_PTR uAddress = FindPattern(NULL, szPattern);

	return (uAddress == (ULONG_PTR)-1) ? 0 : uAddress;
}

// The target of the rel32 at uOffset in a match, resolved the way FindPatternPtr does it, for
// signatures that hold more than one
static ULONG_PTR ResolveRel32(ULONG_PTR uMatch, UINT uOffset)
{
	return uMatch + uOffset + sizeof(LONG) + *(LONG*)(uMatch + uOffset);
}

void FindSteamOffsets(void)
{
	// CriMainCallbackLog (0x14176D2D0), CRI's main error callback slot, for Show Game Errors:
	// mov rbx, CriMainCallbackLog / test rsi, rsi in CriLogInternal (0x140098494, the same address
	// in v1.02). Unique in both builds. This used to be overwritten with CRILogCallbackConsole
	// straight away, so the callback was never installed.
	CRIMainCallbackLog = (CRILogCallbackFn*)FindPatternPtr(NULL, "48 8B 1D ? ? ? ? 48 85 F6", 3);

	CalculateLevel = (CalculateLevelFn)FindPatternPtr(NULL, "E8 ? ? ? ? 45 33 FF 8B 48 10", 1);
	GetConstructionInfo = (GetConstructorFn)FindPattern(NULL, "33 D2 48 8D 05 ? ? ? ? 44 8B C2");

	// The table GetConstructionInfo (0x14048F760, v1.02 0x14048F660) searches, and how many of its
	// rows the whole-id pass covers: lea rax, g_ConstructionInfoList at +2, then add rax, 28h /
	// cmp r8d, <rows> at +0x1C. Both builds have 1135 such rows of 0x28 bytes, at 0x140F456A0
	// (v1.02 0x140F45690). The ten rows after them are its patterns for whole id ranges (id &
	// 0xFFF00, id & 0xFF000, seven per-category rows and a catch-all), not objects of their own.
	if (GetConstructionInfo && (ULONG_PTR)GetConstructionInfo != (ULONG_PTR)-1 &&
		!memcmp((const BYTE*)GetConstructionInfo + 0x1C, "\x48\x83\xC0\x28\x41\x81\xF8", 7))
	{
		g_pConstructionInfoList = (CConstructionInfo<void>*)ResolveRel32((ULONG_PTR)GetConstructionInfo, 5);
		g_uConstructionInfoRows = *(const uint32_t*)((const BYTE*)GetConstructionInfo + 0x23);
	}

	CreateWork = (CreateWorkFn)FindPattern(NULL, "48 89 5C 24 08 57 48 83 EC 20 8B F9 E8 ? ? ? ? 48 8B D8 48 85 C0 0F");
	FindObjectWork = (CObjReadSystem_FindObjectWorkFn)FindPattern(NULL, "48 89 5C 24 08 57 48 83 EC 20 8B F9 48 8B 0D");

	GetEntityInfoFromHandle = (GetEntityInfoFromHandleFn)FindPattern(NULL, "8B 11 85 D2 74 ?? 8B C2"); // gets CEntityInfo for everything
	GetEntityFromHandle = (GetEntityFromHandleFn)FindPatternPtr(NULL, "E8 ? ? ? ? 48 85 C0 75 5C", 1); // for enemies
	GetEntityFromHandle2 = (GetEntityFromHandleFn)FindPatternPtr(NULL, "E8 ? ? ? ? 8B EE 48 85 C0", 1); // for localplayer, cam entity etc...
	ItemManager_GetItemNameById = (CItemManager_GetItemNameByIdFn)FindPatternPtr(NULL, "E8 ? ? ? ? 48 8D 15 ? ? ? ? EB 26", 1);
	ItemManager_GetItemIdByName = (CItemManager_GetItemIdByNameFn)FindPatternPtr(NULL, "E8 ? ? ? ? 44 8B C3 8B D0", 1);
	ItemManager_AddItem = (CItemManager_AddItemFn)FindPattern(NULL, "48 8B C4 57 41 54 41 55 41 56 41 57 48 81 EC ? ? ? ? 48 C7 44 24 ? ? ? ? ? 48 89 58 ? 48 89 68 ? 48 89 70 ? 45 8B F0"); // [E8 ? ? ? ? EB 10 33 D2 + 1]
	ItemManager_UseItem = (CItemManager_UseItemFn)FindPattern(NULL, "40 53 48 83 EC ? 48 8B D9 48 8D 0D ? ? ? ? E8 ? ? ? ? F3 0F 10 15"); //[E8 ? ? ? ? C7 45 ? ? ? ? ? 48 8D 75 D0 + 1] 
	//CItemManager_UseItemByIdFn 40 53 55 57 48 83 EC ? 0F 29 74 24 ? | [E8 ? ? ? ? 48 63 83 ? ? ? ? 48 8B CB + 1]
	ResetCamera = (CCameraGame::ResetCameraFn)FindPatternPtr(NULL, "E8 ? ? ? ? 41 81 A6", 1);
	ChangePlayer = (ChangePlayerFn)FindPattern(NULL, "40 53 48 83 EC 30 48 8B D9 E8 ? ? ? ? 81 25");
	SetLocalPlayer = (SetLocalPlayerFn)FindPattern(NULL, "40 53 56 57 48 83 EC 70 48 8B F1");
	EntityInfo_Delete = (CEntityInfo::DeleteFn)FindPattern(NULL, "40 53 48 83 EC 20 8B 41 2C");
	SceneStateSystem_Insert = (CSceneStateNames::InsertFn)FindPattern(NULL, "40 56 57 41 54 41 56 41 57 48 83 EC ? 48 C7 44 24 ? ? ? ? ? 48 89 5C 24 ? 48 89 6C 24 ? 4D 8B F1"); // [E8 ? ? ? ? 48 89 45 E0 + 1]
	SceneEntitySystem_FindByObjectId = (CSceneEntitySystem::FindByObjectIdFn)FindPattern(NULL, "48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 57 48 83 EC 20 83 79 48 00 48");
	// CSceneEntitySystem::RemoveNode (0x140150C60, v1.02 0x140150B70), through its test of the
	// scene-state tree's root (cmp byte ptr [rax+19h], 0)
	SceneEntitySystem_RemoveNode = (CSceneEntitySystem::RemoveNodeFn)FindPatternOrNull("40 57 48 83 EC 30 48 C7 44 24 ? ? ? ? ? 48 89 5C 24 ? 48 89 74 24 ? 48 8B F9 48 8B 19 48 85 DB 74 ? 48 8B 05 ? ? ? ? 80 78 19 00 75");
	SceneEntitySystem_CreateEntity = (CSceneEntitySystem::CreateEntityFn)FindPattern(NULL, "48 8B C4 56 57 41 56 48 83 EC ? 48 C7 40 B8 ? ? ? ? 48 89 58 ? 48 89 68 ? 4C 8B F2 48 8B F1");
	//(CSceneEntitySystem::UpdateFn)FindPattern(NULL, "40 56 57 41 56 48 83 EC 30 48 C7 44 24 20 FE FF FF FF 48 89 5C 24 58 48 89 6C 24 60 4C 8B F1");
	SceneStateSystem_Set = (CSceneStateSystem_SetFn)FindPattern(NULL, "48 89 5C 24 ? 57 48 83 EC 20 48 8B 02 48 8B FA"); // [E8 ? ? ? ? EB 38 48 89 5C 24 ? + 1]
	// this is the internal function both work but using the internal func probably has consecquences I am not aware of
	//SceneStateSystem_Set = (CSceneStateSystem_SetFn)FindPattern(NULL, "48 8B C4 57 48 81 EC 60 04 00 00 48 C7 44 24 ? FE FF FF FF 48 89 58 ? 48 89 70"); // [E8 ? ? ? ? 84 C0 74 19 B2 01 + 1]
	// CSceneStateSystem::SetInternal (0x140356800; v1.02 0x140356700) finds the state's definition,
	// clears its group and inserts it into m_States. Features::AddSceneState does the same without
	// clearing the group, through SetInternal's calls to CSceneStateSystem_FindDefinition
	// (0x140326D30) and CSceneStateArray_InsertByName (0x14034C170), then sends the change notice
	// Set does (CSceneStateSystem_NotifyChanged, 0x14035CC40). Each call site is unique in both builds.
	SceneStateSystem_FindDefinition = (CSceneStateSystem_FindDefinitionFn)FindPatternPtr(NULL, "48 8D 51 60 48 8B C8 E8 ? ? ? ? 48 8B D8 48 85 C0 0F 84", 8);
	SceneStateArray_InsertByName = (CSceneStateArray_InsertByNameFn)FindPatternPtr(NULL, "48 8B D3 48 8D 8F 90 00 00 00 E8 ? ? ? ? 48 8B 43 08", 11);
	SceneStateSystem_NotifyChanged = (CSceneStateSystem_NotifyChangedFn)FindPatternPtr(NULL, "84 C0 75 0B 48 8B 5C 24 38 48 83 C4 20 5F C3 B2 01 48 8B CB E8", 21);

	ExecuteSound = (PlaySoundFn)FindPattern(NULL, "40 56 57 41 56 48 83 EC 30 48 C7 44 24 20 FE FF FF FF 48 89 5C 24 58 48 89 6C 24 60 48 8B E9");

	FNV1Hash = (FNV1HashFn)FindPattern(NULL, "40 56 48 81 EC ? ? ? ? 48 8B F1 48 85 C9"); // [E8 ? ? ? ? 8B D3 C1 FA 08 + 1]
	HashStringCRC32 = (HashStringCRC32Fn)FindPattern(NULL, "48 85 C9 0F 85 ? ? ? ? 33 C0 C3 CC CC CC CC 48 89 5C 24");

	CreateUIFromId = (CreateUIFromIdFn)FindPattern(NULL, "40 53 48 83 EC 20 44 8B 0D");

	// CUserManager::GiveAchievement (0x1409C7250; v1.02 0x1409C6910), the head up to its
	// "jz/jns" user-index check, which is where UserManager_AdvanceAchievement (0x1409C3310),
	// otherwise identical, has "jz/js".
	UserManager_GiveAchievement = (CUserManager_GiveAchievementFn)FindPatternOrNull("40 57 48 83 EC 30 48 C7 44 24 ? ? ? ? ? 48 89 5C 24 ? 8B D9 48 8D 3D ? ? ? ? 48 89 7C 24 ? 83 3D ? ? ? ? ? 74 ? 48 8B CF FF 15 ? ? ? ? 90 8B 05 ? ? ? ? 85 C0 74 ? 79");

	// Create UI Element used v1.02 RVAs, which land mid-function on build 26534214. Both of these
	// are unique in v1.02 and build 26534214 and resolve to the same functions in each.
	// UnitDataList_GetCategoryText (0x1407DC940) looks a game text up the way the button does:
	// lea rcx, [rsp+20h] / call HashLowercaseStringNoLenCRC32 (0x14014C5B0) / mov edx, eax /
	// lea rcx, g_UiFileSlots (0x141420B10) / mov r8d, ebx / call DialogUI::System::GetText (0x1409BD770).
	if (ULONG_PTR uGetText = FindPatternOrNull("48 8D 4C 24 20 E8 ? ? ? ? 8B D0 48 8D 0D ? ? ? ? 44 8B C3 E8"))
	{
		HashLowercaseStringNoLenCRC32 = (HashLowercaseStringNoLenCRC32Fn)ResolveRel32(uGetText, 6);
		g_pUiFileSlots = (void*)ResolveRel32(uGetText, 15);
		DialogUI_System_GetText = (DialogUI_System_GetTextFn)ResolveRel32(uGetText, 23);
	}

	// DialogUI::System::CreateBlackBannerDialog (0x1402F49A0): lea rcx, g_DialogUISystem (0x140F04018)
	// ... call DialogUI::System::CreateBlackBanner (0x1402F5140)
	if (ULONG_PTR uBlackBanner = FindPatternOrNull("48 8D 0D ? ? ? ? 44 89 5C 24 30 44 89 44 24 28 44 8B C2 4C 89 4C 24 20 8B D7 4C 8B CB E8"))
	{
		g_pDialogUISystem = (void*)ResolveRel32(uBlackBanner, 3);
		DialogUI_System_CreateBlackBanner = (DialogUI_System_CreateBlackBannerFn)ResolveRel32(uBlackBanner, 31);
	}

	// Pl0000::SetFlightUnit (0x1404F8400, v1.02 0x1404F8300), for Create Entity's flight unit. Its
	// head through lea rdx, [rcx+15B64h] (&m_hFlightSuit); the old RVA 0x4F8300 is now
	// Pl0000_EnterFighterForm+0x40.
	Pl0000_SetFlightUnit = (Pl0000_SetFlightUnitFn)FindPatternOrNull("48 8B C4 55 57 41 54 41 56 41 57 48 8D A8 ? ? ? ? 48 81 EC ? ? ? ? 48 C7 45 ? FE FF FF FF 48 89 58 10 48 89 70 18 0F 29 70 C8 0F 29 78 B8 48 8B 05 ? ? ? ? 48 33 C4 48 89 85 ? ? ? ? 48 8B F1 48 8D 91 64 5B 01 00");

	// First person. Both are unique in v1.02 and build 26534214 and resolve to the same function in each.
	// Pl0000_TurnTowardHeading (0x1404FE750, v1.02 0x1404FE650), by its head through
	// mov rcx, [rcx+610h] (m_pInfo, for the tick base) / movaps xmm7, xmm1 (the heading).
	Pl0000_TurnTowardHeading = (Pl0000_TurnTowardHeadingFn)FindPatternOrNull("48 89 5C 24 08 57 48 83 EC 60 0F 29 74 24 50 48 8D 3D ? ? ? ? 0F 29 7C 24 40 48 8B D9 48 8B 89 10 06 00 00 0F 28 F9");
	// CCameraInstance_SetZNear (0x14027ADA0, v1.02 0x14027ACA0), the whole function: store the near
	// plane at +0x268 and raise the projection dirty flag at +0x284
	CameraInstance_SetZNear = (CCameraInstance_SetZNearFn)FindPatternOrNull("48 8B 01 F3 0F 11 88 68 02 00 00 C7 80 84 02 00 00 01 00 00 00 C3");

	// ExCollision_ResetOrigin (0x14040D1F0, v1.02 0x14040D0F0), for No Model Collision. The whole
	// function but the tail jmp: GetOrigin, IsZero, IsNaN, copy into m_vOrigin (+0xE0), then
	// lea rcx, [rbx+100h] (m_vPreviousOrigin). Without that last lea it also matches
	// ExCollision_UpdateOrigin (0x1404118A0). Unique in v1.02 and build 26534214.
	ExCollision_ResetOrigin = (ExCollision_ResetOriginFn)FindPatternOrNull("40 53 48 83 EC 30 48 8D 54 24 20 48 8B D9 E8 ? ? ? ? 48 8D 4C 24 20 E8 ? ? ? ? 85 C0 75 1F 48 8D 4C 24 20 E8 ? ? ? ? 85 C0 75 11 48 8D 8B E0 00 00 00 48 8D 54 24 20 E8 ? ? ? ? 48 8D 93 E0 00 00 00 48 8D 8B 00 01 00 00");

	// The checks the game makes before a free spawn, for Create Entity. ModelSetSpawnList_Update
	// (0x14082B190, v1.02 0x14082AD70) spawns its "FreeEnemy" only once both answer 1:
	// mov r8d, edi / lea rcx, g_ObjReadSystem / mov edx, ebx / call CObjReadSystem::RequestStartAsscessory /
	// test eax, eax / jz, the same for CObjReadSystem::RequestStartFilter, then movsxd rax, [rsi+0E60h].
	// Unique in both builds: 0x14084E950 and 0x14084EB30 on 0x1413ECC38 (v1.02 0x14084E530 and
	// 0x14084E710 on 0x1413EC8B8).
	if (ULONG_PTR uFreeSpawn = FindPatternOrNull("44 8B C7 48 8D 0D ? ? ? ? 8B D3 E8 ? ? ? ? 85 C0 0F 84 ? ? ? ? 44 8B C7 48 8D 0D ? ? ? ? 8B D3 E8 ? ? ? ? 85 C0 0F 84 ? ? ? ? 48 63 86"))
	{
		g_pObjReadSystem = (CObjReadSystem*)ResolveRel32(uFreeSpawn, 6);
		ObjReadSystem_RequestStartAsscessory = (CObjReadSystem_IsObjectReadyFn)ResolveRel32(uFreeSpawn, 13);
		ObjReadSystem_RequestStartFilter = (CObjReadSystem_IsObjectReadyFn)ResolveRel32(uFreeSpawn, 38);
	}

	// What the same free spawner calls around them, for Create Entity's loading: it requests the
	// files once, waits for the checks above, spawns, and releases the request once the entity is
	// gone. Both by their heads through the g_ObjNoPreloadIds test, which they share; the register
	// moves and frame offsets tell them apart. Unique in both builds:
	// CObjReadSystem::RequestStart (0x1408511B0, v1.02 0x140850D90)
	ObjReadSystem_RequestStart = (CObjReadSystem_RequestStartFn)FindPatternOrNull("40 57 41 54 41 55 41 56 41 57 48 83 EC 30 48 C7 44 24 20 FE FF FF FF 48 89 5C 24 68 48 89 6C 24 70 48 89 74 24 78 41 8B E8 8B FA 4C 8B E9 48 8D 59 10 48 89 5C 24 60 83 7B 28 00 74 0A 48 8B CB FF 15 ? ? ? ? 90 8B 05 ? ? ? ? 83 F8 FF");
	// CObjReadSystem_ReleaseRequest (0x140850FB0, v1.02 0x140850B90)
	ObjReadSystem_ReleaseRequest = (CObjReadSystem_ReleaseRequestFn)FindPatternOrNull("40 57 41 56 41 57 48 83 EC 30 48 C7 44 24 20 FE FF FF FF 48 89 5C 24 58 48 89 6C 24 60 48 89 74 24 68 45 8B F0 8B FA 48 8B E9 48 8D 59 10 48 89 5C 24 50 83 7B 28 00 74 0A 48 8B CB FF 15 ? ? ? ? 90 8B 05 ? ? ? ? 83 F8 FF");

	// SaveIO Hook has to be reworked as CSaveDataDevice now resides in rbx instead of rcx.
	// Probably the best solution is to rebuild the function and call that from the hook.
	SaveFileIO = (CSaveDataDevice::SaveFileIOFn)FindPattern(NULL, "8B 4B 30 83 E9 01");
	ReadSaveSlots = (CSaveDataDevice::ReadSaveSlotsFn)FindPatternPtr(NULL, "E8 ? ? ? ? 48 8B B5 ? ? ? ? 48 85 F6", 1);
	ReadSaveData = (CSaveDataDevice::ReadSaveDataFn)FindPatternPtr(NULL, "E8 ? ? ? ? EB 08 48 8B CB E8 ? ? ? ? 48 8B B5", 1);
	WriteSaveData = (CSaveDataDevice::WriteSaveDataFn)FindPattern(NULL, "40 53 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 84 24 ? ? ? ? 48 8B D9 8B 49 34");
	DeleteSaveData = (CSaveDataDevice::DeleteSaveDataFn)CSaveDataDevice_DeleteSaveData; // Inlined on new version hmmm this may not work may have to rebuild function

	CreateTask = (CreateTaskFn)FindPattern(NULL, "48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 48 89 7C 24 20 41 56 48 83 EC 30 44 8B 1D");
	JobManager_CreateTask = (CJobManager::CreateTaskFn)FindPattern(NULL, "48 89 5C 24 10 48 89 6C 24 18 56 48 83 EC 20 48 8B 59 18 49");
	// JobManager_ReleaseTask/SuspendTask and this a should be equivalent 
	// typedef DWORD(*SuspendTask_SetGameBitFlagsFn)(DWORD fFlags);
	//JobManager_PauseTask2 = (CJobManager::PauseTaskFn)FindPattern(NULL, "48 89 5C 24 08 48 89 74 24 10 57 48 83 EC 20 48 8B 3D ? ? ? ? 8B");
	JobManager_DestroyTask = (CJobManager::DestroyTaskFn)FindPattern(NULL, "48 89 5C 24 08 57 48 83 EC 20 8B 42 28");
	JobManager_Yield = (CJobManager::YieldFn)FindPattern(NULL, "40 53 48 83 EC 20 48 8B 59 20");
	JobManager_SuspendTask = (CJobManager::SuspendTaskFn)FindPattern(NULL, "48 89 5C 24 08 57 48 83 EC 20 48 8B 59 20");
	YieldToNextTask = (CJobManager_YieldToNextTaskFn)FindPattern(NULL, "40 53 48 83 EC 20 48 8B 05 ? ? ? ? 48 8B 58 20 83 7B 28 04 74 0C 48 8D 0D ? ? ? ? E8 ? ? ? ? 48 8B 4B 40 45 33 C0 C7 43 28 00 00 00 00 41 8D 50 01 FF 15 ? ? ? ? 48 83 C4 20 5B E9");
	SetTaskFunction = (CJobManager_SetTaskFunctionFn)FindPattern(NULL, "48 89 5C 24 08 48 89 74 24 10 57 48 83 EC 20 48 8B 05 ? ? ? ? 48 8B FA 48 8B F1");

	WetObjectManager_SetDry = (CWetObjManager::SetDryFn)FindPattern(NULL, "48 85 D2 0F 84 ? ? ? ? 57 48 83 EC 30 48 C7 44 24 ? ? ? ? ? 48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 48 8B EA 48 8B F9 83 79 28 00 74 61");
	WetObjectManager_SetWet = (CWetObjManager::SetWetFn)FindPatternPtr(NULL, "E8 ? ? ? ? 41 BE ? ? ? ? FF C7", 1);
	WetObjectManager_AddLocalEntity = (CWetObjManager::AddLocalEntityFn)FindPatternPtr(NULL, "E8 ? ? ? ? 44 89 BF ? ? ? ? 48 C7 87 ? ? ? ? ? ? ? ? 4C 89 AF", 1);
	CameraGame_SetLookAt = (CCameraGame::SetLookAtFn)FindPatternPtr(NULL, "E8 ? ? ? ? 33 C9 4C 89 67 18", 1);
	CameraGame_SetLookAtNoDistance = (CCameraGame::SetLookAtFn)FindPattern(NULL, "40 53 48 81 EC 80 00 00 00 0F 29 74 24 70 0F 57");
	DestroyBuddy = (DestroyBuddyFn)FindPattern(NULL, "40 53 48 83 EC 30 48 C7 44 24 ? ? ? ? ? 48 8B D9 48 8B 01");
	ExCollision_GetOBBMax = (ExCollision_GetOBBMaxFn)FindPattern(NULL, "48 89 5C 24 ? 56 48 83 EC ? 48 83 79 08 00");

	// "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8B 44 24 ? 33 F6"
	CreateTexture = (CreateTextureFn)FindPattern(NULL, "48 89 5C 24 08 48 89 74 24 10 57 48 83 EC 20 48 8B F1 49 8B F8");
	TextureResourceManager_FindResource = (CTextureResourceManager_FindResourceFn)FindPattern(NULL, "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 33 FF 8B F1");

	g_pDecreaseHealth[NOP_DAMAGE_ENEMY] = (byte*)FindPattern(NULL, "29 BB ? ? ? ? 8B 83 ? ? ? ? 79 0A");
	// No Fall Damage NOPs the call that applies fall-recovery damage (10% of max HP) in
	// Pl0000_ExecuteState10: mov r8d, 1 / mov rcx, rbx / <max HP * 0.1 into edx> / call
	// Pl0000_TakeDamage (+28). It used to NOP the subtraction inside Pl0000_TakeDamage, which
	// also blocked damage from failed hacks, self-destruct and other player states.
	ULONG_PTR uFallRecoveryDamage = FindPattern(NULL, "41 B8 01 00 00 00 48 8B CB 66 0F 6E C0 0F 5B C0 F3 0F 59 05 ? ? ? ? F3 0F 2C D0 E8 ? ? ? ? FF 83");
	g_pDecreaseHealth[NOP_DAMAGE_WORLD] = (uFallRecoveryDamage && uFallRecoveryDamage != (ULONG_PTR)-1) ? (byte*)uFallRecoveryDamage + 28 : NULL;
	
	g_pDebugFlags = (uint32_t*)FindPatternPtr(NULL, "8B 05 ? ? ? ? 0F BA E0 0E 73 07", 2);
	// "or dword ptr [rip+disp32], imm32": rip-relative from the end of the instruction, which is
	// 4 bytes past the end of the displacement because of the immediate. Without the +4 this was
	// 0x141029B7C, an unreferenced dword, instead of DBG word 0 at 0x141029B80 (bit 31 is the
	// game's debug free camera, which Freecamera now uses).
	{
		ULONG_PTR uCameraFlags = FindPatternPtr(NULL, "81 0D ? ? ? ? ? ? ? ? B9 01 00 00 00 81 0D", 2);
		g_pCameraFlags = uCameraFlags ? (uint32_t*)(uCameraFlags + sizeof(uint32_t)) : NULL;
	}
	g_pGameFlags = (uint32_t*)FindPatternPtr(NULL, "0F B6 05 ? ? ? ? F6 D0", 3);
	g_pGameFlags2 = (uint32_t*)FindPatternPtr(NULL, "85 05 ? ? ? ? 75 ? 39 59 24", 2);

	// Anti-Framerate Cap. Graphics_LimitFrameAndPresent (0x14024A250, v1.02 0x14024A150) skips its
	// Sleep and spin wait only while g_uGameFlags bit 4 is set: test byte ptr [g_uGameFlags], 4 /
	// jnz <present> (0x14024A2B5). Setting that bit from Present did nothing: the limiter has
	// already run by then, and Os_PumpWindows (0x14026E53E) rebuilds the bit from the window's
	// state on every pump. So the jnz is patched into a jmp instead (Setup). Unique in both
	// builds; this points at the jnz.
	if (ULONG_PTR uFramecap = FindPatternOrNull("F6 05 ? ? ? ? 04 0F 85 ? ? ? ? 48 8D 8C 24 ? ? ? ? FF 15"))
		g_pAntiFramerateCap_Test4 = (BYTE*)(uFramecap + 7);

	g_pEntityList = (CEntityList*)FindPatternPtr(NULL, "44 8B 0D ? ? ? ? 44 39 0D ? ? ? ? 75 24", 3);
	g_pLocalPlayerHandle = (EntityHandle_t*)FindPatternPtr(NULL, "48 8D 15 ? ? ? ? 48 8D 4C 24 ? E8 ? ? ? ? 48 8B D0 48 8D 4C 24 ? E8 ? ? ? ? 48 8B C8 E8 ? ? ? ? 48 8B C8 E8 ? ? ? ? 48 8B D0", 3);
	g_pEmilHandle = (EntityHandle_t*)FindPatternPtr(NULL, "0F 84 ? ? ? ? 48 8D 15 ? ? ? ? 48 8D 4D 77", 9);
	g_piMoney = (int32_t*)FindPatternPtr(NULL, "48 83 EC 68 4C 8D 05 ? ? ? ? 48 8D 15", 7);
	g_piExperience = (int32_t*)FindPatternPtr(NULL, "8B 15 ? ? ? ? 48 8D 8F ? ? ? ? E8 ? ? ? ? 8B 48 04", 2);

	g_pJobManager = (CJobManager*)FindPatternPtr(NULL, "48 8D 0D ? ? ? ? E8 ? ? ? ? 85 C0 0F 84 00 01 00 00", 3);
	g_pSceneEntitySystem = (CSceneEntitySystem*)FindPatternPtr(NULL, "48 8D 35 ? ? ? ? 48 89 74 24 ? C7 44 24 ? ? ? ? ? C7 44 24", 3);
	g_pSceneStateSystem = (CSceneStateSystem*)FindPatternPtr(NULL, "48 89 05 ? ? ? ? 48 8D 0D ? ? ? ? 48 8D 05 ? ? ? ? 48 89 05 ? ? ? ? E8", 3);
	// g_pSceneStateSystem_Scenes (0x140F8C4E0; v1.02 0x140F8C220), the scene-state names: the lea rcx
	// of a call to CSceneStateSystem::Insert. The pattern matches 70 such calls in each build, and
	// every one of them loads this table, so the first match does. The features used to look it up
	// each on their own.
	g_pSceneStateSystem_Scenes = (CSceneStateNames*)FindPatternPtr(NULL, "48 8D 0D ? ? ? ? E8 ? ? ? ? 48 89 44 24 ? 48 85 C0 74 ? 8B 50", 3);
	g_pWetObjectManager = (CWetObjManager*)FindPatternPtr(NULL, "48 8B D9 48 8D 0D ? ? ? ? E8 ? ? ? ? 48 8D 8B", 6);
	g_pCollisionDataObjectManager = ReadGamePtr<CCollisionDataObjectManager*>(FindPatternPtr(NULL, "48 8D 0D ? ? ? ? E8 ? ? ? ? 48 83 C4 58 C3", 3));
	g_pTextureResourceManager = (CTextureResourceManager*)FindPatternPtr(NULL, "48 8D 0D ? ? ? ? E8 ? ? ? ? 90 48 8B 4B 10 48 85 C9", 3);
	g_pModelAnalyzer = (CModelAnalyzer*)FindPatternPtr(NULL, "48 8D 4D E7 48 8D 05", 7);
	g_pItemManager = (CItemManager*)FindPatternPtr(NULL, "48 8D 0D ? ? ? ? 8B 94 87 ? ? ? ? E8 ? ? ? ? 85 C0 74 4F", 3);
	// g_ITEM_MANAGER (0x140F56120, v1.02 0x140F56110) from the UseItem script command (0x140338EBB):
	// lea rdx, <item name> / lea rcx, g_ITEM_MANAGER / call CItemManager::UseItem
	g_pGameItemManager = (CItemManager*)FindPatternPtr(NULL, "48 8D 15 ? ? ? ? 48 8D 0D ? ? ? ? E8 ? ? ? ? C7 45 ? 01 00 00 00 48 8D 75", 10);
	g_pYorhaManager = ReadGamePtr<CYorhaManager*>(FindPatternPtr(NULL, "48 89 0D ? ? ? ? 48 83 C4 28 C3 48 85 C0 48 89 05 ? ? ? ? 48 8B C8 0F 95 C0 48 83 C4 28 C3 CC 89 91", 3));
	g_pNPCManager = ReadGamePtr<CNPCManager*>(FindPatternPtr(NULL, "48 89 1D ? ? ? ? 48 85 DB 40 0F 95 C7 8B C7 48 8B 5C 24 ? 48 83 C4 30", 3));

	// g_LightManager (0x14102B0F0, v1.02 0x14102ADB0), for Move Sun: lea rcx, g_LightManager /
	// call CLightManager::Update in MainUpdateExec. Unique in both builds.
	g_pLightManager = (CLightManager*)FindPatternPtr(NULL, "85 C0 75 05 E8 ? ? ? ? 48 8D 0D ? ? ? ? E8 ? ? ? ? BA 01 00 00 00 48 8D 0D", 12);
	g_pEnemyManager = ReadGamePtr<CEmBaseManager*>(FindPatternPtr(NULL, "48 8B 0D ? ? ? ? 48 8B 01 8B D3 FF 90 ? ? ? ? 48 8B 0D ? ? ? ? 48 8B 01 8B D3 FF 90 ? ? ? ? 44 89 BE", 3));
	g_pUserManager = ReadGamePtr<CUserManager*>(FindPatternPtr(NULL, "48 89 1D ? ? ? ? 48 85 DB 74 2A", 3));
	g_pNetworkDevice = ReadGamePtr<CNetworkDevice*>(FindPatternPtr(NULL, "48 8B 3D ? ? ? ? 33 DB 48 8B EA", 3));
	g_pMemoryDevice = (CMemoryDevice*)FindPatternPtr(NULL, "4C 89 64 24 ? 48 8D 0D ? ? ? ? 4C 89 7C 24", 8);

	g_pCamera = (CCameraGame*)FindPatternPtr(NULL, "81 25 ? ? ? ? ? ? ? ? 48 8D 0D ? ? ? ? E8 ? ? ? ? 48 8B 03", 13);
	g_pViewMatrix = (VMatrix*)FindPatternPtr(NULL, "48 8D 4E 10 48 8D 15", 7);
	g_szDataDirectoryPath = (CHAR*)FindPatternPtr(NULL, "48 8D 3D ? ? ? ? 48 8B CF FF 15", 3);

	// HwRofsFst_MountCpk (CPK_LoadFromPath in the IDB) and the mount table it fills, for Load CPK
	CpkMount = (CpkMountFn)FindPattern(NULL, "40 55 57 41 57 48 83 EC 30 33 FF 48 8B EA 44 8B F9 48 85 D2 75");

	// lea r8, g_HwRofsFstMounts / mov [rsp+50h], rbx / mov rax, r8 / lea rdx, <end of the table>
	ULONG_PTR uCpkMountTable = FindPattern(NULL, "4C 8D 05 ? ? ? ? 48 89 5C 24 50 49 8B C0 48 8D 15");

	if (uCpkMountTable && uCpkMountTable != (ULONG_PTR)-1)
	{
		g_pCpkEntries = (CpkEntry*)(uCpkMountTable + 7 + *(LONG*)(uCpkMountTable + 3));
		g_pCpkEntriesEnd = (CpkEntry*)(uCpkMountTable + 22 + *(LONG*)(uCpkMountTable + 18));
	}
	g_pDirectInput8 = ReadGamePtr<IDirectInput8A*>(FindPatternPtr(NULL, "0F 88 ? ? ? ? 48 8B 0D ? ? ? ? 4C 8D 0D", 9));
	g_pKeyboard = (Keyboard_t*)FindPatternPtr(NULL, "48 8B 0D ? ? ? ? 4C 8D 44 24 ? BA", 3);
	// mov eax, 1 / mov [g_Mouse], rcx in CreateDirectInput8Create. The old "B8 ? ? ? ?" head missed on
	// build 26534214, where the preceding mov rcx, [rip+disp] has a B8 byte in its displacement.
	g_pMouse = (Mouse_t*)FindPatternPtr(NULL, "B8 01 00 00 00 48 89 0D ? ? ? ? C7 05", 8);
	g_pOtManager = ReadGamePtr<COtManager*>(FindPatternPtr(NULL, "4C 8B C1 48 8B 0D ? ? ? ? 48 85 C9", 6));
	g_pModelManager = ReadGamePtr<CModelManager*>(FindPatternPtr(NULL, "48 8B F9 48 8B 35", 6));
	g_pGraphics = ReadGamePtr<CGraphics*>(FindPatternPtr(NULL, "48 89 1D ? ? ? ? 48 85 DB 0F 84 ? ? ? ? 49 8B D6", 3));

	// Without CGraphics there's no swapchain to hook (issue #13: crashed here on unknown builds)
	if (g_pGraphics && g_pGraphics->m_Display.m_pWindowedSwapChain)
	{
		g_pSwapChain = g_pGraphics->m_Display.m_pWindowedSwapChain->m_pSwapChain;
		//g_pGraphicDevice = g_pGraphics->m_Display.m_pGraphicDevice;
		g_pGraphicDevice = g_pGraphics->m_pGraphicalDevice;
	}

	g_hWnd = ReadGamePtr<HWND>(FindPatternPtr(NULL, "48 8B 15 ? ? ? ? 48 8B 01 FF 50 68 85 C0 0F 88 ? ? ? ? 85 DB", 3));
}

// Set once Setup() has patched and hooked everything. Unhook() checks it because Setup()
// can return early (unsupported build, no window procedure), and then the patches, hooks
// and ImGui backend that Unhook() tears down were never created.
static volatile BOOL s_bSetupComplete = FALSE;

void Setup(void)
{
#if defined(_DEBUG) || defined(VERBOSE)
	STACK_TIMER(timer);
	Log::AttachConsole(L"2B Hook Debug Console");
#endif
	srand((unsigned int)time(NULL));

	{
		STACK_TIMER(AVX_timer);

		HashStringCRC32 = (HashStringCRC32Fn)FindPatternAVXEx(GetModuleHandleA(NULL),
			"\x48\x85\xC9\x0F\x85????\x33\xC0\xC3\xCC\xCC\xCC\xCC\x48\x89\x5C\x24");
	}

	// add VEH?
	SetUnhandledExceptionFilter(&UnhandledExceptionHandler);
	g_pExceptionHandlers.push_back(&UnhandledExceptionHandlerChild);

	g_CreateCtx.hControlSemaphore = CreateSemaphore(NULL, 0, 1, TEXT("SpawnCtrl"));

	NierVersionInfo* pVersion = QueryNierBinaryVersion();
	SHA256DigestStr SHA256Hash;

	// The hash can be missing if the game binary couldn't be read or hashed
	if (pVersion->m_pHash)
	{
		SHA256_PrintDigest((SHA256Digest*)pVersion->m_pHash, &SHA256Hash);
		LOG("SHA256: %s\n", SHA256Hash.Hash);
	}
	else
	{
		LOG("SHA256: (unavailable, couldn't hash the game binary)\n");
	}

	LOG("Detected %s\n", pVersion->m_szVersion);

#if defined(_DEBUG) || defined(VERBOSE)
	DumpModules();
#endif

	//const char* szModule = "ntoskrnl.exe";
	//const char* szFunction = "IoAllocateMdl";

	//PVOID pfnIoAllocateMdl = GetExportAddressFromDisk(szModule, szFunction);

	//LOG("Found %s!%s = %p\n", szModule, szModule, pfnIoAllocateMdl);

	FindOffsets();

	LogOffsets();

	// The overlay and InitHooks() dereference all of these unconditionally. If a signature
	// didn't match (an unknown or updated game build), stop cleanly instead of crashing
	// later (issue #13).
	const struct { const char* szName; const void* p; } RequiredOffsets[] =
	{
		{ "CGraphics", g_pGraphics },
		{ "SwapChain", g_pSwapChain },
		{ "GraphicDevice", g_pGraphicDevice },
		{ "hWnd", g_hWnd },
		{ "Keyboard", g_pKeyboard },
		{ "Mouse", g_pMouse },
		{ "Camera", g_pCamera },
		{ "ModelAnalyzer", g_pModelAnalyzer },
		{ "SaveFileIO", (const void*)SaveFileIO },
	};
	BOOL bMissingOffset = FALSE;

	for (const auto& Offset : RequiredOffsets)
	{
		if (!Offset.p)
		{
			LERROR("Required game offset %s wasn't found\n", Offset.szName);
			bMissingOffset = TRUE;
		}
	}

	if (bMissingOffset)
	{
		LERROR("2B Hook Failed Initialization!\nRequired game offsets weren't found, so this game version isn't supported yet. "
			"Please report it with the SHA256 above.\n");
		return;
	}

	ExposeHiddenXInputFunctions();

	QueryProcessHeaps(&g_phHeaps, NULL);

	// Keep checking this result - a failed ImGui backend init is otherwise silent, and
	// everything downstream still runs as if the overlay were alive.
	HRESULT hrInitD3D11 = InitD3D11();

	// FAILED(hrInitD3D11)
	if (hrInitD3D11 != S_OK)
		LERROR("InitD3D11 did not fully succeed (%08X) - ImGui cannot render!\n", hrInitD3D11);

	CAdapterOutputPair AdapterOutput;

	g_pGraphicDevice->GetCurrentAdapterOutput(AdapterOutput);

	g_pMenu = new CMenu(AdapterOutput);

	//g_pRenderer->Initialize(g_pDevice, g_pDeviceContext);

	// This early return happens before InitHooks(), so a failure here means no Present
	// hook and therefore no overlay at all.
	SetLastError(ERROR_SUCCESS);

	oWndProc = (WNDPROC)SetWindowLongPtr(g_hWnd, GWLP_WNDPROC, (LONG_PTR)WndProc);

	if (!oWndProc)
	{
		LERROR("2B Hook Failed Initialization!\nCould not get old wndproc function pointer! (hWnd %p, error %u)\n",
			g_hWnd, GetLastError());
		return;
	}

	//InitializeNopMemory(nop_neon_scenery, FindPattern(NULL, "45 8B FC 0F 29 44 08"), 5);
	InitializeNopMemory(&nop_Health[NOP_DAMAGE_ENEMY], g_pDecreaseHealth[NOP_DAMAGE_ENEMY], 6);
#ifdef DENUVO_STEAM_BUILD
	InitializeNopMemory(&nop_Health[NOP_DAMAGE_WORLD], g_pDecreaseHealth[NOP_DAMAGE_WORLD], 6);
#else
	// The 5-byte call to Pl0000_TakeDamage in fall recovery (see FindSteamOffsets)
	InitializeNopMemory(&nop_Health[NOP_DAMAGE_WORLD], g_pDecreaseHealth[NOP_DAMAGE_WORLD], 5);
#endif
	//InitializeNopMemory(&nop_Framecap[NOP_FRAMECAP_SLEEP], g_pAntiFramerateCap_Sleep, 6);
	//InitializeNopMemory(&nop_Framecap[NOP_FRAMECAP_SPINLOCK], g_pAntiFramerateCap_Spinlock, 2);

#ifdef DENUVO_STEAM_BUILD
	InitializeBytePatchMemory(&bp_Framecap, g_pAntiFramerateCap_Test4, JmpFramecap, 7);
#else
	// The frame limiter's jnz rel32 (0F 85) becomes jmp rel32 (E9) + nop to the same place. The
	// jmp is a byte shorter, so its displacement is one more. Nothing is patched unless the bytes
	// there really are that jnz.
	static BYTE s_JmpFramecap[6] = { 0xE9, 0x00, 0x00, 0x00, 0x00, 0x90 };
	BYTE* pFramecapJnz = (g_pAntiFramerateCap_Test4 && g_pAntiFramerateCap_Test4[0] == 0x0F && g_pAntiFramerateCap_Test4[1] == 0x85) ?
		g_pAntiFramerateCap_Test4 : NULL;

	if (pFramecapJnz)
	{
		const LONG lDisplacement = *(LONG*)(pFramecapJnz + 2) + 1;

		memcpy(&s_JmpFramecap[1], &lDisplacement, sizeof(lDisplacement));
	}

	InitializeBytePatchMemory(&bp_Framecap, pFramecapJnz, s_JmpFramecap, sizeof(s_JmpFramecap));
#endif
	// Pl0000_CallDialogTutorial, the script command that opens tutorial dialogs: lea eax, [rdx-1] /
	// mov r8d, edx / cmp eax, 0Ch / ja <ret> / ... / jmp Tutorial_Open. Turning the ja (+9) into
	// a jmp makes it always return. The old "77 07 8B CA" was from the Denuvo build and matched
	// nothing on v1.02 or build 26534214.
	ULONG_PTR uCallDialogTutorial = FindPattern(NULL, "8D 42 FF 44 8B C2 83 F8 0C 77 ? 33 D2 41 8B C8 E9");

	InitializeBytePatchMemory(&bp_NoTutorialDialogs,
		(uCallDialogTutorial && uCallDialogTutorial != (ULONG_PTR)-1) ? (void*)(uCallDialogTutorial + 9) : NULL, LocalJmp, 1);

	InitHooks();

	s_bSetupComplete = TRUE;

	g_pConsole->Log(ImVec4(0.0f, 0.525f, 0.0f, 1.0f), "2B Hook v" HOOK_VERSION_STRING " Initialization Complete! GLHF");

	LOG("2B Hook v" HOOK_VERSION_STRING " Initialization Complete!\n");
}

void Unhook(void)
{
	// Installed in DllMain, before Setup, so it comes out even when Setup never finished
	HeapScale::Uninstall();

	// Nothing below exists if Setup() bailed out: RestoreMemory() __debugbreak()s on the
	// never-initialized patch structs, ImGui has no backend and oWndProc is null.
	if (!s_bSetupComplete)
		return;

	MaterialTint::Shutdown();
	MotionPlayer::Shutdown();

	// Only if the slot still holds ours
	if (CRIMainCallbackLog)
		InterlockedCompareExchangePointer((PVOID*)CRIMainCallbackLog, NULL, (PVOID)&CRILogCallbackConsole);

	RestoreMemory(&bp_NoTutorialDialogs);
	RestoreMemory(&bp_Framecap);
	//RestoreMemory(&nop_Framecap[NOP_FRAMECAP_SLEEP]);
	//RestoreMemory(&nop_Framecap[NOP_FRAMECAP_SPINLOCK]);
	RestoreMemory(&nop_Health[NOP_DAMAGE_WORLD]);
	RestoreMemory(&nop_Health[NOP_DAMAGE_ENEMY]);

	CloseHandle(g_CreateCtx.hControlSemaphore);

	// FIXME: Unhook functions with hookfunc

	for (VirtualTableHook* it : g_pRubyInstancesHooks)
		delete it;

	delete g_pOleLoadPictureHook;
	delete g_pSetUnhandledExceptionFilterHook;
	delete g_pQueryPerformanceCounterHook;
	delete g_pClipCursorHook;

#ifdef ENABLE_MEMORYDEVICE_HOOK
	delete g_pMemoryDeviceHook;
#endif

	delete g_pFactoryHook;
	delete g_pSwapChainHook;
	delete g_pDeviceContextHook;
	delete g_pKeyboardHook;
	delete g_pMouseHook;
	delete g_pModelAnalyzerHook;

	delete g_pMenu;

	Vars.Gameplay.bGodmode = false;
	Vars.Gameplay.bNoEnemyDamage = false;
	Vars.Gameplay.bNoWorldDamage = false;

	Features::ApplyHealthMods(true);
	Features::ApplyFreeCamFlags(true);

	EspRenderer::Shutdown();
	ImGui_ImplDX11_Shutdown();

	SetWindowLongPtr(g_hWnd, GWLP_WNDPROC, (LONG_PTR)oWndProc);

	free(g_phHeaps);

#if defined(_DEBUG) || defined(VERBOSE)
	Log::DetachConsole();
#endif

	// This used to end in FreeLibraryAndExitThread. Unhook only runs from DllMain, which the
	// loader calls while it unloads this DLL, so that freed it a second time and ended the
	// thread that was unloading it, with the loader lock held. A thread that unloads 2BHook
	// itself would call FreeLibraryAndExitThread after Unhook.
}

BOOL WINAPI DllMain(HINSTANCE hInstDll, DWORD dwReason, LPVOID lpContext)
{
	g_hInstance = hInstDll;

	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
		// Before the game makes its heaps, if it hasn't yet (the mods folder at startup)
		HeapScale::InstallEarly(hInstDll);
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Setup, NULL, 0, NULL);
		break;
	case DLL_PROCESS_DETACH:
		// lpContext isn't NULL when the process is exiting: the window's X, Alt+F4, the game's own
		// quit. Its other threads are gone by then, and the locks they held stay held, so the
		// teardown could wait forever on one: the D3D objects ImGui releases, the console, the
		// heap. Windows frees all of it, and the patches don't matter in a process that is ending,
		// so Unhook only runs when the DLL is unloaded from a live process.
		if (!lpContext)
			Unhook();
		break;
	default:
		break;
	}
	return TRUE;
}