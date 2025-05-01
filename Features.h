#pragma once

#include <Fluorine\Memory.h>

#include "ReversedStructs.h"
#include "Vector3.h"
#include "Log.h"
#include "Console.h"

CBehaviorAppBase* GetEntityByHandle(const CEntityList* pList, EntityHandle hEntity);
CBehaviorAppBase* GetEntityFromHandleGlobal(EntityHandle* phEntity);
int GetModelMeshIndex(const CModelWork* pWork, const char* szMesh);


namespace Features
{
	static void ApplyHealthMods(void)
	{
		if (Vars.Gameplay.bGodmode || Vars.Gameplay.bNoEnemyDamage)
			NopMemory(&nop_Health[NOP_DAMAGE_ENEMY]);

		if (Vars.Gameplay.bGodmode || Vars.Gameplay.bNoWorldDamage)
			NopMemory(&nop_Health[NOP_DAMAGE_WORLD]);

		if (!Vars.Gameplay.bGodmode && !Vars.Gameplay.bNoEnemyDamage)
			RestoreMemory(&nop_Health[NOP_DAMAGE_ENEMY]);

		if (!Vars.Gameplay.bGodmode && !Vars.Gameplay.bNoWorldDamage)
			RestoreMemory(&nop_Health[NOP_DAMAGE_WORLD]);
	}

	static void AddEntityToBlackList()
	{

	}

	static void ChangeBuddy(int buddy)
	{
		// I don't know what this is. It appears to be related to scene entity system
		static CSceneEntitySystem* pSceneEntitySystem = (CSceneEntitySystem*)FindPatternPtr(NULL, "48 8D 0D ? ? ? ? E8 ? ? ? ? 48 89 44 24 ? 48 85 C0 74 ? 8B 50", 3);
		unsigned int crc;
		SceneState* pScene;

		switch (buddy)
		{
		case PROTAGONIST_2B:
			crc = HashStringCRC32("Buddy/2B", 8);
			pScene = (SceneState*)SceneEntitySystem_FindSceneState(pSceneEntitySystem, crc, "Buddy/2B", 8);
			LOG("return value: %x\n", pScene);
			break;
		case PROTAGONIST_A2:
			crc = HashStringCRC32("Buddy/A2", 8);
			pScene = (SceneState*)SceneEntitySystem_FindSceneState(pSceneEntitySystem, crc, "Buddy/A2", 8);
			LOG("return value: %x\n", pScene);
			break;
		case PROTAGONIST_9S:
			crc = HashStringCRC32("Buddy/9S", 8);
			pScene = (SceneState*)SceneEntitySystem_FindSceneState(pSceneEntitySystem, crc, "Buddy/9S", 8);
			LOG("return value: %x\n", pScene);
			break;
		default:
			return;
		}
		SceneStateSystem_Set(g_pSceneStateSystem, &pScene);
		// old ver
		//((SceneStateSystem_SetFn)(0x14001EBE0))((void*)0x14158CBC0, &pScene);
	}

	// exprimential
	static void AddBuddy(void)
	{
		// I don't know what this is. It appears to be related to scene entity system
		static CSceneEntitySystem* pRBTreeScenes = (CSceneEntitySystem*)FindPatternPtr(NULL, "48 8D 0D ? ? ? ? E8 ? ? ? ? 48 89 44 24 ? 48 85 C0 74 ? 8B 50", 3);
		unsigned int crc;
		void* pScene;
		__int64 val;

		crc = HashStringCRC32("Buddy/A2", 8);
		pScene = SceneEntitySystem_FindSceneState(pRBTreeScenes, crc, "Buddy/A2", 8);


		//(*(bool(*)(void*, __int64*))(0x14001EC80))((void*)0x14158CBC0, &ret);

		val = (*(__int64(*)(void*, unsigned long*))(0x140058A90))((void*)0x14158CC50, (DWORD*)(*(QWORD*)pScene + 0x20)); // != 0x6CFB210

		(*(__int64(*)(__int64, void*))(0x140053B00))(0x6CFB210, pScene);
	}

