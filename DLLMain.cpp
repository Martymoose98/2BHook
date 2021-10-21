#include <Windows.h>
#include <VersionHelpers.h>
#include <d3d11.h>
#include <D3Dcompiler.h>
#define  DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <stdio.h>

#include "Memory.h"
#include "cpk.h"
#include "mot.h"
#include "Hooks.h"
#include "VirtualTableHook.h"
#include "ImportTableHook.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "xinput.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dbghelp.lib")

CMemory* g_pMemory = new CMemory();

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

	// FIXME: DirectInput8 devices freeze on aquire when locked with their critical
	// section. Deadlock probable; mitigate.

	// Take ownership of the internal CDIDev's critical section to stop crashes
	//IDirectInputDevice_Lock(g_pKeyboard->pDevice);

	// Initalize a virtual table hook for the keyboard
	g_pKeyboardHook = new VirtualTableHook((ULONG_PTR**)g_pKeyboard->pDevice);

	// Release ownership of the internal CDIDev's critical section
	//IDirectInputDevice_Unlock(g_pKeyboard->pDevice);

	g_pKeyboard->bAcquired = FALSE;

	// Take ownership of the internal CDIDev's critical section to stop crashes
	//IDirectInputDevice_Lock(g_pMouse->pDevice);

	// Initalize a virtual table hook for the mouse
	g_pMouseHook = new VirtualTableHook((ULONG_PTR**)g_pMouse->pDevice);

	// Release ownership of the internal CDIDev's critical section
	//IDirectInputDevice_Unlock(g_pMouse->pDevice);

	g_pMouse->bAcquired = FALSE;

	g_pCameraHook = new VirtualTableHook((ULONG_PTR**)g_pCamera);


#ifdef ENABLE_MEMORYDEVICE_HOOK
	g_pMemoryDeviceHook = new VirtualTableHook((ULONG_PTR**)g_pMemoryDevice); // for larger child heap sizes
#endif

	for (MrubyImpl* it : g_pRubyInstances)
	{
		g_pRubyInstancesHooks.emplace_back(new VirtualTableHook((ULONG_PTR**)it));
		oMRubyLoadScript = (MRubyLoadScriptFn)g_pRubyInstancesHooks.back()->HookFunction((ULONG_PTR)hkMRubyLoadScript, 2);
	}

	g_pQueryPerformanceCounterHook = new ImportTableHook("kernel32.dll", "QueryPerformanceCounter", (LPCVOID)hkQueryPerformanceCounter);
	oQueryPerformanceCounter = (QueryPerformaceCounterFn)g_pQueryPerformanceCounterHook->GetOriginalFunction();

	g_pClipCursorHook = new ImportTableHook("user32.dll", "SetCursorPos", (LPCVOID)hkSetCursorPos);
	oSetCursorPos = (SetCursorPosFn)g_pClipCursorHook->GetOriginalFunction();

	g_pXInputGetStateHook = new ImportTableHook("xinput1_3.dll", "XInputGetState", (LPCVOID)hkXInputGetState);
	oXInputGetState = (XInputGetStateFn)g_pXInputGetStateHook->GetOriginalFunction();

	g_pSetUnhandledExceptionFilterHook = new ImportTableHook("kernel32.dll", "SetUnhandledExceptionFilter", (LPCVOID)hkSetUnhandledExceptionFilter);
	oSetUnhandledExceptionFilter = (SetUnhandledExceptionFilterFn)g_pSetUnhandledExceptionFilterHook->GetOriginalFunction();

	oPresent = (PresentFn)g_pSwapChainHook->HookFunction((ULONG_PTR)hkPresent, 8);
	oCreateSwapChain = (CreateSwapChainFn)g_pFactoryHook->HookFunction((ULONG_PTR)hkCreateSwapChain, 10);
	oPSSetShaderResources = (PSSetShaderResourcesFn)g_pDeviceContextHook->HookFunction((ULONG_PTR)hkPSSetShaderResources, 8);
	oDrawIndexed = (DrawIndexedFn)g_pDeviceContextHook->HookFunction((ULONG_PTR)hkDrawIndexed, 12);
	oDraw = (DrawFn)g_pDeviceContextHook->HookFunction((ULONG_PTR)hkDraw, 13);
	oClearRenderTargetView = (ClearRenderTargetViewFn)g_pDeviceContextHook->HookFunction((ULONG_PTR)hkClearRenderTargetView, 50);
	oKeyboardAcquire = (AcquireFn)g_pKeyboardHook->HookFunction((ULONG_PTR)hkKeyboardAcquire, 7);
	oKeyboardGetDeviceState = (GetDeviceStateFn)g_pKeyboardHook->HookFunction((ULONG_PTR)hkKeyboardGetDeviceState, 9);
	oMouseAcquire = (AcquireFn)g_pMouseHook->HookFunction((ULONG_PTR)hkMouseAcquire, 7);
	oMouseGetDeviceState = (GetDeviceStateFn)g_pMouseHook->HookFunction((ULONG_PTR)hkMouseGetDeviceState, 9);
	oCCameraGameSetViewAngles = (CCameraGameSetViewAnglesFn)g_pCameraHook->HookFunction((ULONG_PTR)hkCCameraGameSetViewAngles, 1);
	oCCameraGameMove = (CCameraGameMoveFn)g_pCameraHook->HookFunction((ULONG_PTR)hkCCameraGameMove, 2);

	ZeroMemory(&g_UserManagerSaveFileIO, sizeof(HOOK_FUNC));
	ZeroMemory(&g_CreateEntityHook, sizeof(HOOK_FUNC));
	ZeroMemory(&g_LoadWordBlacklist, sizeof(HOOK_FUNC));

