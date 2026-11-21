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
		// Nothing to draw without a collision tree, and vOrigin/vMin/vMax would otherwise be
		// whatever the previous node left in them (or uninitialized on the first one).
		if (!pNode->m_Context.m_pColTree)
			continue;

		// The tree's root node is a centre and half extents, not two corners (Martin's IDA type
		// CCollisionTreeNode {m_vCenter, m_vExtents}; the port builds a node's box as
		// centre +/- extents x {+-1}, CollisionObject.cpp:1014). Reading them as min/max put the
		// box at (centre + extents) / 2 with half the size.
		const CCollisionTreeNode* pRoot = pNode->m_Context.m_pColTree;

		vOrigin = pRoot->m_vCenter;
		vMin = pRoot->m_vExtents * -1.0f;
		vMax = pRoot->m_vExtents;

		mTrans.InitTransform(Vector3(0, 0, 1), Vector3(1, 0, 0), Vector3(0, 1, 0), vOrigin);
		pList->Add3DBox(vMin, vMax, mTrans, ImColor(222, 0, 222));

		if (Vars.Visuals.bCollisionDebugObjectInfo && WorldToScreen(vOrigin, vMin2D))
		{
			// The condition used to be just "m_nNameCount - 1", which never looks at i: any
			// count other than 1 looped forever and read straight off the end of m_pNames.
			for (int i = 0; i < pNode->m_Context.m_nNameCount; ++i)
			{
				Vector2 vLine2D = vMin2D;
				vLine2D.y += i * ImGui::GetFontSize();	// one name per line, not all on top of each other

				pList->AddTextArgs(vLine2D, ImColor(22, 0, 222), FRF_CENTER_H | FRF_OUTLINE,
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
			// Only for the label. Bounds-checked: what m_uScenePropIndex indexes is unconfirmed,
			// and past the list's end it read whatever followed the array. m_uItems is the capacity;
			// m_uSize is the live count, and slots are not compacted.
			pInfo = (pObj->m_uScenePropIndex < g_pEntityList->m_uItems) ? g_pEntityList->m_pItems[pObj->m_uScenePropIndex].second : NULL;
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

			// m_AABB is already world space: CCollisionDataObject_Update (0x1401B98C0, port
			// CollisionObject.cpp:851) rebuilds it every frame as the union of the live
			// segments' boxes, which are built through the bone matrices. It used to be
			// re-centred on the owning entity's position and turned by its rotation, which drew
			// the box offset and spun with the entity. Draw it as it is, axis-aligned.
			vOrigin = pObj->m_AABB.m_vMin + ((pObj->m_AABB.m_vMax - pObj->m_AABB.m_vMin) / 2.0f);
			vMax = (pObj->m_AABB.m_vMax - pObj->m_AABB.m_vMin) / 2.0f;
			vMin = vMax * -1.0f;
			Color = pObject ? ImColor(0, 255, 128) : ImColor(0, 255, 222);

			mTrans.InitTransform(Vector3(0, 0, 1), Vector3(1, 0, 0), Vector3(0, 1, 0), vOrigin);

			pList->Add3DBox(vMin, vMax, mTrans, Color);
			//}

			if (Vars.Visuals.bCollisionDebugObjectInfo && WorldToScreen(vOrigin, vMin2D))
			{
				// m_nNames - 1 is index -1 for an object with no names.
				pList->AddTextArgs(vMin2D, ImColor(222, 0, 222), FRF_CENTER_H | FRF_OUTLINE,
					"%s %s idx: %x", (pObj->m_nNames > 0) ? pObj->m_pNames[pObj->m_nNames - 1].m_szName : "", (pInfo) ? pInfo->m_szEntityType : "", pObj->m_uScenePropIndex);
			}
		}
	}
}