	// exprimential
	static void AddPlayer(int player)
	{
		// I don't know what this is. It appears to be related to scene entity system (
		static CSceneEntitySystem* pRBTreeScenes = (CSceneEntitySystem*)FindPatternPtr(NULL, "48 8D 0D ? ? ? ? E8 ? ? ? ? 48 89 44 24 ? 48 85 C0 74 ? 8B 50", 3);
		unsigned int crc;
		SceneState* pScene;

		switch (player)
		{
		case PROTAGONIST_2B:
			//crc = HashStringCRC32("PL/2B", 8);
			//pScene = (SceneState*)FindSceneState(&cs, crc, "PL/2B", 8);
			crc = HashStringCRC32("Load/A29S", 10);
			pScene = (SceneState*)SceneEntitySystem_FindSceneState(pRBTreeScenes, crc, "Load/A29S", 10); // Load/A29S | Load/A22B	
			SceneStateSystem_Set(g_pSceneStateSystem, &pScene);
			break;
		case PROTAGONIST_A2:
			crc = HashStringCRC32("PL/A2", 5);
			pScene = (SceneState*)SceneEntitySystem_FindSceneState(pRBTreeScenes, crc, "PL/A2", 5);
			SceneStateSystem_Set(g_pSceneStateSystem, &pScene);
			break;
		default:
			break;
		}
	}

	static void ApplyHorizontalCollision(Pl0000* pEntity)
	{
		if (!pEntity)
			return;

		pEntity->m_HorizontalCollision.m_hOwner = pEntity->m_pInfo->m_hEntity;
		pEntity->m_HorizontalCollision.m_bEnabled = TRUE;
		pEntity->m_HorizontalCollision.m_bCollision = FALSE;
	}

	static void RemoveHorizontalCollision(Pl0000* pEntity)
	{
		if (!pEntity)
			return;

		pEntity->m_HorizontalCollision.m_hOwner = 0;
		pEntity->m_HorizontalCollision.m_bEnabled = FALSE;
		pEntity->m_HorizontalCollision.m_bCollision = FALSE;
		pEntity->m_HorizontalCollision.m_vMin = pEntity->m_vPosition;
	}

	static void UnlockAllAchievements(void)
	{
		for (uint32_t id = 0; id < g_pUserManager->m_pAchivementDevice->m_uMaxAchievement; ++id)
			UnlockAchievement(0, 0, id);
	}

	//WIP: head mesh comes back on animations this needs to be fixed
	// also camera should rotate with bone
	// NOTE: rainbow hair is not compatible with this feature
	static void Firstperson(CBehavior* pEntity)
	{
		if (pEntity &&
			(Vars.Misc.bCameraFlags & Variables_t::Misc_t::CameraFlg::CAMERA_FIRSTPERSON) &&
			!(Vars.Misc.bCameraFlagsOld & Variables_t::Misc_t::CameraFlg::CAMERA_FIRSTPERSON))
		{
			CMeshPart* pFaceNorm = GetModelMesh(&pEntity->m_Work, "facial_normal");
			CMeshPart* pFaceSerious = GetModelMesh(&pEntity->m_Work, "facial_serious");
			CMeshPart* pMask = GetModelMesh(&pEntity->m_Work, "Eyemask");
			CMeshPart* pHair = GetModelMesh(&pEntity->m_Work, "Hair");

			if (pFaceNorm)
			{
				pFaceNorm->m_vColor.w = 0.0f;
				pFaceNorm->m_bUpdate = TRUE;
			}

			if (pFaceSerious)
			{
				pFaceSerious->m_vColor.w = 0.0f;
				pFaceSerious->m_bUpdate = TRUE;
			}

			if (pMask)
			{
				pMask->m_vColor.w = 0.0f;
				pMask->m_bUpdate = TRUE;
			}

			if (pHair)
			{
				pHair->m_vColor.w = 0.0f;
				pHair->m_bUpdate = TRUE;
			}
		}
	}


