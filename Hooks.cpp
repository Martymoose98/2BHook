#include "Hooks.h"


PresentFn oPresent;
ResizeBuffersFn oResizeBuffers;
CreateSwapChainFn oCreateSwapChain;
DrawIndexedFn oDrawIndexed;
DrawFn oDraw;
PSSetShaderResourcesFn oPSSetShaderResources;
ClearRenderTargetViewFn oClearRenderTargetView;
QueryPerformaceCounterFn oQueryPerformanceCounter;
SetUnhandledExceptionFilterFn oSetUnhandledExceptionFilter;
ReleaseFn oKeyboardRelease;
AcquireFn oKeyboardAcquire;
GetDeviceStateFn oKeyboardGetDeviceState;
ReleaseFn oMouseRelease;
AcquireFn oMouseAcquire;
GetDeviceStateFn oMouseGetDeviceState;
SetCursorPosFn oSetCursorPos;
XInputGetStateFn oXInputGetState;
UpdateModelPartsFn oUpdateModelParts;
CreateEntityFn oCreateEntity;
MRubyLoadScriptFn oMRubyLoadScript;
CCameraGame_SetViewAnglesFn oCCameraGameSetViewAngles;
CCameraGame_MoveFn oCCameraGameMove;
CModelAnalyzer_CreateModelShaderModuleFn oCreateModelShaderModule;
WNDPROC oWndProc;


static ID3D11RasterizerState* pNorm, * pBias;
//static bool hookupdate = false;
//VirtualTableHook m_ctxhook;
//VirtualTableHook m_shaderhook[MAX_MATERIAL_TEXTURES];

// WIP
void CModelShaderModule_Draw(CModelShaderModule* pThis, CModelEntryData* pData)
{
	typedef __int64 (* OriginalFn)(CModelShaderModule*, CModelEntryData*);
	typedef __int64 (*Original2Fn)(CModelShaderModule*, CModelShader*, CModelExtendWork*);
	//((OriginalFn)m_shaderhook.GetFunctionAddress(3))(pThis, pData);
	//pThis->Draw(pData);

	//pThis->ApplyExternalForces(pData->m_pWork->m_pModelShaders, pData->m_pWork->m_pNext->m_pModelExtend2);
	//pThis->Update(pData->m_pWork->m_pModelShaders, pData->m_pWork->m_pNext->m_pModelExtend2, 2);
	//pThis->ApplyExternalForces(pData->m_pWork->m_pModelShaders, pData->m_pWork->m_pNext->m_pModelExtend2);
	CModelMatrixTable* pTbl = pData->m_pMatrices[0];

	for (int i = 0; i < pTbl->m_nMatrices; ++i)
	{

		Matrix4x4* pBoneMat = (Matrix4x4*)pData->m_pConstBuffer[i]->m_pResources;

		pBoneMat->Transpose();

		Matrix4x4 RotM;

		RotM.InitAxisAngle(pData->m_pWork->m_pModelExtend2->m_pParent->m_matTransform.GetAxis(RIGHT), M_PI_F / 2.0f);

		*pBoneMat = RotM * (*pBoneMat);

		pBoneMat->Transpose();
	}

	//((Original2Fn)m_shaderhook[0].GetFunctionAddress(2))(pThis, pData->m_pWork->m_pModelShaders, pData->m_pWork->m_pModelExtend2);

	//((OriginalFn)m_shaderhook[0].GetFunctionAddress(3))(pThis, pData);
	//pThis->Draw(pData);

	// Exception thrown at 0x000000014500537C in NieRAutomata.exe: 
	// 0xC0000005: Access violation reading location 0xFFFFFFFFFFFFFFFF.
	//if (pData && GetAsyncKeyState(VK_END) & 1)
	//{
	//	//14093FF50
	//	typedef __int64(*R_DrawModelFn)(CModelDrawContext* a1,
	//		CGraphics* pGraphics, CModelMatrixTable* pModelMatrixTable,
	//		CModelInstanceParam* pModelInstanceParam, CConstantBufferInfo* a5);

	//	((R_DrawModelFn)(0x14093FF50))(*pData->m_pCtx, g_pGraphics, *pData->m_pMatrices,
	//		(pData->m_pParams) ? (*pData->m_pParams) : NULL, (CConstantBufferInfo*)&pData->m_iStartIndex);
	//}
}

