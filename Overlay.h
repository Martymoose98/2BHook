#pragma once

#include "ImGui/imgui.h"
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
	FRF_CENTER_H = 4
};
typedef unsigned int FontRenderFlags;

struct OverlayDrawList : public ImDrawList
{
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

	void AddHealthBar(int x, int y, int height, int health, int max_health)
	{
		int R = 255 - (int)(health * (255.f / max_health));
		int G = (int)(health * (255.f / max_health));
		int pxhealth = (height - ((height * health) / max_health));
		ImColor col = ImColor(R, G, 0);

		AddTextArgs(ImVec2((float)(x + 2), (float)(y + pxhealth)), col, FRF_LEFT, "%i/%i HP", health, max_health);
		AddRectFilled(ImVec2((float)(x - 4), (float)(y + pxhealth)), ImVec2((float)(x - 2), (float)(y + pxhealth + height - pxhealth + 1)), col);
		AddRectBordered(x - 5, y - 1, 3, height + 2, 1, ImColor(0, 0, 0));
	}

	void AddBone(Vector3Aligned vStart, Vector3Aligned vEnd, const ImColor& color)
	{
		ImVec2 vBoneStart, vBoneEnd;

		if ((!WorldToScreen(vStart, (Vector2&)vBoneStart)) || (!WorldToScreen(vEnd, (Vector2&)vBoneEnd)))
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

		AddText(pos, col, szBuffer);
	}
};

class Overlay
{
public:
	Overlay() {}
	~Overlay() {}

	void Render(void) { Render((OverlayDrawList*)ImGui::GetOverlayDrawList()); }

