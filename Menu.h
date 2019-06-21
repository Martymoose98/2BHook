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

	ImGui::InputInt((Vars.Gameplay.bLevelBuffMode) ? "Enemy Level (1-99):" : "Enemy Level Tolerance (+/-):", (Vars.Gameplay.bLevelBuffMode) ? &Vars.Gameplay.iEnemyLevel : &Vars.Gameplay.iEnemyLevelTolerance, 1, 5);
	ImGui::Checkbox("Balance Enemy Levels", &Vars.Gameplay.bBalanceEnemyLevels);
	ImGui::SameLine();
	ImGui::Checkbox("Exclusively Positive", &Vars.Gameplay.bExclusivelyPositiveTolerance);
	ImGui::Checkbox("Absolute Level", &Vars.Gameplay.bLevelBuffMode);

	ImGui::InputInt("Money:", g_piMoney, 1000, 10000);
	ImGui::SameLine();

	if (ImGui::Button("Max Money"))
		*g_piMoney = MAX_MONEY;

	ImGui::PopItemWidth();
	ImGui::Columns();

	char szDesc[64];
	sprintf_s(szDesc, "Item Name or Id: (%s)", ((GetItemByIdFn)(0x1405E0FD0))(0, Vars.Gameplay.iSpawnItemId));
	ImGui::InputText(szDesc, Vars.Gameplay.szItemName, _ARRAYSIZE(Vars.Gameplay.szItemName));
	int id = ((GetItemIdByNameFn)(0x1405DE6E0))(0, Vars.Gameplay.szItemName);
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

		//ObjReadSystem::Work* pWork = GetWork(Vars.SpawnEntities[Vars.Gameplay.iSelectedEntityType].m_ObjectId);

		//if (pWork)
		//{
		//	BOOL s = PreloadModel(pWork);	
		//	RequestEnd(0, Vars.SpawnEntities[Vars.Gameplay.iSelectedEntityType].m_ObjectId);
		//}

		EntityInfo* pInfo = Features::CreateEntity(Vars.SpawnEntities[Vars.Gameplay.iSelectedEntityType].m_szClass, Vars.SpawnEntities[Vars.Gameplay.iSelectedEntityType].m_ObjectId, &set_info);

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

	ImGui::Checkbox("Display Enemy Info", &Vars.Visuals.bEnemyInfo);

	ImGui::SameLine();

	ImGui::Checkbox("Display NPC Info", &Vars.Visuals.bNPCInfo);

	ImGui::SameLine();

	ImGui::Checkbox("Display Collision Object Info", &Vars.Visuals.bCollisionObjectInfo);

	ImGui::Separator();

	//ImGui::Checkbox("Display Emil Info", &Vars.Visuals.bEmilInfo);

	ApplyModelMods(pCameraEnt);
}

static void MiscTab()
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

	ImGui::InputFloat3("Viewangles", (float*)&g_pCamera->m_viewangles);

	ImGui::Checkbox("SpeedMeister", &Vars.Gameplay.bSpeedMeister);
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

}

TODO("v0.18 add customization for keybinds")
static void ConfigTab()
{
	LPCTSTR szConfig;

	ImGui::Checkbox("Ignore Input", &Vars.Menu.bIgnoreInputWhenOpened);
	ImGui::ListBox("Configs", &Vars.Menu.Config.iSelectedConfig, ConfigCallback, Vars.Menu.Config.pHead, FindDataListCount(Vars.Menu.Config.pHead));
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

	ImGui::SameLine();

	if (ImGui::Button("Set Player"))
	{
		Features::SetPlayer(handles[Vars.Gameplay.iSelectedEntityHandle]);
	}

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
	sprintf_s(szModelPart, "Pod ModelPart: (%s)", pPod->m_pModelParts[Vars.Gameplay.iSelectedPodModelPart].m_szModelPart);
	ImGui::SliderInt(szModelPart, &Vars.Gameplay.iSelectedPodModelPart, 0, pPod->m_nModelParts - 1);

	sprintf_s(szModelPart, "Pod ModelPart Color: (%s)", pPod->m_pModelParts[Vars.Gameplay.iSelectedPodModelPart].m_szModelPart);

	if (ImGui::Checkbox("Enabled", (bool*)&pPod->m_pModelParts[Vars.Gameplay.iSelectedPodModelPart].m_bShow))
		pPod->m_pModelParts[Vars.Gameplay.iSelectedPodModelPart].m_bUpdate = TRUE;

	ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.3f);
	
	ImGui::ColorPicker4(szModelPart, (float*)&pPod->m_pModelParts[Vars.Gameplay.iSelectedPodModelPart].m_vColor);
	
	ImGui::PopItemWidth();

	pPod->m_pModelParts[Vars.Gameplay.iSelectedModelPart].m_bUpdate = TRUE;
	pPod->m_pModelParts[Vars.Gameplay.iSelectedModelPart].m_bShow = TRUE;
}

static void ApplyModelMods(Pl0000* pEntity)
{
	if (!pEntity)
		return;

	if (ImGui::Button("Wet Entity"))
		Features::WetEntity(pEntity, 127);

	ImGui::SameLine();

	ImGui::Checkbox("Ghost Model", &Vars.Gameplay.bGhostModel);

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

	if (Vars.Gameplay.iSelectedModelPart > pEntity->m_nModelParts)
		Vars.Gameplay.iSelectedModelPart = pEntity->m_nModelParts;

	char szModelPart[64];
	sprintf_s(szModelPart, "ModelPart: (%s)", pEntity->m_pModelParts[Vars.Gameplay.iSelectedModelPart].m_szModelPart);
	ImGui::SliderInt(szModelPart, &Vars.Gameplay.iSelectedModelPart, 0, pEntity->m_nModelParts - 1);

	sprintf_s(szModelPart, "ModelPart Color: (%s)", pEntity->m_pModelParts[Vars.Gameplay.iSelectedModelPart].m_szModelPart);

	if (ImGui::Checkbox("Enabled", (bool*)&pEntity->m_pModelParts[Vars.Gameplay.iSelectedModelPart].m_bShow))
		pEntity->m_pModelParts[Vars.Gameplay.iSelectedModelPart].m_bUpdate = TRUE;

	ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.3f);

	if (!strcmp(pEntity->m_pModelParts[Vars.Gameplay.iSelectedModelPart].m_szModelPart, "Hair"))
	{
		if (!Vars.Gameplay.bRainbowHair)
		{
			ImGui::ColorPicker4(szModelPart, (float*)&pEntity->m_pModelParts[Vars.Gameplay.iSelectedModelPart].m_vColor);
			pEntity->m_pModelParts[Vars.Gameplay.iSelectedModelPart].m_bUpdate = TRUE;
			pEntity->m_pModelParts[Vars.Gameplay.iSelectedModelPart].m_bShow = TRUE;
		}
	}
	else
	{
		ImGui::ColorPicker4(szModelPart, (float*)&pEntity->m_pModelParts[Vars.Gameplay.iSelectedModelPart].m_vColor);
		pEntity->m_pModelParts[Vars.Gameplay.iSelectedModelPart].m_bUpdate = TRUE;
		pEntity->m_pModelParts[Vars.Gameplay.iSelectedModelPart].m_bShow = TRUE;
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