	//WIP: head mesh comes back on animations this needs to be fixed
	// also camera should rotate with bone
	static void Thirdperson(CBehavior* pEntity)
	{
		if (pEntity &&
			!(Vars.Misc.bCameraFlags & Variables_t::Misc_t::CameraFlg::CAMERA_FIRSTPERSON) &&
			(Vars.Misc.bCameraFlagsOld & Variables_t::Misc_t::CameraFlg::CAMERA_FIRSTPERSON))
		{
			CMeshPart* pFaceNorm = GetModelMesh(&pEntity->m_Work, "facial_normal");
			CMeshPart* pFaceSerious = GetModelMesh(&pEntity->m_Work, "facial_serious");
			CMeshPart* pMask = GetModelMesh(&pEntity->m_Work, "Eyemask");
			CMeshPart* pHair = GetModelMesh(&pEntity->m_Work, "Hair");

			if (pFaceNorm)
			{
				pFaceNorm->m_vColor.w = 1.0f;
				pFaceNorm->m_bUpdate = TRUE;
			}

			if (pFaceSerious)
			{
				pFaceSerious->m_vColor.w = 0.0f;
				pFaceSerious->m_bUpdate = TRUE;
			}

			if (pMask)
			{
				pMask->m_vColor.w = 1.0f;
				pMask->m_bUpdate = TRUE;
			}

			if (pHair)
			{
				pHair->m_vColor.w = 1.0f;
				pHair->m_bUpdate = TRUE;
			}
		}
	}

	// Maybe for perma wet we can call WetObjectManager_SetDry(g_pWetObjectManager, pEntity->m_pInfo); then, set the wet level manually
	// then undo it when we want to go back to normal DOESN'T WORK
	// TODO("We need to remove the handle from the wetobjmanager with WetObjectManager_SetDry(g_pWetObjectManager, pEntity->m_pInfo); after the wet time has elapsed")
	static int WetEntity(CBehaviorAppBase* pEntity, uint8_t wetness)
	{
		int i = 0;

		if (!pEntity)
			return -1;

		WetObjectManager_AddLocalEntity(g_pWetObjectManager, pEntity->m_pInfo);

		for (; i < ARRAYSIZE(g_pWetObjectManager->m_LocalHandles); ++i)
			if (g_pWetObjectManager->m_LocalHandles[i] == pEntity->m_pInfo->m_hEntity)
				break;

		WetObjectManager_SetWet(g_pWetObjectManager, wetness, i);

		return i;
	}

	static void DeleteEntity(EntityHandle_t hEntity)
	{
		CBehaviorAppBase* pEntity = GetEntityFromHandleGlobal(&hEntity);

		if (!pEntity)
			return;

		EntityInfo_Delete(pEntity->m_pInfo);
	}

	static void SetPlayer(EntityHandle_t hEntity)
	{
		CBehaviorAppBase* pEntity = GetEntityFromHandle(&hEntity);

		if (!pEntity)
			return;

		SetLocalPlayer(&pEntity->m_pInfo->m_hEntity);

		//*(PDWORD)0x1419AED20 = 0x80000000;

		ResetCamera(g_pCamera);
	}

	static void ChangePlayerEx(CBehaviorAppBase* pCameraEnt)
	{
		if (pCameraEnt && g_pLocalPlayer)
		{
			ChangePlayer(g_pLocalPlayer);
			//pCameraEnt = g_pCamera->m_pCamEntity;//GetEntityFromHandle(&g_pCamera->m_hEntity);
		}
	}

	static void SwapPlayer(void)
	{
		ChangePlayerEx(g_pCamera->m_pCamEntity);
	}

	static void DuplicateBuddyAsNPC(void)
	{
		if (g_pLocalPlayer)
			g_pLocalPlayer->m_hBuddy.m_uValue = 0;
	}

