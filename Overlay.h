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

	// REF: https://www.unknowncheats.me/forum/counterstrike-global-offensive/124388-bounding-esp-boxes.html	
	void Add2DBox(const Vector3& vMin, const Vector3& vMax, const Matrix4x4& mTransform, float flThickness,
		const ImColor& color, ImRect& R)
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
			float flLeft = vCorners2D[3].x;	// left
			float flTop = vCorners2D[3].y;	// top
			float flRight = flLeft;			// right
			float flBottom = flTop;			// bottom

			for (int i = 0; i < ARRAYSIZE(vCorners2D); i++)
			{
				if (flLeft > vCorners2D[i].x)
					flLeft = vCorners2D[i].x;

				if (flTop < vCorners2D[i].y)
					flTop = vCorners2D[i].y;

				if (flRight < vCorners2D[i].x)
					flRight = vCorners2D[i].x;

				if (flBottom > vCorners2D[i].y)
					flBottom = vCorners2D[i].y;
			}

			R = ImRect(flLeft, flBottom, flRight, flTop);

			AddRectCorners(flLeft, flBottom, flRight - flLeft, flTop - flBottom, flThickness, color);
			//AddLine(ImVec2(left, bottom), ImVec2(left + flThickness, top), color, flThickness);
			//AddLine(ImVec2(left, top), ImVec2(right, top), color, flThickness);
			//AddLine(ImVec2(right, top), ImVec2(right, bottom), color, flThickness);
			//AddLine(ImVec2(right, bottom), ImVec2(left, bottom), color, flThickness);
		}
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

	void AddTextArgs(ImFont& font, ImVec2& pos, ImU32 col, FontRenderFlags flags, const char* szText, ...)
	{
		char szBuffer[1024];
		va_list args;

		va_start(args, szText);

		int n = _vscprintf(szText, args);

		if (n > sizeof(szBuffer))
			return;

		vsprintf_s(szBuffer, szText, args);
		va_end(args);
		
		ImVec2 size = font.CalcTextSizeA(font.LegacySize, FLT_MAX, 120.f, szBuffer);

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

		ImVec2 size = pFont->CalcTextSizeA(pFont->LegacySize, FLT_MAX, 120.f, szBuffer);

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
	Overlay() : m_pList(NULL)
	{
	}

	~Overlay() { if (m_pList) IM_DELETE(m_pList); }

	OverlayDrawList* m_pList;

	OverlayDrawList* CreateDrawList(void);

	void RenderCollisions(OverlayDrawList* pList);

	void RenderCollisions2(OverlayDrawList* pList);

	void Render(bool bUseBuiltInOverlay);

	void Render(const char* szParentWndName);

	void Render(OverlayDrawList* pList);
};
extern Overlay* g_pOverlay;