BOOL DrawIndexedPrimitive(CGraphicContextDx11* pThis, RenderInfo* pInfo)
{
	ID3D11RasterizerState* pRS;
	ID3D11DepthStencilState* pDSS;
	UINT uStencilRef;
	CModelDrawData* pDrawData;
	BOOLEAN bChams = FALSE;

	BOOL b = (*(BOOL(*)(void*, ID3D11DeviceContext*))(0x140941130))(&pThis->m_RenderContext, pThis->m_pContext);

	if (g_pCamera && g_pCamera->m_pCamEntity && Vars.Visuals.bChams)
	{
		pDrawData = g_pCamera->m_pCamEntity->m_Work.m_pModelData->m_pDrawData;

		for (int i = 0; i < pDrawData->m_nBatches; ++i)
		{
			CBatch* pBatch = &pDrawData->m_pBatches[i];

			if (pInfo->m_uVertexCount == pBatch->m_nVertices && pInfo->m_uIndexCountPerInstance == pBatch->m_nPrimitives)
			{
				bChams = TRUE;
				break;
			}
		}
	}

	if (bChams)
	{
		pThis->m_pContext->OMGetDepthStencilState(&pDSS, &uStencilRef);
		pDSS->Release();
		pThis->m_pContext->RSGetState(&pRS);
		pRS->Release();

		pThis->m_pContext->OMSetDepthStencilState(g_pDepthStencilStates[DISABLED], 1);

		if (Vars.Visuals.iChamType)
		{
			pThis->m_pContext->PSSetShader(g_pRed, NULL, 0);
		}
		else
		{
			pThis->m_pContext->PSSetShaderResources(0, 1, &g_pRedSRV);
			//pThis->m_pContext->PSSetSamplers(0, 1, &g_pSamplerState);
		}

		(*(BOOL(*)(void*, ID3D11DeviceContext*, RenderInfo*))(0x140941380))(&pThis->m_RenderContext, pThis->m_pContext, pInfo);

		pThis->m_pContext->OMSetDepthStencilState(g_pDepthStencilStates[READ_NO_WRITE], 1);
		pThis->m_pContext->RSSetState(pBias);

		if (Vars.Visuals.iChamType)
		{
			pThis->m_pContext->PSSetShader(g_pGreen, NULL, 0);
		}
		else
		{
			pThis->m_pContext->PSSetShaderResources(0, 1, &g_pGreenSRV);
			//pThis->m_pContext->PSSetSamplers(0, 1, &g_pSamplerState);
		}

		(*(BOOL(*)(void*, ID3D11DeviceContext*, RenderInfo*))(0x140941380))(&pThis->m_RenderContext, pThis->m_pContext, pInfo);
		pThis->m_pContext->RSSetState(pRS);
		//pThis->m_pContext->RSSetState(pNorm);
		pThis->m_pContext->OMSetDepthStencilState(pDSS, uStencilRef);

		//b = (*(BOOL(*)(void*, ID3D11DeviceContext*))(0x140941130))(&pThis->m_RenderContext, pThis->m_pContext);
	}

	return (b) ? (*(BOOL(*)(void*, ID3D11DeviceContext*, RenderInfo*))(0x140941380))(&pThis->m_RenderContext, pThis->m_pContext, pInfo) : b;
}

