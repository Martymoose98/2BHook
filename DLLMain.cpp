#include <Windows.h>
#include <VersionHelpers.h>
#include <d3d11.h>
#include <D3Dcompiler.h>
#include <dinput.h>
#include <stdio.h>

#include "Memory.h"
#include "cpk.h"
#include "Hooks.h"
#include "VirtualTableHook.h"
#include "ImportTableHook.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "xinput.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dbghelp.lib")

CMemory* g_pMemory = new CMemory;

CONST BYTE LocalJmp[] = { 0xEB };
CONST BYTE XorRdxRdx[] = { 0x48, 0x31, 0xD2 };
CONST BYTE JmpFramecap[] = { 0xE9, 0x93, 0x00, 0x00, 0x00, 0x90, 0x90 };
BYTE RdiJumpHook[] = { 0x90, 0x48, 0xBF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xD7 };
BYTE RdiJumpHook2[] = { 0x90, 0x57, 0x48, 0xBF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xD7, 0x5F };
BYTE opcodes_save_file_io[] = { 0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xD0, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };

void InitHooks()
{
	g_pFactoryHook = new VirtualTableHook((QWORD**)g_pFactory);
	g_pSwapChainHook = new VirtualTableHook((QWORD**)g_pSwapChain);
	g_pDeviceContextHook = new VirtualTableHook((QWORD**)g_pDeviceContext);
	g_pKeyboardHook = new VirtualTableHook((QWORD**)g_pKeyboard->pKeyboard);
	g_pMouseHook = new VirtualTableHook((QWORD**)g_pMouse->pMouse);
	
	g_pQueryPerformanceCounterHook = new ImportTableHook("kernel32.dll", "QueryPerformanceCounter", (LPCVOID)hkQueryPerformanceCounter);
	oQueryPerformanceCounter = (QueryPerformaceCounterFn)g_pQueryPerformanceCounterHook->GetOriginalFunction();

	g_pClipCursorHook = new ImportTableHook("user32.dll", "SetCursorPos", (LPCVOID)hkSetCursorPos);
	oSetCursorPos = (SetCursorPosFn)g_pClipCursorHook->GetOriginalFunction();

	g_pXInputGetStateHook = new ImportTableHook("xinput1_3.dll", "XInputGetState", (LPCVOID)hkXInputGetState);
	oXInputGetState = (XInputGetStateFn)g_pXInputGetStateHook->GetOriginalFunction();

	g_pSetUnhandledExceptionFilterHook = new ImportTableHook("kernel32.dll", "SetUnhandledExceptionFilter", (LPCVOID)hkSetUnhandledExceptionFilter);
	oSetUnhandledExceptionFilter = (SetUnhandledExceptionFilterFn)g_pSetUnhandledExceptionFilterHook->GetOriginalFunction();

	oPresent = (PresentFn)g_pSwapChainHook->HookFunction((QWORD)hkPresent, 8);
	oCreateSwapChain = (CreateSwapChainFn)g_pFactoryHook->HookFunction((QWORD)hkCreateSwapChain, 10);
	oPSSetShaderResources = (PSSetShaderResourcesFn)g_pDeviceContextHook->HookFunction((QWORD)hkPSSetShaderResources, 8);
	oDrawIndexed = (DrawIndexedFn)g_pDeviceContextHook->HookFunction((QWORD)hkDrawIndexed, 12);
	oDraw = (DrawFn)g_pDeviceContextHook->HookFunction((QWORD)hkDraw, 13);
	oClearRenderTargetView = (ClearRenderTargetViewFn)g_pDeviceContextHook->HookFunction((QWORD)hkClearRenderTargetView, 50);
	oKeyboardAcquire = (AcquireFn)g_pKeyboardHook->HookFunction((QWORD)hkKeyboardAcquire, 7);
	oKeyboardGetDeviceState = (GetDeviceStateFn)g_pKeyboardHook->HookFunction((QWORD)hkKeyboardGetDeviceState, 9);
	oMouseAcquire = (AcquireFn)g_pMouseHook->HookFunction((QWORD)hkMouseAcquire, 7);
	oMouseGetDeviceState = (GetDeviceStateFn)g_pMouseHook->HookFunction((QWORD)hkMouseGetDeviceState, 9);

	*(QWORD*)&opcodes_save_file_io[2] = (QWORD)hkSaveFileIO;

	InitalizeBytePatchMemory(&bp_save_file_io, g_pMemory->FindPattern(NULL, "8B 51 30 FF CA"), opcodes_save_file_io, 24);

	g_pMemory->PatchBytes(&bp_save_file_io);

	oUpdateModelParts = (UpdateModelPartsFn)g_pMemory->FindPatternPtr64(NULL, "E8 ? ? ? ? 41 8B EC 44 89 64 24 ?", 1);

	*(QWORD*)&RdiJumpHook2[4] = (QWORD)hkUpdateModelPartsThunk;

	//InitalizeBytePatchMemory(&bp_UpdateModelParts, oUpdateModelParts, RdiJumpHook2, 15);

	//g_pMemory->PatchBytes(&bp_UpdateModelParts);

	oCreateEntity = (CreateEntityFn)g_pMemory->FindPattern(NULL, "48 89 5C 24 ? 48 89 4C 24 ? 55 48 83 EC 20");
	QWORD qwContainingFunc = g_pMemory->FindPatternPtr64(NULL, "E8 ? ? ? ? 85 C0 75 1F 48 8B CD", 1);

	*(QWORD*)&RdiJumpHook[3] = (QWORD)hkCreateEntityThunk;

	//InitalizeBytePatchMemory(&bp_CreateEntity[0], qwContainingFunc + 0x676, RdiJumpHook, 13);

	// g_pMemory->PatchBytes(&bp_CreateEntity[0]); //doesn't seem to do much

	InitalizeBytePatchMemory(&bp_CreateEntity[1], qwContainingFunc + 0x135, RdiJumpHook, 13);

	g_pMemory->PatchBytes(&bp_CreateEntity[1]); 
	
	/*HOOK_FUNC hf = { 0 };
	g_pMemory->HookFunc64((VOID*)(qwContainingFunc + 0x129), hkCreateEntityThunk, 25, &hf);*/

	HOOK_FUNC hf = { 0 };
	
	//g_pMemory->HookFunc64((VOID*)0x140607011, hkLoadWordBlacklist, 157, &hf); //caller
	g_pMemory->HookFunc64((VOID*)0x140606940, hkLoadWordBlacklistThunk, 20, &hf);//callee

	InitalizeNopMemory(&nop_HairColor, g_pMemory->FindPattern(NULL, "0F 29 22 49 8B CE"), 12)

	g_pMemory->NopMemory(&nop_HairColor);
}

