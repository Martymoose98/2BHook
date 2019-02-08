#include "Hooks.h"

PresentFn oPresent;
CreateSwapChainFn oCreateSwapChain;
DrawIndexedFn oDrawIndexed;
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

static bool hookupdate = false;
VirtualTableHook m_ctxhook;

void(*oSetViewport)(CGraphicContextDx11* pThis, int x, int y, int width, int height, float min_depth, float max_depth);

void hkSetViewport(CGraphicContextDx11* pThis, int x, int y, int width, int height, float min_depth, float max_depth)
{
	LOG("X=%i, Y=%i, W=%i, H=%i, MIN_DEPTH=%f, MAX_DEPTH=%f\n", x, y, width, height, min_depth, max_depth);

	oSetViewport(pThis, x, y, width, height, min_depth, max_depth);
}

BOOL sub_140942DC0(CGraphicContextDx11 *a1, int* a2)
{
//	*(DWORD *)((QWORD)&a1->m_pVertexShader2 + 0x10) = 1;
	//*(ID3D11PixelShader**)((QWORD)&a1->m_pVertexShader2 + 0x8) = g_pGreen;
	//BOOL rax = (*(BOOL(*)(CGraphicContextDx11*, int*))(0x14501D450))(a1, a2);
	//return rax;
	a1->m_vClearColor = Vector4(1, 0,0,0);
	BOOL b = (*(BOOL(*)(void*, ID3D11DeviceContext*))(0x140941130))(&a1->m_pVertexShader2, a1->m_pContext);

	return (b) ? (*(BOOL(*)(void*, ID3D11DeviceContext*, int*))(0x140941380))(&a1->m_pVertexShader2, a1->m_pContext, a2) : b;

	//return (*(BOOL(*)(CGraphicContextDx11*, int*))(0x140942E20))(a1, a2);
} 

signed __int64 sub_145023100(CGraphicContextDx11 *a1, signed int *a2, __int64 a3, unsigned int dwAlignedOffset)
{
	return 0;
}

__int64 sub_14501F020(CGraphicContextDx11 *a1, __int64 a2, unsigned int a3)
{
	return 0;
}

void CTX_FLUSH(CGraphicContextDx11 *a1)
{
	a1->m_pContext->Flush();
	LOG("CTX FLUSHED!");
}

HRESULT __fastcall hkPresent(IDXGISwapChain* pThis, UINT SyncInterval, UINT Flags)
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
		m_ctxhook.Initialize((QWORD**)g_pGraphics->m_pContext);
		//oSetViewport = (void(*)(CGraphicContextDx11*, int, int, int, int, float, float))m_ctxhook.HookFunction((QWORD)hkSetViewport, 14);
		m_ctxhook.HookFunction((QWORD)sub_140942DC0, 46);
		m_ctxhook.HookFunction((QWORD)sub_145023100, 49);
		//m_ctxhook.HookFunction((QWORD)sub_145023100, 48);
		m_ctxhook.HookFunction((QWORD)sub_145023100, 50);
		m_ctxhook.HookFunction((QWORD)CTX_FLUSH, 51);
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

	if (pCameraEnt && (GetAsyncKeyState(VK_F7) & 0x8000))
	{
		//(*(void(*)(Pl0000*))(0x14391AA80))(pCameraEnt);
		//(*(void(*)(Pl0000*))(0x1401ADB90))(pCameraEnt); //createA2wig
	}

	if (pCameraEnt && (GetAsyncKeyState(VK_F8) & 1))
	{
		Features::AddPlayer(1);
	}