HRESULT hkPresent(IDXGISwapChain* pThis, UINT SyncInterval, UINT Flags)
{
	ImGui::GetIO().MouseDrawCursor = g_pMenu->IsOpen();

	//g_pGraphics->m_iTimeStep = 0x100;

	g_pQueryPerformanceCounterHook->Unhook();	//unhook cause it has the potential to fuck with imgui because it calls QueryPerformanceCounter, then rehook it

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();

	ImGui::NewFrame();

	g_pQueryPerformanceCounterHook->Rehook();

	// FIXME: change when other Pl entity's are reversed
	g_pLocalPlayer = static_cast<Pl0000*>(GetEntityFromHandle2(g_pLocalPlayerHandle));

	Pl0000* pCameraEnt = static_cast<Pl0000*>(g_pCamera->m_pCamEntity);

#if defined(_DEBUG) && defined(DENUVO_STEAM_BUILD)

	if (!hookupdate)
	{
		hookupdate = true;
		m_ctxhook.Initialize((ULONG_PTR**)g_pGraphics->m_pContext);
		m_ctxhook.HookFunction((ULONG_PTR)DrawIndexedPrimitive, 46);

		D3D11_RASTERIZER_DESC nrasterizer_desc;
		ZeroMemory(&nrasterizer_desc, sizeof(nrasterizer_desc));
		nrasterizer_desc.FillMode = D3D11_FILL_SOLID;
		//nrasterizer_desc.CullMode = D3D11_CULL_BACK; //flickering
		nrasterizer_desc.CullMode = D3D11_CULL_NONE;
		nrasterizer_desc.FrontCounterClockwise = false;
		nrasterizer_desc.DepthBias = 0;
		nrasterizer_desc.SlopeScaledDepthBias = 0.0f;
		nrasterizer_desc.DepthBiasClamp = 0.0f;
		nrasterizer_desc.DepthClipEnable = true;
		nrasterizer_desc.ScissorEnable = false;
		nrasterizer_desc.MultisampleEnable = false;
		nrasterizer_desc.AntialiasedLineEnable = false;
		g_pDevice->CreateRasterizerState(&nrasterizer_desc, &pNorm);

		D3D11_RASTERIZER_DESC rasterizer_desc;
		ZeroMemory(&rasterizer_desc, sizeof(rasterizer_desc));
		rasterizer_desc.FillMode = D3D11_FILL_SOLID;
		rasterizer_desc.CullMode = D3D11_CULL_FRONT; // D3D11_CULL_NONE;
		rasterizer_desc.FrontCounterClockwise = false;
		float bias = 1000.0f;
		float bias_float = -bias;
		bias_float /= 10000.0f;
		rasterizer_desc.DepthBias = (INT)(*(UINT*)&bias_float / (1.f / 8388608.f));
		rasterizer_desc.SlopeScaledDepthBias = 0.0f;
		rasterizer_desc.DepthBiasClamp = 0.0f;
		rasterizer_desc.DepthClipEnable = true;
		rasterizer_desc.ScissorEnable = false;
		rasterizer_desc.MultisampleEnable = false;
		rasterizer_desc.AntialiasedLineEnable = false;
		g_pDevice->CreateRasterizerState(&rasterizer_desc, &pBias);
	}

	if (GetAsyncKeyState(VK_F8) & 1)
		m_ctxhook.Unhook();

	if (GetAsyncKeyState(VK_F9) & 1)
		m_ctxhook.Rehook();

	if (GetAsyncKeyState(VK_OEM_3) & 1)
	{
		for (size_t i = 0; i < g_pEntityInfoList->m_dwSize; ++i)
		{
			CEntityInfo* pInfo = g_pEntityInfoList->m_pItems[i].second;

			if (pInfo)
				LOG("Entity: %s Handle: %x Address: %llx\n", pInfo->m_szEntityType, pInfo->m_hEntity, pInfo->m_pEntity);
		}
	}

	if (GetAsyncKeyState(VK_F1) & 1)
		LOG("Animation ID: %d\n", ++Vars.Gameplay.iAnimation);

	if (GetAsyncKeyState(VK_F2) & 1)
		LOG("Animation ID: %d\n", --Vars.Gameplay.iAnimation);

	if (GetAsyncKeyState(VK_F4) & 1)
	{
		if (g_pLocalPlayer)
		{
			m_shaderhook.Initialize((ULONG_PTR**)g_pLocalPlayer->m_Work.m_pModelShaders[10].m_pShader);
			m_shaderhook.HookFunction((ULONG_PTR)CModelShaderModule_Draw, 3);
		}
#if 0
		typedef BOOL(*CreateModelShaderModuleFn)(CModelShader* pEnt_390, CMaterialInfo* pInfo, void* pModelWork);
		static bool bInit = false;
		static CMaterial* pReplace;

		CMaterialInfo info = { 0 };

		if (!bInit)
		{
			bInit = true;
			pReplace = LoadMaterial("X:\\Documents\\Visual Studio 2017\\Projects\\2B Hook\\2B Hook\\Hair.dds");
		}

		info.m_pModelAnalyzer = (void*)0x141101C60;
		info.m_pMaterial = pReplace;
		info.m_bUnknownsExist = *MakePtr(int*, g_pLocalPlayer->m_pModelData, 0xE0) > 0;
		((CreateModelShaderModuleFn)(0x140871EF0))(&g_pLocalPlayer->m_pModelShaders[10], &info, &g_pLocalPlayer->m_ModelExtendWork);
		g_pLocalPlayer->m_pModelShaders[10].m_pMaterial = info.m_pMaterial;
#endif
	}

	if (GetAsyncKeyState(VK_F11) & 1)
	{
		Features::TeleportAllEnemiesToEncirclePoint(pCameraEnt->m_vPosition, 50);
	}
#endif

	// wip color isn't updating
#if 0
	if (Vars.Gameplay.bRainbowModel)
	{
		Vector3 rgb;

		Vars.Gameplay.flModelTintHue += 0.00005f;

		if (Vars.Gameplay.flModelTintHue > 1.f)
			Vars.Gameplay.flModelTintHue = 0.0f;

		ImGui::ColorConvertHSVtoRGB(Vars.Gameplay.flModelTintHue, 1.0f, 1.f, rgb[0], rgb[1], rgb[2]);
		g_pLocalPlayer->m_pModelInfo->m_vTint = Vector4(rgb);
	}
#endif

	//bool bDerived = RTTI::DerivesFrom(pCameraEnt, typeid(Pl0000));

	if (pCameraEnt)
	{
		// FIXME: HACK move code to HOOK
		//if (!hookupdate)
		//{
		//	hookupdate = true;

		//	for (int i = 0; i < g_pLocalPlayer->m_Work.m_nMaterialShaders; ++i)
		//	{
		//		m_shaderhook[i].Initialize((ULONG_PTR**)g_pLocalPlayer->m_Work.m_pModelShaders[i].m_pShader);
		//		m_shaderhook[i].HookFunction((ULONG_PTR)CModelShaderModule_Draw, 3);
		//	}		
		//}

		if (Vars.Gameplay.bRainbowHair)
		{
			Vars.Gameplay.flModelTintHue += 0.005f;

			if (Vars.Gameplay.flModelTintHue > 1.f)
				Vars.Gameplay.flModelTintHue = 0.0f;

			int idx = GetModelMeshIndex(&pCameraEnt->m_Work, "Hair");

			if (idx != -1)
			{
				pCameraEnt->m_Work.m_pMeshes[idx].m_vColor = Color::FromHSB(Vars.Gameplay.flModelTintHue, 1.f, 1.f);
				pCameraEnt->m_Work.m_pMeshes[idx].m_bUpdate = TRUE;
				pCameraEnt->m_Work.m_pMeshes[idx].m_bShow = TRUE;
			}
		}

		if (Vars.Gameplay.bRainbowPod)
		{
			Pl0000* pPod = static_cast<Pl0000*>(GetEntityFromHandle(&pCameraEnt->m_hPod));

			if (pPod)
			{
				Vars.Gameplay.flPodTintHue += 0.005f;

				if (Vars.Gameplay.flPodTintHue > 1.f)
					Vars.Gameplay.flPodTintHue = 0.0f;

				int idx = GetModelMeshIndex(&pCameraEnt->m_Work, "Hair");

				if (idx != -1)
				{
					pPod->m_Work.m_pMeshes[idx].m_vColor = Color::FromHSB(Vars.Gameplay.flPodTintHue, 1.f, 1.f);
					pPod->m_Work.m_pMeshes[idx].m_bUpdate = TRUE;
					pPod->m_Work.m_pMeshes[idx].m_bShow = TRUE;
				}
			}
		}
	}

	if (Vars.Gameplay.bBalanceEnemyLevels)
		Features::BuffEnemies();

	Features::ApplyHealthMods();

	if (Vars.Misc.bAntiFramerateCap)
	{
		PatchBytes(&bp_Framecap);
		NopMemory(&nop_Framecap[NOP_FRAMECAP_SLEEP]);
	}
	else
	{
		RestoreMemory(&bp_Framecap);
		RestoreMemory(&nop_Framecap[NOP_FRAMECAP_SLEEP]);
	}

	if (Vars.Gameplay.bNoTutorialDialogs)
		PatchBytes(&bp_NoTutorialDialogs);
	else
		RestoreMemory(&bp_NoTutorialDialogs);

	g_pOverlay->Render(true);

	g_pMenu->Draw(ImVec2(875, 600));

	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	//COtManager::GetGraphicCommand(176i64);
	return oPresent(pThis, (Vars.Misc.bAntiVSync) ? 0 : SyncInterval, Flags);
}

