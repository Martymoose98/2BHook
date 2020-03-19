#pragma once
#include "ReversedStructs.h"
#include "Vector3.h"
#include "Log.h"

class Features
{
public:

	static void ApplyHealthMods(void)
	{
		if (Vars.Gameplay.bGodmode || Vars.Gameplay.bNoEnemyDamage)
			g_pMemory->NopMemory(&nop_Health[NOP_DAMAGE_ENEMY]);

		if (Vars.Gameplay.bGodmode || Vars.Gameplay.bNoWorldDamage)
			g_pMemory->NopMemory(&nop_Health[NOP_DAMAGE_WORLD]);

		if (!Vars.Gameplay.bGodmode && !Vars.Gameplay.bNoEnemyDamage)
			g_pMemory->RestoreMemory(&nop_Health[NOP_DAMAGE_ENEMY]);

		if (!Vars.Gameplay.bGodmode && !Vars.Gameplay.bNoWorldDamage)
			g_pMemory->RestoreMemory(&nop_Health[NOP_DAMAGE_WORLD]);
	}

	static void AddEntityToBlackList()
	{

	}

	static void ChangeBuddy(int buddy)
	{
		CRITICAL_SECTION cs;
		unsigned int crc;
		SceneState* pScene;

		switch (buddy)
		{
		case PROTAGONIST_2B:

			crc = HashStringCRC32("Buddy/2B", 8);
			pScene = (SceneState*)FindSceneState(&cs, crc, "Buddy/2B", 8);
			LOG("return value: %x\n", pScene);

			((SceneStateSystem_SetInternalFn)(0x14001EC80))((void*)0x14158CBC0, &pScene);
			//((SceneStateSystem_SetFn)(0x14001EBE0))((void*)0x14158CBC0, &pScene);

			break;
		case PROTAGONIST_A2:
			crc = HashStringCRC32("Buddy/A2", 8);
			pScene = (SceneState*)FindSceneState(&cs, crc, "Buddy/A2", 8);
			LOG("return value: %x\n", pScene);

			((SceneStateSystem_SetInternalFn)(0x14001EC80))((void*)0x14158CBC0, &pScene);
			//((SceneStateSystem_SetFn)(0x14001EBE0))((void*)0x14158CBC0, &ret); //parent function
			break;
		case PROTAGONIST_9S:
			crc = HashStringCRC32("Buddy/9S", 8);
			pScene = (SceneState*)FindSceneState(&cs, crc, "Buddy/9S", 8);
			LOG("return value: %x\n", pScene);

			((SceneStateSystem_SetInternalFn)(0x14001EC80))((void*)0x14158CBC0, &pScene);
			//((SceneStateSystem_SetFn)(0x14001EBE0))((void*)0x14158CBC0, &ret);
			break;
		default:
			break;
		}
	}

	// exprimential
	static void AddBuddy()
	{
		CRITICAL_SECTION cs;
		unsigned int crc;
		void* pScene;
		__int64 val;

		crc = HashStringCRC32("Buddy/A2", 8);
		pScene = FindSceneState(&cs, crc, "Buddy/A2", 8);


		//(*(bool(*)(void*, __int64*))(0x14001EC80))((void*)0x14158CBC0, &ret);

		val = (*(__int64(*)(void*, unsigned long*))(0x140058A90))((void*)0x14158CC50, (DWORD*)(*(QWORD*)pScene + 0x20)); // != 0x6CFB210

		(*(__int64(*)(__int64, void*))(0x140053B00))(0x6CFB210, pScene);
	}

	// exprimential
	static void AddPlayer(int player)
	{
		CRITICAL_SECTION cs;
		unsigned int crc;
		SceneState* pScene;

		switch (player)
		{
		case PROTAGONIST_2B:
			//crc = HashStringCRC32("PL/2B", 8);
			//pScene = (SceneState*)FindSceneState(&cs, crc, "PL/2B", 8);
			crc = HashStringCRC32("Load/A29S", 10);
			pScene = (SceneState*)FindSceneState(&cs, crc, "Load/A29S", 10); // Load/A29S | Load/A22B	
			((SceneStateSystem_SetInternalFn)(0x14001EC80))((void*)0x14158CBC0, &pScene);
			break;

			break;
		case PROTAGONIST_A2:
			crc = HashStringCRC32("PL/A2", 5);
			pScene = (SceneState*)FindSceneState(&cs, crc, "PL/A2", 5);
			((SceneStateSystem_SetInternalFn)(0x14001EC80))((void*)0x14158CBC0, &pScene);
		default:
			break;
		}
	}

