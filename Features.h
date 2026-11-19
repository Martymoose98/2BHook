#pragma once

#include <Fluorine\Memory.h>

#include "ReversedStructs.h"
#include "Vector3.h"
#include "Log.h"
#include "Console.h"

CBehaviorAppBase* GetEntityByHandle(const CEntityList* pList, EntityHandle hEntity);
CBehaviorAppBase* GetEntityFromHandleGlobal(EntityHandle_t* phEntity);
Pl0000* GetPl0000(CBehaviorAppBase* pEntity);
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

	// Freecamera turns on the game's own debug free camera. DBG word 0 (0x141029B80) bit 31 makes
	// CAM_THINK_UPDATE run CCameraGame_Freecam instead of the camera's Move, skips the update's
	// pose blend and shake, stops platforms carrying the camera, and locks the player's gamepad.
	// DBG word 3 bit 28 picks its fly mode (CameraFreeControl_MoveFly) over orbiting a target
	// (port Camera.cpp:10817-10882, 11424-11509; nothing in the game itself sets either bit).
	// Re-asserted every frame like Godmode's bits; switching off clears only the bits this set,
	// and the game eases back from where the free camera left the pose.
	inline void ApplyFreeCamFlags(bool bUnloading = false)
	{
		constexpr uint32_t DBG0_FREECAM = 0x80000000;
		constexpr uint32_t DBG3_FREECAM_FLY = 0x10000000;
		static bool s_bApplied = false;
		static uint32_t s_uFreecamBefore = 0, s_uFlyBefore = 0;

		if (!g_pCameraFlags)
			return;

		volatile LONG* pWord0 = (volatile LONG*)&g_pCameraFlags[0];
		volatile LONG* pWord3 = (volatile LONG*)&g_pCameraFlags[3];
		const bool bWanted = !bUnloading && (Vars.Misc.uCameraFlags & Variables_t::Misc_t::CameraFlg::CAMERA_FREE);

		if (bWanted)
		{
			if (!s_bApplied)
			{
				s_uFreecamBefore = g_pCameraFlags[0] & DBG0_FREECAM;
				s_uFlyBefore = g_pCameraFlags[3] & DBG3_FREECAM_FLY;
			}

			if (!(g_pCameraFlags[3] & DBG3_FREECAM_FLY))
				InterlockedOr(pWord3, DBG3_FREECAM_FLY);

			if (!(g_pCameraFlags[0] & DBG0_FREECAM))
				InterlockedOr(pWord0, (LONG)DBG0_FREECAM);
		}
		else if (s_bApplied)
		{
			InterlockedAnd(pWord0, ~(LONG)(DBG0_FREECAM & ~s_uFreecamBefore));
			InterlockedAnd(pWord3, ~(LONG)(DBG3_FREECAM_FLY & ~s_uFlyBefore));
		}

		s_bApplied = bWanted;
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

	// The model's alpha, set the way CModelInfo_SetColor (0x140198C00) sets its colour: only when it
	// changes, and with MODELINFO_FLAG_TINT_DIRTY, which CModelWork_Update (0x140196620) tests to
	// mark the model's draw entries for update. Without the flag the new alpha wasn't picked up.
	inline void SetModelAlpha(CModel* pModel, float flAlpha)
	{
		CModelInfo* pInfo = pModel ? pModel->m_pModelInfo : NULL;

		if (!pInfo || pInfo->m_vTint.w == flAlpha)
			return;

		pInfo->m_vTint.w = flAlpha;
		pInfo->m_uFlags |= MODELINFO_FLAG_TINT_DIRTY;
	}

	// Hide Pod holds the model alpha of the pod the camera entity has out at 0. It used to be set
	// from the Visuals tab, on whichever pod was out while the tab was drawn, and unticking showed
	// only that one again: a pod switched away from while ticked stayed hidden for good. Now
	// hkPresent calls this every frame. Every pod it hides is kept, by handle, with the alpha it
	// had; they are all held hidden while ticked, and unticking puts each one's alpha back. They
	// are looked up again through their handles every frame, never kept as pointers, since a pod
	// can be gone by the next frame: GetEntityInfoFromHandle (0x140745F20) rejects a handle whose
	// list slot is free or has been reused (the handle's tag no longer matches), and a pod that is
	// gone is dropped.
	struct HiddenPod
	{
		EntityHandle_t m_hPod;
		float m_flAlpha;		// what it had before Hide Pod set it to 0
	};

	inline void ApplyHidePod(CBehaviorAppBase* pCameraEnt)
	{
		static HiddenPod s_HiddenPods[8];	// four pod slots a player, for two players
		static int s_nHiddenPods = 0;

		// The pod that is out, m_hPods[m_iActivePod] (Pl0000 +0x10810, +0x10820), picked the way
		// Pl0000_GetActivePod (0x14033C2D0) does, once GetPl0000 confirms the camera entity is a Pl0000
		Pl0000* pOwner = Vars.Gameplay.bHidePod ? GetPl0000(pCameraEnt) : NULL;

		if (pOwner && pOwner->m_iActivePod >= 0 && pOwner->m_iActivePod < (int32_t)ARRAYSIZE(pOwner->m_hPods))
		{
			EntityHandle_t hPod = pOwner->m_hPods[pOwner->m_iActivePod];
			CBehaviorAppBase* pPod = GetEntityFromHandleGlobal(&hPod);
			bool bTracked = false;

			for (int i = 0; i < s_nHiddenPods; ++i)
				bTracked |= (s_HiddenPods[i].m_hPod.m_uValue == hPod.m_uValue);

			if (pPod && pPod->m_pModelInfo && !bTracked && s_nHiddenPods < (int)ARRAYSIZE(s_HiddenPods))
				s_HiddenPods[s_nHiddenPods++] = { hPod, pPod->m_pModelInfo->m_vTint.w };
		}

		for (int i = 0; i < s_nHiddenPods; )
		{
			CBehaviorAppBase* pPod = GetEntityFromHandleGlobal(&s_HiddenPods[i].m_hPod);

			if (pPod && Vars.Gameplay.bHidePod)
			{
				SetModelAlpha(pPod, 0.0f);
				++i;
				continue;
			}

			if (pPod)
				SetModelAlpha(pPod, s_HiddenPods[i].m_flAlpha);

			s_HiddenPods[i] = s_HiddenPods[--s_nHiddenPods];
		}
	}

	static void AddEntityToBlackList()
	{

	}

	// CSceneStateSystem::Insert (0x140150E70, SceneEntitySystem_FindSceneState here) hands back the
	// name's node with a reference taken, which the game drops once the state is set
	// (ColosseumAction_Update, 0x14039F2AF: Insert, CSceneStateSystem::Set, then
	// CSceneEntitySystem::RemoveNode). Without that, every press leaked a reference and the node
	// could never be freed.
	static void ReleaseSceneState(SceneState** ppScene)
	{
		if (SceneEntitySystem_RemoveNode)
			SceneEntitySystem_RemoveNode(ppScene);
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
		ReleaseSceneState(&pScene);
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

	// No Model Collision switches off the camera entity's horizontal collision (m_HorizontalCollision,
	// Pl0000 +0x18C0). It used to be applied from the Gameplay tab, so only while that tab was
	// drawn, and unticked it forced the collision on every frame, which undid the game's own
	// switch-offs (events, the flight unit). Now hkPresent calls this every frame and it acts only
	// when the tick or the camera entity changes: it switches the collision off once, and unticking
	// (or the camera moving on) puts back the owner and enabled state that entity had. Entities
	// are tracked by handle, since a pointer from an earlier frame may be gone.
	inline void ApplyNoCollision(CBehaviorAppBase* pCameraEnt)
	{
		static EntityHandle s_hApplied = 0;		// the entity whose collision is off
		static EntityHandle s_hSavedOwner = 0;
		static BOOL s_bSavedEnabled = FALSE;

		Pl0000* pWanted = Vars.Gameplay.bNoCollision ? GetPl0000(pCameraEnt) : NULL;
		const EntityHandle hWanted = pWanted ? pWanted->m_pInfo->m_hEntity.m_uValue : 0;

		if (hWanted == s_hApplied)
			return;

		if (s_hApplied)
		{
			EntityHandle_t hApplied;
			hApplied.m_uValue = s_hApplied;

			if (Pl0000* pApplied = GetPl0000(GetEntityFromHandleGlobal(&hApplied)))
			{
				pApplied->m_HorizontalCollision.m_hOwner = s_hSavedOwner;
				pApplied->m_HorizontalCollision.m_bEnabled = s_bSavedEnabled;
			}
		}

		s_hApplied = hWanted;

		if (!pWanted)
			return;

		s_hSavedOwner = pWanted->m_HorizontalCollision.m_hOwner;
		s_bSavedEnabled = pWanted->m_HorizontalCollision.m_bEnabled;

		pWanted->m_HorizontalCollision.m_hOwner = 0;
		pWanted->m_HorizontalCollision.m_bEnabled = FALSE;
		pWanted->m_HorizontalCollision.m_bCollision = FALSE;
		pWanted->m_HorizontalCollision.m_vMin = pWanted->m_vPosition;
	}

	// UnlockAchievement is only resolved for the Denuvo build; on v1.02 and build 26534214 it stayed
	// NULL, so this called address 0. CUserManager::GiveAchievement (0x1409C7250) takes the id,
	// locks the achievement device and hands it to CAchievementDevice::GiveAchievement for user 0,
	// which ignores ids past CAchievementDevice::m_nAchievements (+0x60), the bound used here.
	static void UnlockAllAchievements(void)
	{
		if (!g_pUserManager || !g_pUserManager->m_pAchivementDevice)
			return;

		for (uint32_t id = 0; id < g_pUserManager->m_pAchivementDevice->m_uMaxAchievement; ++id)
		{
#ifdef DENUVO_STEAM_BUILD
			UnlockAchievement(0, 0, id);
#else
			if (UserManager_GiveAchievement)
				UserManager_GiveAchievement(id);
#endif
		}
	}

	// First person's head hiding (the camera itself is placed in hkCCameraGameSetViewAngles).
	// The meshes the port hides in first person (ExcitedDelirium FirstPerson.cpp) get alpha 0,
	// which the engine skips when drawing - every frame, because some animations and face
	// swaps turn them back on (the old version set them once on entering, so they came back,
	// and it covered four of them). Leaving first person, or the camera moving to another
	// entity, puts back the alphas they had. Alpha 0 also drops the head's shadow; the port
	// avoids that by skipping the meshes in the main view only, which needs a draw hook.
	// Rainbow Hair only changes the colour, so the two work together.
	// Only these head meshes are hidden, never the body: they all sit within a few centimetres
	// of the eye, around it, so some of them are in the view whichever way it looks. The body,
	// the arms and what she holds stay drawn; the camera keeps out of them instead, with the
	// pitch held 10 degrees short of straight down, the look held within reach of her heading,
	// the eye in front of her face toward the look and a near plane of a few centimetres
	// (hkCCameraGameSetViewAngles, ApplyFirstPersonNearPlane).
	inline void UpdateFirstPersonHead(CBehaviorAppBase* pEntity)
	{
		static const char* const s_szHeadMeshes[] =
		{
			"Hair", "Eyelash", "Eyelash_serious", "Eyemask", "facial_normal", "facial_serious",
			"Feather", "Armor_Head", "Face"
		};

		static CBehaviorAppBase* s_pHidden = NULL;
		static float s_flSavedAlpha[ARRAYSIZE(s_szHeadMeshes)];

		if (s_pHidden && s_pHidden != pEntity)
		{
			// Only restored while that entity is still the one the camera shows; a pointer
			// from an earlier frame may be gone
			if (g_pCamera && g_pCamera->m_pCamEntity == s_pHidden)
			{
				for (size_t i = 0; i < ARRAYSIZE(s_szHeadMeshes); ++i)
				{
					CMeshPart* pMesh = GetModelMesh(&s_pHidden->m_Work, s_szHeadMeshes[i]);

					if (pMesh)
					{
						pMesh->m_vColor.w = s_flSavedAlpha[i];
						pMesh->m_bUpdate = TRUE;
					}
				}
			}

			s_pHidden = NULL;
		}

		if (!pEntity)
			return;

		for (size_t i = 0; i < ARRAYSIZE(s_szHeadMeshes); ++i)
		{
			CMeshPart* pMesh = GetModelMesh(&pEntity->m_Work, s_szHeadMeshes[i]);

			if (!pMesh)
				continue;

			if (s_pHidden != pEntity)
				s_flSavedAlpha[i] = pMesh->m_vColor.w;

			if (pMesh->m_vColor.w != 0.0f)
			{
				pMesh->m_vColor.w = 0.0f;
				pMesh->m_bUpdate = TRUE;
			}
		}

		s_pHidden = pEntity;
	}

	// First person turns her to face the look, after the port's CFirstPerson::OnPlayerUpdate
	// (ExcitedDelirium FirstPerson.cpp:287-299). Once the look is more than FIRSTPERSON_TURN_START
	// off her heading she is turned toward it until she is within FIRSTPERSON_TURN_STOP, and only
	// while she stands idle: state 0 (+0x670), Pl0000_ExecuteState's case 0 calling
	// Pl0000_ExecuteIdle (0x140503A90). Walking and running already turn her to the input, which
	// goes the way the view looks, and every other state owns her heading.
	// Pl0000_TurnTowardHeading (0x1404FE750) eases her yaw (+0x94) toward the heading through
	// CameraAngle_Approach, the fraction and the step scaled by her tick base, and reads nothing but
	// m_pInfo and the yaw; the fraction and step are the port's (FirstPerson.cpp:38-39). This runs
	// in CCameraGame_Update, which GameCamera_ThinkUpdate (0x140725200) calls from
	// Game_PerFrameUpdate (0x140763A17) between two behaviour group runs (0x1404E1210, 0x1404724B0).
	// Those hand the entity updates to CWorkerJobPool::RunBatch (0x140270C80), which waits for every
	// worker before it returns, so nothing else is updating her while her yaw is turned here.
	// pPlayer NULL (first person not placed) stops a turn in progress.
	inline void TurnFirstPersonBody(Pl0000* pPlayer, float flLookYaw)
	{
		constexpr float FIRSTPERSON_TURN_START = DEGTORAD(60.0f);
		constexpr float FIRSTPERSON_TURN_STOP = DEGTORAD(5.0f);
		constexpr float FIRSTPERSON_TURN_FRACTION = 0.3f;
		constexpr float FIRSTPERSON_TURN_STEP = DEGTORAD(30.0f);	// a tick
		static bool s_bTurning = false;

		if (!pPlayer || !Pl0000_TurnTowardHeading)
		{
			s_bTurning = false;
			return;
		}

		const float flOff = fabsf(remainderf(flLookYaw - pPlayer->m_matModelToWorld.GetAxis(3).y, 2.0f * M_PI_F));

		if (flOff > FIRSTPERSON_TURN_START)
			s_bTurning = true;
		else if (flOff < FIRSTPERSON_TURN_STOP)
			s_bTurning = false;

		if (s_bTurning && pPlayer->m_iAnimationId == 0)
			Pl0000_TurnTowardHeading(pPlayer, flLookYaw, FIRSTPERSON_TURN_FRACTION, FIRSTPERSON_TURN_STEP);
	}

	// First person's near plane. The eye sits a few centimetres in front of her face, and the game's
	// own near plane can cut into her arms and what she holds when she looks down. While first person
	// is placed, the near plane is held at FIRSTPERSON_ZNEAR, or at the game's when that is nearer.
	// CCameraInstance_SetZNear (0x14027ADA0) stores +0x268 and sets the projection dirty flag
	// (+0x284), and CCameraInstance_SetViewProjection (0x14027AB20) rebuilds the projection from
	// it; that runs from CCameraGame_UpdateProjectionAndShake (0x140743C10), which CCameraGame_Update
	// calls (0x14073E521) right after SetViewAngles, so a change made in the SetViewAngles hook shows
	// the same frame. The game itself sets the near plane only now and then: its 13 callers are the
	// environment light sets (CEnvironmentLight_SelectActiveSets, 0x140788370, on the game camera,
	// g_CameraGame +0x60), events, hacking and the pause map. So a near plane that isn't the one set
	// here is the game's new one, and leaving first person puts back the game's, unless the game has
	// set another since.
	inline void ApplyFirstPersonNearPlane(CCameraGame* pCamera, bool bPlaced)
	{
		constexpr float FIRSTPERSON_ZNEAR = 0.05f;			// metres
		static const CCameraInstance* s_pInstance = NULL;	// the instance holding ours, or NULL
		static float s_flGameZNear = 0.0f;
		static float s_flZNear = 0.0f;

		CCameraInstance* pInstance = pCamera ? pCamera->m_pInstance : NULL;

		if (!CameraInstance_SetZNear || !pInstance)
			return;

		if (bPlaced)
		{
			if (s_pInstance != pInstance || pInstance->m_flZNear != s_flZNear)
				s_flGameZNear = pInstance->m_flZNear;

			s_pInstance = pInstance;
			s_flZNear = min(s_flGameZNear, FIRSTPERSON_ZNEAR);

			if (pInstance->m_flZNear != s_flZNear)
				CameraInstance_SetZNear(&pCamera->m_pInstance, s_flZNear);
		}
		else if (s_pInstance)
		{
			if (s_pInstance == pInstance && pInstance->m_flZNear == s_flZNear)
				CameraInstance_SetZNear(&pCamera->m_pInstance, s_flGameZNear);

			s_pInstance = NULL;
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

	// GetEntityFromHandle (0x140491240) ends in CEntityInfo_GetEmBase (0x140308550), so it only ever
	// returns enemies; Set Player and the teleports work on any entity and only use what every
	// entity has (m_pInfo, the position and transform), so they take the untyped lookup.
	static void SetPlayer(EntityHandle_t hEntity)
	{
		CBehaviorAppBase* pEntity = GetEntityFromHandleGlobal(&hEntity);

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

	// Clears the player's buddy handle (m_hBuddy, Pl0000 +0x1647C). It was called DuplicateBuddyAsNPC,
	// but nothing is duplicated: the buddy entity stays in the world.
	static void UnlinkBuddy(void)
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
		CBehavior* pTeleporter = GetEntityFromHandleGlobal(&hTeleporter);
		CBehavior* pTeleportee = GetEntityFromHandleGlobal(&hTeleportee);

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

	// The level goes through the entity's own SetLevel (vtable slot 104; EmBase_SetLevel 0x140631700
	// for enemies), which is 1-based, clamps to 1..99 and also sets the difficulty-scaled HP, the stun
	// gauge and the XP reward, so the levels passed here are levels, not indices.
	static void BalanceEnemyLevel(CBehaviorAppBase* pEnemy, int iMinLevel, int iMaxLevel)
	{
		if (!pEnemy)
			return;

		int iLevel = pEnemy->GetLevel();

		if (iLevel > iMaxLevel || iLevel < iMinLevel)
		{
			pEnemy->SetLevel(_RandomInt(iMinLevel, iMaxLevel));

			CCONSOLE_DEBUG_LOG(ImColor(0xff8f20d4), "Enemy lvl [cur: %i, min: %i, max %i]", iLevel, iMinLevel, iMaxLevel);
		}
	}

	// Absolute Level used to write the level and HP into the enemy directly, which skipped the
	// difficulty HP scaling, the stun gauge and the XP reward that EmBase_SetLevel sets up.
	static void BuffEnemiesAbsolute(void)
	{
		const int iLevel = min(MAX_LEVELS, max(MIN_LEVELS, Vars.Gameplay.iEnemyLevel));

		for (size_t i = 0; i < g_pEnemyManager->m_handles.m_Count; ++i)
			BalanceEnemyLevel(GetEntityFromHandleGlobal(&g_pEnemyManager->m_handles.m_pItems[i]), iLevel, iLevel);
	}

	static void BuffEnemiesTolerance(void)
	{
		// m_iLevel is a Pl0000 field, and the local player can be set to any entity (Set Player).
		// GetEntityFromHandle2 (0x1403877A0) only returns Pl0000s (CEntityInfo::GetPl0000).
		Pl0000* pLocal = static_cast<Pl0000*>(GetEntityFromHandle2(g_pLocalPlayerHandle));

		if (!pLocal)
			return;

		int iAbsLevelTolerance = labs(Vars.Gameplay.iEnemyLevelTolerance);
		// Level 1 is the lowest there is (EmBase_SetLevel clamps to 1..99). A minimum of 0 let the
		// random pick land on 0, out of range.
		int iMinLevel = max(MIN_LEVELS, (Vars.Gameplay.bExclusivelyPositiveTolerance) ? pLocal->m_iLevel : pLocal->m_iLevel - iAbsLevelTolerance);
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
		ReleaseSceneState(&pScene);
	}

	// The game passes g_ITEM_MANAGER (0x140F56120) as this to both: HapMiscState_AddItem
	// (0x1403827C0) and the UseItem script command (0x140338EC2). These used to get the item
	// database (g_pItemManager, 0x14133B890), which CItemManager::UseItem (0x140844B60) passes on to
	// UseItemById (0x140844300), and that writes through it. AddItem doesn't read this at all.
	static void AddItem(int id, int quantity)
	{
		ItemManager_AddItem(g_pGameItemManager, id, quantity);

		if (Vars.Gameplay.bInstantEquip && g_pGameItemManager)
		{
			ItemManager_UseItem(g_pGameItemManager, ItemManager_GetItemNameById(g_pItemManager, id));
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

	// Create Entity's result. JobManager_CreateTask (CJobManager::CreateTask, 0x140270140) only links
	// CreateEntityStub into the job manager's run list and returns, so the entity is made later, when
	// that task runs. CreateEntity used to return g_CreateCtx.pInfo straight away - the previous
	// spawn's, or NULL - and the menu made that the buddy, Emil or the flight unit. The stub now
	// records the new entity's handle here, and ApplyCreatedEntity hooks it up from hkPresent.
	enum SpawnState : LONG
	{
		SPAWN_IDLE,
		SPAWN_QUEUED,	// CreateEntityStub hasn't run yet
		SPAWN_DONE,		// m_hEntity is set, 0 if the game couldn't create it
	};

	struct PendingSpawn_t
	{
		volatile LONG m_lState = SPAWN_IDLE;
		uint32_t m_ObjectId = 0;
		EntityHandle_t m_hOwner;	// the camera entity when Create Entity was pressed
		EntityHandle_t m_hEntity;
	};

	inline PendingSpawn_t g_PendingSpawn;

	// this fucking thread will hang the whole game if not returned promptly wtf!
	static __declspec(noreturn) void CreateEntityStub(CreateStub_t* pParams)
	{
		static CJobManager* pJobMgrAlt = (CJobManager*)
			FindPatternPtr(NULL, "48 8D 0D ? ? ? ? E8 ? ? ? ? 41 8B D7 48 8D 0D ? ? ? ? E8 ? ? ? ? EB 99", 3);

		static void* pFunc = (void*)FindPattern(NULL, "48 83 EC 48 48 8D 0D ? ? ? ? E8");

		// Stops the SceneTask thread from running
		//JobManager_PauseTask(g_pJobManager, 1); 

		pParams->pInfo = SceneEntitySystem_CreateEntity(g_pSceneEntitySystem, &pParams->Create);

		// The handle rather than the info, which may be gone by the time hkPresent looks at it
		g_PendingSpawn.m_hEntity.m_uValue = (pParams->pInfo) ? pParams->pInfo->m_hEntity.m_uValue : 0;
		InterlockedExchange(&g_PendingSpawn.m_lState, SPAWN_DONE);

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
	//
	// Queues the spawn and returns whether it was queued; ApplyCreatedEntity finishes it. One at a
	// time, since the task reads g_CreateCtx when it runs. CJobManager::CreateTask takes a task off
	// m_pFreeTasks (+0x18) and raises the fatal error g_szTaskIsOutOfWork when there is none, so
	// an exhausted pool is checked for first and the spawn refused.
	static bool CreateEntity(const char* szName, uint32_t ObjectId, OPTIONAL set_info_t* pSetInfo, EntityHandle_t hOwner)
	{
		static CJobManager* pJobMgrAlt = (CJobManager*)
			FindPatternPtr(NULL, "48 8D 0D ? ? ? ? E8 ? ? ? ? 41 8B D7 48 8D 0D ? ? ? ? E8 ? ? ? ? EB 99", 3);

		if (InterlockedCompareExchange(&g_PendingSpawn.m_lState, SPAWN_QUEUED, SPAWN_IDLE) != SPAWN_IDLE)
		{
			g_pConsole->Warn("Create Entity: the last entity is still being created");
			return false;
		}

		if (!pJobMgrAlt || !JobManager_CreateTask || !pJobMgrAlt->m_pFreeTasks)
		{
			InterlockedExchange(&g_PendingSpawn.m_lState, SPAWN_IDLE);
			g_pConsole->Warn("Create Entity: no free task to create it on, try again later");
			return false;
		}

		g_PendingSpawn.m_ObjectId = ObjectId;
		g_PendingSpawn.m_hOwner = hOwner;
		g_PendingSpawn.m_hEntity.m_uValue = 0;

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

		return true;
	}

	// Hooks up what Create Entity made, once CreateEntityStub has run: a 2B, A2 or 9S becomes the
	// buddy of whoever pressed it (m_hBuddy), Emil the game's Emil (g_pEmilHandle), and a flight
	// unit is boarded through Pl0000::SetFlightUnit (0x1404F8400), which takes the one
	// m_hFlightSuit names. The spawner is looked up again by handle and used only if it is still
	// there and a Pl0000.
	inline void ApplyCreatedEntity(void)
	{
		if (g_PendingSpawn.m_lState != SPAWN_DONE)
			return;

		EntityHandle_t hEntity = g_PendingSpawn.m_hEntity;
		EntityHandle_t hOwner = g_PendingSpawn.m_hOwner;
		const uint32_t ObjectId = g_PendingSpawn.m_ObjectId;

		InterlockedExchange(&g_PendingSpawn.m_lState, SPAWN_IDLE);

		if (!GetEntityInfoFromHandle(&hEntity))
		{
			g_pConsole->Warn("Create Entity: ObjectId %x wasn't created", ObjectId);
			return;
		}

		Pl0000* pOwner = GetPl0000(GetEntityFromHandleGlobal(&hOwner));

		switch (ObjectId)
		{
		case OBJECTID_2B:
		case OBJECTID_A2:
		case OBJECTID_9S:
			if (pOwner)
				pOwner->m_hBuddy = hEntity;
			break;
		case 0x11010:
			if (g_pEmilHandle)
				*g_pEmilHandle = hEntity;
			break;
		case 0x10010:
			if (pOwner && Pl0000_SetFlightUnit)
			{
				pOwner->m_hFlightSuit = hEntity;
				Pl0000_SetFlightUnit(pOwner);
			}
			break;
		}
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