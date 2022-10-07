#pragma once
#include "Hooks.h"

void DisplayEntityHandles(void);
void ApplyModelMods(Pl0000* pEntity);
bool BlacklistItemCallback(void* data, int idx, const char** out_text);
bool ConfigCallback(void* data, int idx, const char** out_text);

static void GameplayTab(Pl0000* pCameraEnt)
{
	ImGui::Checkbox("Godmode", &Vars.Gameplay.bGodmode);
	ImGui::SameLine();
	ImGui::Checkbox("No Fall Damage", &Vars.Gameplay.bNoWorldDamage);
	ImGui::SameLine();
	ImGui::Checkbox("No Enemy Damage", &Vars.Gameplay.bNoEnemyDamage);

	ImGui::Columns(2);
	ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.2f);

	ImGui::InputInt("Experience:", g_piExperience, 1, 5);
	ImGui::InputInt("Level:", &Vars.Gameplay.iLevel, 1, 5);
	ImGui::SameLine();

	if (ImGui::Button("Apply Level"))
	{
		if (g_piExperience)
		{
			Vars.Gameplay.iLevel = min(MAX_LEVELS, max(MIN_LEVELS, Vars.Gameplay.iLevel));
			*g_piExperience = g_pLocalPlayer->m_LevelsContainer.m_levels[Vars.Gameplay.iLevel - 1].m_iMinimumExperience;
		}
	}

	ImGui::InputInt("Money:", g_piMoney, 1000, 10000);
	ImGui::SameLine();

	if (ImGui::Button("Max Money"))
		*g_piMoney = MAX_MONEY;

	ImGui::PopItemWidth();
	ImGui::NextColumn();
	ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.2f);

	ImGui::InputInt((Vars.Gameplay.bLevelBuffMode) ? "Enemy Level (1-99):" : "Enemy Level Tolerance (+/-):", (Vars.Gameplay.bLevelBuffMode) ? &Vars.Gameplay.iEnemyLevel : &Vars.Gameplay.iEnemyLevelTolerance, 1, 5);
	ImGui::Checkbox("Balance Enemy Levels", &Vars.Gameplay.bBalanceEnemyLevels);
	ImGui::SameLine();
	ImGui::Checkbox("Exclusively Positive", &Vars.Gameplay.bExclusivelyPositiveTolerance);
	ImGui::Checkbox("Absolute Level", &Vars.Gameplay.bLevelBuffMode);

	ImGui::PopItemWidth();
	ImGui::Columns();

	char szDesc[64];
	char* szName = (char*)ItemManager_GetItemNameById(g_pItemManager, Vars.Gameplay.iSpawnItemId);
	// FIXME: Broken on the new version this fucking function keeps crashing
	sprintf_s(szDesc, "Item Name or Id: (%s)", szName);
	ImGui::InputText(szDesc, Vars.Gameplay.szItemName, _ARRAYSIZE(Vars.Gameplay.szItemName));
	int id = ItemManager_GetItemIdByName(g_pItemManager, Vars.Gameplay.szItemName);
	Vars.Gameplay.iSpawnItemId = (id != -1) ? id : atoi(Vars.Gameplay.szItemName);
	ImGui::InputInt("Quantity", &Vars.Gameplay.iSpawnItemQuantity);
	ImGui::Checkbox("Instant Equip", &Vars.Gameplay.bInstantEquip);
	ImGui::SameLine();

	if (ImGui::Button("Spawn Item"))
		Features::AddItem(Vars.Gameplay.iSpawnItemId, Vars.Gameplay.iSpawnItemQuantity);

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
		set_info.m_vRotation = pCameraEnt->m_matModelToWorld.GetAxis(POSITION); //  Vector3Aligned(0, 1, 0);
		set_info.m_vScale = Vars.Gameplay.vSpawnEntityScale;
		set_info.m_i0x080 = -1;
		set_info.m_i0x084 = 0;
		set_info.m_i0x088 = 1;
		set_info.m_i0x08C = -1;
		set_info.m_i0x07C = 1; //flags
		set_info.m_dwSetType = 1;
		set_info.m_dw0x74 = 0;
		set_info.m_i0x078 = 0;

		//ObjReadSystem::Work* pWork = GetWork(Vars.SpawnEntities[Vars.Gameplay.iSelectedEntityType].m_ObjectId);

		//if (pWork)
		//{
		//	BOOL s = PreloadModel(pWork);	
		//	RequestEnd(0, Vars.SpawnEntities[Vars.Gameplay.iSelectedEntityType].m_ObjectId);
		//}

		CEntityInfo* pInfo = Features::CreateEntity(Vars.SpawnEntities[Vars.Gameplay.iSelectedEntityType].m_szClass, Vars.SpawnEntities[Vars.Gameplay.iSelectedEntityType].m_ObjectId, &set_info);

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
			case 0x11010:
				*g_pEmilHandle = pInfo->m_hEntity;
				break;
			case 0x10010:
				typedef void (*Pl_ChangeFlightSuitFn)(void* pEnt);
				//pCameraEnt->m_bFlightSuit = TRUE;
				pCameraEnt->m_hFlightSuit = pInfo->m_hEntity;
				((Pl_ChangeFlightSuitFn)((ULONG_PTR)GetModuleHandle(NULL) + 0x4F8300))(pCameraEnt); /*0x4E9DB0*/
				//((Pl_ChangeFlightSuitFn)((ULONG_PTR)GetModuleHandle(NULL) + 0x4E0340))(pCameraEnt);
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
		Features::SwapPlayer();
	}

	ImGui::SameLine();

	if (ImGui::Button("Destroy Buddy") && g_pLocalPlayer)
	{
		Pl0000* pBuddy = GetEntityFromHandle2(&g_pLocalPlayer->m_hBuddy);

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

	ImGui::Checkbox("No Model Collision", &Vars.Gameplay.bNoCollision);

	Vars.Gameplay.bNoCollision ? Features::RemoveHorizontalCollision(pCameraEnt) : Features::ApplyHorizontalCollision(pCameraEnt);

	ImGui::SameLine();

	if (pCameraEnt)
		ImGui::Checkbox("Model Gravity", (bool*)&pCameraEnt->m_VerticalCollision.m_bEnabled);

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
}