#endif

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

	//if (GetAsyncKeyState(VK_F7) & 1) // setBuddyAiType
	//	(*(__int64(*)(void*, int))(0x140246F70))(g_pLocalPlayer, 2);

	//if (GetAsyncKeyState(VK_F9) & 1) // SetInputIgnore(void*, float) (freezes 2b)
	//	(*(__int64(*)(void*, float))(0x1401E9320))(g_pLocalPlayer, Vars.Gameplay.bFreezePlayer);

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

	if (!Vars.Gameplay.nBones)
	{
		if (pCameraEnt && !Vars.Misc.bLoading)
			Vars.Gameplay.nBones = pCameraEnt->m_nBones;
		else
			Vars.Gameplay.nBones = 0;
	}

	if (pCameraEnt)
		pCameraEnt->m_nBones = (Vars.Gameplay.bGhostModel) ? 0 : Vars.Gameplay.nBones;

	Features::ApplyHealthMods();

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
			ImGui::SetWindowSize(ImVec2(860, 600));
			ImGui::Text("Average %.3f ms / frame(%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::Separator();
			ImGui::Checkbox("Godmode", &Vars.Gameplay.bGodmode);
			ImGui::SameLine();
			ImGui::Checkbox("No Fall Damage", &Vars.Gameplay.bNoWorldDamage);
			ImGui::SameLine();
			ImGui::Checkbox("No Enemy Damage", &Vars.Gameplay.bNoEnemyDamage);

			
	
			ImGui::Columns(2);
			ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.2f);
			//ImGui::SetColumnWidth(0, ImGui::GetWindowWidth() * 0.33f);

			ImGui::InputInt("Experience:", g_piExperience, 1, 5);
			ImGui::InputInt("Level:", &Vars.Gameplay.iLevel, 1, 5);
			ImGui::Checkbox("Temporary Level", &Vars.Gameplay.bTemporaryLevel);
			ImGui::SameLine();

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

			ImGui::PopItemWidth();
			ImGui::NextColumn();
			ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.2f);

			ImGui::InputInt("Enemy Level Tolerance (+/-):", &Vars.Gameplay.iEnemyLevelTolerance, 1, 5);
			ImGui::Checkbox("Balance Enemy Levels", &Vars.Gameplay.bBalanceEnemyLevels);
			ImGui::SameLine();
			ImGui::Checkbox("Exclusively Positive", &Vars.Gameplay.bExclusivelyPositiveTolerance);

			ImGui::InputInt("Money:", g_piMoney, 1000, 10000);
			ImGui::SameLine();

			if (ImGui::Button("Max Money"))
				*g_piMoney = MAX_MONEY;

	
			ImGui::PopItemWidth();
			ImGui::Columns();

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
			sprintf_s(szDesc, "Item Name or Id: (%s)", ((GetItemByIdFn)(0x1405E0FD0))(0, Vars.Gameplay.iSpawnItemId));
			ImGui::InputText(szDesc, Vars.Gameplay.szItemName, _ARRAYSIZE(Vars.Gameplay.szItemName));
			int id = ((GetItemIdByNameFn)(0x1405DE6E0))(0, Vars.Gameplay.szItemName);
			Vars.Gameplay.iSpawnItemId = (id != -1) ? id : atoi(Vars.Gameplay.szItemName);

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

			ImGui::ListBox("Entity", &Vars.Gameplay.iSelectedEntityType, Vars.EntityTypeList, _ARRAYSIZE(Vars.EntityTypeList));
			ImGui::InputFloat3("Entity Scale", (float*)&Vars.Gameplay.vSpawnEntityScale);

			if (ImGui::Button("Create Entity"))
			{
				set_info_t set_info;

				set_info.m_mat = Matrix4x4();
				set_info.m_vPosition = pCameraEnt->m_vPosition + (pCameraEnt->m_matTransform.GetAxis(FORWARD) * 2.f);
				set_info.m_vRotation = pCameraEnt->m_matModelToWorld.GetAxis(3); //  Vector3Aligned(0, 1, 0);
				set_info.m_vScale = Vars.Gameplay.vSpawnEntityScale;
				set_info.m_i0x080 = -1;
				set_info.m_i0x084 = 0;
				set_info.m_i0x088 = 1;
				set_info.m_i0x08C = -1;
				set_info.m_i0x07C = 1;
				set_info.m_dw0x70 = 0;
				set_info.m_dw0x74 = 0;
				set_info.m_i0x078 = 0;

				EntityInfo* pInfo = Features::CreateEntity(Vars.SpawnEntities[Vars.Gameplay.iSpawnObjectId].m_szClass, Vars.SpawnEntities[Vars.Gameplay.iSelectedEntityType].m_ObjectId, &set_info);

				if (pInfo)
				{
					switch (pInfo->m_ObjectId)			
					{
					case OBJECTID_2B:
						pCameraEnt->m_hBuddy = pInfo->m_hEntity;
						SetSceneEntity("buddy_2B", pInfo);
						SetSceneEntity("buddy", pInfo);
						break;
					case OBJECTID_A2:
						pCameraEnt->m_hBuddy = pInfo->m_hEntity;
						SetSceneEntity("buddy_A2", pInfo);
						SetSceneEntity("buddy", pInfo);
						break;
					case OBJECTID_9S:
						pCameraEnt->m_hBuddy = pInfo->m_hEntity;
						SetSceneEntity("buddy_9S", pInfo);
						SetSceneEntity("buddy", pInfo);
						break;
					case 0x10010:
						//pCameraEnt->m_bFlightSuit = TRUE;
						pCameraEnt->m_hFlightSuit = pInfo->m_hEntity;
						//*MakePtr(EntityHandle*, pInfo->m_pParent, 0xC50) = pCameraEnt->m_pInfo->m_hEntity;
						*MakePtr(DWORD*, pInfo->m_pParent, 0xC7C) = -1;
						break;
					}	
				}
			}

			ImGui::ListBox("Animations", &Vars.Gameplay.iSelectedAnimation, Vars.AnimationListBoxList, _ARRAYSIZE(Vars.AnimationListBoxList));

			if (ImGui::Button("Play Animation"))
			{
				pCameraEnt->Animate(Vars.Gameplay.iSelectedAnimation, 0, 1, 0);
			}

			DisplayEntityHandles();

			ImGui::SameLine();

			if (ImGui::Button("Change Player"))
			{
				if (pCameraEnt)
				{
					pCameraEnt->m_nBones = Vars.Gameplay.nBones;
					Vars.Gameplay.nBones = 0;
					ChangePlayer(g_pLocalPlayer);
					pCameraEnt = GetEntityFromHandle(&g_pCamera->m_hEntity);
				}
			}

			ImGui::SameLine();

			if (ImGui::Button("Destroy Buddy") && g_pLocalPlayer)
			{
				Pl0000* pBuddy = GetEntityFromHandle(&g_pLocalPlayer->m_hBuddy);

				if (pBuddy)
					DestroyBuddy(pBuddy);
			}

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

			ImGui::ListBox("Spawn Blacklist", &Vars.Gameplay.iSelectedBlacklistItem, BlacklistItemCallback, Vars.Gameplay.SpawnBlacklist.data(), (int)Vars.Gameplay.SpawnBlacklist.size());
			ImGui::InputText("Blacklist Item", Vars.Gameplay.szBlacklistName, 32);
			
			if (ImGui::Button("Add Item"))
			{
				Vars.Gameplay.SpawnBlacklist.emplace_back(Vars.Gameplay.szBlacklistName);
			}

			ImGui::SameLine();

			if (ImGui::Button("Delete Item"))
			{
				Vars.Gameplay.SpawnBlacklist.erase(std::find(Vars.Gameplay.SpawnBlacklist.cbegin(), Vars.Gameplay.SpawnBlacklist.cend(), Vars.Gameplay.SpawnBlacklist.data()[Vars.Gameplay.iSelectedBlacklistItem]));
			}

			ApplyModelMods(pCameraEnt);

			ImGui::InputFloat3("Viewangles", (float*)&g_pCamera->m_viewangles);

			ImGui::Checkbox("No Tutorial Dialogs", &Vars.Gameplay.bNoTutorialDialogs);
			ImGui::Checkbox("SpeedMeister", &Vars.Gameplay.bSpeedMeister);
			ImGui::SameLine();
			ImGui::SliderFloat("Speed Multiplier", &Vars.Gameplay.flSpeedMultiplier, -10.0f, 10.0f); // when the framerate is uncapped this can go higher, else x5 will freeze the game because the game thinks the frame rate is too high so it sleeps the threads
			ImGui::Separator();
			ImGui::Checkbox("Wireframe", &Vars.Misc.bWireframe);
			ImGui::SameLine();
			ImGui::Checkbox("Anti-VSync", &Vars.Misc.bAntiVSync);
			ImGui::SameLine();
			ImGui::Checkbox("Anti-Framerate Cap", &Vars.Misc.bAntiFramerateCap);
			ImGui::SameLine();
			ImGui::Checkbox("Ignore Input", &Vars.Menu.bIgnoreInputWhenOpened);
			ImGui::InputText("Mount Custom Cpk", Vars.Misc.szCpkName, _ARRAYSIZE(Vars.Misc.szCpkName));

			if (ImGui::Button("Mount"))
			{
				CpkEntry* pEntry = Features::LoadCpk(Vars.Misc.szCpkName);
				Vars.Misc.bCpkLoaded = pEntry != NULL;
			}

			ImGui::SameLine();
			ImGui::Text((Vars.Misc.bCpkLoaded) ? "%s successfully mounted!" : "%s failed to mount!", Vars.Misc.szCpkName);
			ImGui::SameLine();

			if (ImGui::Button("Backup Save"))
			{
				Vars.Misc.bBackupSave = !BackupSave(Vars.Misc.nSlot);
			}

			ImGui::SameLine();
			ImGui::Text(Vars.Misc.bBackupSave ? "Succeded!" : "Failed!");

			ImGui::InputText("Config Name", Vars.Menu.Config.szName, _ARRAYSIZE(Vars.Menu.Config.szName));

			if (ImGui::Button("Load"))
				g_pConfig->Load(Vars.Menu.Config.szName);
	

			ImGui::SameLine();

			if (ImGui::Button("Save"))
				g_pConfig->Save(Vars.Menu.Config.szName);
		}
		ImGui::End();
	}

	g_pOverlay->Render();

	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	return oPresent(pThis, SyncInterval, Flags); //Anti-VSync Here bud (Vars.Misc.bAntiVSync) ? 0 : SyncInterval
}