#ifdef DENUVO_STEAM_BUILD
	g_pMemory->HookFunc64(SaveFileIO, hkSaveFileIO, 45, &g_UserManagerSaveFileIO);

	// FIXME:  this could be moved idk it's debatable  
	oCreateEntity = (CreateEntityFn)g_pMemory->FindPattern(NULL, "48 89 5C 24 ? 48 89 4C 24 ? 55 48 83 EC 20");
	QWORD qwContainingFunc = g_pMemory->FindPatternPtr(NULL, "E8 ? ? ? ? 85 C0 75 1F 48 8B CD", 1);

	//THERE IS ANOTHER CreateEntity call at: qwContainingFunc + 0x676
	g_pMemory->HookFunc64((LPVOID)(qwContainingFunc + 0x132), hkCreateEntityThunk, 16, &g_CreateEntityHook);

	// FIXME: sig this hard coded offset for both version clown
	//g_pMemory->HookFunc64((VOID*)0x140607011, hkLoadWordBlacklist, 157, &hf); //caller
	g_pMemory->HookFunc64((VOID*)0x140606940, hkLoadWordBlacklistThunk, 20, &g_LoadWordBlacklist);//callee
#else
	// FIXME: This hook works but is digusting
	g_pMemory->HookFunc64(SaveFileIO, hkSaveFileIOThunk, 46, &g_UserManagerSaveFileIO);

	// FIXME: this could be moved idk it's debatable  
	oCreateEntity = (CreateEntityFn)g_pMemory->FindPatternPtr(NULL, "E8 ? ? ? ? 48 89 47 60 48 85 C0 75 9D", 1);
	QWORD qwContainingFunc = g_pMemory->FindPatternPtr(NULL, "E8 ? ? ? ? 85 C0 75 25 48 8B CD", 1);

	//THERE IS ANOTHER CreateEntity call at: qwContainingFunc + 0x6A1
	g_pMemory->HookFunc64((LPVOID)(qwContainingFunc + 0x237), hkCreateEntityThunk, 16, &g_CreateEntityHook);
#endif // !OLD_DENUVO_STEAM_BUILD

	// FIXME: should move to more appropriate location, as it isn't a hook. 
	// Also, forget what this does but it's not critical for the dll's operation.
	// If I do keep I need to resig the new version
	InitalizeNopMemory(&nop_HairColor, g_pMemory->FindPattern(NULL, "0F 29 22 49 8B CE"), 12)

		g_pMemory->NopMemory(&nop_HairColor);
}

void CreateRenderTarget(void)
{
	DXGI_SWAP_CHAIN_DESC sd;
	g_pSwapChain->GetDesc(&sd);

	// Create the render target
	ID3D11Texture2D* pBackBuffer;
	D3D11_RENDER_TARGET_VIEW_DESC render_target_view_desc;

	ZeroMemory(&render_target_view_desc, sizeof(render_target_view_desc));

	render_target_view_desc.Format = sd.BufferDesc.Format;
	render_target_view_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

	g_pSwapChain->GetBuffer(0, IID_ID3D11Texture2D, (void**)&pBackBuffer);

	if (pBackBuffer)
	{
		g_pDevice->CreateRenderTargetView(pBackBuffer, &render_target_view_desc, &g_pRenderTargetView);
		g_pDeviceContext->OMSetRenderTargets(1, &g_pRenderTargetView, NULL);
		pBackBuffer->Release();
	}
}

void CreateStencilDescription(void)
{
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
	g_pDevice->CreateDepthStencilState(&StencilDesc, &g_pDepthStencilStates[ENABLED]);

	StencilDesc.DepthEnable = false;
	StencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	g_pDevice->CreateDepthStencilState(&StencilDesc, &g_pDepthStencilStates[DISABLED]);

	StencilDesc.DepthEnable = false;
	StencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	StencilDesc.StencilEnable = false;
	StencilDesc.StencilReadMask = 0xFF;
	StencilDesc.StencilWriteMask = 0x0;
	g_pDevice->CreateDepthStencilState(&StencilDesc, &g_pDepthStencilStates[NO_READ_NO_WRITE]);

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
	g_pDevice->CreateDepthStencilState(&StencilDesc, &g_pDepthStencilStates[READ_NO_WRITE]);
}

