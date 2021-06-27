#pragma once

#include "ImGui/imgui.h"
#include "ImGui/imgui_internal.h"
#include "Math.h"
#include "Utils.h"
#include "Globals.h"
#include "Configuration.h"

enum eFontRenderFlags
{
	FRF_NONE = 0,
	FRF_LEFT = 0,
	FRF_RIGHT = 1,
	FRF_CENTER_V = 2,
	FRF_CENTER_H = 4,

	FRF_OUTLINE = 16
};
typedef unsigned int FontRenderFlags;

namespace ImGuiEx 
{
	static void AddDrawListToDrawData(ImVector<ImDrawList*>* out_list, ImDrawList* draw_list)
	{
		if (draw_list->CmdBuffer.empty())
			return;

		// Remove trailing command if unused
		ImDrawCmd& last_cmd = draw_list->CmdBuffer.back();
		if (last_cmd.ElemCount == 0 && last_cmd.UserCallback == NULL)
		{
			draw_list->CmdBuffer.pop_back();
			if (draw_list->CmdBuffer.empty())
				return;
		}

		// Draw list sanity check. Detect mismatch between PrimReserve() calls and incrementing _VtxCurrentIdx, _VtxWritePtr etc. May trigger for you if you are using PrimXXX functions incorrectly.
		IM_ASSERT(draw_list->VtxBuffer.Size == 0 || draw_list->_VtxWritePtr == draw_list->VtxBuffer.Data + draw_list->VtxBuffer.Size);
		IM_ASSERT(draw_list->IdxBuffer.Size == 0 || draw_list->_IdxWritePtr == draw_list->IdxBuffer.Data + draw_list->IdxBuffer.Size);
		IM_ASSERT((int)draw_list->_VtxCurrentIdx == draw_list->VtxBuffer.Size);

		// Check that draw_list doesn't use more vertices than indexable (default ImDrawIdx = unsigned short = 2 bytes = 64K vertices per ImDrawList = per window)
		// If this assert triggers because you are drawing lots of stuff manually:
		// A) Make sure you are coarse clipping, because ImDrawList let all your vertices pass. You can use the Metrics window to inspect draw list contents.
		// B) If you need/want meshes with more than 64K vertices, uncomment the '#define ImDrawIdx unsigned int' line in imconfig.h to set the index size to 4 bytes.
		//    You'll need to handle the 4-bytes indices to your renderer. For example, the OpenGL example code detect index size at compile-time by doing:
		//      glDrawElements(GL_TRIANGLES, (GLsizei)pcmd->ElemCount, sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, idx_buffer_offset);
		//    Your own engine or render API may use different parameters or function calls to specify index sizes. 2 and 4 bytes indices are generally supported by most API.
		// C) If for some reason you cannot use 4 bytes indices or don't want to, a workaround is to call BeginChild()/EndChild() before reaching the 64K limit to split your draw commands in multiple draw lists.
		if (sizeof(ImDrawIdx) == 2)
			IM_ASSERT(draw_list->_VtxCurrentIdx < (1 << 16) && "Too many vertices in ImDrawList using 16-bit indices. Read comment above");

		out_list->push_back(draw_list);
	}