	static void ApplyHorizontalCollision(Pl0000* pEntity)
	{
		if (!pEntity)
			return;

		pEntity->m_HorizontalCollision.m_hOwner = pEntity->m_pInfo->m_hEntity;
	}

	static void RemoveHorizontalCollision(Pl0000* pEntity)
	{
		if (!pEntity)
			return;

		pEntity->m_HorizontalCollision.m_hOwner = 0;
		pEntity->m_HorizontalCollision.m_bCollison = FALSE;
	}

	static void UnlockAllAchievements(void)
	{
		for (unsigned int id = 0; id < g_pUserManager->m_pAchivementDevice->m_uMaxAchievement; ++id)
			UnlockAchievement(0, 0, id);
	}

	TODO("We need to remove the handle from the wetobjmanager with WetObjectManager_SetDry(0, pEntity->m_pInfo); after the wet time has elapsed")
	static int WetEntity(Pl0000* pEntity, byte wetness)
	{
		int i = 0;

		if (!pEntity)
			return -1;

		WetObjectManager_AddLocalEntity(0, pEntity->m_pInfo);

		for (; i < ARRAYSIZE(g_pWetObjectManager->m_Localhandles); ++i)
			if (g_pWetObjectManager->m_Localhandles[i] == pEntity->m_pInfo->m_hEntity)
				break;

		WetObjectManager_SetWet(0, wetness, i);

		return i;
	}

	static void SetPlayer(EntityHandle hEntity)
	{
		Pl0000* pEntity = GetEntityFromHandle(&hEntity);

		if (!pEntity)
			return;

		SetLocalPlayer(&pEntity->m_pInfo->m_hEntity);

		*(PDWORD)0x1419AED20 = 0x80000000;

		ResetCamera(g_pCamera);
	}

	static void ChangePlayerEx(Pl0000* pCameraEnt)
	{
		if (pCameraEnt && g_pLocalPlayer)
		{
			pCameraEnt->m_nBones = Vars.Gameplay.nBones;
			Vars.Gameplay.nBones = 0;
			ChangePlayer(g_pLocalPlayer);
			pCameraEnt = GetEntityFromHandle(&g_pCamera->m_hEntity);
		}
	}

	static void SwapPlayer(void)
	{
		ChangePlayerEx(GetEntityFromHandle(&g_pCamera->m_hEntity));
	}

	static void DuplicateBuddyAsNPC()
	{
		if (g_pLocalPlayer)
			g_pLocalPlayer->m_hBuddy = 0;
	}

	static void Airstuck()
	{
		if (g_pLocalPlayer)
			(*(__int64(*)(void*))(0x1401F08A0))(g_pLocalPlayer);
	}

	static void TeleportEntityToOther(EntityHandle hTeleporter, EntityHandle hTeleportee)
	{
		Pl0000* pTeleporter = GetEntityFromHandle(&hTeleporter);
		Pl0000* pTeleportee = GetEntityFromHandle(&hTeleportee);

		if (pTeleportee && pTeleporter)
			pTeleportee->m_vPosition = pTeleporter->m_vPosition + pTeleporter->m_matTransform.GetAxis(FORWARD) * 4.f;
	}

	static void TeleportForwardEx(Pl0000* pEntity, float flSpeed)
	{
		if (pEntity)
			pEntity->m_vPosition += pEntity->m_matTransform.GetAxis(FORWARD) * flSpeed;
	}

	static void TeleportForward(void)
	{
		TeleportForwardEx(GetEntityFromHandle(&g_pCamera->m_hEntity), 0.5f);
	}