HRESULT InitD3D11(void)
{
	HRESULT hr = g_pSwapChain->GetDevice(IID_ID3D11Device, (void**)&g_pDevice); // i'm using IID_ID3D11Device for c compatiblity alternatively you can use __uuidof

	if (FAILED(hr))
	{
		LOG("2B Hook Failed Initalization!\nCould not obtain a ID3D11Device pointer! HRESULT %x\n", hr);
		return hr;
	}

	hr = g_pSwapChain->GetParent(IID_IDXGIFactory, (void**)&g_pFactory);

	if (FAILED(hr))
	{
		LOG("2B Hook Failed Initalization!\nCould not obtain a IDXGIFactory pointer! HRESULT %x\n", hr);
		return hr;
	}

	g_pDevice->GetImmediateContext(&g_pDeviceContext);

	ImGui::CreateContext();
	ImGui::StyleColorsDark();

	if (!ImGui_ImplDX11_Init(g_hWnd, g_pDevice, g_pDeviceContext))
	{
		LOG("2B Hook Failed Initalization!\nCould not initalize ImGui!\n");
		return S_FALSE;
	}

	if (!ImGui_ImplDX11_CreateDeviceObjects())
	{
		LOG("2B Hook Failed Initalization!\nCould not create device objects!\n");
		return S_FALSE;
	}

	CreateStencilDescription();
	CreateRenderTarget();

	//wireframe
	D3D11_RASTERIZER_DESC RenderWireframeDesc;
	g_pDeviceContext->RSGetState(&g_pRenderWireframeState); // retrieve the current state
	g_pRenderWireframeState->GetDesc(&RenderWireframeDesc);    // get the desc of the state
	RenderWireframeDesc.FillMode = D3D11_FILL_WIREFRAME;
	RenderWireframeDesc.CullMode = D3D11_CULL_NONE;
	// create a whole new rasterizer state
	g_pDevice->CreateRasterizerState(&RenderWireframeDesc, &g_pRenderWireframeState);

	//solid
	D3D11_RASTERIZER_DESC RenderSolidDesc;
	g_pDeviceContext->RSGetState(&g_pRenderSolidState); // retrieve the current state
	g_pRenderSolidState->GetDesc(&RenderSolidDesc);    // get the desc of the state
	RenderSolidDesc.FillMode = D3D11_FILL_SOLID;
	RenderSolidDesc.CullMode = D3D11_CULL_BACK;
	// create a whole new rasterizer state
	g_pDevice->CreateRasterizerState(&RenderSolidDesc, &g_pRenderSolidState);

	//create sample state
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	sampDesc.MaxAnisotropy = 1;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	hr = g_pDevice->CreateSamplerState(&sampDesc, &g_pSamplerState);
	if (FAILED(hr)) { LOG("Failed to CreateSamplerState"); }

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
	if (FAILED(hr)) { LOG("Failed to CreateTexture2D"); }

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
	if (FAILED(hr)) { LOG("Failed to CreateTexture2D"); }

	//create green shaderresourceview
	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
	memset(&SRVDesc, 0, sizeof(SRVDesc));
	SRVDesc.Format = format;
	SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	SRVDesc.Texture2D.MipLevels = 1;
	hr = g_pDevice->CreateShaderResourceView(g_pTexGreen, &SRVDesc, &g_pGreenSRV);
	if (FAILED(hr)) { LOG("Failed to CreateShaderResourceView"); }
	g_pTexGreen->Release();

	//create red shaderresourceview
	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDescr;
	memset(&SRVDescr, 0, sizeof(SRVDescr));
	SRVDescr.Format = format;
	SRVDescr.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	SRVDescr.Texture2D.MipLevels = 1;
	hr = g_pDevice->CreateShaderResourceView(g_pTexRed, &SRVDescr, &g_pRedSRV);
	if (FAILED(hr)) { LOG("Failed to CreateShaderResourceView"); }
	g_pTexRed->Release();

	GeneratePixelShader(g_pDevice, &g_pRed, 1.0f, 0.0f, 0.0f);
	GeneratePixelShader(g_pDevice, &g_pGreen, 0.0f, 1.0f, 0.0f);

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
	LOG_OFFSET("CEntityList", g_pEntityInfoList);
	LOG_OFFSET("LocalPlayerHandle", g_pLocalPlayerHandle);
	LOG_OFFSET("EmilHandle", g_pEmilHandle);
	LOG_OFFSET("CYorhaManager", g_pYorhaManager);
	LOG_OFFSET("CNPCManager", g_pNPCManager);
	LOG_OFFSET("CEnemyManager", g_pEnemyManager);
	LOG_OFFSET("CUserManager", g_pUserManager);
	LOG_OFFSET("CWetObjectManager", g_pWetObjectManager);
	LOG_OFFSET("CCollisionDataObjectManager", g_pCollisionDataObjectManager);
	LOG_OFFSET("CTextureResourceManager", g_pTextureResourceManager);
	LOG_OFFSET("COtManager", g_pOtManager);
	LOG_OFFSET("CModelManager", g_pModelManager);
	//LOG_OFFSET("MRubyVM", g_pRubyInstances);
	LOG_OFFSET("CGameCamera", g_pCamera);
	LOG_OFFSET("CMemoryDevice", g_pMemoryDevice);
	LOG_OFFSET("CalculateLevel", CalculateLevel);
	LOG_OFFSET("GetEntityFromHandle", GetEntityFromHandle);
	LOG_OFFSET("DataDirectory", g_szDataDirectoryPath);
	LOG_OFFSET("Money", g_piMoney);
	LOG_OFFSET("Experience", g_piExperience);
	LOG_OFFSET("IDirectInput8A", g_pDirectInput8);
	LOG_OFFSET("CKeyboard", g_pKeyboard);
	LOG_OFFSET("CMouse", g_pMouse);
	LOG_OFFSET("CGraphics", g_pGraphics);
	LOG_OFFSET("ViewMatrix", g_pViewMatrix);

	g_pConsole->Log(ImVec4(0.0f, 0.5f, 0.8f, 1.0f), "Data Directory Path: %s", g_szDataDirectoryPath);
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
	CRILogCallback = (CRILogCallbackFn)g_pMemory->FindPatternPtr(NULL, "48 8B 35 ? ? ? ? 48 8B 1D ? ? ? ?", 10);
	CRILogCallback = CRILogCallbackConsole; // CRILogCallbackWinConsole

	CalculateLevel = (CalculateLevelFn)g_pMemory->FindPattern(NULL, "44 8B 91 ? ? ? ? 45 33 C9 41 8B C1 45 85 D2 7E");
	GetEntityFromHandle = (GetEntityFromHandleFn)g_pMemory->FindPattern(NULL, "40 53 48 83 EC ? 8B 11 85 D2 74"); // there is like 50 identical functions kek
	GetItemNameById = (GetItemNameByIdFn)g_pMemory->FindPatternPtr(NULL, "E8 ? ? ? ? 33 FF 8B D7", 1);
	GetItemIdByName = (GetItemIdByNameFn)g_pMemory->FindPatternPtr(NULL, "E8 ? ? ? ? 85 C0 78 11", 1);
	SetLocalPlayer = (SetLocalPlayerFn)g_pMemory->FindPatternPtr(NULL, "E8 ? ? ? ? C6 43 48 00 81 7B ? ? ? ? ?", 1);
	UnlockAchievement = (UnlockAchievementFn)g_pMemory->FindPatternPtr(NULL, "E8 ? ? ? ? 8B D8 83 3D ? ? ? ? ?", 1);
	ResetCamera = (ResetCameraFn)g_pMemory->FindPatternPtr(NULL, "E8 ? ? ? ? 41 0F 28 07", 1);
	ChangePlayer = (ChangePlayerFn)g_pMemory->FindPattern(NULL, "40 53 48 83 EC 20 8B 05 ? ? ? ? 48 8B D9 48 8D 4C 24 ? 89 44 24 30 E8 ? ? ? ? 48 85 C0 74 31");
	FindSceneState = (FindSceneStateFn)g_pMemory->FindPatternPtr(NULL, "E8 ? ? ? ? 83 CE 01", 1);
	SetSceneEntity = (SetSceneEntityFn)g_pMemory->FindPatternPtr(NULL, "E8 ? ? ? ? 8B 87 ? ? ? ? 41 3B C4", 1);

	SaveFileIO = (CSaveDataDevice_SaveFileIOFn)g_pMemory->FindPattern(NULL, "8B 51 30 FF CA");
	ReadSaveSlots = (CSaveDataDevice_ReadSaveSlotsFn)g_pMemory->FindPatternPtr(NULL, "E8 ? ? ? ? 48 8B AE ? ? ? ? 48 85 ED 74 38", 1);
	ReadSaveData = (CSaveDataDevice_ReadSaveDataFn)g_pMemory->FindPatternPtr(NULL, "E8 ? ? ? ? EB 05 E8 ? ? ? ? 48 8B AE ? ? ?", 1);
	WriteSaveData = (CSaveDataDevice_WriteSaveDataFn)g_pMemory->FindPatternPtr(NULL, "E8 ? ? ? ? EB 0C E8 ? ? ? ? EB 05 E8 ? ? ? ? 48 8B AE ? ? ? ? ", 1);
	DeleteSaveData = (CSaveDataDevice_DeleteSaveDataFn)g_pMemory->FindPattern(NULL, "40 53 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 84 24 ? ? ? ? 48 8B D9 8B 49 34");

	WetObjectManager_SetWet = (CWetObjectManager_SetWetFn)g_pMemory->FindPatternPtr(NULL, "E8 ? ? ? ? 4C 8D 05 ? ? ? ? 41 FF C6", 1);
	WetObjectManager_SetDry = (CWetObjectManager_SetDryFn)g_pMemory->FindPattern(NULL, "48 85 D2 0F 84 ? ? ? ? 53 48 83 EC 20 83 3D ? ? ? ? ? 48 8B DA 0F 84 ? ? ? ? 48 89 74 24 ? 48 8D 35 ? ? ? ? 48 89 7C 24 ? 48 8B CE FF 15 ? ? ? ? 4C 8B 1D ? ? ? ? 44 8B 15 ? ? ? ? 45 33 C9 48 8D 0D ? ? ? ? 48 8D 3D ? ? ? ? 44 8B 01 45 85 C0 74 34 41 8B C0 C1 F8 08 0F B7 D0 41 3B D2 73 26 48 03 D2 41 8B 04 D3 41 33 C0 A9 ? ? ? ? 75 15 49 8B 44 D3 ? 48 85 C0 74 0B F6 40 2C 03 75 05 48 3B C3 74 0E 48 83 C1 04 41 FF C1 48 3B CF 7C B8 EB 1B");
	WetObjectManager_AddLocalEntity = (CWetObjectManager_AddLocalEntityFn)g_pMemory->FindPatternPtr(NULL, "E8 ? ? ? ? 44 89 AF ? ? ? ? 48 C7 87 ? ? ? ? ? ? ? ?", 1);
	CameraGame_SetLookAt = (CCameraGame_SetLookAtFn)g_pMemory->FindPatternPtr(NULL, "E8 ? ? ? ? 83 7F 10 01", 1);
	DestroyBuddy = (DestroyBuddyFn)g_pMemory->FindPattern(NULL, "40 53 48 83 EC 30 48 C7 44 24 ? ? ? ? ? 48 8B D9 48 8B 01");
	FNV1Hash = (FNV1HashFn)g_pMemory->FindPatternPtr(NULL, "E8 ? ? ? ? 85 C0 74 A3", 1);
	HashStringCRC32 = (HashStringCRC32Fn)g_pMemory->FindPattern(NULL, "40 57 83 C8 FF");
	GetConstructionInfo = (GetConstructorFn)g_pMemory->FindPattern(NULL, "33 D2 44 8B C9");
	GetOBBMax = (ExCollision_GetOBBMaxFn)g_pMemory->FindPattern(NULL, "48 8B C4 48 89 68 18 56");

	QueryHeap = (QueryHeapFn)g_pMemory->FindPattern(NULL, "48 83 EC 28 4C 8B D9 C7 41 ? ? ? ? ?");
	GetWork = (GetWorkFn)g_pMemory->FindPattern(NULL, "40 57 48 83 EC 20 8B F9");
	PreloadFile = (PreloadFileFn)g_pMemory->FindPatternPtr(NULL, "E8 ? ? ? ? 8D 5F 01", 1);
	RequestEnd = (ObjReadSystem_RequestEndFn)g_pMemory->FindPatternPtr(NULL, "E8 ? ? ? ? 41 8D 46 FF", 1);
	PreloadModel = (ObjReadSystem_PreloadModelFn)g_pMemory->FindPatternPtr(NULL, "E8 ? ? ? ? 33 FF 45 33 C0", 1);
	GetGraphicCommand = (COtManager_GetGraphicCommandFn)g_pMemory->FindPattern(NULL, "48 83 EC 08 48 8B 05 ? ? ? ?");

	CpkMount = (CpkMountFn)g_pMemory->FindPattern(NULL, "48 83 EC 28 44 8B C9");

	g_pEntityInfoList = (CEntityList*)g_pMemory->FindPatternPtr(NULL, "4C 8B 4A 10 48 8D 15 ? ? ? ? 4D 8D 89 ? ? ? ?", 7);
	g_pLocalPlayerHandle = (EntityHandle*)g_pMemory->FindPatternPtr(NULL, "45 33 F6 4C 8D 25 ? ? ? ? 4C 8D 05 ? ? ? ?", 6);
	g_pEmilHandle = (EntityHandle*)g_pMemory->FindPatternPtr(NULL, "8B 15 ? ? ? ? 85 D2 74 59", 2);
	g_piMoney = (int*)g_pMemory->FindPatternPtr(NULL, "48 8D 3D ? ? ? ? 48 8D 8D ? ? ? ?", 3);
	g_piExperience = (int*)g_pMemory->FindPatternPtr(NULL, "8B 15 ? ? ? ? 75 06", 2);
	g_pYorhaManager = *(YorhaManager**)g_pMemory->FindPatternPtr(NULL, "48 8B D1 48 8B 0D ? ? ? ? 48 8B 01", 6);
	g_pNPCManager = *(NPCManager**)g_pMemory->FindPatternPtr(NULL, "75 B9 48 8B 0D ? ? ? ?", 5);
	g_pEnemyManager = *(EmBaseManager**)g_pMemory->FindPatternPtr(NULL, "4C 89 A3 ? ? ? ? 48 8B 0D ? ? ? ?", 10);
	g_pUserManager = *(CUserManager**)g_pMemory->FindPatternPtr(NULL, "74 1D 48 8B 0D ? ? ? ? 48 85 C9", 5);
	g_pWetObjectManager = (CWetObjManager*)(g_pMemory->ReadPtr64(g_pMemory->FindPatternPtr(NULL, "E8 ? ? ? ? 44 89 AF ? ? ? ? 48 C7 87 ? ? ? ? ? ? ? ?", 1) + 0x1B, 3));
	g_pCollisionDataObjectManager = *(CCollisionDataObjectManager**)g_pMemory->FindPatternPtr(NULL, "0F 45 C7 48 8B 3D ? ? ? ?", 6);
	g_pTextureResourceManager = *(CTextureResourceManager**)g_pMemory->FindPatternPtr(NULL, "48 8B D1 48 8D 0D ?? ?? ?? ?? 4C 89 44 24 ??", 6);
	g_pCamera = (CCameraGame*)g_pMemory->FindPatternPtr(NULL, "4C 8D 05 ? ? ? ? 4C 89 D1", 3);
	g_pSceneStateSystem = (CSceneStateSystem*)g_pMemory->FindPatternPtr(NULL, "48 8D 0D ? ? ? ? E8 ? ? ? ? 90 EB 52", 3);
	g_pSceneEntitySystem = (CSceneEntitySystem*)g_pMemory->FindPatternPtr(NULL, "48 8D 35 ?? ?? ?? ?? 4C 8D 35 ?? ?? ?? ?? 48 8D 54 24 ??", 3); //48 8D 35 ? ? ? ? 4C 8D 35 ? ? ? ? 48 8D 54 24 ?
	g_pMemoryDevice = (CMemoryDevice*)g_pMemory->FindPatternPtr(NULL, "48 8B DA 48 8D 0D ? ? ? ? E8 ? ? ? ?", 6);
	g_pDecreaseHealth[NOP_DAMAGE_ENEMY] = (byte*)g_pMemory->FindPattern(NULL, "29 BB ? ? ? ? 8B 83 ? ? ? ? 41 0F 48 C5");
	g_pDecreaseHealth[NOP_DAMAGE_WORLD] = (byte*)g_pMemory->FindPattern(NULL, "29 BB ? ? ? ? 8B 83 ? ? ? ? BD ? ? ? ? 0F 48 C5");

	g_szDataDirectoryPath = (LPSTR)g_pMemory->FindPatternPtr(NULL, "E8 ? ? ? ? 48 8D 3D ? ? ? ? 48 8B CF", 8);
	g_pDirectInput8 = *(IDirectInput8A**)g_pMemory->FindPatternPtr(NULL, "48 8B 0D ? ? ? ? 48 85 C9 74 06 48 8B 01 FF 50 10 48 89 35 ? ? ? ? 48 89 35 ? ? ? ? 48 89 35", 3);
	g_pKeyboard = (Keyboard_t*)g_pMemory->FindPatternPtr(NULL, "48 8B 0D ? ? ? ? 48 85 C9 74 06 48 8B 01 FF 50 10 48 89 35 ? ? ? ? 48 89 35 ? ? ? ? 89", 3);
	g_pMouse = (Mouse_t*)g_pMemory->FindPatternPtr(NULL, "48 8D 0D ? ? ? ? 44 8B C3 E8 ? ? ? ?", 3);
	g_pGraphics = *(CGraphics**)g_pMemory->FindPatternPtr(NULL, "48 8D 05 ? ? ? ? 48 83 C4 ? C3 CC CC CC CC CC CC CC CC 48 89 4C 24 ? 57", 3);
	g_pOtManager = (COtManager*)g_pMemory->FindPatternPtr(NULL, "4C 89 78 D8 4C 8D 2D ? ? ? ?", 7);
	g_pModelManager = (CModelManager*)g_pMemory->FindPatternPtr(NULL, "48 8B C7 48 89 05 ? ? ? ?", 6);
	g_pViewMatrix = (VMatrix*)g_pMemory->FindPatternPtr(NULL, "0F 29 02 0F 28 2D ? ? ? ?", 6);
	//g_pSwapChain = *(IDXGISwapChain**)((*(byte**)g_pMemory->FindPatternPtr64(NULL, "48 89 35 ? ? ? ? 48 85 C9 74 ? 39 35 ? ? ? ? 74 ? 48 8B 01 BA ? ? ? ? FF 10 48 8B 0D ? ? ? ? 48 85 C9 74 ? 39 35 ? ? ? ? 74 ? 48 8B 01 BA ? ? ? ? FF 10 48 8B 0D ? ? ? ? 48 89 35 ? ? ? ? C7 05 ? ? ? ? ? ? ? ? 48 89 35 ? ? ? ? 48 85 C9 74 ? 39 35 ? ? ? ? 74 ? 48 8B 01 BA ? ? ? ? FF 10 48 8B 0D ? ? ? ? 48 85 C9 74 ? 39 35 ? ? ? ? 74 ? 48 8B 01 BA ? ? ? ? FF 10 48 8B 0D ? ? ? ? 48 89 35 ? ? ? ? C7 05 D8 ? ? ? ? ? ? ? ?", 3)) + 0xE0);
	g_pSwapChain = g_pGraphics->m_Display.m_pSwapChain->m_pSwapChain;
	g_pGraphicDevice = g_pGraphics->m_Display.m_pGraphicDevice;

	if (!IsWindows10OrGreater()) // for some reason it causes a crash on windows 7
		g_pSecondarySwapChain = (IDXGISwapChain*)(*(byte**)((*(byte**)((*(byte**)((*(byte**)((byte*)g_pGraphics + 0x1B8)) + 0x140))) + 0x10)));// I have no idea what this swapchain is for, but it points to the right place

	g_pAntiVSync = (byte*)g_pMemory->FindPattern(NULL, "0F 94 D2 45 31 C0 FF 50 40"); //unused
	g_pAntiFramerateCap_Sleep = (byte*)g_pMemory->FindPattern(NULL, "8B CA FF 15 ? ? ? ? 48 8D 4C 24 ?") + 2;
	g_pAntiFramerateCap_Spinlock = g_pAntiFramerateCap_Sleep + 0x48;
	g_pAntiFramerateCap_Test4 = (byte*)g_pMemory->FindPattern(NULL, "F6 05 ? ? ? ? ? 0F 29 74 24 ?");
	g_hWnd = *(HWND*)g_pMemory->FindPatternPtr(NULL, "48 89 05 ? ? ? ? 48 85 C0 0F 84 ? ? ? ? 0F 57 C0", 3);

	for (const MrubyImpl* pVM = *(MrubyImpl**)g_pMemory->FindPatternPtr(NULL, "48 89 0D ? ? ? ? 48 89 81 ? ? ? ?", 3); pVM; pVM = pVM->m_pNext)
		g_pRubyInstances.emplace_back((MrubyImpl*)((byte*)pVM - 0x20));
}