	// xref Pl0000.setBedStandup 48 83 EC 38 48 8B 01 45 33 C9 C7 44 24 20 00 00 00 00 41 8D 51 6D
	static void AirstuckEx(CBehavior* pEntity)
	{
		if (pEntity)
			pEntity->Animate(109, 2, 0, 0); // CBehavior:: vindex: 18 / old denuvo 0x1401F08A0
	}

	static void Airstuck(void)
	{
		AirstuckEx(g_pLocalPlayer);
	}

	static void TeleportEntityToOther(EntityHandle_t hTeleporter, EntityHandle_t hTeleportee)
	{
		CBehavior* pTeleporter = GetEntityFromHandle(&hTeleporter);
		CBehavior* pTeleportee = GetEntityFromHandle(&hTeleportee);

		if (pTeleportee && pTeleporter)
			pTeleportee->m_vPosition = pTeleporter->m_vPosition + pTeleporter->m_matTransform.GetAxis(FORWARD) * 4.f;
	}

	static void TeleportScalarEx(CObj* pEntity, eTransformMatrix Axis, float flSpeed)
	{
		if (pEntity)
			pEntity->m_vPosition += pEntity->m_matTransform.GetAxis(Axis) * flSpeed;
	}

	static void TeleportScalar(eTransformMatrix Axis, float flSpeed)
	{
		if (g_pCamera)
			TeleportScalarEx(g_pCamera->m_pCamEntity, Axis, flSpeed);
	}

	static bool* GetModelGravityEx(Pl0000* pEntity)
	{
		if (!pEntity)
			return NULL;

		return (bool*)&pEntity->m_VerticalCollision.m_bEnabled;
	}

	static bool* GetModelGravity(void)
	{
		if (!g_pCamera)
			return NULL;

		return GetModelGravityEx(static_cast<Pl0000*>(g_pCamera->m_pCamEntity));
	}

	static float* GetEntityOBBYEx(Pl0000* pEntity)
	{
		if (!pEntity)
			return NULL;

		return &pEntity->m_VerticalCollision.m_vPosition.y;
	}

	static float* GetEntityOBBY()
	{
		if (!g_pCamera)
			return NULL;

		return GetEntityOBBYEx(static_cast<Pl0000*>(g_pCamera->m_pCamEntity));
	}

	static void PlayAnimationEx(CBehavior* pEntity)
	{
		if (pEntity)
			pEntity->Animate(Vars.Gameplay.iSelectedAnimation, 0, 0, 0);
	}

	static void PlayAnimation(void)
	{
		PlayAnimationEx(g_pCamera->m_pCamEntity);
	}

	static void SetEnemyLevel(void* pEnemy, int iLevel)
	{
		if (!pEnemy)
			return;

		ExExpInfo* pInfo = MakePtr(ExExpInfo*, pEnemy, 0x6378);
		int* pLevel = MakePtr(int*, pEnemy, 0x28030);

		if ((*pLevel) != iLevel && iLevel > 0 && iLevel <= pInfo->m_nLevels)
		{
			int* pHealth = MakePtr(int*, pEnemy, 0x858);
			int* pMaxHealth = MakePtr(int*, pEnemy, 0x85C);
			Level_t* pProperLevel = &pInfo->m_levels[iLevel - 1];
			*pLevel = pProperLevel->m_iLevel;
			*pHealth = pProperLevel->m_iHealth;
			*pMaxHealth = pProperLevel->m_iHealth;
		}
	}

	static void BalanceEnemyLevel(CBehaviorAppBase* pEnemy, int iMinLevel, int iMaxLevel)
	{
		if (!pEnemy)
			return;

		int iLevel = pEnemy->GetLevel();
		//int* pLevel = MakePtr(int*, pEnemy, 0x28030);
		//if ((*pLevel) > iMaxLevel || (*pLevel) < iMinLevel)
		if (iLevel > iMaxLevel || iLevel < iMinLevel)
		{
			pEnemy->SetLevel(_RandomInt(iMinLevel, iMaxLevel));
			//SetEnemyLevel(pEnemy, _RandomInt(iMinLevel, iMaxLevel));

			CCONSOLE_DEBUG_LOG(ImColor(0xff8f20d4), "Enemy lvl [cur: %i, min: %i, max %i]", iLevel, iMinLevel, iMaxLevel);
		}
	}