HRESULT __fastcall hkCreateSwapChain(IDXGIFactory* pThis, IUnknown* pDevice, DXGI_SWAP_CHAIN_DESC* pDesc, IDXGISwapChain** ppSwapChain)
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


void __fastcall hkPSSetShaderResources(ID3D11DeviceContext* pThis, UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView* const * ppShaderResourceViews)
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

	pThis->PSGetConstantBuffers(g_PixelShaderStartSlot, 1, &g_pPixelShaderBuffer);

	if (g_pPixelShaderBuffer)
	{
		g_pPixelShaderBuffer->GetDesc(&g_PixelShaderBufferDesc);
		g_pPixelShaderBuffer->Release();
		g_pPixelShaderBuffer = NULL;
	}


	if (Vars.Misc.bWireframe && g_Stride == 28)
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

void __fastcall hkClearRenderTargetView(ID3D11DeviceContext* pThis, ID3D11RenderTargetView* pRenderTargetView, const FLOAT ColorRGBA[4])
{
	oClearRenderTargetView(pThis, pRenderTargetView, ColorRGBA);
}

HRESULT __fastcall hkKeyboardAcquire(IDirectInputDevice8A* pThis)
{
	g_pKeyboardHook->Unhook();

	HRESULT hr = oKeyboardAcquire(pThis);

	g_pKeyboardHook->Rehook();

	return hr;
}