	void Render(OverlayDrawList* pList)
	{
		Vector2 vBottom2D, vTop2D, vStart2D, vEnd2D;
		Vector3 vStart, vEnd, vBottom, vTop;
		int iTarget = -1;
		float best = FLT_MAX;

		if (g_pCamera)
		{
			Pl0000* pCameraEntity = GetEntityFromHandle(&g_pCamera->m_hEntity);

			if (pCameraEntity)
			{
				vBottom = pCameraEntity->m_vPosition;
				vTop = Vector3(pCameraEntity->m_vPosition.x, pCameraEntity->m_vPosition.y + 1.6f, pCameraEntity->m_vPosition.z);

				if (Vars.Visuals.bEspBox && WorldToScreen(vBottom, vBottom2D) && WorldToScreen(vTop, vTop2D))
				{
					int h = (int)(vBottom2D.y - vTop2D.y);
					int w = h / 2;

					pList->AddRectCorners((int)vBottom2D.x - (w / 2), (int)(vBottom2D.y - h), w, h, 5, ImColor(23, 128, 150));
					pList->AddText(ImVec2(vBottom2D.x, vBottom2D.y), ImColor(0, 255, 0), (pCameraEntity->m_ObjectId == 0x10000) ? "2B" : (pCameraEntity->m_ObjectId == 0x10100) ? "A2" : (pCameraEntity->m_ObjectId == 0x10200) ? "9S" : "Unknown");
					pList->AddHealthBar((int)vBottom2D.x + 10 + (w / 2), (int)(vBottom2D.y - h), h, pCameraEntity->m_iHealth, pCameraEntity->m_iMaxHealth);
				}

				if (Vars.Visuals.bTraceLine)
				{
					vStart = pCameraEntity->m_pBones[11].m_vPosition;
					vEnd = vStart + (pCameraEntity->m_matTransform.GetAxis(FORWARD) * Vars.Visuals.flTraceLength);

					if (WorldToScreen(vStart, vStart2D) && WorldToScreen(vEnd, vEnd2D))
					{
						pList->AddLine(*(ImVec2*)&vStart2D, *(ImVec2*)&vEnd2D, ImColor(120, 255, 0));
					}
				}

				if (Vars.Visuals.bDebugLocalPlayerSkeleton)
				{
					for (int i = 0; i < pCameraEntity->m_nBones; ++i)
					{
						if (WorldToScreen(pCameraEntity->m_pBones[i].m_vPosition, vTop2D))
							pList->AddTextArgs(ImVec2(vTop2D.x, vTop2D.y), ImColor(0, 255, 0), FRF_CENTER_H, "%i", i);
					}
				}

				if (Vars.Visuals.bSkeleton)
				{
					for (int i = 0; i < pCameraEntity->m_nBones; ++i)
					{
						Bone* pCur = &pCameraEntity->m_pBones[i];

						if (pCur->m_pParent)
							pList->AddBone(pCur->m_vPosition, pCur->m_pParent->m_vPosition, ImColor(200, 0, 0));
					}
				}
			}
		}

		if (Vars.Visuals.bEnemyInfo)
		{
			for (int i = 0; i < g_pEnemyManager->m_handles.m_count; ++i)
			{
				Pl0000* pCur = GetEntityFromHandle(&g_pEnemyManager->m_handles.m_pItems[i]);

				if (pCur && g_pCamera->m_pEntity && WorldToScreen(pCur->m_vPosition, vBottom2D))
				{
					pList->AddTextArgs(ImVec2(vBottom2D.x, vBottom2D.y), ImColor(0, 255, 0), FRF_CENTER_H, "Enemy: %s HP: %i/%i Dist: %.1fm", pCur->m_pInfo->m_szEntityType, pCur->m_iHealth,
						pCur->m_iMaxHealth, g_pCamera->m_pEntity->m_vPosition.DistTo(pCur->m_vPosition));

					float fov = g_pCamera->m_vPosition.DistTo(pCur->m_vPosition);

					if (fov < best)
					{
						iTarget = i;
						best = fov;
					}
				}
			}
		}

		if (Vars.Visuals.bNPCInfo)
		{
			for (int i = 0; i < g_pNPCManager->m_handles.m_count; ++i)
			{
				Pl0000* pCur = GetEntityFromHandle(&g_pNPCManager->m_handles.m_pItems[i]);

				if (pCur && WorldToScreen(pCur->m_vPosition, vBottom2D))
				{
					pList->AddTextArgs(ImVec2(vBottom2D.x, vBottom2D.y), ImColor(0, 255, 0), FRF_CENTER_H, "NPC: %s HP: %i/%i", pCur->m_pInfo->m_szEntityType, pCur->m_iHealth, pCur->m_iMaxHealth);
				}
			}
		}
		
#ifdef _DEBUG
		if (g_pCamera)
			pList->AddTextArgs(ImVec2(100, 100), ImColor(255, 255, 0), FRF_CENTER_H, "ang (%f,%f,%f)", RADTODEG(g_pCamera->m_viewangles.x), RADTODEG(g_pCamera->m_viewangles.y), RADTODEG(g_pCamera->m_viewangles.z));
#endif

		if (iTarget != -1)
		{
			Pl0000* pTarget = GetEntityFromHandle(&g_pEnemyManager->m_handles.m_pItems[iTarget]);

			if (WorldToScreen(pTarget->m_vPosition, vBottom2D))
				pList->AddText(ImVec2(vBottom2D.x, vBottom2D.y), ImColor(255, 0, 0), "Target");

			if (pTarget && GetAsyncKeyState(VK_XBUTTON1) & 0x8000)
			{
				g_pCamera->m_viewangles.x = Math::LookAt(g_pCamera->m_vPosition, pTarget->m_vPosition).x;
			//	Vector3 vAng = Math::CalcAngle(g_pCamera->m_matTransform.GetAxis(0), pTarget->m_vPosition);
				//Math::VectorTAngle(pPod->m_vPosition, pTarget->m_vPosition, pPod->m_matTransform, vTAng);

				//g_pCamera->m_viewangles = vAng;
				//pPod->m_matTransform.GetAxis(2) = vTAng;
			}
		}
	}
};
extern Overlay* g_pOverlay;