// Whether any part of a world-space sphere can be on screen. The planes come from the view-projection
// WorldToScreen projects with (Gribb and Hartmann): a point, as a row vector, is in view while
// -w <= x <= w and -w <= y <= w (WorldToScreen's x / w and y / w run -1..1 across the screen) and
// w > 0, so each side plane is column 3 plus or minus column 0 or 1, and the camera plane column 3.
// The sphere is out once it lies wholly behind any one of them.
static bool IsSphereInView(const Vector3& vCentre, float flRadius)
{
	const VMatrix& vMatrix = g_pCamera->m_pInstance->m_ViewProjection;

	// The camera plane (w), then x >= -w, x <= w, y >= -w and y <= w
	static const struct { int iAxis; float flSign; } Planes[] = { { 0, 0.0f }, { 0, 1.0f }, { 0, -1.0f }, { 1, 1.0f }, { 1, -1.0f } };

	for (const auto& Plane : Planes)
	{
		float flPlane[4];

		for (int i = 0; i < 4; ++i)
			flPlane[i] = vMatrix[i][3] + Plane.flSign * vMatrix[i][Plane.iAxis];

		const float flNormalLength = sqrtf(flPlane[0] * flPlane[0] + flPlane[1] * flPlane[1] + flPlane[2] * flPlane[2]);
		const float flDistance = flPlane[0] * vCentre.x + flPlane[1] * vCentre.y + flPlane[2] * vCentre.z + flPlane[3];

		if (flNormalLength > 0.0f && flDistance < -flRadius * flNormalLength)
			return false;
	}

	return true;
}

// Whether the box and the skeleton are worth fitting to pEntity this frame: it needs a model with
// meshes and a skeleton, and the sphere its model can be in has to reach the screen, so enemies off
// screen or behind the camera don't have every bone projected. CModelData::Init (0x140190BD0)
// copies the WMB header's bounding box, a centre and an extent in model space, to CModelData
// +0x10 / +0x20 (Martin's m_vBoxCentre / m_vBoxExtent). Whether the extent is half or whole,
// |centre| + |extent| reaches every corner from the model's origin; it is scaled by the longest of
// m_matTransform's axes, and padded by half again plus a metre since the box is the bind pose and an
// animation can swing a limb past it.
static bool IsEntityInView(CBehaviorAppBase* pEntity)
{
	const CModelData* pModelData = pEntity->m_Work.m_pModelData;

	if (!pModelData || pEntity->m_Work.m_nMeshes <= 0 || !pEntity->m_pBones || pEntity->m_nBones <= 0)
		return false;

	Matrix4x4& mTransform = pEntity->m_matTransform;
	const float flScale = sqrtf(max(mTransform.GetAxis(0).LengthSqr(), max(mTransform.GetAxis(1).LengthSqr(), mTransform.GetAxis(2).LengthSqr())));
	const float flModelRadius = pModelData->m_vBoxCentre.Length() + pModelData->m_vBoxExtent.Length();

	return IsSphereInView(mTransform.GetAxis(3), flModelRadius * flScale * 1.5f + 1.0f);
}

// The ESP box for any entity with a skeleton, fitted to its bones: every bone in front of the camera
// is projected, and their screen-space min and max place the 2D box, the label under it and the
// health bar beside it. The 3D box uses the same bones' extents along the entity's own axes
// (m_matTransform rows 0-2, origin row 3, as Matrix4x4::Transform applies it), so it fits whatever
// pose and size the model is in. It used to be a fixed 1 x 1.5 x 1 m box for the camera entity only
// (shorter than 2B, so it stopped at her shoulders). szLabel NULL draws no label; the health bar
// needs a max health above 0, since AddHealthBar divides by it.
void Overlay::RenderEntityBox(OverlayDrawList* pList, CBehaviorAppBase* pEntity, const char* szLabel,
	int iHealth, int iMaxHealth, const ImColor& Color)
{
	Vector2 vBone2D;

	if (!pEntity->m_pBones || pEntity->m_nBones <= 0)
		return;

	Matrix4x4& mTransform = pEntity->m_matTransform;
	Vector3 vOrigin = mTransform.GetAxis(3);
	Vector3 vAxes[3] = { mTransform.GetAxis(0), mTransform.GetAxis(1), mTransform.GetAxis(2) };
	float flAxisLengthSqr[3] = { vAxes[0].LengthSqr(), vAxes[1].LengthSqr(), vAxes[2].LengthSqr() };
	Vector3 vLocalMin(FLT_MAX, FLT_MAX, FLT_MAX), vLocalMax(-FLT_MAX, -FLT_MAX, -FLT_MAX);
	ImRect Rect(FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX);
	int nProjected = 0;

	for (int i = 0; i < pEntity->m_nBones; ++i)
	{
		const Vector3 vBone = pEntity->m_pBones[i].m_vPosition;
		Vector3 vDelta = vBone - vOrigin;

		for (int a = 0; a < 3; ++a)
		{
			float flLocal = (flAxisLengthSqr[a] > 0.0f) ? vDelta.Dot(vAxes[a]) / flAxisLengthSqr[a] : 0.0f;

			vLocalMin[a] = min(vLocalMin[a], flLocal);
			vLocalMax[a] = max(vLocalMax[a], flLocal);
		}

		if (WorldToScreen(vBone, vBone2D))
		{
			Rect.Add(ImVec2(vBone2D.x, vBone2D.y));
			++nProjected;
		}
	}

	if (nProjected < 2)
		return;

	// Bones sit inside the body, the hair and the shoe soles reach past them
	Rect.Expand(Rect.GetHeight() * 0.04f);

	// Nothing of it on screen (WorldToScreen projects past the screen's edges too)
	if (!Rect.Overlaps(ImRect(0.0f, 0.0f, (float)g_pGraphicDevice->m_iScreenWidth, (float)g_pGraphicDevice->m_iScreenHeight)))
		return;

	if (Vars.Visuals.b2DEspBox)
		pList->AddRectCorners((int)Rect.Min.x, (int)Rect.Min.y, (int)Rect.GetWidth(), (int)Rect.GetHeight(), 5, Color);
	else
		pList->Add3DBox(vLocalMin, vLocalMax, mTransform, Color);

	if (szLabel)
	{
		ImVec2 vLabel(Rect.GetCenter().x, Rect.Max.y + 2.0f);

		pList->AddTextArgs(vLabel, ImColor(0, 255, 0), FRF_CENTER_H | FRF_OUTLINE, "%s", szLabel);
	}

	if (iMaxHealth > 0)
		pList->AddHealthBar((int)Rect.Max.x + 10, (int)Rect.Min.y, (int)Rect.GetHeight(), min(max(iHealth, 0), iMaxHealth), iMaxHealth);
}