	static void PlayAnimationEx(Pl0000* pEntity)
	{
		if (pEntity)
			pEntity->Animate(Vars.Gameplay.iSelectedAnimation, 0, 0, 0);
	}

	static void PlayAnimation(void)
	{
		PlayAnimationEx(GetEntityFromHandle(&g_pCamera->m_hEntity));
	}

	static void BuffEnemies(void)
	{
		if (Vars.Gameplay.bLevelBuffMode)
			BuffEnemiesAbsolute();
		else
			BuffEnemiesTolerance();
	}

	static void BuffEnemiesAbsolute(void)
	{
		for (QWORD i = 0; i < g_pEnemyManager->m_handles.m_count; ++i)
			SetEnemyLevel(GetEntityFromHandle(&g_pEnemyManager->m_handles.m_pItems[i]), Vars.Gameplay.iEnemyLevel);
	}

	static void BuffEnemiesTolerance(void)
	{
		Pl0000* pLocal = GetEntityFromHandle(g_pLocalPlayerHandle);

		if (!pLocal)
			return;

		int iMinLevel = max(0, (Vars.Gameplay.bExclusivelyPositiveTolerance) ? pLocal->m_iLevel - 1 : pLocal->m_iLevel - Vars.Gameplay.iEnemyLevelTolerance - 1);
		int iMaxLevel = min(MAX_LEVELS, pLocal->m_iLevel + Vars.Gameplay.iEnemyLevelTolerance - 1);

		for (QWORD i = 0; i < g_pEnemyManager->m_handles.m_count; ++i)
			BalanceEnemyLevel(GetEntityFromHandle(&g_pEnemyManager->m_handles.m_pItems[i]), iMinLevel, iMaxLevel);
	}

	static void BalanceEnemyLevel(void* pEnemy, int iMinLevel, int iMaxLevel)
	{
		if (!pEnemy)
			return;

		int* pLevel = MakePtr(int*, pEnemy, 0x28030);

		if ((*pLevel) > iMaxLevel || (*pLevel) < iMinLevel)
		{
			int* pHealth = MakePtr(int*, pEnemy, 0x858);
			int* pMaxHealth = MakePtr(int*, pEnemy, 0x85C);
			ExExpInfo* pInfo = MakePtr(ExExpInfo*, pEnemy, 0x6378);
			Level_t* pProperLevel = &pInfo->m_levels[RandomInt(iMinLevel, iMaxLevel)];
			*pLevel = pProperLevel->m_iLevel;
			*pHealth = pProperLevel->m_iHealth;
			*pMaxHealth = pProperLevel->m_iHealth;
		}
	}

	static void SetEnemyLevel(void* pEnemy, int iLevel)
	{
		if (!pEnemy)
			return;

		int* pLevel = MakePtr(int*, pEnemy, 0x28030);

		if ((*pLevel) != iLevel && iLevel > 0 && iLevel < MAX_LEVELS)
		{
			int* pHealth = MakePtr(int*, pEnemy, 0x858);
			int* pMaxHealth = MakePtr(int*, pEnemy, 0x85C);
			ExExpInfo* pInfo = MakePtr(ExExpInfo*, pEnemy, 0x6378);
			Level_t* pProperLevel = &pInfo->m_levels[iLevel - 1];
			*pLevel = pProperLevel->m_iLevel;
			*pHealth = pProperLevel->m_iHealth;
			*pMaxHealth = pProperLevel->m_iHealth;
		}
	}

	static void AddPod(int pod)
	{
		CRITICAL_SECTION cs;
		unsigned int crc;
		SceneState* pScene;

		switch (pod)
		{
		case POD_A:
			crc = HashStringCRC32("own_podA", 8);
			pScene = (SceneState*)FindSceneState(&cs, crc, "own_podA", 8);
			((SceneStateSystem_SetInternalFn)(0x14001EC80))((void*)0x14158CBC0, &pScene);
			break;
		case POD_B:
			crc = HashStringCRC32("own_podB", 8);
			pScene = (SceneState*)FindSceneState(&cs, crc, "own_podB", 8);
			((SceneStateSystem_SetInternalFn)(0x14001EC80))((void*)0x14158CBC0, &pScene);
			break;
		case POD_C:
			crc = HashStringCRC32("own_podC", 8);
			pScene = (SceneState*)FindSceneState(&cs, crc, "own_podC", 8);
			((SceneStateSystem_SetInternalFn)(0x14001EC80))((void*)0x14158CBC0, &pScene);
			break;
		default:
			break;
		}
	}

