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

	static void SetPlayer(EntityHandle hEntity)
	{
		Pl0000* pEntity = GetEntityFromHandle(&hEntity);

		if (!pEntity)
			return;

		SetLocalPlayer(&pEntity->m_pInfo->m_hParent);

		*(PDWORD)0x1419AED20 = 0x80000000;

		ResetCamera(g_pCamera);
	}

	static void ChangePlayerEx(Pl0000* pCameraEnt)
	{
		if (pCameraEnt)
		{
			pCameraEnt->m_nBones = Vars.Gameplay.nBones;
			Vars.Gameplay.nBones = 0;
			ChangePlayer(g_pLocalPlayer);
			pCameraEnt = GetEntityFromHandle(&g_pCamera->m_hEntity);
		}
	}

	static void SwapPlayer()
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

	static void TeleportForward()
	{
		if (g_pLocalPlayer)
			g_pLocalPlayer->m_vPosition += g_pLocalPlayer->m_matTransform.GetAxis(FORWARD) * 0.5f;
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

	static void AddItem(int id)
	{
		typedef __int64(__fastcall* HandleSpecialItemsFn)(__int64 thisrcx, int item_id);
		//const char* pItem = ((GetItemByIdFn)(0x1405E0FD0))(0, id); //emil head

		//(*(void(*)(void* a1, signed int a2))(0x1401AE440))(g_pLocalPlayer, 15); // 5


		//g_pLocalPlayer->m_dwAccessory = ((HandleSpecialItemsFn)(0x1405E1170))(0, id);
		//(*(void(*)(void* a1, signed int a2))(0x1401AE440))(g_pLocalPlayer, g_pLocalPlayer->m_dwAccessory);

		((AddItemFn)(0x1405DC410))(0, id);

		if (Vars.Gameplay.bInstantEquip)
		{
			((UseItemFn)(0x1405DC5A0))(0, id);
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

		float step = (2.0f * M_PI_F) / g_pEnemyManager->m_handles.m_count;
		float rad = 0.0f;
		Vector2 xz(vPosition.x, vPosition.z);
		Vector3 vEndpoint;
		float sin, cos;

		for (QWORD i = 0; i < g_pEnemyManager->m_handles.m_count; ++i, rad += step)
		{
			Pl0000* pEntity = GetEntityFromHandle(&g_pEnemyManager->m_handles.m_pItems[i]);

			Math::SinCos(rad, &sin, &cos);

			vEndpoint = vPosition + (pEntity->m_matTransform.GetAxis(FORWARD) * flRadius);

			xz.x += cos * (vEndpoint.x - vPosition.x) - sin * (vEndpoint.z - vPosition.z); // + vPosition.x;
			xz.y += sin * (vEndpoint.x - vPosition.x) + cos * (vEndpoint.z - vPosition.z); // + vPosition.z;

			pEntity->m_vPosition.x = xz.x;
			pEntity->m_vPosition.z = xz.y;
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

	static void AddModelPart(Pl0000* pEntity)
	{
		CRITICAL_SECTION cs;

		if (!pEntity)
			return;

		InitializeCriticalSection(&cs);
		EnterCriticalSection(&cs);

		pEntity->m_nModelParts++;


		LeaveCriticalSection(&cs);
	}

	static void MoveSun()
	{
		Sun* sun = (Sun*)0x14160EB40;
		//sun->m_vPosition.x += 3;
	}

	static void CreateEntity(const char* szName, int objectId)
	{
		Create_t c;

		ZeroMemory(&c, sizeof(c));

		c.m_szName = szName;
		c.m_ObjectIds[0] = objectId;
		c.m_ObjectIds[1] = objectId;

		CREATE_ENTITY(&c);
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

		return CpkMount(0, szFullpath);
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
		UINT  dwDataEndOffset, dwDataStartOffset, length, dwWordByteLength, Index;

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
					Index = lpDataEnd - lpDataStart;
					length = dwWordByteLength;
					do
					{
						current_byte = (pData++)[Index];
						*(pData - 1) = current_byte - 19;
					} while (--length);
				}
				*((WCHAR*)&lpDataStart[dwWordByteLength]) = 0; 	//*((WCHAR*)(lpDataStart + dwWordByteLength)) = 0;
				pChecker->m_pEntries[i++].lpszBannedWord = (LPWSTR)lpDataStart;
			} while (i < pChecker->m_dwWordCount);
		}
	}
};