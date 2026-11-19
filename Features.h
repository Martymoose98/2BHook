#pragma once

#include <Fluorine\Memory.h>

#include "ReversedStructs.h"
#include "Vector3.h"
#include "Log.h"
#include "Console.h"

CBehaviorAppBase* GetEntityByHandle(const CEntityList* pList, EntityHandle hEntity);
CBehaviorAppBase* GetEntityFromHandleGlobal(EntityHandle_t* phEntity);
int GetModelMeshIndex(const CModelWork* pWork, const char* szMesh);


namespace Features
{
	// inline (not static) so the Godmode state below is shared by DLLMain.cpp and Hooks.cpp
	// bUnloading: called from Unhook, which also runs in DLL_PROCESS_DETACH while the game exits.
	// The player is freed by then (g_pLocalPlayer is only refreshed per frame), so only the code
	// patches and the debug flags are restored and the player is not touched.
	inline void ApplyHealthMods(bool bUnloading = false)
	{
		// No Enemy Damage and No Fall Damage stay as NOPs. Checked in build 26534214, the only flag
		// tests on those paths are:
		// - the Godmode debug bits below, which block both paths at once;
		// - two game-state bits that also lock player input during events and menus
		//   ([0x141029B10] & 0x200000 "not playable", [0x141029B00] & 0x100000 "event mode").
		// Fall recovery (Pl0000_ExecuteState10) calls Pl0000_TakeDamage with no check of its own, so
		// NOP_DAMAGE_WORLD NOPs that one call (failed hacks and self-destruct still hurt).
		if (Vars.Gameplay.bNoEnemyDamage)
			NopMemory(&nop_Health[NOP_DAMAGE_ENEMY]);
		else
			RestoreMemory(&nop_Health[NOP_DAMAGE_ENEMY]);

		if (Vars.Gameplay.bNoWorldDamage)
			NopMemory(&nop_Health[NOP_DAMAGE_WORLD]);
		else
			RestoreMemory(&nop_Health[NOP_DAMAGE_WORLD]);

		// The hacking ship: the hacking minigame and the Ending E credits shooter (issue #14).
		// Pl0000_HitStep skips both ship-hit paths while the ship's own i-frame flag is set, and
		// Pl0000_TakeEndRollShipHit has no debug-flag check of its own, so keep the game's i-frames
		// running instead of patching code. Pl0000_ExecuteState66/70 count the timer down and clear
		// the flag at zero; when both toggles go off, the i-frames are released below straight
		// away, and neither toggle can switch the other off. Godmode's debug bit 0x4000 already makes HitStep skip
		// ship hits; doing this for Godmode too keeps the ship safe if the game rewrites the debug
		// flags.
		Pl0000* pPlayer = bUnloading ? NULL : g_pLocalPlayer;
		const bool bInShip = pPlayer && (pPlayer->m_ObjectId & 0xFFFFF000) == 0x10000 &&
			(pPlayer->m_iAnimationId == PL_STATE_HACKING_SHIP || pPlayer->m_iAnimationId == PL_STATE_ENDROLL_SHIP);

		// The player whose ship i-frames are being held, so they can be released the moment
		// both toggles are off instead of when the game's countdown runs out (up to a second).
		// Only released while still flying that ship: Pl0000_EnterState66 clears the flag, the
		// timer and the hits taken at the start of every hack anyway, and a player pointer
		// from an earlier frame may be gone.
		static Pl0000* s_pShipHeld = NULL;

		if ((Vars.Gameplay.bGodmode || Vars.Gameplay.bInvincibleHackingShip) && bInShip)
		{
			if (!(pPlayer->m_uHackingFlags & HACKING_FLAG_INVINCIBLE))
				InterlockedOr((volatile LONG*)&pPlayer->m_uHackingFlags, HACKING_FLAG_INVINCIBLE);

			if (pPlayer->m_flHackingInvincibleTime < 1.0f)
				pPlayer->m_flHackingInvincibleTime = 1.0f;

			s_pShipHeld = pPlayer;
		}
		else if (s_pShipHeld)
		{
			if (bInShip && pPlayer == s_pShipHeld)
			{
				InterlockedAnd((volatile LONG*)&pPlayer->m_uHackingFlags, ~(LONG)HACKING_FLAG_INVINCIBLE);
				pPlayer->m_flHackingInvincibleTime = 0.0f;
			}

			s_pShipHeld = NULL;
		}

		// One log line when a ship flight starts or ends, and whenever the ship's hits left or
		// i-frame flag change, so a ship that won't die shows in 2BHook.log which of the three
		// things that stop ship damage was set: the i-frame flag (Pl0000_HitStep skips the
		// hit), debug bit 0x4000 (HitStep skips every hit) or debug bit 0x8000
		// (Pl0000_TakeHackingShipHit takes no life). A vanilla hack starts with flags 0,
		// i-frames 0, 3 hits left and debug flags 0.
		static bool s_bWasInShip = false;
		static int32_t s_iLastShipLife = 0;
		static uint32_t s_uLastShipFlag = 0;

		if (bInShip)
		{
			const uint32_t uFlag = pPlayer->m_uHackingFlags & HACKING_FLAG_INVINCIBLE;

			if (!s_bWasInShip || pPlayer->m_iHackingShipLife != s_iLastShipLife || uFlag != s_uLastShipFlag)
			{
				Log::Log("Hacking ship", "state %d, hits left %d, flags %08X, i-frames %.2f, debug flags %08X, Godmode %d, Invincible Hacking Ship %d\n",
					pPlayer->m_iAnimationId, pPlayer->m_iHackingShipLife, pPlayer->m_uHackingFlags, pPlayer->m_flHackingInvincibleTime,
					g_pDebugFlags ? *g_pDebugFlags : 0, Vars.Gameplay.bGodmode, Vars.Gameplay.bInvincibleHackingShip);
			}

			s_iLastShipLife = pPlayer->m_iHackingShipLife;
			s_uLastShipFlag = uFlag;
		}
		else if (s_bWasInShip)
		{
			Log::Log("Hacking ship", "left the ship\n");
		}

		s_bWasInShip = bInShip;

		// Godmode uses the game's own debug invincibility bits, not the NOPs above. With 0x4000
		// Pl0000::CalculateDamage returns before doing anything, 0x8000 (DBG_PLAYER_NO_DAMAGE)
		// zeroes the damage, and Pl0000_TakeDamage does nothing if either is set - so both NOP
		// sites are covered. 0x4000 also makes Pl0000_HitStep skip a hit before it reaches the
		// flight unit, hacking ship or Ending E ship code (every path to them passes that test).
		constexpr uint32_t DBG_FLAG_INVINCIBLE = 0x4000 | 0x8000;
		static bool s_bDebugGodmode = false;
		static uint32_t s_uBitsSetBeforeGodmode = 0;

		if (!g_pDebugFlags)
			return;

		if (Vars.Gameplay.bGodmode)
		{
			if (!s_bDebugGodmode)
				s_uBitsSetBeforeGodmode = *g_pDebugFlags & DBG_FLAG_INVINCIBLE;

			// Re-assert every frame rather than once: the game rewrites the whole flag block in
			// places (the post-clear debug menu), and that would quietly switch Godmode off.
			if ((*g_pDebugFlags & DBG_FLAG_INVINCIBLE) != DBG_FLAG_INVINCIBLE)
				InterlockedOr((volatile LONG*)g_pDebugFlags, DBG_FLAG_INVINCIBLE);
		}
		else if (s_bDebugGodmode)
		{
			// Clear only the bits Godmode set, so ones set by hand in the Debug Flags box survive.
			InterlockedAnd((volatile LONG*)g_pDebugFlags, ~(LONG)(DBG_FLAG_INVINCIBLE & ~s_uBitsSetBeforeGodmode));
		}

		s_bDebugGodmode = Vars.Gameplay.bGodmode;
	}