HRESULT CreateRenderTarget(void);

HRESULT hkResizeBuffers(IDXGISwapChain* pThis, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags)
{
	LOG("pSwapChain %p | pWindowedSwapChain %p\n", g_pGraphics->m_Display.m_pSwapChain, g_pGraphics->m_Display.m_pWindowedSwapChain);

	if (g_pRenderTargetView)
	{
		g_pDeviceContext->OMSetRenderTargets(0, NULL, NULL);
		g_pRenderTargetView->Release();
	}

	HRESULT hr = oResizeBuffers(pThis, BufferCount, Width, Height, NewFormat, SwapChainFlags);

	if (SUCCEEDED(hr))
	{
		ImGui_ImplDX11_InvalidateDeviceObjects();
		CreateRenderTarget();
	}
	return hr;
}

HRESULT hkCreateSwapChain(IDXGIFactory* pThis, IUnknown* pDevice, DXGI_SWAP_CHAIN_DESC* pDesc, IDXGISwapChain** ppSwapChain)
{
	pDesc->BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	HRESULT hr = oCreateSwapChain(pThis, pDevice, pDesc, ppSwapChain);

	if (SUCCEEDED(hr))
	{
		g_pSwapChain = *ppSwapChain;
		g_pSwapChainHook->Relocate((ULONG_PTR**)g_pSwapChain);

		ImGui_ImplDX11_InvalidateDeviceObjects();
	}

	return hr;
}

HRESULT hkCreateSwapChainForHwnd(IDXGIFactory2* pThis, IUnknown* pDevice, DXGI_SWAP_CHAIN_DESC* pDesc, IDXGISwapChain** ppSwapChain)
{
	return E_NOTIMPL;
}

void hkPSSetShaderResources(ID3D11DeviceContext* pThis, UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView* const* ppShaderResourceViews)
{
	D3D11_SHADER_RESOURCE_VIEW_DESC Desc;

	for (UINT i = 0; i < NumViews; ++i)
	{
		ID3D11ShaderResourceView* pShaderResourceView = ppShaderResourceViews[i];

		if (pShaderResourceView)
		{
			pShaderResourceView->GetDesc(&Desc);

			if ((Desc.ViewDimension == D3D11_SRV_DIMENSION_BUFFER) || (Desc.ViewDimension == D3D11_SRV_DIMENSION_BUFFEREX))
				continue;//Skip buffer resources
		}
	}

	oPSSetShaderResources(pThis, StartSlot, NumViews, ppShaderResourceViews);
}