	static void BuffEnemiesAbsolute(void)
	{
		for (size_t i = 0; i < g_pEnemyManager->m_handles.m_Count; ++i)
			SetEnemyLevel(GetEntityFromHandleGlobal(&g_pEnemyManager->m_handles.m_pItems[i]), Vars.Gameplay.iEnemyLevel);
	}

	static void BuffEnemiesTolerance(void)
	{
		Pl0000* pLocal = (Pl0000*)GetEntityFromHandleGlobal(g_pLocalPlayerHandle);

		if (!pLocal)
			return;

		int iAbsLevelTolerance = labs(Vars.Gameplay.iEnemyLevelTolerance);
		int iMinLevel = max(0, (Vars.Gameplay.bExclusivelyPositiveTolerance) ? pLocal->m_iLevel : pLocal->m_iLevel - iAbsLevelTolerance);
		int iMaxLevel = min(MAX_LEVELS, pLocal->m_iLevel + iAbsLevelTolerance);

		for (size_t i = 0; i < g_pEnemyManager->m_handles.m_Count; ++i)
			BalanceEnemyLevel(GetEntityFromHandleGlobal(&g_pEnemyManager->m_handles.m_pItems[i]), iMinLevel, iMaxLevel);
	}

	static void BuffEnemies(void)
	{
		if (Vars.Gameplay.bLevelBuffMode)
			BuffEnemiesAbsolute();
		else
			BuffEnemiesTolerance();
	}

	static void AddPod(int pod)
	{
		// I don't know what this is. It appears to be related to scene entity system
		static CSceneEntitySystem* pSceneEntitySystem = (CSceneEntitySystem*)FindPatternPtr(NULL, "48 8D 0D ? ? ? ? E8 ? ? ? ? 48 89 44 24 ? 48 85 C0 74 ? 8B 50", 3);
		unsigned int crc;
		SceneState* pScene;

		switch (pod)
		{
		case POD_A:
			crc = HashStringCRC32("own_podA", 8);
			pScene = (SceneState*)SceneEntitySystem_FindSceneState(pSceneEntitySystem, crc, "own_podA", 8);
			break;
		case POD_B:
			crc = HashStringCRC32("own_podB", 8);
			pScene = (SceneState*)SceneEntitySystem_FindSceneState(pSceneEntitySystem, crc, "own_podB", 8);
			break;
		case POD_C:
			crc = HashStringCRC32("own_podC", 8);
			pScene = (SceneState*)SceneEntitySystem_FindSceneState(pSceneEntitySystem, crc, "own_podC", 8);
			break;
		default:
			return;
		}
		SceneStateSystem_Set(g_pSceneStateSystem, &pScene);
	}

	static void AddItem(int id, int quantity)
	{
		// I don't know what this is. I just know it's related to the item manager
		static CItemManager* pItemManager = (CItemManager*)FindPatternPtr(NULL, "7E CA 48 8D 15 ? ? ? ?", 5);

		ItemManager_AddItem(pItemManager, id, quantity);

		if (Vars.Gameplay.bInstantEquip)
		{
			ItemManager_UseItem(pItemManager, ItemManager_GetItemNameById(g_pItemManager, id));
		}
	}

	static void TeleportAllEnemiesToPoint(const Vector3& vPosition)
	{
		for (QWORD i = 0; i < g_pEnemyManager->m_handles.m_Count; ++i)
		{
			CBehavior* pEntity = GetEntityFromHandle(&g_pEnemyManager->m_handles.m_pItems[i]);
			pEntity->m_vPosition = vPosition;
		}
	}

