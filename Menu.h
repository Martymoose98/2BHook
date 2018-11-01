#pragma once
#include "Hooks.h"

static void DisplayEntityHandles()
{
	QWORD count = g_pNPCManager->m_handles.m_count + g_pYorhaManager->m_handles.m_count + g_pEnemyManager->m_handles.m_count;
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

static void ApplyModelMods(Pl0000* pEntity)
{
	if (!pEntity)
		return;

	ImGui::Checkbox("Ghost Model", &Vars.Gameplay.bGhostModel);

	ImGui::SameLine();

	ImGui::Checkbox("Model Gravity", (bool*)&pEntity->m_obb.m_bGravity);

	ImGui::SameLine();

	ImGui::Checkbox("Draw ESP Box", &Vars.Visuals.bEspBox);

	ImGui::SameLine();

	ImGui::Checkbox("Draw Skeleton", &Vars.Visuals.bSkeleton);

	ImGui::SameLine();

	ImGui::Checkbox("Debug Skeleton", &Vars.Visuals.bDebugLocalPlayerSkeleton);

	ImGui::DragFloat("Trace Line Length", &Vars.Visuals.flTraceLength, 0.00625f, 0.4f, 5.0f);

	ImGui::Checkbox("Draw Trace Line", &Vars.Visuals.bTraceLine);

	ImGui::SameLine();

	ImGui::Checkbox("Rainbow Hair", &Vars.Gameplay.bRainbowHair);

	ImGui::InputFloat3("Position (X,Y,Z)", (float*)&pEntity->m_vPosition);

	char szModelPart[64];
	sprintf_s(szModelPart, "ModelPart: (%s)", pEntity->m_pModelParts[Vars.Gameplay.iSelectedModelPart].m_szModelPart);
	ImGui::SliderInt(szModelPart, &Vars.Gameplay.iSelectedModelPart, 0, pEntity->m_nModelParts - 1);

	sprintf_s(szModelPart, "ModelPart Color: (%s)", pEntity->m_pModelParts[Vars.Gameplay.iSelectedModelPart].m_szModelPart);

	if (ImGui::Checkbox("Enabled", (bool*)&pEntity->m_pModelParts[Vars.Gameplay.iSelectedModelPart].m_bShow))
		pEntity->m_pModelParts[Vars.Gameplay.iSelectedModelPart].m_bUpdate = TRUE;

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

	if (!Vars.Gameplay.bRainbowModel)
		ImGui::ColorPicker4("Model Tint Color", (float*)&pEntity->m_pModelInfo->m_vTint);

	ImGui::InputFloat3("Model Scale (X,Y,Z)", (float*)&pEntity->m_matModelToWorld.GetAxis(1));
	ImGui::InputFloat3("Model Rotation (Pitch, Yaw, Roll)", (float*)&pEntity->m_matModelToWorld.GetAxis(3));
}