// A line from every bone to its parent, leaving out the root and the bone hanging straight off it
void Overlay::RenderSkeleton(OverlayDrawList* pList, CBehaviorAppBase* pEntity, const ImColor& Color)
{
	if (!pEntity->m_pBones)
		return;

	for (int i = 0; i < pEntity->m_nBones; ++i)
	{
		CBone* pCur = &pEntity->m_pBones[i];

		if (pCur->m_Id != BONE_ROOT_CHILD && pCur->m_pParent && pCur->m_pParent->m_Id != BONE_ROOT)
			pList->AddBone(pCur->m_vPosition, pCur->m_pParent->m_vPosition, Color);
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
	Vector3Aligned vStart, vEnd;
	int iTarget = -1;
	float best = FLT_MAX;

	if (!pList)
		pList = CreateDrawList();

	if (g_pCamera)
	{
		CBehaviorAppBase* pCameraEntity = g_pCamera->m_pCamEntity; //GetEntityFromHandle(&g_pCamera->m_hEntity);// this func doesn't work for 

		if (pCameraEntity)
		{
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

			if (Vars.Visuals.bEspBox)
			{
				const char* szObject = (pCameraEntity->m_ObjectId == OBJECTID_2B) ? "2B" :
					(pCameraEntity->m_ObjectId == OBJECTID_A2) ? "A2" :
					(pCameraEntity->m_ObjectId == OBJECTID_9S) ? "9S" : "Unknown";

				RenderEntityBox(pList, pCameraEntity, szObject, pCameraEntity->m_iHealth, pCameraEntity->m_iMaxHealth, ImColor(23, 128, 150));
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
				RenderSkeleton(pList, pCameraEntity, ImColor(200, 0, 0));

			if (Vars.Visuals.bEnemyInfo)
			{
				// Enemies only ever got this text: from the first overlay (3c95653) on, the box, the
				// health bar and the skeleton were drawn for the camera entity alone. They now get
				// the same ones, under the same Draw ESP Box / Draw 2D Esp Box / Draw Skeleton ticks.
				// GetEntityFromHandle (0x140491240) is the game's enemy lookup: it ends in
				// CEntityInfo_GetEmBase (0x140308550), which returns the entity only when its type
				// info inherits the EmBase type record (CheckTypeInfoInheritance, 0x14026B930), the
				// test the game's own enemy checks make (BattleBgmAction_HasFightingEnemy,
				// 0x14033E030). So map instances (object 0x90001 and the rest of the "sc" range),
				// pods, NPCs and players never get here, unlike through the untyped lookup.
				for (size_t i = 0; i < g_pEnemyManager->m_handles.m_Count; ++i)
				{
					CBehaviorAppBase* pCur = GetEntityFromHandle(&g_pEnemyManager->m_handles.m_pItems[i]);

					if (!pCur || pCur->m_iHealth <= 0)
						continue;

					if ((Vars.Visuals.bEspBox || Vars.Visuals.bSkeleton) && IsEntityInView(pCur))
					{
						if (Vars.Visuals.bEspBox)
							RenderEntityBox(pList, pCur, NULL, pCur->m_iHealth, pCur->m_iMaxHealth, ImColor(220, 60, 60));

						if (Vars.Visuals.bSkeleton)
							RenderSkeleton(pList, pCur, ImColor(200, 0, 0));
					}

					if (WorldToScreen(pCur->m_vPosition, vMin2D))
					{
						pList->AddTextArgs(vMin2D, ImColor(0, 255, 0), FRF_CENTER_H | FRF_OUTLINE, "Enemy: %s Lvl: %i HP: %i/%i Dist: %.1fm", pCur->m_pInfo->m_szEntityType, *MakePtr(int*, pCur, 0x28030), pCur->m_iHealth,
							pCur->m_iMaxHealth, pCameraEntity->m_vPosition.DistTo(pCur->m_vPosition));

						float fov = g_pCamera->m_vPosition.DistTo(pCur->m_vPosition);

						if (fov < best)
						{
							iTarget = (int)i;
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
		// Camera type debug counter, stepped with Home/End. It used to live in hkPresent and
		// step Vars.Misc.nSlot - the save slot hkSaveFileIO records on load and "Backup Save"
		// copies - in every build, so pressing Home/End (even to move the cursor in a menu
		// text box) silently changed which SlotData_N.dat got backed up. It only feeds this
		// readout, so it lives here now, debug builds only.
		static int s_iDebugCameraType = 0;

		if (GetAsyncKeyState(VK_HOME) & 0x1)
			++s_iDebugCameraType;

		if (GetAsyncKeyState(VK_END) & 0x1)
			--s_iDebugCameraType;

		// A named position: AddTextArgs moves it by the alignment, so it takes a reference
		ImVec2 vDebugPos(100, 100);

		pList->AddTextArgs(vDebugPos, ImColor(255, 255, 0), FRF_CENTER_H,
			"ang (%f,%f,%f) fov: %f cam type: %i",
			RADTODEG(g_pCamera->m_vViewangles.x),
			RADTODEG(g_pCamera->m_vViewangles.y),
			RADTODEG(g_pCamera->m_vViewangles.z),
			GetFovFromProjectionMatrix(), s_iDebugCameraType);
#endif
	}

	RenderCollisions(pList);
	RenderCollisions2(pList);

	// GetEntityFromHandle (0x140491240) only returns enemies (it ends in CEntityInfo_GetEmBase,
	// 0x140308550), so no NPC and never Emil was shown. The untyped lookup can return any
	// behaviour, so the health comes from CBehavior's own GetHealth/GetMaxHealth (vtable slots
	// 31/30) rather than BehaviorAppBase's fields.
	if (Vars.Visuals.bNPCInfo)
	{
		for (size_t i = 0; i < g_pNPCManager->m_handles.m_Count; ++i)
		{
			CBehaviorAppBase* pCur = GetEntityFromHandleGlobal(&g_pNPCManager->m_handles.m_pItems[i]);

			if (pCur && WorldToScreen(pCur->m_vPosition, vMin2D))
			{
				pList->AddTextArgs(vMin2D, ImColor(0, 255, 0), FRF_CENTER_H | FRF_OUTLINE, "NPC: %s HP: %i/%i", pCur->m_pInfo->m_szEntityType, pCur->GetHealth(), pCur->GetMaxHealth());
			}
		}

		CBehaviorAppBase* pEmil = g_pEmilHandle ? GetEntityFromHandleGlobal(g_pEmilHandle) : NULL;

		if (pEmil && WorldToScreen(pEmil->m_vPosition, vMin2D))
		{
			pList->AddTextArgs(vMin2D, ImColor(0, 255, 0), FRF_CENTER_H | FRF_OUTLINE, "Emil: %s HP: %i/%i", pEmil->m_pInfo->m_szEntityType, pEmil->GetHealth(), pEmil->GetMaxHealth());
		}
	}
}