	static void TeleportAllEnemiesToEncirclePoint(const Vector3& vPosition, float flRadius)
	{
		if (!g_pEnemyManager->m_handles.m_Count)
			return;

		Vector3 vEndpoint, vDirection;
		float sin, cos;
		float step = (2.0f * M_PI_F) / g_pEnemyManager->m_handles.m_Count;
		float rad = 0.0f;

		for (QWORD i = 0; i < g_pEnemyManager->m_handles.m_Count; ++i, rad += step)
		{
			CBehavior* pEntity = GetEntityFromHandle(&g_pEnemyManager->m_handles.m_pItems[i]);

			vDirection = (vPosition - pEntity->m_vPosition).Normalize();

			Math::SinCos(rad, &sin, &cos);

			vEndpoint = vPosition + (vDirection * flRadius);

			pEntity->m_vPosition.x = (cos * (vEndpoint.x - vPosition.x) - sin * (vEndpoint.z - vPosition.z)) + vPosition.x;
			pEntity->m_vPosition.z = (sin * (vEndpoint.x - vPosition.x) + cos * (vEndpoint.z - vPosition.z)) + vPosition.z;
		}
	}

	//doesn't work tbh
	static void ModelSwap(Pl0000* pEntA, Pl0000* pEntB)
	{
		//CModel tmp;

		//memcpy(&tmp, pEntB, sizeof(CModel));
		//tmp.m_vPosition = pEntA->m_vPosition;
		//memcpy(pEntA, &tmp, sizeof(CModel));
	}

	static void MoveSun()
	{
		Sun* sun = (Sun*)0x14160EB40;
		//sun->m_vPosition.x += 3;
	}

	static void __fastcall _SetTaskFunction(CJobManager* pMgr, void (*pFunc)(void*), void* pParams)
	{
		CTask* pNext; // rbx

		pNext = pMgr->m_pTasks;

		if (pNext->m_Status != TASK_RUNNING)
			//__debugbreak();
		pMgr->m_pTask = pNext;
		pNext->m_pfnCallback = pFunc;
		pNext->m_pParameters = pParams;
		pNext->m_Status = TASK_TRANSFERED;
		ReleaseSemaphore(pNext->m_hTaskSemaphore, 1, NULL);
		//sub_7FF682421280();
	}

	static void __fastcall _StartTask(CJobManager* pMgr, void (*pFunc)(void*), void* pParams)
	{
		CTask* pNext = pMgr->m_pTasks;
		pMgr->m_pTask = pNext;
		pMgr->m_pActiveTasks = pMgr->m_pTask;
		pMgr->m_pFreeTasks = pNext->m_pNext;

		pNext->m_pfnCallback = pFunc;
		pNext->m_pParameters = pParams;
		pNext->m_uSleepTicks = 1;
		pNext->m_Status = TASK_SUSPENDED;
		ReleaseSemaphore(pNext->m_hTaskSemaphore, 1, NULL);
	}

	// this fucking thread will hang the whole game if not returned promptly wtf!
	static __declspec(noreturn) void CreateEntityStub(CreateStub_t* pParams)
	{
		static CJobManager* pJobMgrAlt = (CJobManager*)
			FindPatternPtr(NULL, "48 8D 0D ? ? ? ? E8 ? ? ? ? 41 8B D7 48 8D 0D ? ? ? ? E8 ? ? ? ? EB 99", 3);

		static void* pFunc = (void*)FindPattern(NULL, "48 83 EC 48 48 8D 0D ? ? ? ? E8");

		// Stops the SceneTask thread from running
		//JobManager_PauseTask(g_pJobManager, 1); 

		pParams->pInfo = SceneEntitySystem_CreateEntity(g_pSceneEntitySystem, &pParams->Create);

		// HACK: I don't know how to properly use their async job manager well enough!
		//SetTaskFunction((void(*)(void*))pFunc, NULL);
		//JobManager_PauseTask(pJobMgrAlt, pJobMgrAlt->m_pActiveTasks->m_uTaskId);
		//JobManager_SuspendTask2(pJobMgrAlt, 1);
		JobManager_Yield(pJobMgrAlt);
		//YieldToNextTask();
	}