static void VisualsTab(Pl0000* pCameraEnt)
{
	ImGui::Checkbox("Wireframe", &Vars.Misc.bWireframe);

	ImGui::SameLine();

	ImGui::Checkbox("Chams", &Vars.Visuals.bChams);

	ImGui::SameLine();

	ImGui::Checkbox("Use Shader", (bool*)&Vars.Visuals.iChamType);

	ImGui::SameLine();

	ImGui::Checkbox("Display Enemy Info", &Vars.Visuals.bEnemyInfo);

	ImGui::SameLine();

	ImGui::Checkbox("Display NPC Info", &Vars.Visuals.bNPCInfo);

	ImGui::SameLine();

	ImGui::Checkbox("Display Collision Object Info", &Vars.Visuals.bCollisionObjectInfo);
	ImGui::Checkbox("Display Collision Debug Object Info", &Vars.Visuals.bCollisionDebugObjectInfo);

	ImGui::Separator();

	ImGui::DragFloat("Camera Fov", &Vars.Visuals.flFov, 1.0f, 20.0f, 300.0f);

	ImGui::Separator();
	//ImGui::Checkbox("Display Emil Info", &Vars.Visuals.bEmilInfo);

	ApplyModelMods(pCameraEnt);
}

static void MiscTab(void)
{
	if (ImGui::Button("Teleport Emil To Me"))
		Features::TeleportEntityToOther(*g_pLocalPlayerHandle, *g_pEmilHandle);

	ImGui::SameLine();

	if (ImGui::Button("Teleport Me To Emil"))
		Features::TeleportEntityToOther(*g_pEmilHandle, *g_pLocalPlayerHandle);

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

	ImGui::InputFloat3("Viewangles", (float*)&g_pCamera->m_vViewangles);

	ImGui::Checkbox("Speedhack", &Vars.Gameplay.bSpeedhack);
	ImGui::SameLine();
	ImGui::SliderFloat("Speed Multiplier", &Vars.Gameplay.flSpeedMultiplier, -10.0f, 10.0f); // when the framerate is uncapped this can go higher, else x5 will freeze the game because the game thinks the frame rate is too high so it sleeps the threads
	ImGui::Separator();
	ImGui::InputText("Mount Custom Cpk", Vars.Misc.szCpkName, _ARRAYSIZE(Vars.Misc.szCpkName));

	if (ImGui::Button("Mount"))
	{
		CpkEntry* pEntry = Features::LoadCpk(Vars.Misc.szCpkName);
		Vars.Misc.bCpkLoaded = pEntry != NULL;
	}
	ImGui::SameLine();
	ImGui::Text((Vars.Misc.bCpkLoaded) ? "%s successfully mounted!" : "%s failed to mount!", Vars.Misc.szCpkName);

	if (ImGui::Button("Unlock All Achivements"))
		Features::UnlockAllAchievements();

	ImGui::Checkbox("No Tutorial Dialogs", &Vars.Gameplay.bNoTutorialDialogs);
	ImGui::SameLine();
	ImGui::Checkbox("Anti-VSync", &Vars.Misc.bAntiVSync);
	ImGui::SameLine();
	ImGui::Checkbox("Anti-Framerate Cap", &Vars.Misc.bAntiFramerateCap);
	ImGui::SameLine();

	if (ImGui::Button("Backup Save"))
	{
		Vars.Misc.bBackupSave = !BackupSave(Vars.Misc.nSlot);
	}

	ImGui::SameLine();
	ImGui::Text(Vars.Misc.bBackupSave ? "Succeded!" : "Failed!");

	static int ui_ids[3];

	ImGui::InputInt3("UI Id", ui_ids);
	ImGui::SameLine();

	if (ImGui::Button("Create UI Element"))
	{	
		TODO("Move function typedefs to globals!")
		typedef unsigned int (*HashLowercaseStringCRC32Fn)(const char* szString);
		// Retreives game text from crc32
		typedef wchar_t*(*DialogUI_System_GetTextFn)(void* pThis, int crc32, unsigned int index);
		// Creates a white dialog
		typedef __int64(*DialogUI_System_CreateYesNoDialogFn)(wchar_t* szMessage);
		// Creates a white dialog w/ options
		typedef __int64(*DialogUI_System_CreateDialogFn)(void* pThis, wchar_t* szMessage, int bWhiteDialog, DialogUIWhiteType type, int darkness);
		// Creates a black banner
		typedef __int64(*DialogUI_System_CreateBlackBannerFn)(void* pThis, int a1, int crc32, wchar_t* szMessage, wchar_t* szText, DialogUIBlackType type, unsigned int a8, int darkness);
		

		ULONG_PTR uNier = (ULONG_PTR)GetModuleHandle(NULL);

		unsigned int crc = ((HashLowercaseStringCRC32Fn)(uNier + 0x14C4C0))("CORE_QUEST_EMP_TXT_MONEY");// CORE_MAP_VOICECNG_01
		wchar_t* szMsg = ((DialogUI_System_GetTextFn)(uNier + 0x9BD090))((void*)(uNier + 0x14207A0), crc, 24);
		// e8a92ebf crc32 | CORE_ITEM_NAME_935
		//((DialogUI_System_CreateDialogFn)(uNier + 0x2F4F60))((void*)(uNier + 0xF04018), L"Cazzone", (DialogUIWhiteType)ui_ids[0], ui_ids[1], ui_ids[2]);
		((DialogUI_System_CreateBlackBannerFn)(uNier + 0x2F5040))((void*)(uNier + 0xF04018), -1, 0, szMsg, L"It wasn't supposed to be like this!\nCazzone", (DialogUIBlackType)ui_ids[0], ui_ids[1], ui_ids[2]);
		//((DialogUI_System_CreateYesNoDialogFn)(uNier + 0x2F4760))(L"Bad News!");
		//CreateUIFromId(ui_id);
	}

	// new test shit
	ImGui::InputInt("Debug Flags", (int*)((ULONG_PTR)GetModuleHandle(NULL) + 0x1029844), 1, 100, ImGuiInputTextFlags_CharsHexadecimal); 
	ImGui::InputInt("Camera Flags", (int*)((ULONG_PTR)GetModuleHandle(NULL) + 0x1029840), 1, 100, ImGuiInputTextFlags_CharsHexadecimal);
}