void hkDraw(ID3D11DeviceContext* pThis, UINT VertexCount, UINT StartVertexLocation)
{
	oDraw(pThis, VertexCount, StartVertexLocation);
}

/*
* Any stride bigger than 0x1C (28) is the ui (so far)
*/
void hkDrawIndexed(ID3D11DeviceContext* pThis, UINT IndexCount, UINT StartIndexLocation, INT BaseVertexLocation)
{
	ID3D11Buffer* pVertexBuffers[8];
	UINT Strides[8];
	UINT VertexBufferOffsets[8];

	pThis->IAGetVertexBuffers(0, 8, pVertexBuffers, Strides, VertexBufferOffsets);

	for (int i = 0; i < 8; ++i)
	{
		if (pVertexBuffers[i])
		{
			pVertexBuffers[i]->Release();
			pVertexBuffers[i] = NULL;
		}
	}

	if (Vars.Misc.bWireframe && Strides[0] == 28)
		g_pDeviceContext->RSSetState(g_pRenderWireframeState); // wireframe

	oDrawIndexed(pThis, IndexCount, StartIndexLocation, BaseVertexLocation);
}

void hkClearRenderTargetView(ID3D11DeviceContext* pThis, ID3D11RenderTargetView* pRenderTargetView, const FLOAT ColorRGBA[4])
{
	oClearRenderTargetView(pThis, pRenderTargetView, ColorRGBA);
}

ULONG hkKeyboardRelease(IDirectInputDevice8A* pThis)
{
	g_pKeyboardHook->Unhook();

	ULONG uReferences = oKeyboardRelease(pThis);

	g_pKeyboardHook->Rehook();

	return uReferences;
}

HRESULT hkKeyboardAcquire(IDirectInputDevice8A* pThis)
{
	g_pKeyboardHook->Unhook();

	HRESULT hr = oKeyboardAcquire(pThis);

	g_pKeyboardHook->Rehook();

	return hr;
}

HRESULT hkKeyboardGetDeviceState(IDirectInputDevice8A* pThis, DWORD cbData, LPVOID lpvData)
{
	g_pKeyboardHook->Unhook();

	HRESULT hr = oKeyboardGetDeviceState(pThis, cbData, lpvData);

	g_pKeyboardHook->Rehook();

	return g_pMenu->KeyboardHandler(hr, lpvData, cbData);
}

ULONG hkMouseRelease(IDirectInputDevice8A* pThis)
{
	g_pMouseHook->Unhook();

	ULONG uReferences = oMouseRelease(pThis);

	g_pMouseHook->Rehook();

	return uReferences;
}

HRESULT hkMouseAcquire(IDirectInputDevice8A* pThis)
{
	g_pMouseHook->Unhook();

	HRESULT hr = oMouseAcquire(pThis);

	g_pMouseHook->Rehook();

	return hr;
}

HRESULT hkMouseGetDeviceState(IDirectInputDevice8A* pThis, DWORD cbData, LPVOID lpvData)
{
	//if (Vars.Menu.bOpened && Vars.Menu.bIgnoreInputWhenOpened)
	//	return DIERR_INPUTLOST;

	if (g_pMenu->MouseHandler(lpvData, cbData) == DIERR_INPUTLOST)
	{
		return DIERR_INPUTLOST;
	}

	g_pMouseHook->Unhook();

	HRESULT hr = oMouseGetDeviceState(pThis, cbData, lpvData);

	g_pMouseHook->Rehook();
	

	//memcpy(&Vars.Menu.Input.MouseState, lpvData, cbData);

	return hr;
}

bool hkMRubyLoadScript(MrubyImpl* pThis, MrubyScript* pScript)
{
	CCONSOLE_DEBUG_LOG(ImColor(0xff8f20d4), "FUCK");
	g_pConsole->Log(ImColor(0.5f, 0.f, 0.7f), "Script %x loaded!", pScript->m_dwHash);
	return oMRubyLoadScript(pThis, pScript);
	//if (pThis->m_pNext)
	//{
	//	g_pConsole->Warn("Next is valid!");
	//
	//	return oMRubyLoadScript(pThis, pScript);
	//}
	//return false;
}

