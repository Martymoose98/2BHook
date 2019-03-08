#include "Hooks.h"

PresentFn oPresent;
CreateSwapChainFn oCreateSwapChain;
DrawIndexedFn oDrawIndexed;
DrawFn oDraw;
CreateQueryFn oCreateQuery;
PSSetShaderResourcesFn oPSSetShaderResources;
ClearRenderTargetViewFn oClearRenderTargetView;
QueryPerformaceCounterFn oQueryPerformanceCounter;
SetUnhandledExceptionFilterFn oSetUnhandledExceptionFilter;
AcquireFn oKeyboardAcquire;
GetDeviceStateFn oKeyboardGetDeviceState;
AcquireFn oMouseAcquire;
GetDeviceStateFn oMouseGetDeviceState;
SetCursorPosFn oSetCursorPos;
XInputGetStateFn oXInputGetState;
UpdateModelPartsFn oUpdateModelParts;
CreateEntityFn oCreateEntity;
WNDPROC oWndProc;

static ID3D11RasterizerState* pNorm, * pBias;
static bool hookupdate = false;
VirtualTableHook m_ctxhook;
VirtualTableHook m_devhook;

void(*oSetViewport)(CGraphicContextDx11* pThis, int x, int y, int width, int height, float min_depth, float max_depth);

void hkCreateQuery(ID3D11Device* pDevice, const D3D11_QUERY_DESC *pQueryDesc, ID3D11Query **ppQuery);

void hkSetViewport(CGraphicContextDx11* pThis, int x, int y, int width, int height, float min_depth, float max_depth)
{
	LOG("X=%i, Y=%i, W=%i, H=%i, MIN_DEPTH=%f, MAX_DEPTH=%f\n", x, y, width, height, min_depth, max_depth);

	oSetViewport(pThis, x, y, width, height, min_depth, max_depth);
}

BOOL sub_140942DC0(CGraphicContextDx11 *a1, RenderInfo* a2)
{
	QWORD ret = (QWORD)_ReturnAddress();

	if (ret == 0x14052dfea || (a1->m_shader.m_uStrides[0] == 28 && (a2->m_uVertexCount == 0x0000ba21 || a2->m_uVertexCount == 0x0000ee3b ||
		a2->m_uVertexCount == 0x000242fa || a2->m_uVertexCount == 0x000109f8)))
	{
		a1->m_pContext->OMSetDepthStencilState(g_pDepthStencilStates[DISABLED], 1);

		CPixelShader p;
		p.m_ppShader = &g_pRed;
		g_pGraphics->m_pContext->SetPixelShader(&p);
	}

	BOOL b = (*(BOOL(*)(void*, ID3D11DeviceContext*))(0x140941130))(&a1->m_shader, a1->m_pContext);

	if (ret == 0x14052dfea || (a1->m_shader.m_uStrides[0] == 28 && (a2->m_uVertexCount == 0x0000ba21 || a2->m_uVertexCount == 0x0000ee3b ||
		a2->m_uVertexCount == 0x000242fa || a2->m_uVertexCount == 0x000109f8)))
	{
		(*(BOOL(*)(void*, ID3D11DeviceContext*, RenderInfo*))(0x140941380))(&a1->m_shader, a1->m_pContext, a2);

		CPixelShader p;
		p.m_ppShader = &g_pGreen;
		g_pGraphics->m_pContext->SetPixelShader(&p);
		a1->m_pContext->OMSetDepthStencilState(g_pDepthStencilStates[READ_NO_WRITE], 1);

		b = (*(BOOL(*)(void*, ID3D11DeviceContext*))(0x140941130))(&a1->m_shader, a1->m_pContext);

		return (b) ? (*(BOOL(*)(void*, ID3D11DeviceContext*, RenderInfo*))(0x140941380))(&a1->m_shader, a1->m_pContext, a2) : b;
	
	}
	else 
		return (b) ? (*(BOOL(*)(void*, ID3D11DeviceContext*, RenderInfo*))(0x140941380))(&a1->m_shader, a1->m_pContext, a2) : b;
}

BOOL sub_145021400(CGraphicContextDx11* pThis, signed int *a2, __int64 a3, unsigned int uAlignedByteOffsetForArgs)
{
	return 1;
}