	static void InsertDrawListToDrawData(ImVector<ImDrawList*>* out_list, ImDrawList* before_draw_list, ImDrawList* draw_list)
	{
		if (draw_list->CmdBuffer.empty())
			return;

		// Remove trailing command if unused
		ImDrawCmd& last_cmd = draw_list->CmdBuffer.back();
		if (last_cmd.ElemCount == 0 && last_cmd.UserCallback == NULL)
		{
			draw_list->CmdBuffer.pop_back();
			if (draw_list->CmdBuffer.empty())
				return;
		}

		// Draw list sanity check. Detect mismatch between PrimReserve() calls and incrementing _VtxCurrentIdx, _VtxWritePtr etc. May trigger for you if you are using PrimXXX functions incorrectly.
		IM_ASSERT(draw_list->VtxBuffer.Size == 0 || draw_list->_VtxWritePtr == draw_list->VtxBuffer.Data + draw_list->VtxBuffer.Size);
		IM_ASSERT(draw_list->IdxBuffer.Size == 0 || draw_list->_IdxWritePtr == draw_list->IdxBuffer.Data + draw_list->IdxBuffer.Size);
		IM_ASSERT((int)draw_list->_VtxCurrentIdx == draw_list->VtxBuffer.Size);

		// Check that draw_list doesn't use more vertices than indexable (default ImDrawIdx = unsigned short = 2 bytes = 64K vertices per ImDrawList = per window)
		// If this assert triggers because you are drawing lots of stuff manually:
		// A) Make sure you are coarse clipping, because ImDrawList let all your vertices pass. You can use the Metrics window to inspect draw list contents.
		// B) If you need/want meshes with more than 64K vertices, uncomment the '#define ImDrawIdx unsigned int' line in imconfig.h to set the index size to 4 bytes.
		//    You'll need to handle the 4-bytes indices to your renderer. For example, the OpenGL example code detect index size at compile-time by doing:
		//      glDrawElements(GL_TRIANGLES, (GLsizei)pcmd->ElemCount, sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, idx_buffer_offset);
		//    Your own engine or render API may use different parameters or function calls to specify index sizes. 2 and 4 bytes indices are generally supported by most API.
		// C) If for some reason you cannot use 4 bytes indices or don't want to, a workaround is to call BeginChild()/EndChild() before reaching the 64K limit to split your draw commands in multiple draw lists.
		if (sizeof(ImDrawIdx) == 2)
			IM_ASSERT(draw_list->_VtxCurrentIdx < (1 << 16) && "Too many vertices in ImDrawList using 16-bit indices. Read comment above");

		//bsearch(before_draw_list, out_list->Data, out_list->Size, sizeof(ImDrawList*), [](const void* a, const void* b) -> int { return a == b; });

		out_list->insert(&(*out_list)[out_list->index_from_ptr(&before_draw_list) - 1], draw_list); //may not need the - 1
	}
}

struct OverlayDrawList : public ImDrawList
{
	OverlayDrawList(const ImDrawListSharedData* pShared) : ImDrawList(pShared) { }
	OverlayDrawList(const ImDrawList& list) : ImDrawList(list._Data) { }
	OverlayDrawList(const ImDrawList* pList) : ImDrawList(pList->_Data) { }

	void AddRectBordered(int x, int y, int width, int height, int thickness, const ImColor& color)
	{
		AddRectFilled(ImVec2((float)x, (float)y), ImVec2((float)(x + width), (float)(y + thickness)), color);
		AddRectFilled(ImVec2((float)x, (float)y), ImVec2((float)(x + thickness), (float)(y + height)), color);
		AddRectFilled(ImVec2((float)(x + width), (float)y), ImVec2((float)(x + width + thickness), (float)(y + height)), color);
		AddRectFilled(ImVec2((float)x, (float)(y + height)), ImVec2((float)(x + width + thickness), (float)(y + height + thickness)), color);
	}

	void AddRectCorners(int x, int y, int w, int h, int thickness, const ImColor& color)
	{
		int v_leg = h / 4;
		int h_leg = v_leg / 2;

		//top near
		AddRectFilled(ImVec2((float)x, (float)y), ImVec2((float)(x + h_leg), (float)(y + thickness)), color); //h
		AddRectFilled(ImVec2((float)x, (float)y), ImVec2((float)(x + thickness), (float)(y + v_leg)), color); //v

		//top far
		AddRectFilled(ImVec2((float)((x + w) - h_leg), (float)y), ImVec2((float)(x + w), (float)(y + thickness)), color); //h
		AddRectFilled(ImVec2((float)(x + w), (float)y), ImVec2((float)(x + w + thickness), (float)(y + v_leg)), color); //v

		//btm near
		AddRectFilled(ImVec2((float)x, (float)(y + h)), ImVec2((float)(x + h_leg), (float)(y + h + thickness)), color); // h
		AddRectFilled(ImVec2((float)x, (float)((y + h) - v_leg)), ImVec2((float)(x + thickness), (float)(y + h)), color); //v

		//btm far	
		AddRectFilled(ImVec2((float)((x + w) - (h_leg - thickness)), (float)(y + h)), ImVec2((float)((x + w) - (h_leg - thickness) + h_leg), (float)(y + h + thickness)), color); //h
		AddRectFilled(ImVec2((float)(x + w), (float)((y + h) - v_leg)), ImVec2((float)(x + w + thickness), (float)(y + h)), color); //v
	}