	static void AddItem(int id, int quantity)
	{
		((AddItemFn)(0x1405DC410))(0, id, quantity);

		if (Vars.Gameplay.bInstantEquip)
		{
			((UseItemFn)(0x1405DC5A0))(0, id, quantity);
		}
	}

	static void TeleportAllEnemiesToPoint(const Vector3& vPosition)
	{
		for (QWORD i = 0; i < g_pEnemyManager->m_handles.m_count; ++i)
		{
			Pl0000* pEntity = GetEntityFromHandle(&g_pEnemyManager->m_handles.m_pItems[i]);
			pEntity->m_vPosition = vPosition;
		}
	}

	static void TeleportAllEnemiesToEncirclePoint(const Vector3& vPosition, float flRadius)
	{
		if (!g_pEnemyManager->m_handles.m_count)
			return;

		Vector3 vEndpoint, vDirection;
		float sin, cos;
		float step = (2.0f * M_PI_F) / g_pEnemyManager->m_handles.m_count;
		float rad = 0.0f;

		for (QWORD i = 0; i < g_pEnemyManager->m_handles.m_count; ++i, rad += step)
		{
			Pl0000* pEntity = GetEntityFromHandle(&g_pEnemyManager->m_handles.m_pItems[i]);

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
		CModel tmp;

		memcpy(&tmp, pEntB, sizeof(CModel));
		tmp.m_vPosition = pEntA->m_vPosition;
		memcpy(pEntA, &tmp, sizeof(CModel));
	}

	static void AddModelMesh(Pl0000* pEntity)
	{
		CRITICAL_SECTION cs;

		if (!pEntity)
			return;

		InitializeCriticalSection(&cs);
		EnterCriticalSection(&cs);

		pEntity->m_Work.m_nMeshes++;


		LeaveCriticalSection(&cs);
	}

	static void MoveSun()
	{
		Sun* sun = (Sun*)0x14160EB40;
		//sun->m_vPosition.x += 3;
	}

	static EntityInfo* CreateEntity(const char* szName, int objectId, OPTIONAL set_info_t* pSetInfo)
	{
		Create_t c;

		ZeroMemory(&c, sizeof(c));

		c.m_szName = szName;
		c.m_ObjectIds[0] = objectId;
		c.m_ObjectIds[1] = objectId;
		c.m_iGenerateMode = 1;
		c.m_pSetInfo = pSetInfo;

		return ((SceneEntitySystem_CreateEntityFn)(0x1404F9AA0))((CSceneEntitySystem*)0x14160DFE0, &c);
	}

	// test
	static void ApplyA2Wig(EntityInfo* pInfo)
	{
		//hkCreateEntity((void*)0x1415f6b50, pInfo, 0x10000, -1, (CHeapInstance**)0x1418f6158);
		//sub_1404F9AA0((__int64)&qword_14160DFE0, &szA2Wig);
	}

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

		pChecker->m_dwWordCount = dwWordCount;
	}

	// if this doesn't work might have to create critical section, enter, restore and free the backup array, then leave
	static void DisableWordBlacklist(IN BannedWordChecker* pChecker, OUT PDWORD pdwWordCount)
	{
		if (!pChecker || !pChecker->m_pEntries)
			return;

		if (pdwWordCount)
			*pdwWordCount = pChecker->m_dwWordCount;

		pChecker->m_dwWordCount = 0;
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

		UINT dwNextDataStartOffset = 4;
		DWORD i = 0;

		pChecker->m_dwWordCount = pHdr->dwWordCount;

		if (pChecker->m_dwWordCount > 0)
		{
			pChecker->m_pEntries = new BannedWordChecker::WordEntry[pChecker->m_dwWordCount];

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
			} while (i < pChecker->m_dwWordCount);
		}
	}
};