HRESULT __fastcall hkKeyboardGetDeviceState(IDirectInputDevice8A* pThis, DWORD cbData, LPVOID lpvData)
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

HRESULT __fastcall hkMouseAcquire(IDirectInputDevice8A* pThis)
{
	g_pMouseHook->Unhook();

	HRESULT hr = oMouseAcquire(pThis);

	g_pMouseHook->Rehook();

	return hr;
}

HRESULT __fastcall hkMouseGetDeviceState(IDirectInputDevice8A* pThis, DWORD cbData, LPVOID lpvData)
{
	if (Vars.Menu.bOpened && Vars.Menu.bIgnoreInputWhenOpened)
		return DIERR_INPUTLOST;

	g_pMouseHook->Unhook();

	HRESULT hr = oMouseGetDeviceState(pThis, cbData, lpvData);

	g_pMouseHook->Rehook();

	memcpy(&Vars.Menu.Input.MouseState, lpvData, cbData);

	return hr;
}

void __fastcall hkUpdateModelParts(Pl0000* pEntity)
{
	LOG("Update model hook!");
}

void* __fastcall hkCreateEntity(void* pUnknown, EntityInfo* pInfo, unsigned int objectId, int flags, CHeapInstance** ppHeaps)
{
	ConstructionInfo<void>* pConstruct = GetConstructionInfo(objectId);

	void* pEntity = NULL;

	if (Vars.Gameplay.SpawnBlacklist.empty() || std::find(Vars.Gameplay.SpawnBlacklist.cbegin(), Vars.Gameplay.SpawnBlacklist.cend(), pConstruct->szName) == Vars.Gameplay.SpawnBlacklist.cend()) //strcmp("Em4000", pConstruct->szName) && strcmp("BehaviorFunnel", pConstruct->szName)
		pEntity = oCreateEntity(pUnknown, pInfo, objectId, flags, ppHeaps); //0x1401A2B40

	if (!pEntity)
		LOG("Failed to create %s -> %s (ObjectId = %x)\n", pInfo->m_szEntityType, pConstruct->szName, objectId);
	else
		LOG("Created %s -> %s (ObjectId = %x) Base %llx\n", pInfo->m_szEntityType, pConstruct->szName, objectId, pEntity);

	return pEntity;
}

BOOL __fastcall hkLoadWordBlacklist(BannedWordChecker* pThis, __int64 thisrdx, QWORD *thisr8, const char* szBlacklistName)
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

BOOL __fastcall hkQueryPerformanceCounter(LARGE_INTEGER* lpPerformanceCount)
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

LPTOP_LEVEL_EXCEPTION_FILTER __fastcall hkSetUnhandledExceptionFilter(LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter)
{
	auto& it = std::find(g_pExceptionHandlers.cbegin(), g_pExceptionHandlers.cend(), lpTopLevelExceptionFilter);

	if (it == g_pExceptionHandlers.cend())
		g_pExceptionHandlers.push_back(lpTopLevelExceptionFilter);

	if (lpTopLevelExceptionFilter == UnhandledExceptionHandler)
		return oSetUnhandledExceptionFilter(lpTopLevelExceptionFilter);
	else
		return UnhandledExceptionHandler;
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

	ZeroMemory(&Vars.Menu.Input.emulate, sizeof(XINPUT_STATE)); // need to zero out the input state before emulating new inputs

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