	// FIXME TODO: this function doesn't work properly. It only can spawn the number of entities equal to the
	// number of tasks in the pool. Everything I tried to mitigate this failed.
	// 
	// NOTE: idk how i fixed it but it's fixed just don't spawn too many partners else it crashes
	// RIP: 0x7FF682948722  test    byte ptr [rax+5C4h], 3 ; Logical Compare
	// OWNER: SPAWN_CRASH_NEW_sub_7FF682948620 
	static CEntityInfo* CreateEntity(const char* szName, uint32_t ObjectId, OPTIONAL set_info_t* pSetInfo)
	{
		g_CreateCtx.Create.m_szName = szName;
		g_CreateCtx.Create.m_ObjectIds[0] = ObjectId;
		g_CreateCtx.Create.m_ObjectIds[1] = ObjectId;
		g_CreateCtx.Create.m_iGenerateMode = 1;

		if (pSetInfo)
		{
			g_CreateCtx.SetInfo = *pSetInfo;
			g_CreateCtx.Create.m_pSetInfo = &g_CreateCtx.SetInfo;
			g_CreateCtx.Create.m_iGenerateMode = pSetInfo->m_i0x088;
		}

		static CJobManager* pJobMgrAlt = (CJobManager*)
			FindPatternPtr(NULL, "48 8D 0D ? ? ? ? E8 ? ? ? ? 41 8B D7 48 8D 0D ? ? ? ? E8 ? ? ? ? EB 99", 3);
	
		static CJobManager::FindAndDestoryTaskFn FindDestroyTsk = (CJobManager::FindAndDestoryTaskFn)
			FindPattern(NULL, "48 89 5C 24 08 57 48 83 EC 20 48 8B 41 10 8B");
		static CJobManager::UpdateFn UpdateMgr = (CJobManager::UpdateFn)
			FindPattern(NULL, "48 89 5C 24 08 48 89 74 24 10 57 48 83 EC 40 48 8B 59 10 33");
		
		//if (pJobMgrAlt->m_uActiveTasks > 0)
		//{
		//	//FindDestroyTsk(pJobMgrAlt, 1);
		//	//_StartTask(pJobMgrAlt, (void(*)(void*)) & CreateEntityStub, &g_CreateCtx);
		//	_SetTaskFunction(pJobMgrAlt, (void(*)(void*)) & CreateEntityStub, &g_CreateCtx);
		//	UpdateMgr(pJobMgrAlt);
		//}

		//if (pJobMgrAlt->m_uActiveTasks == 0)
		//	CreateTask(&CreateEntityStub, &g_CreateCtx, 600, "SpawnTask");
		//else
			//_SetTaskFunction(pJobMgrAlt, (void(*)(void*)) & CreateEntityStub, &g_CreateCtx);

		// this version hangs on SleepEx when using g_pJobManager tasks stuck on await state!
		//if (pJobMgrAlt->m_uActiveTasks == 0)
			//JobManager_CreateTask(pJobMgrAlt, &CreateEntityStub, &g_CreateCtx, 0, "SpawnTask");
		//else
		//
		//LONG p;
		//ReleaseSemaphore(g_pJobManager->m_pTask->m_hSemaphore, 1, &p);
		// 
		//FindDestroyTsk(g_pJobManager, 1);
		//_StartTask(g_pJobManager, (void(*)(void*)) & CreateEntityStub, &g_CreateCtx);
		//JobManager_CreateTask(g_pJobManager, &CreateEntityStub, &g_CreateCtx, 0, "SpawnTask");
		JobManager_CreateTask(pJobMgrAlt, &CreateEntityStub, &g_CreateCtx, 0, "SpawnTask");

		//JobManager_DestroyTask(pJobMgrAlt, pJobMgrAlt->m_pActiveTasks);

		return g_CreateCtx.pInfo;
	}

	//static CEntityInfo* CreateEntity(const char* szName, int ObjectId, OPTIONAL set_info_t* pSetInfo)
	//{
	//	Create_t c;

	//	ZeroMemory(&c, sizeof(c));