static void ConfigTab(void)
{
	LPCTSTR szConfig;

	ImGui::Checkbox("Ignore Input", &Vars.Menu.bIgnoreInputWhenOpened);

	for (auto& it : g_pConfig->GetKeybinds())
	{
		KeyOrdinal* pKey = FindKeyOrdinal(it->GetKeycode());
		
		if (pKey)
		{
			if (ImGui::BeginCombo(it->GetName(), pKey->m_szName))
			{
				for (int i = 0; i < ARRAYSIZE(s_Keycodes); ++i)
					if (ImGui::Selectable(s_Keycodes[i].m_szName))
						it->SetKeycode(s_Keycodes[i].m_uKeyCode);

				ImGui::EndCombo();
			}
		}
	}

	ImGui::ListBox("Configs", &Vars.Menu.Config.iSelectedConfig, ConfigCallback, Vars.Menu.Config.pHead, (INT)FindDataListCount(Vars.Menu.Config.pHead));
	ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.5f);
	ImGui::InputText("Config Name", Vars.Menu.Config.szName, _ARRAYSIZE(Vars.Menu.Config.szName));
	ImGui::PopItemWidth();
	ImGui::SameLine();

	PWIN32_FIND_DATA_LIST pSelected = FindDataListNav(Vars.Menu.Config.pHead, Vars.Menu.Config.iSelectedConfig);
	szConfig = Vars.Menu.Config.szName[0] ? Vars.Menu.Config.szName : (pSelected) ? pSelected->m_Data.cFileName : NULL;

	if (ImGui::Button("Refresh"))
	{
		FindDataListFree(Vars.Menu.Config.pHead);
		g_pConfig->EnumerateConfigs(NULL, &Vars.Menu.Config.pHead);
	}

	if (ImGui::Button("Load"))
		g_pConfig->Load(szConfig);

	ImGui::SameLine();

	if (ImGui::Button("Save"))
		g_pConfig->Save(szConfig);
}

