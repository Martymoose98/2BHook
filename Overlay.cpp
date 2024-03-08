#include "Overlay.h"

Overlay* g_pOverlay = new Overlay();


OverlayDrawList* Overlay::CreateDrawList(void)
{
	// ImGui::GetMainViewportGetViewportBgFgDrawList()

	m_pList = IM_NEW(OverlayDrawList)(ImGui::GetDrawListSharedData());
	m_pList->_OwnerName = "2B Hook Overlay";
	/*m_pList = (OverlayDrawList*)malloc(sizeof(OverlayDrawList));
	m_pList->_Data = ImGui::GetDrawListSharedData();*/
	return m_pList;
}

void Overlay::RenderCollisions(OverlayDrawList* pList)
{
	Matrix4x4 mTrans;
	Vector2 vMin2D;
	Vector3Aligned vMin;
	Vector3Aligned vMax;
	Vector3Aligned vOrigin;

	if (!Vars.Visuals.bCollisionObjectInfo2)
		return;

	if (!g_pCollisionDataObjectManager->m_pCurrentCollision)
		return;

	for (CCollisionContextNode* pNode = g_pCollisionDataObjectManager->m_pCurrentCollision; pNode; pNode = pNode->m_pPrevious)
	{
		if (pNode->m_Context.m_pColTree)
		{
			Vector3Aligned* pvMin = &pNode->m_Context.m_pColTree->m_vP1;
			Vector3Aligned* pvMax = &pNode->m_Context.m_pColTree->m_vP2;

			vOrigin = *pvMin + ((pvMax - pvMin) / 2.0f);
			vMin = (pNode->m_Context.m_pColTree->m_vP2 - pNode->m_Context.m_pColTree->m_vP1) / 2.0f;
			vMax = (pNode->m_Context.m_pColTree->m_vP1 - pNode->m_Context.m_pColTree->m_vP2) / 2.0f;
		}

		mTrans.InitTransform(Vector3(0, 0, 1), Vector3(1, 0, 0), Vector3(0, 1, 0), vOrigin);
		pList->Add3DBox(vMin, vMax, mTrans, ImColor(222, 0, 222));

		if (Vars.Visuals.bCollisionDebugObjectInfo && WorldToScreen(vOrigin, vMin2D))
		{
			for (int i = 0; pNode->m_Context.m_nNameCount - 1; ++i)
			{
				pList->AddTextArgs(vMin2D, ImColor(22, 0, 222), FRF_CENTER_H | FRF_OUTLINE,
					"%s", pNode->m_Context.m_pNames[i]);
			}
		}
	}
}