void FindSteamOffsets(void)
{
	CRILogCallback = (CRILogCallbackFn)g_pMemory->FindPatternPtr(NULL, "48 8B 1D ? ? ? ? 48 85 F6", 3);
	CRILogCallback = CRILogCallbackConsole; //CRILogCallbackWinConsole

	CalculateLevel = (CalculateLevelFn)0;
	GetConstructionInfo = (GetConstructorFn)0;
	GetEntityFromHandle = (GetEntityFromHandleFn)g_pMemory->FindPatternPtr(NULL, "E8 ? ? ? ? 8B EE 48 85 C0", 1);
	GetItemNameById = (GetItemNameByIdFn)g_pMemory->FindPatternPtr(NULL, "E8 ? ? ? ? 48 8D 15 ? ? ? ? EB 26", 1);
	GetItemIdByName = (GetItemIdByNameFn)g_pMemory->FindPatternPtr(NULL, "E8 ? ? ? ? 44 8B C3 8B D0", 1);
	ResetCamera = (ResetCameraFn)g_pMemory->FindPatternPtr(NULL, "E8 ? ? ? ? 41 81 A6 ? ? ? ? ? ? ? ?", 1);
	ChangePlayer = (ChangePlayerFn)g_pMemory->FindPattern(NULL, "40 53 48 83 EC 30 48 8B D9 E8 ? ? ? ? 81 25 ? ? ? ? ? ? ? ?");

	FNV1Hash = (FNV1HashFn)g_pMemory->FindPatternPtr(NULL, "E8 ? ? ? ? 8B D3 C1 FA 08", 1);

	// SaveIO Hook has to be reworked as CSaveDataDevice now resides in rbx instead of rcx.
	// Probably the best solution is to rebuild the function and call that from the hook.
	SaveFileIO = (CSaveDataDevice_SaveFileIOFn)g_pMemory->FindPattern(NULL, "8B 4B 30 83 E9 01");
	ReadSaveSlots = (CSaveDataDevice_ReadSaveSlotsFn)g_pMemory->FindPatternPtr(NULL, "E8 ? ? ? ? 48 8B B5 ? ? ? ? 48 85 F6", 1);
	ReadSaveData = (CSaveDataDevice_ReadSaveDataFn)g_pMemory->FindPatternPtr(NULL, "E8 ? ? ? ? EB 08 48 8B CB E8 ? ? ? ? 48 8B B5 ? ? ? ?", 1);
	WriteSaveData = (CSaveDataDevice_WriteSaveDataFn)g_pMemory->FindPattern(NULL, "40 53 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 84 24 ? ? ? ? 48 8B D9 8B 49 34");
	DeleteSaveData = (CSaveDataDevice_DeleteSaveDataFn)CSaveDataDevice_DeleteSaveData; // Inlined on new version hmmm this may not work may have to rebuild function

	WetObjectManager_SetDry = (CWetObjectManager_SetDryFn)g_pMemory->FindPattern(NULL, "48 85 D2 0F 84 ? ? ? ? 57 48 83 EC 30 48 C7 44 24 ? ? ? ? ? 48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 48 8B EA 48 8B F9 83 79 28 00 74 61");
	WetObjectManager_SetWet = (CWetObjectManager_SetWetFn)g_pMemory->FindPatternPtr(NULL, "E8 ? ? ? ? 41 BE ? ? ? ? FF C7", 1);
	WetObjectManager_AddLocalEntity = (CWetObjectManager_AddLocalEntityFn)g_pMemory->FindPatternPtr(NULL, "E8 ? ? ? ? 44 89 BF ? ? ? ? 48 C7 87 ? ? ? ? ? ? ? ? 4C 89 AF ? ? ? ?", 1);


	g_pEntityInfoList = (CEntityList*)g_pMemory->FindPatternPtr(NULL, "44 8B 0D ? ? ? ? 44 39 0D ? ? ? ? 75 24 ", 3);
	g_pLocalPlayerHandle = (EntityHandle*)g_pMemory->FindPatternPtr(NULL, "48 8D 15 ? ? ? ? 48 8D 4C 24 ? E8 ? ? ? ? 48 8B D0 48 8D 4C 24 ? E8 ? ? ? ? 48 8B C8 E8 ? ? ? ? 48 8B C8 E8 ? ? ? ? 48 8B D0", 3);
	g_piMoney = (int*)g_pMemory->FindPatternPtr(NULL, "48 83 EC 68 4C 8D 05 ? ? ? ? 48 8D 15 ? ? ? ?", 7);
	g_piExperience = (int*)g_pMemory->FindPatternPtr(NULL, "8B 15 ? ? ? ? 48 8D 8F ? ? ? ? E8 ? ? ? ? 8B 48 04", 2);

	g_pWetObjectManager = (CWetObjManager*)g_pMemory->FindPatternPtr(NULL, "48 8B D9 48 8D 0D ? ? ? ? E8 ? ? ? ? 48 8D 8B ? ? ? ?", 6);
	g_pYorhaManager = *(YorhaManager**)g_pMemory->FindPatternPtr(NULL, "48 89 0D ? ? ? ? 48 83 C4 28 C3 48 85 C0 48 89 05 ? ? ? ? 48 8B C8 0F 95 C0 48 83 C4 28 C3 CC 89 91 ? ? ? ?", 3);
	g_pNPCManager = *(NPCManager**)g_pMemory->FindPatternPtr(NULL, "48 89 1D ? ? ? ? 48 85 DB 40 0F 95 C7 8B C7 48 8B 5C 24 ? 48 83 C4 30", 3);
	g_pEnemyManager = *(EmBaseManager**)g_pMemory->FindPatternPtr(NULL, "48 8B 0D ? ? ? ? 48 8B 01 8B D3 FF 90 ? ? ? ? 48 8B 0D ? ? ? ? 48 8B 01 8B D3 FF 90 ? ? ? ? 44 89 BE ? ? ? ?", 3);
	g_pUserManager = *(CUserManager**)g_pMemory->FindPatternPtr(NULL, "48 89 1D ? ? ? ? 48 85 DB 74 2A", 3);

	g_pCamera = (CCameraGame*)g_pMemory->FindPatternPtr(NULL, "81 25 ? ? ? ? ? ? ? ? 48 8D 0D ? ? ? ? E8 ? ? ? ? 48 8B 03", 13);
	g_pViewMatrix = (VMatrix*)g_pMemory->FindPatternPtr(NULL, "48 8D 4E 10 48 8D 15 ? ? ? ?", 7);

	g_pDirectInput8 = *(IDirectInput8A**)g_pMemory->FindPatternPtr(NULL, "0F 88 ? ? ? ? 48 8B 0D ? ? ? ? 4C 8D 0D ? ? ? ?", 9);
	g_pKeyboard = (Keyboard_t*)g_pMemory->FindPatternPtr(NULL, "48 8B 0D ? ? ? ? 4C 8D 44 24 ? BA ? ? ? ?", 3);
	g_pMouse = (Mouse_t*)g_pMemory->FindPatternPtr(NULL, "B8 ? ? ? ? 48 89 0D ? ? ? ? C7 05 ? ? ? ? ? ? ? ?", 8);
	g_pGraphics = *(CGraphics**)g_pMemory->FindPatternPtr(NULL, "48 89 1D ? ? ? ? 48 85 DB 0F 84 ? ? ? ? 49 8B D6", 3);
	g_pSwapChain = g_pGraphics->m_Display.m_pSwapChain->m_pSwapChain;
	//g_pGraphicDevice = g_pGraphics->m_Display.m_pGraphicDevice;
	g_pGraphicDevice = g_pGraphics->m_pGraphicalDevice;

	g_hWnd = *(HWND*)g_pMemory->FindPatternPtr(NULL, "48 8B 15 ? ? ? ? 48 8B 01 FF 50 68 85 C0 0F 88 ? ? ? ? 85 DB", 3);
}