	// The model menu's mesh "Enabled" checkbox. It used to take effect only through
	// hkPl0000Update, a vtable hook that is installed only in Debug builds with EXP_ENT_HOOK
	// defined - which it never is - so the checkbox did nothing. A mesh unticked there is hidden
	// again every frame, since the game turns some meshes back on by itself (the face meshes on
	// some animations); ticking it again hands the mesh back to the game.
	struct HiddenMesh
	{
		const CModelWork* m_pWork;
		int32_t m_iMesh;
	};

	inline HiddenMesh g_HiddenMeshes[32];
	inline int g_nHiddenMeshes = 0;

	inline void SetMeshHidden(CModelWork* pWork, int32_t iMesh, bool bHidden)
	{
		if (!pWork || iMesh < 0 || iMesh >= pWork->m_nMeshes)
			return;

		int iFound = -1;

		for (int i = 0; i < g_nHiddenMeshes; ++i)
		{
			if (g_HiddenMeshes[i].m_pWork == pWork && g_HiddenMeshes[i].m_iMesh == iMesh)
				iFound = i;
		}

		if (bHidden && iFound == -1)
		{
			if (g_nHiddenMeshes == ARRAYSIZE(g_HiddenMeshes))
				return;

			g_HiddenMeshes[g_nHiddenMeshes++] = { pWork, iMesh };
		}
		else if (!bHidden && iFound != -1)
		{
			g_HiddenMeshes[iFound] = g_HiddenMeshes[--g_nHiddenMeshes];
		}

		pWork->m_pMeshes[iMesh].m_bShow = bHidden ? FALSE : TRUE;
		pWork->m_pMeshes[iMesh].m_bUpdate = TRUE;
	}

