#pragma once
#include "Hooks.h"
#include "Variables.h"

static void DisplayEntityHandles()
{
	int count = g_pNPCManager->m_handles.m_count + g_pYorhaManager->m_handles.m_count + g_pEnemyManager->m_handles.m_count;
	char** ppszHandles = new char*[count];
	EntityHandle* handles = new EntityHandle[count];

	int i = 0;
	for (; i < g_pNPCManager->m_handles.m_count; ++i)
	{
		ppszHandles[i] = new char[64];
		EntityHandle hCurrent = g_pNPCManager->m_handles.m_pItems[i];
		handles[i] = hCurrent;
		Pl0000* pCurrent = GetEntityFromHandle(&hCurrent);

		sprintf_s(ppszHandles[i], 64, "NPC: %s | Handle: %x | Model: %x", pCurrent ? pCurrent->m_pInfo->m_szEntityType : 0, hCurrent, pCurrent ? pCurrent->m_ObjectId : 0);
	}

	for (; i < g_pNPCManager->m_handles.m_count + g_pEnemyManager->m_handles.m_count; ++i)
	{
		ppszHandles[i] = new char[64];
		EntityHandle hCurrent = g_pEnemyManager->m_handles.m_pItems[i];
		handles[i] = hCurrent;
		Pl0000* pCurrent = GetEntityFromHandle(&hCurrent);

		sprintf_s(ppszHandles[i], 64, "Enemy %s | Handle: %x | Model: %x", pCurrent ? pCurrent->m_pInfo->m_szEntityType : 0, hCurrent, pCurrent ? pCurrent->m_ObjectId : 0);
	}

	for (; i < count; ++i)
	{
		ppszHandles[i] = new char[64];
		EntityHandle hCurrent = g_pYorhaManager->m_handles.m_pItems[i];
		handles[i] = hCurrent;
		Pl0000* pCurrent = GetEntityFromHandle(&hCurrent);

		sprintf_s(ppszHandles[i], 64, "Yorha: %s | Handle: %x | Model: %x", pCurrent ? pCurrent->m_pInfo->m_szEntityType : 0, hCurrent, pCurrent ? pCurrent->m_ObjectId : 0);
	}

	ImGui::ListBox("NPC, Yorha & Enemy Handles", &Vars.Gameplay.iSelectedEntityHandle, ppszHandles, count);

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

	//if (ImGui::Button("Set Player"))
	//{
	//	__int64 ret = oSetLocalPlayer(&handles[Vars.Gameplay.iSelectedYorhaHandle]);
	//
	//	(*(__int64(*)(void*))(0x1401EDA40))(pNPC); //setplayer
	//}

	//ImGui::SameLine();

	for (int i = 0; i < count; ++i)
		delete[] ppszHandles[i];

	delete[] ppszHandles;
	delete[] handles;
}