static void DisplayEntityHandles()
{
	if (!g_pNPCManager)
		return;

	int count = (int)(g_pNPCManager->m_handles.m_count + g_pYorhaManager->m_handles.m_count + g_pEnemyManager->m_handles.m_count);
	char** ppszHandles = new char*[count];
	EntityHandle* handles = new EntityHandle[count];

	int i = 0;
	for (; i < g_pNPCManager->m_handles.m_count; ++i)
	{
		ppszHandles[i] = new char[64];
		EntityHandle hCurrent = g_pNPCManager->m_handles.m_pItems[i];
		handles[i] = hCurrent;
		Pl0000* pCurrent = GetEntityFromHandle(&hCurrent);

		sprintf_s(ppszHandles[i], 64, "NPC: %s | Handle: %x | ObjectId: %x", pCurrent ? pCurrent->m_pInfo->m_szEntityType : 0, hCurrent, pCurrent ? pCurrent->m_ObjectId : 0);
	}

	for (; i < g_pNPCManager->m_handles.m_count + g_pEnemyManager->m_handles.m_count; ++i)
	{
		ppszHandles[i] = new char[64];
		EntityHandle hCurrent = g_pEnemyManager->m_handles.m_pItems[i];
		handles[i] = hCurrent;
		Pl0000* pCurrent = GetEntityFromHandle(&hCurrent);

		sprintf_s(ppszHandles[i], 64, "Enemy %s | Handle: %x | ObjectId: %x", pCurrent ? pCurrent->m_pInfo->m_szEntityType : 0, hCurrent, pCurrent ? pCurrent->m_ObjectId : 0);
	}

	for (; i < count; ++i)
	{
		ppszHandles[i] = new char[64];
		EntityHandle hCurrent = g_pYorhaManager->m_handles.m_pItems[i];
		handles[i] = hCurrent;
		Pl0000* pCurrent = GetEntityFromHandle(&hCurrent);

		sprintf_s(ppszHandles[i], 64, "Yorha: %s | Handle: %x | ObjectId: %x", pCurrent ? pCurrent->m_pInfo->m_szEntityType : 0, hCurrent, pCurrent ? pCurrent->m_ObjectId : 0);
	}

	ImGui::ListBox("NPC, Yorha & Enemy Handles", &Vars.Gameplay.iSelectedEntityHandle, ppszHandles, count);

	ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.25f);

	ImGui::InputInt("Level", &Vars.Gameplay.iSelectedEntityLevel, 1, 5);
	
	ImGui::SameLine();

	if (ImGui::Button("Set Level"))
		Features::BalanceEnemyLevel(GetEntityFromHandle(&handles[Vars.Gameplay.iSelectedEntityHandle]), Vars.Gameplay.iSelectedEntityLevel - 1, Vars.Gameplay.iSelectedEntityLevel - 1);\

	ImGui::PopItemWidth();

	if (ImGui::Button("Set NPC Follow"))
	{
		Pl0000* pNPC = GetEntityFromHandle(&handles[Vars.Gameplay.iSelectedEntityHandle]);

		if (pNPC)
			((NPC_ChangeSetTypeFollowFn)(0x1404B2770))(pNPC);
	}

	ImGui::SameLine();

	if (ImGui::Button("Set NPC Idle"))
	{
		Pl0000* pNPC = GetEntityFromHandle(&handles[Vars.Gameplay.iSelectedEntityHandle]);

		if (pNPC)
			((NPC_ChangeSetTypeIdleFn)(0x1404B2770))(pNPC);
	}