void CreateRenderTarget()
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
	g_pDevice->CreateRenderTargetView(pBackBuffer, &render_target_view_desc, &g_pRenderTargetView);
	g_pDeviceContext->OMSetRenderTargets(1, &g_pRenderTargetView, NULL);
	pBackBuffer->Release();
}

void CreateStencilDescription()
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

HRESULT InitD3D11()
{
	HRESULT result = g_pSwapChain->GetDevice(IID_ID3D11Device, (void**)&g_pDevice); // i'm using IID_ID3D11Device for c compatiblity alternatively you can use __uuidof
	
	if (FAILED(result))
	{
		LOG("2B Hook Failed Initalization!\nCould not obtain a ID3D11Device pointer! HRESULT %x\n", result);
		return result;
	}

	result = g_pSwapChain->GetParent(IID_IDXGIFactory, (void**)&g_pFactory);

	if (FAILED(result))
	{
		LOG("2B Hook Failed Initalization!\nCould not obtain a IDXGIFactory pointer! HRESULT %x\n", result);
		return result;
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

void Setup(void)
{
#if defined(_DEBUG) || defined(VERBOSE)
	STACK_TIMER(timer);
	Log::AttachConsole(L"2B Hook Debug Console");
#endif
	srand((unsigned int)time(NULL));

	CRILogCallback = (CRILogCallbackFn)g_pMemory->FindPatternPtr64(NULL, "48 8B 35 ? ? ? ? 48 8B 1D ? ? ? ?", 3);
	CRILogCallback = CRILogCallbackConsole;//CRILogCallbackV2;//CRILogCallback;
	g_pEntityInfoList = (EntityInfoList*)g_pMemory->FindPatternPtr64(NULL, "4C 8B 4A 10 48 8D 15 ? ? ? ? 4D 8D 89 ? ? ? ?", 7);
	g_pLocalPlayerHandle = (EntityHandle*)g_pMemory->FindPatternPtr64(NULL, "45 33 F6 4C 8D 25 ? ? ? ? 4C 8D 05 ? ? ? ?", 6);
	g_pEmilHandle = (EntityHandle*)g_pMemory->FindPatternPtr64(NULL, "8B 15 ? ? ? ? 85 D2 74 59", 2);
	g_pYorhaManager = *(YorhaManager**)g_pMemory->FindPatternPtr64(NULL, "48 8B D1 48 8B 0D ? ? ? ? 48 8B 01", 6);
	g_pNPCManager = *(NPCManager**)g_pMemory->FindPatternPtr64(NULL, "75 B9 48 8B 0D ? ? ? ?", 5);
	g_pEnemyManager = *(EmBaseManager**)g_pMemory->FindPatternPtr64(NULL, "4C 89 A3 ? ? ? ? 48 8B 0D ? ? ? ?", 10);
	g_pUserManager = *(CUserManager**)g_pMemory->FindPatternPtr64(NULL, "74 1D 48 8B 0D ? ? ? ? 48 85 C9", 5);
	g_pWetObjectManager = (WetObjManager*)(g_pMemory->ReadPtr64(g_pMemory->FindPatternPtr64(NULL, "E8 ? ? ? ? 44 89 AF ? ? ? ? 48 C7 87 ? ? ? ? ? ? ? ?", 1) + 0x1B, 3));
	g_pCamera = (CCameraGame*)g_pMemory->FindPatternPtr64(NULL, "4C 8D 05 ? ? ? ? 4C 89 D1", 3);
	g_pSceneStateSystem = (CSceneStateSystem*)g_pMemory->FindPatternPtr64(NULL, "48 8D 0D ? ? ? ? E8 ? ? ? ? 90 EB 52", 3);
	g_pMemoryDevice = (CMemoryDevice*)g_pMemory->FindPatternPtr64(NULL, "48 8B DA 48 8D 0D ? ? ? ? E8 ? ? ? ?", 6);
	g_pDecreaseHealth[NOP_DAMAGE_ENEMY] = (byte*)g_pMemory->FindPattern(NULL, "29 BB ? ? ? ? 8B 83 ? ? ? ? 41 0F 48 C5");
	g_pDecreaseHealth[NOP_DAMAGE_WORLD] = (byte*)g_pMemory->FindPattern(NULL, "29 BB ? ? ? ? 8B 83 ? ? ? ? BD ? ? ? ? 0F 48 C5");
	CalculateLevel = (CalculateLevelFn)g_pMemory->FindPattern(NULL, "44 8B 91 ? ? ? ? 45 33 C9 41 8B C1 45 85 D2 7E");
	GetEntityFromHandle = (GetEntityFromHandleFn)g_pMemory->FindPattern(NULL, "40 53 48 83 EC ? 8B 11 85 D2 74"); // there is like 50 identical functions kek
	SetLocalPlayer = (SetLocalPlayerFn)g_pMemory->FindPatternPtr64(NULL, "E8 ? ? ? ? C6 43 48 00 81 7B ? ? ? ? ?", 1);
	ResetCamera = (ResetCameraFn)g_pMemory->FindPatternPtr64(NULL, "E8 ? ? ? ? 41 0F 28 07", 1);
	ChangePlayer = (ChangePlayerFn)g_pMemory->FindPattern(NULL, "40 53 48 83 EC 20 8B 05 ? ? ? ? 48 8B D9 48 8D 4C 24 ? 89 44 24 30 E8 ? ? ? ? 48 85 C0 74 31");
	FindSceneState = (FindSceneStateFn)g_pMemory->FindPatternPtr64(NULL, "E8 ? ? ? ? 83 CE 01", 1);
	SetSceneEntity = (SetSceneEntityFn)g_pMemory->FindPatternPtr64(NULL, "E8 ? ? ? ? 8B 87 ? ? ? ? 41 3B C4", 1);
	WetObjectManager_SetWet = (WetObjectManager_SetWetFn)g_pMemory->FindPatternPtr64(NULL, "E8 ? ? ? ? 4C 8D 05 ? ? ? ? 41 FF C6", 1);
	WetObjectManager_SetDry = (WetObjectManager_SetDryFn)g_pMemory->FindPattern(NULL, "48 85 D2 0F 84 ? ? ? ? 53 48 83 EC 20 83 3D ? ? ? ? ? 48 8B DA 0F 84 ? ? ? ? 48 89 74 24 ? 48 8D 35 ? ? ? ? 48 89 7C 24 ? 48 8B CE FF 15 ? ? ? ? 4C 8B 1D ? ? ? ? 44 8B 15 ? ? ? ? 45 33 C9 48 8D 0D ? ? ? ? 48 8D 3D ? ? ? ? 44 8B 01 45 85 C0 74 34 41 8B C0 C1 F8 08 0F B7 D0 41 3B D2 73 26 48 03 D2 41 8B 04 D3 41 33 C0 A9 ? ? ? ? 75 15 49 8B 44 D3 ? 48 85 C0 74 0B F6 40 2C 03 75 05 48 3B C3 74 0E 48 83 C1 04 41 FF C1 48 3B CF 7C B8 EB 1B");
	WetObjectManager_AddLocalEntity = (WetObjectManager_AddLocalEntityFn)g_pMemory->FindPatternPtr64(NULL, "E8 ? ? ? ? 44 89 AF ? ? ? ? 48 C7 87 ? ? ? ? ? ? ? ?", 1);
	DestroyBuddy = (DestroyBuddyFn)g_pMemory->FindPattern(NULL, "40 53 48 83 EC 30 48 C7 44 24 ? ? ? ? ? 48 8B D9 48 8B 01");
	FNV1Hash = (FNV1HashFn)g_pMemory->FindPatternPtr64(NULL, "E8 ? ? ? ? 85 C0 74 A3", 1);
	HashStringCRC32 = (HashStringCRC32Fn)g_pMemory->FindPattern(NULL, "40 57 83 C8 FF");
	GetConstructionInfo = (GetConstructorFn)g_pMemory->FindPattern(NULL, "33 D2 44 8B C9");
	CpkMount = (CpkMountFn)g_pMemory->FindPattern(NULL, "48 83 EC 28 44 8B C9");
	g_szDataDirectoryPath = (LPSTR)g_pMemory->FindPatternPtr64(NULL, "E8 ? ? ? ? 48 8D 3D ? ? ? ? 48 8B CF", 8);
	g_piMoney = (int*)g_pMemory->FindPatternPtr64(NULL, "48 8D 3D ? ? ? ? 48 8D 8D ? ? ? ?", 3);
	g_piExperience = (int*)g_pMemory->FindPatternPtr64(NULL, "8B 15 ? ? ? ? 75 06", 2);
	g_pDirectInput8 = *(IDirectInput8A**)g_pMemory->FindPatternPtr64(NULL, "48 8B 0D ? ? ? ? 48 85 C9 74 06 48 8B 01 FF 50 10 48 89 35 ? ? ? ? 48 89 35 ? ? ? ? 48 89 35", 3);
	g_pKeyboard = (Keyboard_t*)g_pMemory->FindPatternPtr64(NULL, "48 8B 0D ? ? ? ? 48 85 C9 74 06 48 8B 01 FF 50 10 48 89 35 ? ? ? ? 48 89 35 ? ? ? ? 89", 3);
	g_pMouse = (Mouse_t*)g_pMemory->FindPatternPtr64(NULL, "48 8D 0D ? ? ? ? 44 8B C3 E8 ? ? ? ?", 3);
	g_pGraphics = *(CGraphics**)g_pMemory->FindPatternPtr64(NULL, "48 8D 05 ? ? ? ? 48 83 C4 ? C3 CC CC CC CC CC CC CC CC 48 89 4C 24 ? 57", 3);
	g_pViewMatrix = (VMatrix*)g_pMemory->FindPatternPtr64(NULL, "0F 29 02 0F 28 2D ? ? ? ?", 6);
	//g_pSwapChain = *(IDXGISwapChain**)((*(byte**)g_pMemory->FindPatternPtr64(NULL, "48 89 35 ? ? ? ? 48 85 C9 74 ? 39 35 ? ? ? ? 74 ? 48 8B 01 BA ? ? ? ? FF 10 48 8B 0D ? ? ? ? 48 85 C9 74 ? 39 35 ? ? ? ? 74 ? 48 8B 01 BA ? ? ? ? FF 10 48 8B 0D ? ? ? ? 48 89 35 ? ? ? ? C7 05 ? ? ? ? ? ? ? ? 48 89 35 ? ? ? ? 48 85 C9 74 ? 39 35 ? ? ? ? 74 ? 48 8B 01 BA ? ? ? ? FF 10 48 8B 0D ? ? ? ? 48 85 C9 74 ? 39 35 ? ? ? ? 74 ? 48 8B 01 BA ? ? ? ? FF 10 48 8B 0D ? ? ? ? 48 89 35 ? ? ? ? C7 05 D8 ? ? ? ? ? ? ? ?", 3)) + 0xE0);
	g_pSwapChain = *g_pGraphics->m_Display.m_ppSwapChain;

	if (IsWindows10OrGreater()) // for some reason it causes a crash on windows 7
		g_pSecondarySwapChain = (IDXGISwapChain*)(*(byte**)((*(byte**)((*(byte**)((*(byte**)((byte*)g_pGraphics + 0x1B8)) + 0x140))) + 0x10)));// I have no idea what this swapchain is for, but it points to the right place

	g_pGraphicDevice = g_pGraphics->m_Display.m_pGraphicDevice;
	g_pAntiVSync = (byte*)g_pMemory->FindPattern(NULL, "0F 94 D2 45 31 C0 FF 50 40");
	g_pAntiFramerateCap_Sleep = (byte*)g_pMemory->FindPattern(NULL, "8B CA FF 15 ? ? ? ? 48 8D 4C 24 ?") + 2;
	g_pAntiFramerateCap_Spinlock = g_pAntiFramerateCap_Sleep + 0x48;
	g_pAntiFramerateCap_Test4 = (byte*)g_pMemory->FindPattern(NULL, "F6 05 ? ? ? ? ? 0F 29 74 24 ?");
	g_hWnd = *(HWND*)g_pMemory->FindPatternPtr64(NULL, "48 89 05 ? ? ? ? 48 85 C0 0F 84 ? ? ? ? 0F 57 C0", 3);
	
	LogOffsets();

	ExposeHiddenXInputFunctions();

	QueryProcessHeaps(&g_pHeaps, NULL);

	InitD3D11();

	g_pConfig->CreateConfig(NULL);
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
	SetUnhandledExceptionFilter(UnhandledExceptionHandler);
	g_pExceptionHandlers.push_back(UnhandledExceptionHandlerChild);

	//InitalizeNopMemory(nop_neon_scenery, g_pMemory->FindPattern(NULL, "45 8B FC 0F 29 44 08 ?"), 5);
	InitalizeNopMemory(&nop_Health[NOP_DAMAGE_ENEMY], g_pDecreaseHealth[NOP_DAMAGE_ENEMY], 6);
	InitalizeNopMemory(&nop_Health[NOP_DAMAGE_WORLD], g_pDecreaseHealth[NOP_DAMAGE_WORLD], 6);
	InitalizeNopMemory(&nop_Health[NOP_DAMAGE_WORLD], g_pDecreaseHealth[NOP_DAMAGE_WORLD], 6);
	InitalizeNopMemory(&nop_Framecap[NOP_FRAMECAP_SLEEP], g_pAntiFramerateCap_Sleep, 6);
	InitalizeNopMemory(&nop_Framecap[NOP_FRAMECAP_SPINLOCK], g_pAntiFramerateCap_Spinlock, 2);

	InitalizeBytePatchMemory(&bp_Framecap, g_pAntiFramerateCap_Test4, JmpFramecap, 7)
	InitalizeBytePatchMemory(&bp_NoTutorialDialogs, g_pMemory->FindPattern(NULL, "77 07 8B CA"), LocalJmp, 1)

	g_pConsole->Log(ImVec4(0.0f, 0.525f, 0.0f, 1.0f), "2B Hook Initalization Complete! GLHF");

	LOG("2B Hook Initalization Complete!\n");
}

void Unhook()
{
	Vars.Gameplay.bGodmode = false;
	Vars.Gameplay.bNoEnemyDamage = false;
	Vars.Gameplay.bNoWorldDamage = false;

	Features::ApplyHealthMods();

	g_pMemory->RestoreMemory(&bp_NoTutorialDialogs);
	g_pMemory->RestoreMemory(&bp_Framecap);
	g_pMemory->RestoreMemory(&nop_Framecap[NOP_FRAMECAP_SLEEP]);
	g_pMemory->RestoreMemory(&nop_Framecap[NOP_FRAMECAP_SPINLOCK]);
	g_pMemory->RestoreMemory(&nop_Health[NOP_DAMAGE_WORLD]);
	g_pMemory->RestoreMemory(&nop_Health[NOP_DAMAGE_ENEMY]);
	g_pMemory->RestoreMemory(&bp_save_file_io);

	delete g_pSetUnhandledExceptionFilterHook;
	delete g_pQueryPerformanceCounterHook;
	delete g_pClipCursorHook;
	delete g_pFactoryHook;
	delete g_pSwapChainHook;
	delete g_pDeviceContextHook;
	delete g_pKeyboardHook;
	delete g_pMouseHook;
	delete g_pMemory;

	ImGui_ImplDX11_Shutdown();

	SetWindowLongPtr(g_hWnd, GWLP_WNDPROC, (LONG_PTR)oWndProc);

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
		CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)Setup, NULL, NULL, NULL);
		break;
	case DLL_PROCESS_DETACH:
		Unhook();
		break;
	default:
		break;
	}
	return TRUE;
}