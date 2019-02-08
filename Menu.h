#pragma once
#include "Hooks.h"

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

	ImGui::Checkbox("Display Enemy Info", &Vars.Visuals.bEnemyInfo);

	ImGui::SameLine();

	ImGui::Checkbox("Display NPC Info", &Vars.Visuals.bNPCInfo);

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

static bool BlacklistItemCallback(void* data, int idx, const char** out_text)
{
	std::string* pItems = (std::string*)data;

	if (out_text)
		*out_text = pItems[idx].c_str();

	return true;
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

	ImGui::Checkbox("Model Collision", &Vars.Gameplay.bNoCollision);

	Vars.Gameplay.bNoCollision ? Features::RemoveHorizontalCollision(pEntity) : Features::ApplyHorizontalCollision(pEntity);

	ImGui::SameLine();

	ImGui::Checkbox("Model Gravity", (bool*)&pEntity->m_VerticalCollision.m_bEnabled);

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

	ImGui::InputFloat3("Model Scale (X,Y,Z)", (float*)&pEntity->m_matModelToWorld.GetAxis(1));
	ImGui::InputFloat3("Model Rotation (Pitch, Yaw, Roll)", (float*)&pEntity->m_matModelToWorld.GetAxis(3));

	ApplyPodMods(pEntity);
}