#include "Hooks.h"

PresentFn oPresent;
CreateSwapChainFn oCreateSwapChain;
DrawIndexedFn oDrawIndexed;
PSSetShaderResourcesFn oPSSetShaderResources;
ClearRenderTargetViewFn oClearRenderTargetView;
QueryPerformaceCounterFn oQueryPerformanceCounter;
AcquireFn oMouseAcquire;
GetDeviceStateFn oMouseGetDeviceState;
SetCursorPosFn oSetCursorPos;
XInputGetStateFn oXInputGetState;
CreateEntityFn oCreateEntity;
WNDPROC oWndProc;

HRESULT __fastcall hkPresent(IDXGISwapChain* pThis, UINT SyncInterval, UINT Flags)
{
	ImGui::GetIO().MouseDrawCursor = Vars.Menu.bOpened;

	g_pQueryPerformanceCounterHook->Unhook();	//unhook cause it has the potential to fuck with imgui because it calls QueryPerformanceCounter, then rehook it

	ImGui_ImplDX11_NewFrame();

	g_pQueryPerformanceCounterHook->Rehook();

	g_pLocalPlayer = GetEntityFromHandle(g_pLocalPlayerHandle);
	Pl0000* pCameraEnt = GetEntityFromHandle(&g_pCamera->m_hEntity);
	SceneState s;
	//Entity_t* pBuddy = oGetEntityFromHandle(&g_pLocalPlayer->m_hBuddy);
	//QWORD v[3];

	ZeroMemory(&Vars.Menu.Input.emulate, sizeof(XINPUT_STATE)); // need to zero out the input state before emulating new inputs

	if (GetAsyncKeyState(VK_F3) & 1)
	{
		g_pLocalPlayer->m_hBuddy = NULL;
	}

	byte* pCutscene = *(byte**)0x1419925E8;

	if (pCutscene)
	{
		char* current_cutscene = (char*)(pCutscene + 0x1F4);
		//	LOG("Current Cutscene: %s\n", current_cutscene);
	}

	if (GetAsyncKeyState(VK_OEM_3) & 1)
	{
		(*(int(*)(void*))(0x1401AE580))(g_pLocalPlayer);
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

#ifdef _DEBUG
	if (GetAsyncKeyState(VK_END) & 1)
	{
		g_pMemory->PatchBytes(&bp_HairColor);
	}

	if (GetAsyncKeyState(VK_HOME) & 1)
	{
		g_pMemory->RestoreMemory(&bp_HairColor);
	}

	if (GetAsyncKeyState(VK_F1) & 1)
		LOG("Animation ID: %d\n", ++Vars.Gameplay.iAnimation);

	if (GetAsyncKeyState(VK_F2) & 1)
		LOG("Animation ID: %d\n", --Vars.Gameplay.iAnimation);

	if (GetAsyncKeyState(VK_F4) & 1)
	{
		pCameraEnt->Animate(Vars.Gameplay.iAnimation, 0);	// 0 plays, 1 stops?, 2 freezes
		//(*(EntityInfo *(__fastcall*)(Entity_t*))(0x140245C30))(pCameraEnt); //Buddy_UNK
	}
#endif

	if (GetAsyncKeyState(VK_F5) & 1)
	{
		pCameraEnt->Animate(Vars.Gameplay.iSelectedAnimation, 0);
	}

	if (GetAsyncKeyState(VK_F11) & 1)
	{
		Array<EntityHandle>* pHandles = g_pEnemyManager->GetHandles3();

		for (QWORD i = 0; i < pHandles->m_count; ++i)
		{
			LOG("handle[%d] = %x\n", i, pHandles->m_pItems[i]);
		}

		LOG("size %d\n", pHandles->GetSize());
		Features::TeleportAllEnemiesToEncirclePoint(pCameraEnt->m_vPosition, 100);
	}

	if (GetAsyncKeyState(VK_F6) & 1)
	{
		if (pCameraEnt)
		{
			pCameraEnt->m_iVerticeUpdateCount = Vars.Gameplay.nMaxModelVertices;
			Vars.Gameplay.nMaxModelVertices = 0;
			ChangePlayer(g_pLocalPlayer);
			pCameraEnt = GetEntityFromHandle(&g_pCamera->m_hEntity);
		}
	}

	if (GetAsyncKeyState(VK_F7) & 1) // setBuddyAiType
		(*(__int64(*)(void*, int))(0x140246F70))(g_pLocalPlayer, 2);

	if (GetAsyncKeyState(VK_F8) & 1) // setBedStandup
		(*(__int64(*)(void*))(0x1401F08A0))(g_pLocalPlayer);

	if (GetAsyncKeyState(VK_F9) & 1) // SetInputIgnore(void*, float) (freezes 2b)
		(*(__int64(*)(void*, float))(0x1401E9320))(g_pLocalPlayer, Vars.Gameplay.bFreezePlayer);

	if (GetAsyncKeyState(VK_F2) & 1) //setBuddyFlightSuit
	{
		(*(void*(*)(void*))(0x1402475B0))(g_pLocalPlayer);
	}

	//wip vAng is in radians i think
	if (GetAsyncKeyState(VK_F10) & 0x8000)
	{
		Vector3 vAng = g_pLocalPlayer->m_matModelToWorld.GetAxis(3); //*(Vector3*)((byte*)g_pCamera + 0x3f0);
		Vector3 vForward;

		Math::AngleVectors(vAng, &vForward);

		//*(Vector3*)((byte*)g_pCamera + 0x3f0) = vAngles3;

		g_pLocalPlayer->m_vPosition += vForward * 0.5f;
	}

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

	if (!Vars.Gameplay.nMaxModelVertices)
	{
		if (pCameraEnt && !Vars.Misc.bLoading)
			Vars.Gameplay.nMaxModelVertices = pCameraEnt->m_iVerticeUpdateCount;
		else
			Vars.Gameplay.nMaxModelVertices = 0;
	}

	if (pCameraEnt)
		pCameraEnt->m_iVerticeUpdateCount = (Vars.Gameplay.bGhostModel) ? 0 : Vars.Gameplay.nMaxModelVertices;

	if (Vars.Gameplay.bGodmode || Vars.Gameplay.bNoEnemyDamage || Vars.Gameplay.bNoWorldDamage)
		God();
	else
		Ungod();

	if (Vars.Misc.bAntiVSync)
		g_pMemory->PatchBytes(&bp_AntiVSync);
	else
		g_pMemory->RestoreMemory(&bp_AntiVSync);

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
		ImGui::Begin("2B Hook! ~ 2B Owns Me and All :^)", &Vars.Menu.bOpened);
		{
			ImGui::SetWindowSize(ImVec2(800, 600));
			ImGui::Text("Average %.3f ms / frame(%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::Separator();
			ImGui::Checkbox("Godmode", &Vars.Gameplay.bGodmode);
			ImGui::SameLine();
			ImGui::Checkbox("No Fall Damage", &Vars.Gameplay.bNoWorldDamage);
			ImGui::SameLine();
			ImGui::Checkbox("No Enemy Damage", &Vars.Gameplay.bNoEnemyDamage);
			ImGui::InputInt("Level:", &Vars.Gameplay.iLevel, 1, 5);
			ImGui::SameLine();
			ImGui::Checkbox("Temporary Level", &Vars.Gameplay.bTemporaryLevel);

			if (ImGui::Button("Apply Level"))
			{
				if (Vars.Gameplay.bTemporaryLevel)
				{
					Level_t* pLevel = CalculateLevel(&g_pLocalPlayer->m_LevelsContainer, *g_piExperience);
					pLevel->m_iLevel = Vars.Gameplay.iLevel;
				}
				else
				{
					if (Vars.Gameplay.iLevel > MAX_LEVELS)
						Vars.Gameplay.iLevel = MAX_LEVELS;
					else if (Vars.Gameplay.iLevel < 1)
						Vars.Gameplay.iLevel = 1;

					*g_piExperience = g_pLocalPlayer->m_LevelsContainer.m_levels[Vars.Gameplay.iLevel - 1].m_iMinimumExperience;
				}
			}

			ImGui::InputInt("Money:", g_piMoney, 1000, 10000);
			ImGui::SameLine();

			if (ImGui::Button("Max Money"))
				*g_piMoney = MAX_MONEY;

			ImGui::InputText("Sound Name", Vars.Misc.szSoundName, _ARRAYSIZE(Vars.Misc.szSoundName));

			if (ImGui::Button("Play Sound"))
			{
				Sound sound = { Vars.Misc.szSoundName, FNV1Hash(Vars.Misc.szSoundName), 0 };
				((PlaySoundFn)(0x14081FB90))(&sound);
			}

			ImGui::SameLine();

			if (ImGui::Button("Emit Sound"))
			{
				//g_pLocalPlayer->EmitSound("core_level_up", g_pLocalPlayer, 5, 0x5000000, 0x200007F);
				Sound sound = { Vars.Misc.szSoundName, FNV1Hash(Vars.Misc.szSoundName), 0 };
				QWORD rax = ((EmitSoundFn)(0x14081FD80))(&sound, &g_pLocalPlayer);
				LOG("RAX = %x\n", rax);
			}

			char szDesc[64];
			sprintf_s(szDesc, "Item Id: (%s)", ((GetItemByIdFn)(0x1405E0FD0))(0, Vars.Gameplay.iSpawnItemId));

			ImGui::InputInt(szDesc, &Vars.Gameplay.iSpawnItemId, 1, 100, ImGuiInputTextFlags_EnterReturnsTrue);

			ImGui::Checkbox("Instant Equip", &Vars.Gameplay.bInstantEquip);
			ImGui::SameLine();

			if (ImGui::Button("Spawn Item"))
				Features::AddItem(Vars.Gameplay.iSpawnItemId);

			ImGui::SameLine();

			if (ImGui::Button("Spawn Pod A"))
				Features::AddPod(POD_A);

			ImGui::SameLine();

			if (ImGui::Button("Spawn Pod B"))
				Features::AddPod(POD_B);

			ImGui::SameLine();

			if (ImGui::Button("Spawn Pod C"))
				Features::AddPod(POD_C);

			ImGui::ListBox("Animations", &Vars.Gameplay.iSelectedAnimation, Vars.AnimationListBoxList, _ARRAYSIZE(Vars.AnimationListBoxList));

			if (ImGui::Button("Play Animation"))
			{
				pCameraEnt->Animate(Vars.Gameplay.iSelectedAnimation, 0);
			}

			if (ImGui::Button("Change Player"))
			{
				if (pCameraEnt)
				{
					pCameraEnt->m_iVerticeUpdateCount = Vars.Gameplay.nMaxModelVertices;
					Vars.Gameplay.nMaxModelVertices = 0;
					ChangePlayer(g_pLocalPlayer);
					pCameraEnt = GetEntityFromHandle(&g_pCamera->m_hEntity);
				}
			}

			DisplayEntityHandles();

			ImGui::SameLine();

			if (ImGui::Button("Destroy Buddy") && g_pLocalPlayer)
			{
				Pl0000* pBuddy = GetEntityFromHandle(&g_pLocalPlayer->m_hBuddy);

				if (pBuddy)
					DestroyBuddy(pBuddy);
			}

			ImGui::SameLine();

			if (ImGui::Button("Duplicate Buddy As NPC") && g_pLocalPlayer)
			{
				g_pLocalPlayer->m_hBuddy = NULL;
			}

			ImGui::SameLine();

			if (ImGui::Button("A2 As Buddy"))
			{
				Features::ChangeBuddy(PROTAGONIST_A2);
			}

			ImGui::SameLine();

			if (ImGui::Button("2B As Buddy"))
			{
				Features::ChangeBuddy(PROTAGONIST_2B);
			}

			ImGui::SameLine();

			if (ImGui::Button("9S As Buddy"))
			{
				Features::ChangeBuddy(PROTAGONIST_9S);
			}

			ImGui::Checkbox("Ghost Model", &Vars.Gameplay.bGhostModel);


			if (pCameraEnt)
			{
				ImGui::InputFloat3("Position (X,Y,Z)", (float*)&pCameraEnt->m_vPosition);

				char szModelPart[64];
				sprintf_s(szModelPart, "ModelPart: (%s)", pCameraEnt->m_pModelParts[Vars.Gameplay.iSelectedModelPart].m_szModelPart);
				ImGui::SliderInt(szModelPart, &Vars.Gameplay.iSelectedModelPart, 0, pCameraEnt->m_nModelParts - 1);

				sprintf_s(szModelPart, "ModelPart Color: (%s)", pCameraEnt->m_pModelParts[Vars.Gameplay.iSelectedModelPart].m_szModelPart);
				ImGui::ColorPicker4(szModelPart, (float*)&pCameraEnt->m_pModelParts[Vars.Gameplay.iSelectedModelPart].m_vColor);

				if (!Vars.Gameplay.bRainbowModel)
					ImGui::ColorPicker4("Model Tint Color", (float*)&pCameraEnt->m_pModelInfo->m_vTint);

				ImGui::InputFloat3("Model Scale (X,Y,Z)", (float*)&pCameraEnt->m_matModelToWorld.GetAxis(1));
				ImGui::InputFloat3("Model Rotation (Pitch, Yaw, Roll)", (float*)&pCameraEnt->m_matModelToWorld.GetAxis(3));
			}

			ImGui::Checkbox("No Tutorial Dialogs", &Vars.Gameplay.bNoTutorialDialogs);
			ImGui::Checkbox("SpeedMeister", &Vars.Gameplay.bSpeedMeister);
			ImGui::SameLine();
			ImGui::SliderFloat("Speed Multiplier", &Vars.Gameplay.flSpeedMultiplier, -10.0f, 10.0f); // when the framerate is uncapped this can go higher, else x5 will freeze the game because the game thinks the frame rate is too high so it sleeps the threads
			ImGui::Separator();
			ImGui::Checkbox("Anti-VSync", &Vars.Misc.bAntiVSync);
			ImGui::Checkbox("Anti-Framerate Cap", &Vars.Misc.bAntiFramerateCap);

			if (ImGui::Button("Backup Save"))
			{
				Vars.Misc.bBackupSave = !BackupSave(Vars.Misc.nSlot);
			}

			ImGui::SameLine();
			ImGui::Text(Vars.Misc.bBackupSave ? "Succeded!" : "Failed!");
		}
		ImGui::End();
	}
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	// renderer doesn't work rn fml
	//g_pRenderer->SaveState();
	//g_pRenderer->DrawRectOutline(100, 100, 500, 500, 0xFFFFFFFF);
	//g_pRenderer->RestoreState();

#if 0	
	g_pDeviceContext->OMSetRenderTargets(1, &g_pRenderTargetView, NULL);// idk if we need this line since we don't draw
#endif

	/*D3DXVECTOR2 vScreen = { 0, 0 };

	if (WorldToScreen((CONST D3DXVECTOR3*)&g_pLocalPlayer->m_vPosition, &vScreen))
	{
		g_pRenderer->DrawRectOutline((int)vScreen.x, (int)vScreen.y, 100, 300, 0xffaaaaa);
	}*/

	return oPresent(pThis, SyncInterval, Flags); //Anti-VSync Here bud (Vars.Misc.bAntiVSync) ? 0 : SyncInterval
}


TODO("probs find a better way to do this (aka not alloc mem every time and put into a fucc!");
HRESULT __fastcall hkCreateSwapChain(IDXGIFactory* pThis, IUnknown* pDevice, DXGI_SWAP_CHAIN_DESC* pDesc, IDXGISwapChain** ppSwapChain)
{
	pDesc->BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	HRESULT hr = oCreateSwapChain(pThis, pDevice, pDesc, ppSwapChain);

	if (SUCCEEDED(hr))
	{
		g_pSwapChain = *ppSwapChain;

		delete g_pSwapChainHook;

		g_pSwapChainHook = new VirtualTableHook((QWORD***)ppSwapChain);
		oPresent = (PresentFn)g_pSwapChainHook->HookFunction((QWORD)hkPresent, 8);

		ImGui_ImplDX11_InvalidateDeviceObjects();
	}

	return hr;
}


void __fastcall hkPSSetShaderResources(ID3D11DeviceContext* pThis, UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView* const * ppShaderResourceViews)
{
	D3D11_SHADER_RESOURCE_VIEW_DESC desc;

	g_StartSlot = StartSlot;

	for (UINT i = 0; i < NumViews; ++i)
	{
		ID3D11ShaderResourceView* pShaderResourceView = ppShaderResourceViews[i];

		if (pShaderResourceView)
		{
			pShaderResourceView->GetDesc(&desc);

			if ((desc.ViewDimension == D3D11_SRV_DIMENSION_BUFFER) || (desc.ViewDimension == D3D11_SRV_DIMENSION_BUFFEREX))
				continue;//Skip buffer resources
		}
	}

	oPSSetShaderResources(pThis, StartSlot, NumViews, ppShaderResourceViews);
}

int countnum = 0;

/*
* Any stride bigger than 0x1C (28) is the ui (so far)
*
*
*/
void __fastcall hkDrawIndexed(ID3D11DeviceContext* pThis, UINT IndexCount, UINT StartIndexLocation, INT BaseVertexLocation)
{
	pThis->IAGetVertexBuffers(0, 1, &g_pVertexBuffers, &g_Stride, &g_VertexBuffersOffset);

	if (g_pVertexBuffers)
	{
		g_pVertexBuffers->GetDesc(&g_VertexBufferDesc);
		g_pVertexBuffers->Release();
		g_pVertexBuffers = NULL;
	}

	pThis->IAGetIndexBuffer(&g_pIndexBuffer, &g_IndexFormat, &g_IndexOffset);

	if (g_pIndexBuffer)
	{
		g_pIndexBuffer->GetDesc(&g_IndexBufferDesc);
		g_pIndexBuffer->Release();
		g_pIndexBuffer = NULL;
	}

	if (Vars.Misc.bWireframe && g_Stride == 0x1C)
	{
		g_pDeviceContext->RSSetState(g_pRenderWireframeState); // wireframe
	}

	oDrawIndexed(pThis, IndexCount, StartIndexLocation, BaseVertexLocation);
}

void __fastcall hkClearRenderTargetView(ID3D11DeviceContext* pThis, ID3D11RenderTargetView* pRenderTargetView, const FLOAT ColorRGBA[4])
{
	oClearRenderTargetView(pThis, pRenderTargetView, ColorRGBA);
}

HRESULT __fastcall hkMouseAcquire(IDirectInputDevice8A* pThis)
{
	g_pMouseHook->Unhook();

	HRESULT hr = oMouseAcquire(pThis);

	g_pMouseHook->Rehook();

	return hr;
}

HRESULT __fastcall hkMouseGetDeviceState(IDirectInputDevice8A* pThis, DWORD cbData, LPVOID lpvData)
{
	if (Vars.Menu.bOpened)
		return DIERR_INPUTLOST;

	g_pMouseHook->Unhook();

	HRESULT hr = oMouseGetDeviceState(pThis, cbData, lpvData);

	g_pMouseHook->Rehook();

	return hr;
}

void* __fastcall hkCreateEntity(void* pUnknown, EntityInfo* pInfo, unsigned int objectId, int flags, CHeapInstance** ppHeaps)
{
	ConstructionInfo<void>* pConstruct = GetConstructionInfo(objectId);
	
	void* pEntity = NULL;

	if (strcmp("Em4000", pConstruct->szName) && strcmp("EmParts", pConstruct->szName))//if (strcmp("NPC", pConstruct->szName))
		pEntity = oCreateEntity(pUnknown, pInfo, objectId, flags, ppHeaps); //0x1401A2B40

	if (!pEntity)
		LOG("Failed to create %s -> %s (ObjectId = %x)\n", pInfo->m_szEntityType, pConstruct->szName, objectId);
	else
		LOG("Created %s -> %s (ObjectId = %x)\n", pInfo->m_szEntityType, pConstruct->szName, objectId);

	return pEntity;
}

void __fastcall hkSaveFileIO(CSaveDataDevice* pSavedata)
{
	if (!pSavedata)
		return;

	switch (pSavedata->dwFlags)
	{
	case SAVE_FLAGS_READ_SLOTS:
		(*(ReadSaveSlotsFn)(0x14095DD80))(pSavedata);
		return;
	case SAVE_FLAGS_READ:
		(*(ReadSaveDataFn)(0x14095E020))(pSavedata);

		Vars.Misc.nSlot = pSavedata->nSlot;
		Vars.Gameplay.nMaxModelVertices = 0;
		Vars.Misc.bLoading = true;
		return;
	case SAVE_FLAGS_WRITE:
		(*(WriteSaveDataFn)(0x14095E330))(pSavedata);
		return;
	case SAVE_FLAGS_DELETE:
		(*(DeleteSaveDataFn)(0x14095E7B0))(pSavedata);
		return;
	default:
		Vars.Misc.bLoading = false;
		return;
	}
}

BOOL __fastcall hkQueryPerformanceCounter(LARGE_INTEGER* lpPerformanceCount)
{
	static LARGE_INTEGER liPreviousCount;

	BOOL result = oQueryPerformanceCounter(lpPerformanceCount);

	if (Vars.Gameplay.bSpeedMeister)
		lpPerformanceCount->QuadPart += ((lpPerformanceCount->QuadPart - liPreviousCount.QuadPart) * Vars.Gameplay.flSpeedMultiplier);

	liPreviousCount.QuadPart = lpPerformanceCount->QuadPart;
	return result;
}

BOOL __fastcall hkSetCursorPos(int X, int Y)
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

	return ret;
}

LRESULT __fastcall WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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