void Overlay::RenderCollisions2(OverlayDrawList* pList)
{
	if (!Vars.Visuals.bCollisionObjectInfo)
		return;

	Vector2 vMin2D;
	Vector3Aligned vOrigin, vMax, vMin;
	CEntityInfo* pInfo;
	CBehaviorAppBase* pObject;
	Matrix4x4 mTrans;
	ImColor Color;

	for (CCollisionDataEntry* pEntry = g_pCollisionDataObjectManager->m_Data.m_pEntries2; pEntry; pEntry = pEntry->m_pNext)
	{
		CCollisionDataObject* pObj = pEntry->m_pObject;

		if (pObj) //dword30 = 0xff - trees
		{
			pInfo = g_pEntityList->m_pItems[pObj->m_uScenePropIndex].second;
			pObject = (pInfo) ? pInfo->m_pParent : NULL;

			// draws a fuck ton with this idk if i like it it
			/*for (int i = 0; i < pObj->m_nSegments; ++i)
			{
				CCollisionDataSegment* pSegment = &pObj->m_pSegments[i];

				if (pObject && !pObject->m_vPosition.IsZero())
				{
					vOrigin = pObject->m_vPosition;
					vMax = (pSegment->m_vMin - pSegment->m_vMax) / 2.0f;
					vMin = (pSegment->m_vMax - pSegment->m_vMin) / 2.0f;
					mTrans = pObject->m_matTransform;
					Color = ImColor(0, 255, 128);
				}
				else
				{
					vOrigin = pSegment->m_vMin + ((pSegment->m_vMax - pSegment->m_vMin) / 2.0f);
					vMax = (pSegment->m_vMin - pSegment->m_vMax) / 2.0f;
					vMin = (pSegment->m_vMax - pSegment->m_vMin) / 2.0f;
					Color = ImColor(0, 255, 222);

					mTrans.InitTransform(Vector3(0, 0, 1), Vector3(1, 0, 0), Vector3(0, 1, 0), vOrigin);
				}*/

			if (pObject && !pObject->m_vPosition.IsZero())
			{
				vOrigin = pObject->m_vPosition;
				vMax = (pObj->m_AABB.m_vMin - pObj->m_AABB.m_vMax) / 2.0f;
				vMin = (pObj->m_AABB.m_vMax - pObj->m_AABB.m_vMin) / 2.0f;
				mTrans = pObject->m_matTransform;
				Color = ImColor(0, 255, 128);
			}
			else
			{
				vOrigin = pObj->m_AABB.m_vMin + ((pObj->m_AABB.m_vMax - pObj->m_AABB.m_vMin) / 2.0f);
				vMax = (pObj->m_AABB.m_vMin - pObj->m_AABB.m_vMax) / 2.0f;
				vMin = (pObj->m_AABB.m_vMax - pObj->m_AABB.m_vMin) / 2.0f;
				Color = ImColor(0, 255, 222);

				mTrans.InitTransform(Vector3(0, 0, 1), Vector3(1, 0, 0), Vector3(0, 1, 0), vOrigin);
			}

			pList->Add3DBox(vMin, vMax, mTrans, Color);
			//}

			if (Vars.Visuals.bCollisionDebugObjectInfo && WorldToScreen(vOrigin, vMin2D))
			{
				pList->AddTextArgs(vMin2D, ImColor(222, 0, 222), FRF_CENTER_H | FRF_OUTLINE,
					"%s %s idx: %x", pObj->m_pNames[pObj->m_nNames - 1].m_szName, (pInfo) ? pInfo->m_szEntityType : "", pObj->m_uScenePropIndex);
			}
		}
	}
}

void Overlay::Render(bool bUseBuiltInOverlay)
{

	//ImGui::GetDrawData()->AddDrawList(m_pList);

	if (bUseBuiltInOverlay)
	{
		Render((OverlayDrawList*)ImGui::GetForegroundDrawList());
	}
	else
	{
		Render(m_pList);
		ImGui::GetDrawData()->AddDrawList(m_pList);

		//ImGuiEx::AddDrawListToDrawData(ImGui::GetCurrentContext()->Viewports[0]->DrawDataBuilder.Layers[0], m_pList);
	}
}

void Overlay::Render(const char* szParentWndName)
{
	if (!m_pList)
		return;

	ImGuiWindow* pWnd = ImGui::FindWindowByName(szParentWndName);

	if (!pWnd)
		return;

	Render(m_pList);
	ImGuiEx::InsertDrawListToDrawData(ImGui::GetCurrentContext()->Viewports[0]->DrawDataBuilder.Layers[0], pWnd->DrawList, m_pList);
}

