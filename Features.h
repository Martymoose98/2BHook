#pragma once
#include "Log.h"

class Features
{
public:
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
			crc = HashStringCRC32("PL/2B", 8);
			pScene = (SceneState*)FindSceneState(&cs, crc, "PL/2B", 8); // Load/A29S | Load/A22B	
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
			Entity_t* pEntity = GetEntityFromHandle(&g_pEnemyManager->m_handles.m_pItems[i]);
			pEntity->m_vPosition = vPosition;
		}
	}

	static void TeleportAllEnemiesToEncirclePoint(const Vector3& vPosition, float flRadius)
	{
		float step = (2.0f * M_PI) / g_pEnemyManager->m_handles.m_count;
		float rad = 0.0f;
		D3DXVECTOR2 xz(vPosition.x, vPosition.z);
		Vector3 vEndpoint, vForward;
		float sin, cos;

		for (QWORD i = 0; i < g_pEnemyManager->m_handles.m_count; ++i, rad += step)
		{
			Entity_t* pEntity = GetEntityFromHandle(&g_pEnemyManager->m_handles.m_pItems[i]);

			g_pMath->SinCos(rad, &sin, &cos);
			g_pMath->AngleVectors(pEntity->m_matModelToWorld.GetAxis(3), &vForward);

			vEndpoint = vPosition + (vForward * flRadius);

			xz.x += cos * (vEndpoint.x - vPosition.x) - sin * (vEndpoint.z - vPosition.z); // + vPosition.x;
			xz.y += sin * (vEndpoint.x - vPosition.x) + cos * (vEndpoint.z - vPosition.z); // + vPosition.z;

			pEntity->m_vPosition = Vector3Aligned(xz.x, vPosition.y, xz.y);
		}
	}
};