	void Add3DBox(const Vector3& vMin, const Vector3& vMax, const Matrix4x4& mTransform, const ImColor& color)
	{
		Vector2 vCorners2D[8];
		Vector3 vCornersTransform[8];
		Vector3 vCorners[8] =
		{
			Vector3(vMin.x, vMin.y, vMin.z),
			Vector3(vMin.x, vMax.y, vMin.z),
			Vector3(vMax.x, vMax.y, vMin.z),
			Vector3(vMax.x, vMin.y, vMin.z),
			Vector3(vMin.x, vMin.y, vMax.z),
			Vector3(vMin.x, vMax.y, vMax.z),
			Vector3(vMax.x, vMax.y, vMax.z),
			Vector3(vMax.x, vMin.y, vMax.z)
		};

		mTransform.Transform(vCorners[0], vCornersTransform[0]);
		mTransform.Transform(vCorners[1], vCornersTransform[1]);
		mTransform.Transform(vCorners[2], vCornersTransform[2]);
		mTransform.Transform(vCorners[3], vCornersTransform[3]);
		mTransform.Transform(vCorners[4], vCornersTransform[4]);
		mTransform.Transform(vCorners[5], vCornersTransform[5]);
		mTransform.Transform(vCorners[6], vCornersTransform[6]);
		mTransform.Transform(vCorners[7], vCornersTransform[7]);

		if (WorldToScreen(vCornersTransform[0], vCorners2D[0]) && WorldToScreen(vCornersTransform[1], vCorners2D[1]) &&
			WorldToScreen(vCornersTransform[2], vCorners2D[2]) && WorldToScreen(vCornersTransform[3], vCorners2D[3]) &&
			WorldToScreen(vCornersTransform[4], vCorners2D[4]) && WorldToScreen(vCornersTransform[5], vCorners2D[5]) &&
			WorldToScreen(vCornersTransform[6], vCorners2D[6]) && WorldToScreen(vCornersTransform[7], vCorners2D[7]))
		{
			for (int i = 1; i <= 4; ++i)
			{
				AddLine(vCorners2D[i - 1], vCorners2D[i % 4], color);
				AddLine(vCorners2D[i - 1], vCorners2D[i + 3], color);
				AddLine(vCorners2D[i + 3], vCorners2D[i % 4 + 4], color);
			}
		}
	}

	void AddHealthBar(int x, int y, int height, int health, int max_health)
	{
		int G = (int)(health * (255.f / max_health));
		int R = 255 - G;
		int pxhealth = (height - ((height * health) / max_health));
		ImColor col = ImColor(R, G, 0);

		AddTextArgs(ImVec2((float)(x + 2), (float)(y + pxhealth)), col, FRF_LEFT | FRF_OUTLINE, "%i/%i HP", health, max_health);
		AddRectFilled(ImVec2((float)(x - 4), (float)(y + pxhealth)), ImVec2((float)(x - 2), (float)(y + pxhealth + height - pxhealth + 1)), col);
		AddRectBordered(x - 5, y - 1, 3, height + 2, 1, ImColor(0, 0, 0));
	}

	void AddBone(const Vector3Aligned& vStart, const Vector3Aligned& vEnd, const ImColor& color)
	{
		Vector2 vBoneStart, vBoneEnd;

		if ((!WorldToScreen(vStart, vBoneStart)) || (!WorldToScreen(vEnd, vBoneEnd)))
			return;

		AddLine(vBoneStart, vBoneEnd, color);
	}

	void AddTextArgs(ImVec2& pos, ImU32 col, FontRenderFlags flags, const char* szText, ...)
	{
		char szBuffer[1024];
		va_list args;
		ImFont* pFont = ImGui::GetFont();

		if (!pFont)
			return;

		va_start(args, szText);

		int n = _vscprintf(szText, args);

		if (n > sizeof(szBuffer))
			return;

		vsprintf_s(szBuffer, szText, args);
		va_end(args);

		ImVec2 size = pFont->CalcTextSizeA(pFont->FontSize, FLT_MAX, 120.f, szBuffer);

		if (flags & FRF_RIGHT)
			pos.x -= size.x;
		else if (flags & FRF_CENTER_H)
			pos.x -= size.x / 2;

		if (flags & FRF_CENTER_V)
			pos.y -= size.y / 2;

		if (flags & FRF_OUTLINE)
		{
			ImVec2 out[4] = {
				{ pos.x + 1, pos.y },
				{ pos.x - 1, pos.y },
				{ pos.x, pos.y + 1 },
				{ pos.x, pos.y - 1}
			};

			for (auto& curPos : out)
				AddText(curPos, 0xFF000000, szBuffer);
		}

		AddText(pos, col, szBuffer);
	}