BOOL sub_145023100(CGraphicContextDx11* pThis, signed int *a2, __int64 pStruct, unsigned int uAlignedByteOffsetForArgs)//DrawIndexedInstancedIndirect
{
	return 1;
}

BOOL func47(CGraphicContextDx11 *a1, void* a2, unsigned int uInstanceCount)
{
	return 1;
}

BOOL sub_14501F020(CGraphicContextDx11 *a1, void* a2, unsigned int uInstanceCount)
{
	if ((*(BOOL(*)(void*, ID3D11DeviceContext*))(0x140941130))(&a1->m_shader, a1->m_pContext))
		return (*(BOOL(*)(CGraphicContextDx11*, ID3D11DeviceContext *, void*, unsigned int))(0x140941530))(a1, a1->m_pContext, a2, uInstanceCount);
	return 0;
}

__int64 DrawHud(CGraphicContextDx11 *a1, __int64 a2)
{
	return 0;
}

void CTX_FLUSH(CGraphicContextDx11 *a1)
{
	a1->m_pContext->Flush();
	LOG("CTX FLUSHED!");
}

HRESULT hkPresent(IDXGISwapChain* pThis, UINT SyncInterval, UINT Flags)
{
	ImGui::GetIO().MouseDrawCursor = Vars.Menu.bOpened;

	g_pQueryPerformanceCounterHook->Unhook();	//unhook cause it has the potential to fuck with imgui because it calls QueryPerformanceCounter, then rehook it

	ImGui_ImplDX11_NewFrame();

	g_pQueryPerformanceCounterHook->Rehook();

	g_pLocalPlayer = GetEntityFromHandle(g_pLocalPlayerHandle);
	Pl0000* pCameraEnt = GetEntityFromHandle(&g_pCamera->m_hEntity);

#ifdef _DEBUG

	if (!hookupdate)
	{
		hookupdate = true;
		m_devhook.Initialize((QWORD**)g_pDevice);
		oCreateQuery = (CreateQueryFn)m_devhook.HookFunction((QWORD)hkCreateQuery, 24);
		m_ctxhook.Initialize((QWORD**)g_pGraphics->m_pContext);
		//oSetViewport = (void(*)(CGraphicContextDx11*, int, int, int, int, float, float))m_ctxhook.HookFunction((QWORD)hkSetViewport, 14);
		m_ctxhook.HookFunction((QWORD)sub_140942DC0, 46);
		//m_ctxhook.HookFunction((QWORD)DrawHud, 45);
		m_ctxhook.HookFunction((QWORD)func47, 47);
		m_ctxhook.HookFunction((QWORD)sub_14501F020, 48);
		m_ctxhook.HookFunction((QWORD)sub_145021400, 49);
		m_ctxhook.HookFunction((QWORD)sub_145023100, 50);
		m_ctxhook.HookFunction((QWORD)CTX_FLUSH, 51);

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
		rasterizer_desc.CullMode = D3D11_CULL_NONE; //D3D11_CULL_FRONT;
		rasterizer_desc.FrontCounterClockwise = false;
		float bias = 1000.0f;
		float bias_float = -bias;
		bias_float /= 10000.0f;
		rasterizer_desc.DepthBias = *(INT*)&bias_float;
		rasterizer_desc.SlopeScaledDepthBias = 0.0f;
		rasterizer_desc.DepthBiasClamp = 0.0f;
		rasterizer_desc.DepthClipEnable = true;
		rasterizer_desc.ScissorEnable = false;
		rasterizer_desc.MultisampleEnable = false;
		rasterizer_desc.AntialiasedLineEnable = false;
		g_pDevice->CreateRasterizerState(&rasterizer_desc, &pBias);
	}

	if (GetAsyncKeyState(VK_F7) & 1)
		m_ctxhook.Unhook();

	if (GetAsyncKeyState(VK_F8) & 1)
		m_ctxhook.Rehook();


	byte* pCutscene = *(byte**)0x1419925E8;

	if (pCutscene)
	{
		char* current_cutscene = (char*)(pCutscene + 0x1F4);
		//	LOG("Current Cutscene: %s\n", current_cutscene);
	}

	if (GetAsyncKeyState(VK_OEM_3) & 1)
	{
		for (size_t i = 0; i < g_pEntityInfoList->m_dwSize; ++i)
		{
			EntityInfo* pInfo = g_pEntityInfoList->m_pItems[i].second;

			if (pInfo)
				LOG("Entity: %s Handle: %x Address: %llx\n", pInfo->m_szEntityType, pInfo->m_hEntity, pInfo->m_pEntity);
		}

		//(*(void*(*)())(0x1430AC860))();

		//DWORD crc = HashStringCRC32("Ba2014", 6);
		//SceneState* p = (SceneState*)FindSceneState((PCRITICAL_SECTION)0x141ECF350, crc, "Ba2014", 6);
		//((SceneStateSystem_SetInternalFn)(0x14001EC80))((void*)0x14158CBC0, &p);

		//(*(signed __int64(*)(MappedModel*, const char*))(0x14086FCC0))(pCameraEnt->m_pMappedModel, "DLC_mesh_es0202");	

		//(*(__int64(*)(void*))(0x140AF7770))((void*)0x58940000);
		//(*(void(*)(Entity_t*, int))(0x1401EF120))(g_pLocalPlayer, 1); //SetE3TimeTrial
	}

	//if (GetAsyncKeyState(VK_F4) & 1)
	//{
	//	Entity_t* pBuddy = GetEntityFromHandle(&g_pLocalPlayer->m_hBuddy);

	//	*(BOOL*)((byte*)pBuddy + 0x164E8) = TRUE;

	//	(*(SceneState **(*)(SceneState *a1, char *a2))(0x140005D20))(&s, "ON_EVE_HACKING_COMPLETE");

	//	/*CRITICAL_SECTION cs;
	//	DWORD crc = oHashStringCRC32("A2_POD_END", 10);
	//	__int64 ret = ((FindHeapFn)(0x1400538A0))(&cs, crc, "A2_POD_END", 10);

	//	(*(bool(*)(void*, __int64*))(0x14001EC80))((void*)0x14158CBC0, &ret);*/

	//	//(*(__int64(*)(void*, const char*))(0x140005D20))(v, "A2_POD_END");
	//}

	if (GetAsyncKeyState(VK_F1) & 1)
		LOG("Animation ID: %d\n", ++Vars.Gameplay.iAnimation);

	if (GetAsyncKeyState(VK_F2) & 1)
		LOG("Animation ID: %d\n", --Vars.Gameplay.iAnimation);

	if (GetAsyncKeyState(VK_F3) & 1)
	{
		static DWORD diagId = 0;
		typedef void*(*DialogFactoryFn)(QWORD thisrcx, DWORD id);
		void* rax = ((DialogFactoryFn)(0x1408127E0))(0, diagId);
		if ((*(BOOL(*)(void*))(0x140812B80))(rax))
		{
			//*(DWORD*)((BYTE*)rax + 0x58) &= 0xBFFFFFFF;
			*(DWORD*)((BYTE*)rax + 0x58) = 0xC0000000;
			*(DWORD*)((BYTE*)rax + 0x88) |= 3;
			*((void**)0x14105C968) = rax;
			(*(DWORD*)0x14105C91C)++;
		}
		//(*(__int64(*)(void*))0x144807360)((void*)0x1419861E0); //CreateYesNoDialog
	}

	if (pCameraEnt && (GetAsyncKeyState(VK_F8) & 1))
	{
		Features::AddPlayer(1);
	}

	if (GetAsyncKeyState(VK_F11) & 1)
	{
		Array<EntityHandle>* pHandles = g_pEnemyManager->GetHandles3();

		for (QWORD i = 0; i < pHandles->m_count; ++i)
		{
			LOG("handle[%d] = %x\n", i, pHandles->m_pItems[i]);
		}

		LOG("size %d\n", pHandles->GetSize());
		Features::TeleportAllEnemiesToEncirclePoint(pCameraEnt->m_vPosition, pCameraEnt->m_matTransform.GetAxis(FORWARD), 50);
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

	if (pCameraEnt)
	{
		if (Vars.Gameplay.bRainbowHair)
		{
			Vars.Gameplay.flModelTintHue += 0.005f;

			if (Vars.Gameplay.flModelTintHue > 1.f)
				Vars.Gameplay.flModelTintHue = 0.0f;

			Vector4 vRain;
			Color rainbow = Color::FromHSB(Vars.Gameplay.flModelTintHue, 1.f, 1.f);
			rainbow.GetFloatColor((float*)&vRain);

			for (INT i = 0; i < pCameraEnt->m_nModelParts; ++i)
			{
				if (!strcmp(pCameraEnt->m_pModelParts[i].m_szModelPart, "Hair"))
				{
					pCameraEnt->m_pModelParts[i].m_vColor = vRain;
					pCameraEnt->m_pModelParts[i].m_bUpdate = TRUE;
					pCameraEnt->m_pModelParts[i].m_bShow = TRUE;
					break;
				}
			}
		}

		if (Vars.Gameplay.bRainbowPod)
		{
			Pl0000* pPod = GetEntityFromHandle(&pCameraEnt->m_hPod);

			if (pPod)
			{
				Vars.Gameplay.flPodTintHue += 0.005f;

				if (Vars.Gameplay.flPodTintHue > 1.f)
					Vars.Gameplay.flPodTintHue = 0.0f;

				Vector4 vRain;
				Color rainbow = Color::FromHSB(Vars.Gameplay.flPodTintHue, 1.f, 1.f);
				rainbow.GetFloatColor((float*)&vRain);

				for (INT i = 0; i < pPod->m_nModelParts; ++i)
				{
					if (!strcmp(pPod->m_pModelParts[i].m_szModelPart, "Body"))
					{
						pPod->m_pModelParts[i].m_vColor = vRain;
						pPod->m_pModelParts[i].m_bUpdate = TRUE;
						pPod->m_pModelParts[i].m_bShow = TRUE;
						break;
					}
				}
			}
		}
	}

	if (Vars.Gameplay.bBalanceEnemyLevels)
		Features::BuffEnemies();

	Vars.Gameplay.bGhostModelOld = Vars.Gameplay.bGhostModel;

	if (!Vars.Gameplay.nBones)
	{
		if (pCameraEnt && !Vars.Misc.bLoading)
			Vars.Gameplay.nBones = pCameraEnt->m_nBones;
		else
			Vars.Gameplay.nBones = 0;
	}

	if (pCameraEnt && Vars.Gameplay.bGhostModel != Vars.Gameplay.bGhostModelOld)
		pCameraEnt->m_nBones = (Vars.Gameplay.bGhostModel) ? 0 : Vars.Gameplay.nBones;

	Features::ApplyHealthMods();

	if (Vars.Misc.bAntiFramerateCap)
	{
		g_pMemory->PatchBytes(&bp_Framecap);
		g_pMemory->NopMemory(&nop_Framecap[NOP_FRAMECAP_SLEEP]);
	}
	else
	{
		g_pMemory->RestoreMemory(&bp_Framecap);
		g_pMemory->RestoreMemory(&nop_Framecap[NOP_FRAMECAP_SLEEP]);
	}

	if (Vars.Gameplay.bNoTutorialDialogs)
		g_pMemory->PatchBytes(&bp_NoTutorialDialogs);
	else
		g_pMemory->RestoreMemory(&bp_NoTutorialDialogs);

	if (Vars.Menu.bOpened)
	{
		if (ImGui::Begin("2B Hook! ~ 2B Owns Me and All :^)", &Vars.Menu.bOpened))
		{
			ImGui::SetWindowSize(ImVec2(850, 600));
			ImGui::Text("Average %.3f ms / frame(%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

			if (ImGui::BeginTabBar("MainTabs"))
			{
				if (ImGui::BeginTabItem("Gameplay"))
				{
					GameplayTab(pCameraEnt);
					ImGui::EndTabItem();
				}

				if (ImGui::BeginTabItem("Visuals"))
				{
					VisualsTab(pCameraEnt);
					ImGui::EndTabItem();
				}

				if (ImGui::BeginTabItem("Miscellaneous"))
				{
					MiscTab();
					ImGui::EndTabItem();
				}

				if (ImGui::BeginTabItem("Config"))
				{
					ConfigTab();
					ImGui::EndTabItem();
				}

				if (ImGui::BeginTabItem("Console"))
				{
					g_pConsole->Draw("Console");
					ImGui::EndTabItem();
				}

				ImGui::EndTabBar();
			}
		}
		ImGui::End();
	}

	g_pOverlay->Render();

	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	return oPresent(pThis, (Vars.Misc.bAntiVSync) ? 0 : SyncInterval, Flags);
}

HRESULT hkCreateSwapChain(IDXGIFactory* pThis, IUnknown* pDevice, DXGI_SWAP_CHAIN_DESC* pDesc, IDXGISwapChain** ppSwapChain)
{
	pDesc->BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	HRESULT hr = oCreateSwapChain(pThis, pDevice, pDesc, ppSwapChain);

	if (SUCCEEDED(hr))
	{
		g_pSwapChain = *ppSwapChain;
		g_pSwapChainHook->Relocate((QWORD**)g_pSwapChain);

		ImGui_ImplDX11_InvalidateDeviceObjects();
	}

	return hr;
}


void hkPSSetShaderResources(ID3D11DeviceContext* pThis, UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView* const * ppShaderResourceViews)
{
	g_StartSlot = StartSlot;

	for (UINT i = 0; i < NumViews; ++i)
	{
		ID3D11ShaderResourceView* pShaderResourceView = ppShaderResourceViews[i];

		if (pShaderResourceView)
		{
			pShaderResourceView->GetDesc(&g_ShaderResViewDesc);

			if ((g_ShaderResViewDesc.ViewDimension == D3D11_SRV_DIMENSION_BUFFER) || (g_ShaderResViewDesc.ViewDimension == D3D11_SRV_DIMENSION_BUFFEREX))
				continue;//Skip buffer resources
		}
	}

	oPSSetShaderResources(pThis, StartSlot, NumViews, ppShaderResourceViews);
}

void hkCreateQuery(ID3D11Device* pDevice, const D3D11_QUERY_DESC *pQueryDesc, ID3D11Query **ppQuery)
{

	//disables Occlusion which prevents rendering player models through certain objects
	//REDUCES FPS, not recommended, only works if occlusion is client side etc.
	if (pQueryDesc->Query == D3D11_QUERY_OCCLUSION)
	{
		D3D11_QUERY_DESC oqueryDesc = { D3D11_QUERY_TIMESTAMP, pQueryDesc->MiscFlags };

		return oCreateQuery(pDevice, &oqueryDesc, ppQuery);
	}
	
	return oCreateQuery(pDevice, pQueryDesc, ppQuery);
}

void hkDraw(ID3D11DeviceContext* pThis, UINT VertexCount, UINT StartVertexLocation)
{
	oDraw(pThis, VertexCount, StartVertexLocation);
}

/*
* Any stride bigger than 0x1C (28) is the ui (so far)
*
*
*/
void hkDrawIndexed(ID3D11DeviceContext* pThis, UINT IndexCount, UINT StartIndexLocation, INT BaseVertexLocation)
{
	pThis->IAGetVertexBuffers(0, 8, g_pVertexBuffers, g_Stride, g_VertexBuffersOffset);

	for (int i = 0; i < 8; ++i)
	{
		if (g_pVertexBuffers[i])
		{
			g_pVertexBuffers[i]->GetDesc(&g_VertexBufferDesc[i]);
			g_pVertexBuffers[i]->Release();
			g_pVertexBuffers[i] = NULL;
		}
	}

	pThis->IAGetIndexBuffer(&g_pIndexBuffer, &g_IndexFormat, &g_IndexOffset);

	if (g_pIndexBuffer)
	{
		g_pIndexBuffer->GetDesc(&g_IndexBufferDesc);
		g_pIndexBuffer->Release();
		g_pIndexBuffer = NULL;
	}

	pThis->PSGetConstantBuffers(g_PixelShaderStartSlot, 1, &g_pPixelShaderBuffer);

	if (g_pPixelShaderBuffer)
	{
		g_pPixelShaderBuffer->GetDesc(&g_PixelShaderBufferDesc);
		g_pPixelShaderBuffer->Release();
		g_pPixelShaderBuffer = NULL;
	}


	if (Vars.Misc.bWireframe && g_Stride[0] == 28)
	{
		g_pDeviceContext->RSSetState(g_pRenderWireframeState); // wireframe
	}

	//bw: 0x000ffec4, 0x000f58f8, 0x004405ec, 0x00360edc, 0x00556e54, 0x00782154,0x00000198, 0x64 if stride = 16 max bw is 3000
	//if (g_Stride == 28 && g_PixelShaderBufferDesc.ByteWidth == 256 &&
	//	(g_IndexBufferDesc.ByteWidth == 454128 ||  g_IndexBufferDesc.ByteWidth == 1048260 || g_IndexBufferDesc.ByteWidth == 1005816) || g_IndexBufferDesc.ByteWidth == 670248)
	//{
	//	pThis->OMSetDepthStencilState(g_pDepthStencilStates[DISABLED], 1);

	//	pThis->PSSetShader(g_pRed, NULL, 0);

	//	oDrawIndexed(pThis, IndexCount, StartIndexLocation, BaseVertexLocation);

	//	pThis->PSSetShader(g_pGreen, NULL, 0);

	//	//if (pssrStartSlot == 1) //if black screen, find correct pssrStartSlot
	//	pThis->OMSetDepthStencilState(g_pDepthStencilStates[READ_NO_WRITE], 1);
	//}

	oDrawIndexed(pThis, IndexCount, StartIndexLocation, BaseVertexLocation);
}

void hkClearRenderTargetView(ID3D11DeviceContext* pThis, ID3D11RenderTargetView* pRenderTargetView, const FLOAT ColorRGBA[4])
{
	oClearRenderTargetView(pThis, pRenderTargetView, ColorRGBA);
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
	if (Vars.Menu.bOpened && Vars.Menu.bIgnoreInputWhenOpened)
		return DIERR_INPUTLOST;

	g_pKeyboardHook->Unhook();

	HRESULT hr = oKeyboardGetDeviceState(pThis, cbData, lpvData);

	g_pKeyboardHook->Rehook();

	memcpy(&Vars.Menu.Input.OldKeyboardState, &Vars.Menu.Input.KeyboardState, cbData);
	memcpy(&Vars.Menu.Input.KeyboardState, lpvData, cbData);

	for (auto& pKeybind : g_pConfig->GetKeybinds())
	{
		if (pKeybind->GetMode() == IKeybind::KEYBIND_ON_KEYPRESSED)
		{
			if (KEYPRESSED(Vars.Menu.Input.KeyboardState, Vars.Menu.Input.OldKeyboardState, pKeybind->GetKeycode()))
				pKeybind->OnPressed();
		}
		else if (pKeybind->GetMode() == IKeybind::KEYBIND_ON_KEYDOWN)
		{
			if (KEYDOWN(Vars.Menu.Input.KeyboardState, pKeybind->GetKeycode()))
				pKeybind->OnPressed();
		}
	}

	return hr;
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
	if (Vars.Menu.bOpened && Vars.Menu.bIgnoreInputWhenOpened)
		return DIERR_INPUTLOST;

	g_pMouseHook->Unhook();

	HRESULT hr = oMouseGetDeviceState(pThis, cbData, lpvData);

	g_pMouseHook->Rehook();

	memcpy(&Vars.Menu.Input.MouseState, lpvData, cbData);

	return hr;
}

void hkUpdateModelParts(Pl0000* pEntity)
{
	LOG("Update model hook!");
}

void* hkCreateEntity(void* pUnknown, EntityInfo* pInfo, unsigned int objectId, int flags, CHeapInstance** ppHeaps)
{
	ConstructionInfo<void>* pConstruct = GetConstructionInfo(objectId);

	void* pEntity = NULL;

	if (Vars.Gameplay.SpawnBlacklist.empty() || std::find(Vars.Gameplay.SpawnBlacklist.cbegin(), Vars.Gameplay.SpawnBlacklist.cend(), pConstruct->szName) == Vars.Gameplay.SpawnBlacklist.cend()) //strcmp("Em4000", pConstruct->szName) && strcmp("BehaviorFunnel", pConstruct->szName)
		pEntity = oCreateEntity(pUnknown, pInfo, objectId, flags, ppHeaps); //0x1401A2B40

	if (!pEntity)
		g_pConsole->Warn("Failed to create %s -> %s (ObjectId = %x)\n", pInfo->m_szEntityType, pConstruct->szName, objectId);
	else
		g_pConsole->Log(ImVec4(0.0f, 0.525f, 0.0f, 1.0f), "Created %s -> %s (ObjectId = %x) Base %llx\n", (pInfo->m_szEntityType) ? pInfo->m_szEntityType : "EntityLayout", pConstruct->szName, objectId, pEntity);
	/*
		if (!pEntity)
			LOG("Failed to create %s -> %s (ObjectId = %x)\n", pInfo->m_szEntityType, pConstruct->szName, objectId);
		else
			LOG("Created %s -> %s (ObjectId = %x) Base %llx\n", pInfo->m_szEntityType, pConstruct->szName, objectId, pEntity);*/

	return pEntity;
}

BOOL hkLoadWordBlacklist(BannedWordChecker* pThis, __int64 thisrdx, QWORD *thisr8, const char* szBlacklistName)
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

void hkSaveFileIO(CSaveDataDevice* pSavedata)
{
	if (!pSavedata)
		return;

	switch (pSavedata->dwFlags)
	{
	case SAVE_FLAGS_READ_SLOTS:
		(*(ReadSaveSlotsFn)(0x14095DD80))(pSavedata);
		return;
	case SAVE_FLAGS_READ:
		Vars.Misc.bLoading = true;
		(*(ReadSaveDataFn)(0x14095E020))(pSavedata);
		Vars.Misc.nSlot = pSavedata->nSlot;
		Vars.Gameplay.nBones = 0; // read gets run when you're editing some setting
		return;
	case SAVE_FLAGS_WRITE:
		(*(WriteSaveDataFn)(0x14095E330))(pSavedata);
		return;
	case SAVE_FLAGS_DELETE:
		(*(DeleteSaveDataFn)(0x14095E7B0))(pSavedata);
		return;
	}
	Vars.Misc.bLoading = false;
}

BOOL hkQueryPerformanceCounter(LARGE_INTEGER* lpPerformanceCount)
{
	static LARGE_INTEGER liPreviousCount, liFakeCount;
	LARGE_INTEGER liCurrentCount;

	BOOL result = oQueryPerformanceCounter(&liCurrentCount);

	if (Vars.Gameplay.bSpeedMeister)
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
	auto& it = std::find(g_pExceptionHandlers.cbegin(), g_pExceptionHandlers.cend(), lpTopLevelExceptionFilter);

	if (it == g_pExceptionHandlers.cend())
		g_pExceptionHandlers.push_back(lpTopLevelExceptionFilter);

	if (lpTopLevelExceptionFilter == UnhandledExceptionHandler)
		return oSetUnhandledExceptionFilter(lpTopLevelExceptionFilter);
	else
		return UnhandledExceptionHandler;
}

BOOL hkSetCursorPos(int X, int Y)
{
	if (Vars.Menu.bOpened)
		return FALSE;

	return oSetCursorPos(X, Y);
}

DWORD hkXInputGetState(DWORD dwUserIndex, PXINPUT_STATE pState)
{
	DWORD ret = oXInputGetState(dwUserIndex, pState);

	if (Vars.Menu.Input.emulate.Gamepad.wButtons & XINPUT_GAMEPAD_A)
		pState->Gamepad.wButtons |= XINPUT_GAMEPAD_A;

	if (Vars.Menu.Input.emulate.Gamepad.wButtons & XINPUT_GAMEPAD_X)
		pState->Gamepad.wButtons |= XINPUT_GAMEPAD_X;

	if (Vars.Menu.Input.emulate.Gamepad.wButtons & XINPUT_GAMEPAD_Y)
		pState->Gamepad.wButtons |= XINPUT_GAMEPAD_Y;

	if (Vars.Menu.Input.emulate.Gamepad.wButtons & XINPUT_GAMEPAD_B)
		pState->Gamepad.wButtons |= XINPUT_GAMEPAD_B;

	ZeroMemory(&Vars.Menu.Input.emulate, sizeof(XINPUT_STATE)); // need to zero out the input state before emulating new inputs

	return ret;
}

LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_KEYDOWN:
		if (wParam == VK_INSERT)
			Vars.Menu.bOpened = !Vars.Menu.bOpened;

		break;
	default:
		break;
	}

	ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
	return oWndProc(hWnd, uMsg, wParam, lParam);
}