	// Called every frame with the camera entity's model; entries for other models wait until
	// the camera is back on them, since their pointers may be stale meanwhile.
	inline void ApplyHiddenMeshes(CModelWork* pWork)
	{
		if (!pWork)
			return;

		for (int i = 0; i < g_nHiddenMeshes; ++i)
		{
			if (g_HiddenMeshes[i].m_pWork != pWork || g_HiddenMeshes[i].m_iMesh >= pWork->m_nMeshes)
				continue;

			CMeshPart* pMesh = &pWork->m_pMeshes[g_HiddenMeshes[i].m_iMesh];

			if (pMesh->m_bShow)
			{
				pMesh->m_bShow = FALSE;
				pMesh->m_bUpdate = TRUE;
			}
		}
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
	// NOTE: hides the hair through its alpha, which Rainbow Hair leaves alone
	static void Firstperson(CBehavior* pEntity)
	{
		if (pEntity &&
			(Vars.Misc.uCameraFlags & Variables_t::Misc_t::CameraFlg::CAMERA_FIRSTPERSON) &&
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
			!(Vars.Misc.uCameraFlags & Variables_t::Misc_t::CameraFlg::CAMERA_FIRSTPERSON) &&
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

		// AddLocalEntity can decline to register the entity - the array fills up - and the
		// loop then falls out at one past the end. Passing that index to SetWet wrote
		// through it into whatever the game keeps after m_LocalHandles.
		if (i == ARRAYSIZE(g_pWetObjectManager->m_LocalHandles))
			return -1;

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
		static CItemManager* pItemManager = (CItemManager*)FindPatternPtr(NULL, "7E CA 48 8D 15", 5);

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

	struct ParticleStub_t
	{
		CObj* m_pObj;
		int32_t m_iType;
	};

	// this fucking thread will hang the whole game if not returned promptly wtf!
	static __declspec(noreturn) void CreateParticleStub(ParticleStub_t* pParams)
	{
		static CParticle::InitializeFn ParticleInit =
			(CParticle::InitializeFn)FindPattern(NULL, "33 C0 C7 41 08 00 00 00 80 48 89 01");

		static CParticle::ReserveInfoSlotFn ReserveInfoSlot =
			(CParticle::ReserveInfoSlotFn)FindPattern(NULL, "4C 8B D2 41 80");

		static CObj_EmitParticleFn EmitParticle =
			(CObj_EmitParticleFn)FindPattern(NULL, "48 89 5C 24 10 48 89 74 24 18 57 48 83 EC 20 48 8B B9 10");

		static CJobManager* pJobMgrAlt = (CJobManager*)
			FindPatternPtr(NULL, "48 8D 0D ? ? ? ? E8 ? ? ? ? 41 8B D7 48 8D 0D ? ? ? ? E8 ? ? ? ? EB 99", 3);

		static void* pFunc = (void*)FindPattern(NULL, "48 83 EC 48 48 8D 0D ? ? ? ? E8");

		// Stops the SceneTask thread from running
		//JobManager_PauseTask(g_pJobManager, 1); 

		CParticle p;

		ParticleInit(&p);
		ReserveInfoSlot(&p, pParams->m_pObj->m_pInfo, 0xFF);
		p.m_pParent = pParams->m_pObj;

		p.m_matTransform = pParams->m_pObj->m_matTransform;

		EmitParticle(pParams->m_pObj, pParams->m_iType, &p);
		// HACK: I don't know how to properly use their async job manager well enough!
		//SetTaskFunction((void(*)(void*))pFunc, NULL);
		//JobManager_PauseTask(pJobMgrAlt, pJobMgrAlt->m_pActiveTasks->m_uTaskId);
		//JobManager_SuspendTask2(pJobMgrAlt, 1);
		JobManager_Yield(pJobMgrAlt);
		//YieldToNextTask();
	}

	static void CreateParticle(CObj* pObj, int32_t iType)
	{
		thread_local ParticleStub_t p;

		p.m_pObj = pObj;
		p.m_iType = iType;

		static CJobManager* pJobMgrAlt = (CJobManager*)
			FindPatternPtr(NULL, "48 8D 0D ? ? ? ? E8 ? ? ? ? 41 8B D7 48 8D 0D ? ? ? ? E8 ? ? ? ? EB 99", 3);

		static CJobManager::FindAndDestoryTaskFn FindDestroyTsk = (CJobManager::FindAndDestoryTaskFn)
			FindPattern(NULL, "48 89 5C 24 08 57 48 83 EC 20 48 8B 41 10 8B");
		static CJobManager::UpdateFn UpdateMgr = (CJobManager::UpdateFn)
			FindPattern(NULL, "48 89 5C 24 08 48 89 74 24 10 57 48 83 EC 40 48 8B 59 10 33");

		JobManager_CreateTask(pJobMgrAlt, &CreateParticleStub, &p, 0, "ParticleTask");
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

	// Mounts <data directory><szCpkName>.cpk above every archive that's already mounted, the
	// way FileSystem_MountArchive mounts the game's own. Returns its slot in the game's mount
	// table (g_HwRofsFstMounts), or NULL if it couldn't be mounted. This used to walk the table
	// at hard-coded Denuvo-build addresses, which are the middle of unrelated code on v1.02 and
	// build 26534214, and CpkMount was never found outside the Denuvo build.
	static CpkEntry* LoadCpk(const char* szCpkName)
	{
		if (!CpkMount || !g_pCpkEntries || !g_szDataDirectoryPath)
			return NULL;

		char szFullpath[MAX_PATH];

		sprintf_s(szFullpath, "%s%s.cpk", g_szDataDirectoryPath, szCpkName);

		int iLoadOrder = 0;

		for (const CpkEntry* pEntry = g_pCpkEntries; pEntry < g_pCpkEntriesEnd; ++pEntry)
			if (pEntry->m_bInUse && pEntry->m_iLoadOrder >= iLoadOrder)
				iLoadOrder = pEntry->m_iLoadOrder + 1;

		if (!CpkMount(iLoadOrder, szFullpath))
			return NULL;

		for (CpkEntry* pEntry = g_pCpkEntries; pEntry < g_pCpkEntriesEnd; ++pEntry)
			if (pEntry->m_bInUse && pEntry->m_iLoadOrder == iLoadOrder)
				return pEntry;

		return NULL;
	}

	// if this doesn't work might have to create critical section, enter, copy the array contents and zero out the array, then leave
	static void EnableWordBlacklist(IN CBannedWordChecker* pChecker, IN uint32_t uWordCount)
	{
		if (!pChecker || !pChecker->m_pEntries)
			return;

		pChecker->m_uWordCount = uWordCount;
	}

	// if this doesn't work might have to create critical section, enter, restore and free the backup array, then leave
	static void DisableWordBlacklist(IN CBannedWordChecker* pChecker, OPTIONAL OUT uint32_t* puWordCount)
	{
		if (!pChecker || !pChecker->m_pEntries)
			return;

		if (puWordCount)
			*puWordCount = pChecker->m_uWordCount;

		pChecker->m_uWordCount = 0;
	}

	// try to check if the buffer is valid and if so free with heap free ?
	// Rebuilt from old binary but is the same in the new one
	// new version is VECTORIZED LOOOL
	// NEW FUNC SIG "48 89 5C 24 10 48 89 6C 24 18 56 57 41 54 48"
	static void DecryptWordBlacklist(IN CBannedWordChecker* pChecker)
	{
		BYTE current_byte;
		PBYTE pData, lpDataEnd, lpDataStart;
		UINT  dwDataEndOffset, dwDataStartOffset, length, dwWordByteLength;
		INT64 index;

		if (!pChecker || !pChecker->m_pBuffer)
			return;

		CBannedWordChecker::BinaryHeader* pHdr = (CBannedWordChecker::BinaryHeader*)pChecker->m_pBuffer;

		UINT dwNextDataStartOffset = sizeof(CBannedWordChecker::BinaryHeader);
		DWORD i = 0;

		pChecker->m_uWordCount = pHdr->m_uWordCount;

		if (pChecker->m_uWordCount > 0)
		{
			pChecker->m_pEntries = new CBannedWordChecker::WordEntry[pChecker->m_uWordCount];

			do
			{
				dwDataStartOffset = dwNextDataStartOffset;
				dwDataEndOffset = dwNextDataStartOffset + sizeof(uint32_t);
				lpDataStart = (BYTE*)pChecker->m_pBuffer + dwDataStartOffset;
				lpDataEnd = (BYTE*)pChecker->m_pBuffer + dwDataEndOffset;
				pChecker->m_pEntries[i].m_uLength = *(uint32_t*)lpDataStart;
				dwWordByteLength = pChecker->m_pEntries[i].m_uLength * sizeof(wchar_t);
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
				*((wchar_t*)&lpDataStart[dwWordByteLength]) = 0;
				//*((WCHAR*)(lpDataStart + dwWordByteLength)) = 0;
				pChecker->m_pEntries[i++].m_szBannedWord = (wchar_t*)lpDataStart;
			} while (i < pChecker->m_uWordCount);
		}
	}
};