// ORIG FUNCTION SIG: 40 53 48 81 EC ? ? ? ? F6 05
BOOL hkCCameraGameSetViewAngles(CCameraGame* pThis)
{
	/* failed aimbot code

	int iTarget = -1;
	float best = FLT_MAX;

for (int i = 0; i < g_pEnemyManager->m_handles.m_count; ++i)
{
	Pl0000* pCur = GetEntityFromHandle(&g_pEnemyManager->m_handles.m_pItems[i]);

	if (pCur && pCur->m_iHealth > 0)
	{
		float fov = g_pCamera->m_vPosition.DistTo(pCur->m_vPosition);

		if (fov < best)
		{
			iTarget = i;
			best = fov;
		}
	}
}

if (iTarget != -1)
{
	Pl0000* pTarget = GetEntityFromHandle(&g_pEnemyManager->m_handles.m_pItems[iTarget]);
	Pl0000* pPod = GetEntityFromHandle(&g_pLocalPlayer->m_hPod);
	pThis->m_Transform.m_vSource = pTarget->m_pBones[GetBoneIndex(pTarget->m_pModelData, BONE_SPINE1)].m_vPosition;
	pThis->m_Transform.m_vTarget = g_pCamera->m_vPosition;
	pThis->m_Transform.m_vUp = pPod->m_matTransform.GetAxis(UP) * -1;

	((void(*)(CCameraGame*))(0x140832210))(pThis);
}
return TRUE;*/
	if (Vars.Visuals.flFov == 0.0f)
		Vars.Visuals.flFov = RADTODEG(pThis->m_flFov);

	pThis->m_flFov = DEGTORAD(Vars.Visuals.flFov);

	if (pThis->m_pCamEntity && Vars.Misc.bFirstperson)
	{
		Vector3 vForward;

		Features::Firstperson(pThis->m_pCamEntity);
		Math::AngleVectors(pThis->m_vViewangles, &vForward);

		short sHead = GetBoneIndex(pThis->m_pCamEntity->m_Work.m_pModelData, BONE_HEAD);

		Vector3Aligned& vHeadPos = pThis->m_pCamEntity->m_pBones[sHead].m_vPosition;

		pThis->m_vSource = vHeadPos + pThis->m_pCamEntity->m_matTransform.GetAxis(FORWARD) * .095f;

		pThis->m_vTarget = vHeadPos + vForward * 4.f;

		Vars.Misc.bFirstpersonOld = Vars.Misc.bFirstperson;

		CameraGame_SetLookAt(pThis);
		return TRUE;
	}

	Features::Thirdperson(pThis->m_pCamEntity);

	Vars.Misc.bFirstpersonOld = Vars.Misc.bFirstperson;

	return oCCameraGameSetViewAngles(pThis);
}

// ORIG FUNCTION SIG: 40 53 57 48 83 EC 68 48 8B D9
void* hkCCameraGameMove(CCameraGame* pThis)
{
	return oCCameraGameMove(pThis);
}

CModelShaderModule* hkCreateModelShaderModule(CModelAnalyzer* pThis, CModelShader* pShader, DWORD* a3, CHeapInfo* a4)
{
	// Changes the shader vvv causes artifacts
	//pShader->m_pMaterial->m_szShaderName = "CLT00_XXXXX";
	CTextureDescription Desc;

#if 0
	static CTargetTexture* pTex = CreateTextureR("C:\\Users\\marty\\source\\repos\\2B Hook\\2B Hook\\Kim Jong Un Flood.dds", Desc);
	int nTexture = pThis->FindTextureIndexByName("g_AlbedoMap");

	uint32 crc = pShader->m_pMaterial->m_TextureIds[0];//HashStringCRC32("g_AlbedoMap", strlen("g_AlbedoMap"));
	pShader->m_pMaterial->m_TextureIds[0] = pShader->m_pMaterial->m_TextureIds[1];
	pShader->m_pMaterial->m_TextureIds[1] = crc;

	CModelShaderModule* pModelShader = oCreateModelShaderModule(pThis, pShader, a3, a4);

	g_pConsole->Log(ImVec4(0.0f, 0.525f, 0.0f, 1.0f), "Created Shader:\n\t Material: %s\n\t Shader: %s\n\t Technique: %s\n",
		pShader->m_pMaterial->m_szName, pShader->m_pMaterial->m_szShaderName, pShader->m_pMaterial->m_szTechniqueName);

#else
	CModelShaderModule* pModelShader = oCreateModelShaderModule(pThis, pShader, a3, a4);
#endif
	return pModelShader;
}

// TODO: new hook doesn't need last param and shadow space in thunk
// FIXME: entities that are not loaded in before creation will crash the game still!
void* hkCreateEntity(void* pUnknown, CEntityInfo* pInfo, unsigned int uObjectId, int iGroupId, CHeapInfo* pHeapInfo)
{
	CConstructionInfo<void>* pConstruct = GetConstructionInfo(uObjectId);
	void* pEntity = NULL;

	// pEntity = oCreateEntity(pUnknown, pInfo, uObjectId, iGroupId, pHeapInfo); //0x1401A2B40 old denuvo
	if (Vars.Gameplay.SpawnBlacklist.empty() || std::find(Vars.Gameplay.SpawnBlacklist.cbegin(), Vars.Gameplay.SpawnBlacklist.cend(), pConstruct->m_szName) == Vars.Gameplay.SpawnBlacklist.cend())
		pEntity = oCreateEntity(pUnknown, pInfo, uObjectId, iGroupId);

	if (!pEntity)
		g_pConsole->Warn("Failed to create %s -> %s (ObjectId = %x, SetType %x)\n", pInfo->m_szEntityType, pConstruct->m_szName, uObjectId, pInfo->m_uSetType);
	else
		g_pConsole->Log(ImVec4(0.0f, 0.525f, 0.0f, 1.0f), "Created %s -> %s (ObjectId = %x, SetType %x) Base %llx\n", (pInfo->m_szEntityType) ? pInfo->m_szEntityType : "EntityLayout", pConstruct->m_szName, uObjectId, pInfo->m_uSetType, pEntity);

	return pEntity;
}