void Overlay::Render(OverlayDrawList* pList)
{
	Vector2 vMin2D, vMax2D, vStart2D, vEnd2D;
	Vector3Aligned vStart, vEnd, vMin, vMax;
	int iTarget = -1;
	float best = FLT_MAX;

	if (!pList)
		pList = CreateDrawList();

	if (g_pCamera)
	{
		CBehaviorAppBase* pCameraEntity = g_pCamera->m_pCamEntity; //GetEntityFromHandle(&g_pCamera->m_hEntity);// this func doesn't work for 

		if (pCameraEntity)
		{
			// FIXME: hack until we figure out the other classes
			ExCollision* pCollision = &((Pl0000*)pCameraEntity)->m_VerticalCollision;

			vMin = pCameraEntity->m_vPosition;

			/* Alternative to GetOBBMax
			__m128 v1 = *(__m128*) & pCollision->v0xA0;
			vMax = _mm_add_ps(pCameraEntity->m_vPosition, _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(v1, v1, 0xAA), pCameraEntity->m_matCol.m2),
				_mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(v1, v1, 0x55), pCameraEntity->m_matCol.m1),
					_mm_mul_ps(_mm_shuffle_ps(v1, v1, 0), pCameraEntity->m_matCol.m0))));*/

			ExCollision_GetOBBMax(pCollision, &vMax);

			for (int i = 0; i < pCameraEntity->m_Work.m_nMeshes; ++i)
			{/*
				CMesh2* pMesh = &pCameraEntity->m_Work.m_pMeshes[i];
				CModelMatrixTable** p = pCameraEntity->m_Work.m_pMatrices;
				Matrix4x4 matMesh;

				CBone* pBone = &pCameraEntity->m_pBones[GetBoneIndex(pCameraEntity->m_pModelData, pMesh->m_pBones[0].m_sId)];
				matMesh.InitTransform(pBone->m_vRotation, pBone->m_vPosition);
				pList->Add3DBox(pMesh->m_vMin, pMesh->m_vMax, matMesh, ImColor(200, 100, 0));
				*/
			}


			if (Vars.Visuals.bEspBox && WorldToScreen(vMin, vMin2D) && WorldToScreen(vMax, vMax2D))
			{
				int h = (int)(vMin2D.y - vMax2D.y);
				int w = h / 2;

				if (Vars.Visuals.b2DEspBox)
				{
					pList->AddRectCorners((int)vMin2D.x - (w / 2), (int)(vMin2D.y - h), w, h, 5, ImColor(23, 128, 150));
					pList->AddTextArgs(vMin2D, ImColor(0, 255, 0), FRF_CENTER_H | FRF_OUTLINE, (pCameraEntity->m_ObjectId == 0x10000) ? "2B" : (pCameraEntity->m_ObjectId == 0x10100) ? "A2" : (pCameraEntity->m_ObjectId == 0x10200) ? "9S" : "Unknown");
					pList->AddHealthBar((int)vMin2D.x + 20 + (w / 2), (int)(vMin2D.y - h), h, pCameraEntity->m_iHealth, pCameraEntity->m_iMaxHealth);
				}
				else
				{
					pList->Add3DBox(Vector3(-0.5f, 0, -0.5f), Vector3(0.5f, 1.5f, 0.5f), pCameraEntity->m_matTransform, ImColor(23, 128, 150));
					pList->AddTextArgs(vMin2D, ImColor(0, 255, 0), FRF_CENTER_H | FRF_OUTLINE, (pCameraEntity->m_ObjectId == 0x10000) ? "2B" : (pCameraEntity->m_ObjectId == 0x10100) ? "A2" : (pCameraEntity->m_ObjectId == 0x10200) ? "9S" : "Unknown");
					pList->AddHealthBar((int)vMin2D.x + 30 + (w / 2), (int)(vMin2D.y - h), h, pCameraEntity->m_iHealth, pCameraEntity->m_iMaxHealth);
				}
			}

			if (Vars.Visuals.bTraceLine)
			{
				vStart = pCameraEntity->m_pBones[GetBoneIndex(pCameraEntity->m_Work.m_pModelData, BONE_HEAD)].m_vPosition;
				vEnd = vStart + (pCameraEntity->m_matTransform.GetAxis(FORWARD) * Vars.Visuals.flTraceLength);

				if (WorldToScreen(vStart, vStart2D) && WorldToScreen(vEnd, vEnd2D))
					pList->AddLine(vStart2D, vEnd2D, ImColor(120, 255, 0));
			}

			if (Vars.Visuals.bDebugLocalPlayerSkeleton)
			{
				for (int i = 0; i < pCameraEntity->m_nBones; ++i)
				{
					if (WorldToScreen(pCameraEntity->m_pBones[i].m_vPosition, vMax2D))
						pList->AddTextArgs(vMax2D, ImColor(0, 255, 0), FRF_CENTER_H | FRF_OUTLINE, "%i %i", i, pCameraEntity->m_pBones[i].m_Id);
				}
			}

			if (Vars.Visuals.bSkeleton)
			{
				for (int i = 0; i < pCameraEntity->m_nBones; ++i)
				{
					CBone* pCur = &pCameraEntity->m_pBones[i];

					if (pCur->m_Id != BONE_ROOT_CHILD && pCur->m_pParent && pCur->m_pParent->m_Id != BONE_ROOT)
						pList->AddBone(pCur->m_vPosition, pCur->m_pParent->m_vPosition, ImColor(200, 0, 0));
				}
			}

			if (Vars.Visuals.bEnemyInfo)
			{
				for (int i = 0; i < g_pEnemyManager->m_handles.m_count; ++i)
				{
					CBehaviorAppBase* pCur = GetEntityFromHandle(&g_pEnemyManager->m_handles.m_pItems[i]);

					if (pCur && pCur->m_iHealth > 0 && WorldToScreen(pCur->m_vPosition, vMin2D))
					{
						pList->AddTextArgs(vMin2D, ImColor(0, 255, 0), FRF_CENTER_H | FRF_OUTLINE, "Enemy: %s Lvl: %i HP: %i/%i Dist: %.1fm", pCur->m_pInfo->m_szEntityType, *MakePtr(int*, pCur, 0x28030), pCur->m_iHealth,
							pCur->m_iMaxHealth, pCameraEntity->m_vPosition.DistTo(pCur->m_vPosition));

						float fov = g_pCamera->m_vPosition.DistTo(pCur->m_vPosition);

						if (fov < best)
						{
							iTarget = i;
							best = fov;
						}
					}
				}
			}
#if 0
			if (iTarget != -1)
			{
				Pl0000* pTarget = GetEntityFromHandle(&g_pEnemyManager->m_handles.m_pItems[iTarget]);
				Pl0000* pPod = GetEntityFromHandle(&g_pLocalPlayer->m_hPod);
				Vector3& vSrc = pPod->m_pBones[GetBoneIndex(pPod->m_pModelData, 1552)].m_vPosition;
				Vector3& vDst = pTarget->m_pBones[GetBoneIndex(pTarget->m_pModelData, BONE_SPINE1)].m_vPosition;

				if (WorldToScreen(vDst, vMin2D))
					pList->AddText(vMin2D, ImColor(255, 0, 0), "Target");

				if (WorldToScreen(vSrc, vMin2D))
					pList->AddText(vMin2D, ImColor(255, 0, 0), "Src");

				if (GetAsyncKeyState(VK_XBUTTON1) & 0x8000)
					Math::CalcAngle(vSrc, vDst, g_pCamera->m_viewangles);
			}
#endif
		}

#ifdef _DEBUG
		pList->AddTextArgs(ImVec2(100, 100), ImColor(255, 255, 0), FRF_CENTER_H,
			"ang (%f,%f,%f) fov: %f cam type: %i",
			RADTODEG(g_pCamera->m_vViewangles.x),
			RADTODEG(g_pCamera->m_vViewangles.y),
			RADTODEG(g_pCamera->m_vViewangles.z),
			GetFovFromProjectionMatrix(), Vars.Misc.nSlot);
#endif
	}

	RenderCollisions(pList);
	RenderCollisions2(pList);

	if (Vars.Visuals.bNPCInfo)
	{
		for (int i = 0; i < g_pNPCManager->m_handles.m_count; ++i)
		{
			CBehaviorAppBase* pCur = GetEntityFromHandle(&g_pNPCManager->m_handles.m_pItems[i]);

			if (pCur && WorldToScreen(pCur->m_vPosition, vMin2D))
			{
				pList->AddTextArgs(vMin2D, ImColor(0, 255, 0), FRF_CENTER_H | FRF_OUTLINE, "NPC: %s HP: %i/%i", pCur->m_pInfo->m_szEntityType, pCur->m_iHealth, pCur->m_iMaxHealth);
			}
		}

		CBehaviorAppBase* pEmil = GetEntityFromHandle(g_pEmilHandle);

		if (pEmil && WorldToScreen(pEmil->m_vPosition, vMin2D))
		{
			pList->AddTextArgs(vMin2D, ImColor(0, 255, 0), FRF_CENTER_H | FRF_OUTLINE, "Emil: %s HP: %i/%i", pEmil->m_pInfo->m_szEntityType, pEmil->m_iHealth, pEmil->m_iMaxHealth);
		}
	}
}