void Setup(void)
{
#if defined(_DEBUG) || defined(VERBOSE)
	STACK_TIMER(timer);
	Log::AttachConsole(L"2B Hook Debug Console");
#endif
	srand((unsigned int)time(NULL));

	FindOffsets();

	LogOffsets();

	ExposeHiddenXInputFunctions();

	QueryProcessHeaps(&g_phHeaps, NULL);

	InitD3D11();

	for (UINT i = 0; i < g_pGraphicDevice->m_uAdapters; ++i)
	{
		CAdapter* pAdapter = &g_pGraphicDevice->m_pAdapters[i];

		LOG("ADAPTER: %ls\n", pAdapter->m_szDescription);

		for (UINT j = 0; j < pAdapter->m_uOutputs; ++j)
		{
			COutput* pOutput = &pAdapter->m_pOutputs[j];

			LOG("OUTPUT: %ls\n", pOutput->m_szDeviceName);
		}
	}

	g_pConfig->CreateConfig(NULL);
	g_pConfig->EnumerateConfigs(NULL, &Vars.Menu.Config.pHead);
	//g_pRenderer->Initalize(g_pDevice, g_pDeviceContext);

	//no ini file
	ImGui::GetIO().IniFilename = NULL;

	oWndProc = (WNDPROC)SetWindowLongPtr(g_hWnd, GWLP_WNDPROC, (LONG_PTR)WndProc);

	if (!oWndProc)
	{
		LOG("2B Hook Failed Initalization!\nCould not get old wndproc function pointer!\n");
		return;
	}

	InitHooks();

	// add VEH?
	oSetUnhandledExceptionFilter(UnhandledExceptionHandler);
	g_pExceptionHandlers.push_back(UnhandledExceptionHandlerChild);

	//InitalizeNopMemory(nop_neon_scenery, g_pMemory->FindPattern(NULL, "45 8B FC 0F 29 44 08 ?"), 5);
	InitalizeNopMemory(&nop_Health[NOP_DAMAGE_ENEMY], g_pDecreaseHealth[NOP_DAMAGE_ENEMY], 6);
	InitalizeNopMemory(&nop_Health[NOP_DAMAGE_WORLD], g_pDecreaseHealth[NOP_DAMAGE_WORLD], 6);
	InitalizeNopMemory(&nop_Framecap[NOP_FRAMECAP_SLEEP], g_pAntiFramerateCap_Sleep, 6);
	InitalizeNopMemory(&nop_Framecap[NOP_FRAMECAP_SPINLOCK], g_pAntiFramerateCap_Spinlock, 2);

	InitalizeBytePatchMemory(&bp_Framecap, g_pAntiFramerateCap_Test4, JmpFramecap, 7);
	InitalizeBytePatchMemory(&bp_NoTutorialDialogs, g_pMemory->FindPattern(NULL, "77 07 8B CA"), LocalJmp, 1);

	g_pConsole->Log(ImVec4(0.0f, 0.525f, 0.0f, 1.0f), "2B Hook Initalization Complete! GLHF");

	LOG("2B Hook Initalization Complete!\n");
}