BOOL hkLoadWordBlacklist(BannedWordChecker* pThis, __int64 thisrdx, QWORD* thisr8, const char* szBlacklistName)
{
#if 0
	QWORD size;
	LPCVOID src;
	LPVOID dst;
	BYTE current_byte;
	PBYTE pData, lpDataEnd, lpDataStart;
	UINT  dwDataEndOffset, dwDataStartOffset, length, dwWordByteLength, Index;
	UINT dwNextDataStartOffset = 4;
	DWORD i = 0;

	if (*thisr8)
		size = (*(QWORD(*)(QWORD*, const char*))(0x1409291C0))(thisr8, szBlacklistName);
	else
		size = 0;

	pThis->m_qwBufferSize = size;

	if (size < 1)
	{
		(*(void(*)(BannedWordChecker*))(0x140606AC0))(pThis);  //FreeBannedWordChecker
		return FALSE;
	}

	src = (*(LPCVOID(*)(QWORD*, const char*))(0x140928FD0))(thisr8, szBlacklistName);

	if (!src)
	{
		(*(void(*)(BannedWordChecker*))(0x140606AC0))(pThis);  //FreeBannedWordChecker
		return FALSE;
	}

	dst = (*(AllocHeapMemoryFn)(0x140920200))(size, (CHeapInstance**)0x14147B490);

	pThis->m_pBuffer = dst;

	if (!dst)
	{
		(*(void(*)(BannedWordChecker*))(0x140606AC0))(pThis);  //FreeBannedWordChecker
		return FALSE;
	}

	memcpy(dst, src, size);

	pThis->m_dwWordCount = *(DWORD*)pThis->m_pBuffer;

	if (pThis->m_dwWordCount < 1)
	{
		(*(void(*)(BannedWordChecker*))(0x140606AC0))(pThis);  //FreeBannedWordChecker
		return FALSE;
	}

	pThis->m_pEntries = (BannedWordChecker::WordEntry*)(*(AllocHeapMemoryFn)(0x140920200))(16 * pThis->m_dwWordCount, (CHeapInstance**)0x14147B490);

	if (!pThis->m_pEntries)
	{
		(*(void(*)(BannedWordChecker*))(0x140606AC0))(pThis);  //FreeBannedWordChecker
		return FALSE;
	}

	if (pThis->m_dwWordCount > 0)
	{
		do
		{
			dwDataStartOffset = dwNextDataStartOffset;
			dwDataEndOffset = dwNextDataStartOffset + 4;
			lpDataStart = (BYTE*)pThis->m_pBuffer + dwDataStartOffset;
			lpDataEnd = (BYTE*)pThis->m_pBuffer + dwDataEndOffset;
			pThis->m_pEntries[i].dwLength = *(DWORD*)lpDataStart;
			dwWordByteLength = pThis->m_pEntries[i].dwLength * sizeof(WCHAR);
			dwNextDataStartOffset = dwWordByteLength + dwDataEndOffset;

			if (dwWordByteLength)
			{
				pData = lpDataStart;
				Index = lpDataEnd - lpDataStart;
				length = dwWordByteLength;
				do
				{
					current_byte = (pData++)[Index];
					*(pData - 1) = current_byte - 19;
				} while (--length);
			}
			*((WCHAR*)&lpDataStart[dwWordByteLength]) = 0; 	//*((WCHAR*)(lpDataStart + dwWordByteLength)) = 0;
			pThis->m_pEntries[i++].lpszBannedWord = (LPWSTR)lpDataStart;
		} while (i < pThis->m_dwWordCount);
	}

	Features::DisableWordBlacklist(pThis, NULL);

	return TRUE;
#endif

	Features::DisableWordBlacklist(pThis, NULL);
	return TRUE;
}