	void AddTextArgs(const ImFont& font, ImVec2& pos, ImU32 col, FontRenderFlags flags, const char* szText, ...)
	{
		char szBuffer[1024];
		va_list args;

		va_start(args, szText);

		int n = _vscprintf(szText, args);

		if (n > sizeof(szBuffer))
			return;

		vsprintf_s(szBuffer, szText, args);
		va_end(args);

		ImVec2 size = font.CalcTextSizeA(font.FontSize, FLT_MAX, 120.f, szBuffer);

		if (flags & FRF_RIGHT)
			pos.x -= size.x;
		else if (flags & FRF_CENTER_H)
			pos.x -= size.x / 2;

		if (flags & FRF_CENTER_V)
			pos.y -= size.y / 2;

		if (flags & FRF_OUTLINE)
		{
			ImVec2 out[4] = {
				{ pos.x + 1, pos.y },
				{ pos.x - 1, pos.y },
				{ pos.x, pos.y + 1 },
				{ pos.x, pos.y - 1}
			};

			for (auto& curPos : out)
				AddText(curPos, 0xFF000000, szBuffer);
		}

		AddText(pos, col, szBuffer);
	}
};

class Overlay
{
public:
	Overlay() : m_pList(NULL) {}
	~Overlay() { if (m_pList) IM_DELETE(m_pList); }

	OverlayDrawList* m_pList;

	OverlayDrawList* CreateDrawList(void)
	{
		m_pList = IM_NEW(OverlayDrawList)(ImGui::GetDrawListSharedData());
		m_pList->_OwnerName = "2B Hook Overlay";
		/*m_pList = (OverlayDrawList*)malloc(sizeof(OverlayDrawList));
		m_pList->_Data = ImGui::GetDrawListSharedData();*/
		return m_pList;
	}

	void Render(bool bUseBuiltInOverlay) 
	{
		if (bUseBuiltInOverlay)
		{
			Render((OverlayDrawList*)ImGui::GetOverlayDrawList());
		}
		else
		{
			Render(m_pList);
			ImGuiEx::AddDrawListToDrawData(&ImGui::GetCurrentContext()->DrawDataBuilder.Layers[0], m_pList);
		}
	}

	void Render(const char* szParentWndName)
	{
		if (!m_pList)
			return;
	
		ImGuiWindow* pWnd = ImGui::FindWindowByName(szParentWndName);

		if (!pWnd)
			return;

		Render(m_pList);
		ImGuiEx::InsertDrawListToDrawData(&ImGui::GetCurrentContext()->DrawDataBuilder.Layers[0], pWnd->DrawList, m_pList);
	}