#ifdef  _DEBUG
	ImGui::SameLine();

	if (ImGui::Button("Set Player"))
	{
		Features::SetPlayer(handles[Vars.Gameplay.iSelectedEntityHandle]);
	}
#endif

	for (int i = 0; i < count; ++i)
		delete[] ppszHandles[i];

	delete[] ppszHandles;
	delete[] handles;
}

static void ApplyPodMods(Pl0000* pOwner)
{
	if (!pOwner)
		return;

	Pl0000* pPod = GetEntityFromHandle(&pOwner->m_hPod);

	if (!pPod)
		return;

	ImGui::Checkbox("Rainbow Pod", &Vars.Gameplay.bRainbowPod);
	ImGui::SameLine();
	ImGui::Checkbox("Hide Pod", &Vars.Gameplay.bHidePod);

	pPod->m_pModelInfo->m_vTint.w = Vars.Gameplay.bHidePod ? 0.f : 1.f;

	char szModelPart[64];
	sprintf_s(szModelPart, "Pod Mesh: (%s)", pPod->m_Work.m_pMeshes[Vars.Gameplay.iSelectedPodMesh].m_szMeshName);
	ImGui::SliderInt(szModelPart, &Vars.Gameplay.iSelectedPodMesh, 0, pPod->m_Work.m_nMeshes - 1);

	sprintf_s(szModelPart, "Pod Mesh Color: (%s)", pPod->m_Work.m_pMeshes[Vars.Gameplay.iSelectedPodMesh].m_szMeshName);

	if (ImGui::Checkbox("Enabled", (bool*)&pPod->m_Work.m_pMeshes[Vars.Gameplay.iSelectedPodMesh].m_bShow))
		pPod->m_Work.m_pMeshes[Vars.Gameplay.iSelectedPodMesh].m_bUpdate = TRUE;

	ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.3f);
	
	ImGui::ColorPicker4(szModelPart, (float*)&pPod->m_Work.m_pMeshes[Vars.Gameplay.iSelectedPodMesh].m_vColor);
	
	ImGui::PopItemWidth();

	pPod->m_Work.m_pMeshes[Vars.Gameplay.iSelectedModelMesh].m_bUpdate = TRUE;
	pPod->m_Work.m_pMeshes[Vars.Gameplay.iSelectedModelMesh].m_bShow = TRUE;
}