	//	c.m_szName = szName;
	//	c.m_ObjectIds[0] = ObjectId;
	//	c.m_ObjectIds[1] = ObjectId;
	//	c.m_iGenerateMode = 1;
	//	c.m_pSetInfo = pSetInfo;

	//	if (pSetInfo)
	//		c.m_iGenerateMode = pSetInfo->m_i0x088;
	//	
	//	return SceneEntitySystem_CreateEntity(g_pSceneEntitySystem, &c);
	//}

	// broken
	static CpkEntry* LoadCpk(const char* szCpkName)
	{
		char szFullpath[MAX_PATH];

		sprintf_s(szFullpath, "%s%s.cpk", g_szDataDirectoryPath, szCpkName);

		CpkEntry* pEntry = (CpkEntry*)0x141991D90;

		while (pEntry->m_bInUse && pEntry < (CpkEntry*)0x141992590) ++pEntry;

		return CpkMount((--pEntry)->m_iLoadOrder + 1, szFullpath);
	}

	// if this doesn't work might have to create critical section, enter, copy the array contents and zero out the array, then leave
	static void EnableWordBlacklist(IN BannedWordChecker* pChecker, IN DWORD dwWordCount)
	{
		if (!pChecker || !pChecker->m_pEntries)
			return;

		pChecker->m_uWordCount = dwWordCount;
	}

	// if this doesn't work might have to create critical section, enter, restore and free the backup array, then leave
	static void DisableWordBlacklist(IN BannedWordChecker* pChecker, OPTIONAL OUT PDWORD pdwWordCount)
	{
		if (!pChecker || !pChecker->m_pEntries)
			return;

		if (pdwWordCount)
			*pdwWordCount = pChecker->m_uWordCount;

		pChecker->m_uWordCount = 0;
	}

	// try to check if the buffer is valid and if so free with heap free ?
	static void DecryptWordBlacklist(IN BannedWordChecker* pChecker)
	{
		BYTE current_byte;
		PBYTE pData, lpDataEnd, lpDataStart;
		UINT  dwDataEndOffset, dwDataStartOffset, length, dwWordByteLength;
		INT64 index;

		if (!pChecker || !pChecker->m_pBuffer)
			return;

		BannedWordChecker::BannedWordBinaryHeader* pHdr = (BannedWordChecker::BannedWordBinaryHeader*)pChecker->m_pBuffer;

		UINT dwNextDataStartOffset = sizeof(BannedWordChecker::BannedWordBinaryHeader);
		DWORD i = 0;

		pChecker->m_uWordCount = pHdr->uWordCount;

		if (pChecker->m_uWordCount > 0)
		{
			pChecker->m_pEntries = new BannedWordChecker::WordEntry[pChecker->m_uWordCount];

			do
			{
				dwDataStartOffset = dwNextDataStartOffset;
				dwDataEndOffset = dwNextDataStartOffset + 4;
				lpDataStart = (BYTE*)pChecker->m_pBuffer + dwDataStartOffset;
				lpDataEnd = (BYTE*)pChecker->m_pBuffer + dwDataEndOffset;
				pChecker->m_pEntries[i].dwLength = *(DWORD*)lpDataStart;
				dwWordByteLength = pChecker->m_pEntries[i].dwLength * sizeof(WCHAR);
				dwNextDataStartOffset = dwWordByteLength + dwDataEndOffset;

				if (dwWordByteLength)
				{
					pData = lpDataStart;
					index = lpDataEnd - lpDataStart;
					length = dwWordByteLength;
					do
					{
						current_byte = (pData++)[index];
						*(pData - 1) = current_byte - 19;
					} while (--length);
				}
				*((WCHAR*)&lpDataStart[dwWordByteLength]) = 0; 	//*((WCHAR*)(lpDataStart + dwWordByteLength)) = 0;
				pChecker->m_pEntries[i++].lpszBannedWord = (LPWSTR)lpDataStart;
			} while (i < pChecker->m_uWordCount);
		}
	}
};