	void Render(OverlayDrawList* pList)
	{
		Vector2 vMin2D, vMax2D, vStart2D, vEnd2D;
		Vector3Aligned vStart, vEnd, vMin, vMax;
		int iTarget = -1;
		float best = FLT_MAX;

		if (!pList)
			pList = CreateDrawList();

		if (g_pCamera)
		{
			Pl0000* pCameraEntity = GetEntityFromHandle(&g_pCamera->m_hEntity);

			if (pCameraEntity)
			{
				ExCollision* pCollision = &pCameraEntity->m_VerticalCollision;
				
				vMin = pCameraEntity->m_vPosition;
				GetOBBMax(pCollision, &vMax);

				Matrix4x4 matMesh;

				for (int i = 0; i < pCameraEntity->m_Work.m_nMeshes; ++i)
				{			
					CMesh2* pMesh = &pCameraEntity->m_Work.m_pMeshes[i];
					CModelMatrixTable** p = pCameraEntity->m_Work.m_pMatrices;
					
					/*	
					CBone* pBone = &pCameraEntity->m_pBones[GetBoneIndex(pCameraEntity->m_pModelData, pMesh->m_pBones[0].m_sId)];
					matMesh.InitTransform(pBone->m_vRotation, pBone->m_vPosition);
					pList->Add3DBox(pMesh->m_vMin, pMesh->m_vMax, matMesh, ImColor(200, 100, 0));
					*/
				}
				/*
				__m128 v1 = *(__m128*)&pCollision->v0xA0;
				vMax = _mm_add_ps(pCameraEntity->m_vPosition, _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(v1, v1, 0xAA), pCameraEntity->m_matCol.m2),
				_mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(v1, v1, 0x55), pCameraEntity->m_matCol.m1),
				_mm_mul_ps(_mm_shuffle_ps(v1, v1, 0), pCameraEntity->m_matCol.m0))));
				*/

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
					vStart = pCameraEntity->m_pBones[GetBoneIndex(pCameraEntity->m_pModelData, BONE_HEAD)].m_vPosition;
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
						Pl0000* pCur = GetEntityFromHandle(&g_pEnemyManager->m_handles.m_pItems[i]);

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
		}

		if (Vars.Visuals.bNPCInfo)
		{
			for (int i = 0; i < g_pNPCManager->m_handles.m_count; ++i)
			{
				Pl0000* pCur = GetEntityFromHandle(&g_pNPCManager->m_handles.m_pItems[i]);

				if (pCur && WorldToScreen(pCur->m_vPosition, vMin2D))
				{
					pList->AddTextArgs(vMin2D, ImColor(0, 255, 0), FRF_CENTER_H | FRF_OUTLINE, "NPC: %s HP: %i/%i", pCur->m_pInfo->m_szEntityType, pCur->m_iHealth, pCur->m_iMaxHealth);
				}
			}

			Pl0000* pEmil = GetEntityFromHandle(g_pEmilHandle);

			if (pEmil && WorldToScreen(pEmil->m_vPosition, vMin2D))
			{
				pList->AddTextArgs(vMin2D, ImColor(0, 255, 0), FRF_CENTER_H | FRF_OUTLINE, "Emil: %s HP: %i/%i", pEmil->m_pInfo->m_szEntityType, pEmil->m_iHealth, pEmil->m_iMaxHealth);
			}
		}

		if (Vars.Visuals.bCollisionObjectInfo)
		{
			CCollisionDataObject* pCur;
			Vector3Aligned vOrigin, vMax, vMin;
			CEntityInfo* pInfo;
			Pl0000* pObject;
			Matrix4x4 mTrans;

			for (CCollisionDataObject** p = g_pCollisionDataObjectManager->m_ppData; p; p = (CCollisionDataObject**) *++p)
			{
				pCur = *p;
				if (pCur) //dword30 = 0xff - trees
				{
					pInfo = g_pEntityInfoList->m_pItems[pCur->m_uScenePropIndex].second;
					pObject = (pInfo) ? pInfo->m_pParent : NULL;

					if (pObject && !pObject->m_vPosition.IsZero())
					{
						vOrigin = pObject->m_vPosition;
						vMax = (pCur->m_vMin - pCur->m_vMax) / 2;
						vMin = (pCur->m_vMax - pCur->m_vMin) / 2;
						mTrans = pObject->m_matTransform;
					}
					else
					{
						vOrigin = pCur->m_vMin + ((pCur->m_vMax - pCur->m_vMin) / 2);
						vMax = Vector3Aligned();
						vMin = pCur->m_vMax - pCur->m_vMin;
						mTrans.InitTransform(Vector3(0, 0, 1), Vector3(1, 0, 0), Vector3(0, 1, 0), pCur->m_vMin);
					}

					pList->Add3DBox(vMin, vMax, mTrans, ImColor(0, 255, 222));
#ifdef _DEBUG
					if (WorldToScreen(vOrigin, vMin2D))
					{
						pList->AddTextArgs(vMin2D, ImColor(0, 255, 0), FRF_CENTER_H | FRF_OUTLINE,
							"%s idx: %x f30 %x, f38 %x", (pInfo) ? pInfo->m_szEntityType : "",
							pCur->m_uScenePropIndex, pCur->dword30, pCur->dword38);
					}
#endif
				}
			}
		}

#ifdef _DEBUG
		if (g_pCamera)
		{
			pList->AddTextArgs(ImVec2(100, 100), ImColor(255, 255, 0), FRF_CENTER_H,
				"ang (%f,%f,%f)", RADTODEG(g_pCamera->m_vViewangles.x),
				RADTODEG(g_pCamera->m_vViewangles.y), 
				RADTODEG(g_pCamera->m_vViewangles.z));
		}			
#endif
	}
};
extern Overlay* g_pOverlay;