static void ApplyModelMods(Pl0000* pEntity)
{
	if (!pEntity)
		return;

	if (ImGui::Button("Wet Entity"))
		Features::WetEntity(pEntity, 127);

	ImGui::SameLine();

	ImGui::Checkbox("Firstperson", &Vars.Misc.bFirstperson);

	ImGui::SameLine();

	ImGui::Checkbox("Draw Trace Line", &Vars.Visuals.bTraceLine);

	ImGui::SameLine();

	ImGui::Checkbox("Draw ESP Box", &Vars.Visuals.bEspBox);

	ImGui::SameLine();

	ImGui::Checkbox("Draw 2D Esp Box", &Vars.Visuals.b2DEspBox);

	ImGui::Checkbox("Draw Skeleton", &Vars.Visuals.bSkeleton);

	ImGui::SameLine();

	ImGui::Checkbox("Debug Skeleton", &Vars.Visuals.bDebugLocalPlayerSkeleton);

	ImGui::DragFloat("Trace Line Length", &Vars.Visuals.flTraceLength, 0.00625f, 0.4f, 5.0f);

	ImGui::InputFloat3("Position (X,Y,Z)", (float*)&pEntity->m_vPosition);
	ImGui::InputFloat3("Model Scale (X,Y,Z)", (float*)&pEntity->m_matModelToWorld.GetAxis(1));
	ImGui::InputFloat3("Model Rotation (Pitch, Yaw, Roll)", (float*)&pEntity->m_matModelToWorld.GetAxis(3));

	ImGui::Checkbox("Rainbow Hair", &Vars.Gameplay.bRainbowHair);

	if (Vars.Gameplay.iSelectedModelMesh > pEntity->m_Work.m_nMeshes)
		Vars.Gameplay.iSelectedModelMesh = pEntity->m_Work.m_nMeshes;

	CMesh2* pSelectedMesh = &pEntity->m_Work.m_pMeshes[Vars.Gameplay.iSelectedModelMesh];

	char szModelPart[64];
	sprintf_s(szModelPart, "Mesh: %s", pSelectedMesh->m_szMeshName);
	ImGui::SliderInt(szModelPart, &Vars.Gameplay.iSelectedModelMesh, 0, pEntity->m_Work.m_nMeshes - 1);

	sprintf_s(szModelPart, "Mesh %s Color", pSelectedMesh->m_szMeshName);

	if (ImGui::Checkbox("Enabled", (bool*)&pSelectedMesh->m_bShow))
		pSelectedMesh->m_bUpdate = TRUE;

	ImGui::InputText("Custom Texture (DDS)", Vars.Gameplay.szModelTextureName, MAX_PATH);
	ImGui::SameLine();

	if (ImGui::Button("Load"))
	{
		CTextureDescription desc;
		CTargetTexture* pTexture = CreateTexture(Vars.Gameplay.szModelTextureName, desc);

		if (pTexture)
		{
			CMesh2* pMesh = &g_pLocalPlayer->m_Work.m_pMeshes[Vars.Gameplay.iSelectedModelMesh];

			for (int i = 0; i < pMesh->m_nShaderInfo; ++i)
			{
				CMaterial* pMat = pMesh->m_pShaderInfo[i].m_pShader->m_pMaterial;

				SwapTexture(pMat->m_TextureIds[0], pTexture->m_pTexture);

				/*
				for (int k = 0; k < MAX_MATERIAL_TEXTURES - 1; ++k)
					SwapTexture(pMat->m_TextureIds[k], pTexture->m_pTexture);
				*/
			}
		}
	}

	ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.3f);

	if (!strcmp(pSelectedMesh->m_szMeshName, "Hair"))
	{
		if (!Vars.Gameplay.bRainbowHair)
		{
			ImGui::ColorPicker4(szModelPart, (float*)&pSelectedMesh->m_vColor);
			pSelectedMesh->m_bUpdate = TRUE;
			pSelectedMesh->m_bShow = TRUE;
		}
	}
	else
	{
		ImGui::ColorPicker4(szModelPart, (float*)&pSelectedMesh->m_vColor);
		pSelectedMesh->m_bUpdate = TRUE;
		pSelectedMesh->m_bShow = TRUE;
	}

	ImGui::SameLine();

	if (!Vars.Gameplay.bRainbowModel)
		ImGui::ColorPicker4("Model Tint Color", (float*)&pEntity->m_pModelInfo->m_vTint);

	ImGui::PopItemWidth();

	ApplyPodMods(pEntity);
}

static bool BlacklistItemCallback(void* data, int idx, const char** out_text)
{
	if (out_text)
		*out_text = ((std::string*)data)[idx].c_str();

	return true;
}

/*
this might be needed
	for (int i = 0; i < idx; pEntry = pEntry->m_pNext)
		if (!pEntry->m_pNext)
			return false;
*/
static bool ConfigCallback(void* data, int idx, const char** out_text)
{
	PWIN32_FIND_DATA_LIST pEntry = FindDataListNav((PCWIN32_FIND_DATA_LIST)data, idx);

	if (pEntry)
	{
		if (out_text)
			*out_text = pEntry->m_Data.cFileName;

		return true;
	}
	else return false;
}