// for replacing the the startup picture but needs to be run really early to work
// doesn't get run in time
// g_pOleLoadPictureHook = new ImportTableHook("oleaut32.dll", "OleLoadPicture", (LPCVOID)hkOleLoadPicture);
// if you find a way to make it work you need this code ^^^^ (proxy dll)
HRESULT hkOleLoadPicture(LPSTREAM lpStream, LONG lSize, BOOL fRunmode, REFIID riid, LPVOID* lplpvObj)
{
	DWORD dwBytesRead;
	HANDLE hFile = CreateFileA("X:\\Pictures\\Anime\\4c165dbb0f55958a27312bff6c2d1b8b86d506d8.jpg", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile != INVALID_HANDLE_VALUE)
	{
		DWORD dwSize = GetFileSize(hFile, NULL);
		HGLOBAL hPicture = GlobalAlloc(GMEM_ZEROINIT, dwSize);

		if (hPicture)
		{
			ReadFile(hFile, hPicture, dwSize, &dwBytesRead, NULL);

			if (lpStream)
			{
				lpStream->Release();
				lpStream = NULL;
			}

			if (SUCCEEDED(CreateStreamOnHGlobal(hPicture, FALSE, &lpStream)))
			{
				HGLOBAL hOriginalPicture = (HGLOBAL*)0x141584E78; // FIXME: Untested and old version offset 

				if (hOriginalPicture)
				{
					GlobalFree(hOriginalPicture);
					hOriginalPicture = hPicture;
				}
			}
		}
		CloseHandle(hFile);
	}

	return ((OleLoadPictureFn)(g_pOleLoadPictureHook->GetOriginalFunction()))(lpStream, lSize, fRunmode, riid, lplpvObj);
}

void hkSaveFileIO(CSaveDataDevice* pSavedata)
{
	if (!pSavedata)
		return;

	switch (pSavedata->dwFlags)
	{
	case SAVE_FLAGS_READ_SLOTS:
		ReadSaveSlots(pSavedata);
		return;
	case SAVE_FLAGS_READ:
		Vars.Misc.bLoading = true;
		ReadSaveData(pSavedata);
		Vars.Misc.nSlot = pSavedata->nSlot;
		return;
	case SAVE_FLAGS_WRITE:
		WriteSaveData(pSavedata);
		return;
	case SAVE_FLAGS_DELETE:
		DeleteSaveData(pSavedata);
		return;
	}
	Vars.Misc.bLoading = false;
}

// still shit
BOOL hkQueryPerformanceCounter(LARGE_INTEGER* lpPerformanceCount)
{
	static LARGE_INTEGER liPreviousCount, liFakeCount;
	LARGE_INTEGER liCurrentCount;

	BOOL result = oQueryPerformanceCounter(&liCurrentCount);

	if (Vars.Gameplay.bSpeedhack)
	{
		liFakeCount.QuadPart += liCurrentCount.QuadPart + (ULONGLONG)((double)(liCurrentCount.QuadPart - liPreviousCount.QuadPart) * Vars.Gameplay.flSpeedMultiplier);
		lpPerformanceCount->QuadPart = liFakeCount.QuadPart;
		liPreviousCount.QuadPart = liCurrentCount.QuadPart;
	}
	else
	{
		lpPerformanceCount->QuadPart = liCurrentCount.QuadPart;
	}

	return result;
}

LPTOP_LEVEL_EXCEPTION_FILTER hkSetUnhandledExceptionFilter(LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter)
{
	if (lpTopLevelExceptionFilter != &UnhandledExceptionHandler)
	{
		auto& it = std::find(g_pExceptionHandlers.cbegin(), g_pExceptionHandlers.cend(), lpTopLevelExceptionFilter);

		if (it == g_pExceptionHandlers.cend())
			g_pExceptionHandlers.push_back(lpTopLevelExceptionFilter);
	}
	return &UnhandledExceptionHandler;
}

BOOL hkSetCursorPos(int X, int Y)
{
	if (g_pMenu->IsOpen())
		return FALSE;

	return oSetCursorPos(X, Y);
}

DWORD hkXInputGetState(DWORD dwUserIndex, PXINPUT_STATE pState)
{
	ZeroMemory(&Vars.Menu.Input.emulate, sizeof(XINPUT_STATE)); // need to zero out the input state before emulating new inputs

	if (Vars.Menu.Input.emulate.Gamepad.wButtons & XINPUT_GAMEPAD_A)
		pState->Gamepad.wButtons |= XINPUT_GAMEPAD_A;

	if (Vars.Menu.Input.emulate.Gamepad.wButtons & XINPUT_GAMEPAD_X)
		pState->Gamepad.wButtons |= XINPUT_GAMEPAD_X;

	if (Vars.Menu.Input.emulate.Gamepad.wButtons & XINPUT_GAMEPAD_Y)
		pState->Gamepad.wButtons |= XINPUT_GAMEPAD_Y;

	if (Vars.Menu.Input.emulate.Gamepad.wButtons & XINPUT_GAMEPAD_B)
		pState->Gamepad.wButtons |= XINPUT_GAMEPAD_B;

	return oXInputGetState(dwUserIndex, pState);
}

LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
	return oWndProc(hWnd, uMsg, wParam, lParam);
}