void Unhook(void)
{
	g_pMemory->RestoreMemory(&bp_NoTutorialDialogs);
	g_pMemory->RestoreMemory(&bp_Framecap);
	g_pMemory->RestoreMemory(&nop_Framecap[NOP_FRAMECAP_SLEEP]);
	g_pMemory->RestoreMemory(&nop_Framecap[NOP_FRAMECAP_SPINLOCK]);
	g_pMemory->RestoreMemory(&nop_Health[NOP_DAMAGE_WORLD]);
	g_pMemory->RestoreMemory(&nop_Health[NOP_DAMAGE_ENEMY]);

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
	delete g_pMemory;

	Vars.Gameplay.bGodmode = false;
	Vars.Gameplay.bNoEnemyDamage = false;
	Vars.Gameplay.bNoWorldDamage = false;

	Features::ApplyHealthMods();

	ImGui_ImplDX11_Shutdown();

	FindDataListFree(Vars.Menu.Config.pHead);

	SetWindowLongPtr(g_hWnd, GWLP_WNDPROC, (LONG_PTR)oWndProc);

	free(g_phHeaps);

#if defined(_DEBUG) || defined(VERBOSE)
	Log::DetachConsole();
#endif

	FreeLibraryAndExitThread(g_hInstance, EXIT_SUCCESS);
}

BOOL WINAPI DllMain(HINSTANCE hInstDll, DWORD dwReason, LPVOID lpContext)
{
	g_hInstance = hInstDll;

	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Setup, NULL, 0, NULL);
		break;
	case DLL_PROCESS_DETACH:
		Unhook();
		break;
	default:
		break;
	}
	return TRUE;
}