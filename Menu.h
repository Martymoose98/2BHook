#pragma once
#include <Windows.h>

#include <string>
#include <string_view>

#include "ImGui/imgui_internal.h"

#include "Console.h"
#include "Utils.h"
#include "Variables.h"

#include "SDK/Entity/CObj.h"


class CConfig;

struct ImColorScheme : public ImColor
{
	//constexpr ImColorScheme(ImColor& Color, float flAlpha)
	//{
	//	this->Value = Color.Value;
	//	this->Value.w = flAlpha;
	//}

	// Slightly Skewed for alpha blending
	static void Analogous(ImColor& Color, ImColor& Secondary, ImColor& Tertiary, float flVariance)
	{
		WORD H, L, S;

		//WORD Alpha = 0xFF;
		WORD Alpha = (WORD)((1.0f - sinf(M_PI_F * flVariance)) * 255.f);

		ColorRGBToHLS(Color, &H, &L, &S);
		Secondary = (Alpha << IM_COL32_A_SHIFT) | ColorHLSToRGB(ImLerp<WORD>(H, 255, flVariance), L, S);
		Tertiary = (Alpha << IM_COL32_A_SHIFT) | ColorHLSToRGB(ImLerp<WORD>(H, 0, -flVariance), L, S);
	}

	static ImColor AlphaModulate(ImColor& Color, float flAlpha)
	{
		ImColor ColorOut = Color;

		ColorOut.Value.w *= flAlpha;

		return ColorOut;
	}

	static ImColor Secondary(ImColor& Color)
	{
		float h = 0.0f, s = 0.0f, v = 0.0f;
		float r = 0.0f, g = 0.0f, b = 0.0f;


		ImGui::ColorConvertRGBtoHSV(Color.Value.x, Color.Value.y, Color.Value.z, h, s, v);
		ImGui::ColorConvertHSVtoRGB(h, s, v * 0.60f, r, g, b);

		return ImColor(r, g, b, Color.Value.w * (1.0f - v));
	}

	//static constexpr ImColor ()
};

class CMenu
{
public:
	CMenu(const CAdapter* pAdapter);

	~CMenu(void);

	void Draw(const ImVec2 vSize);

	HRESULT KeyboardHandler(HRESULT Result, LPVOID lpvData, DWORD cbData);
	HRESULT MouseHandler(LPVOID lpvData, DWORD cbData);

	bool IsOpen(void) const;

	ImGuiStyle* ApplyStyle(ImColor& Primary = ImColor(7, 74, 25, 255),
		ImColor& PrimaryBg = ImColor(7, 17, 74, 220));

	void LoadConfig(LPCTSTR szConfig);
	void SaveConfig(LPCTSTR szConfig);

private:
	void GameplayTab(Pl0000* pCameraEnt);
	void VisualsTab(CBehaviorAppBase* pCameraEnt);
	void SpawnTab(Pl0000* pCameraEnt);
	void MiscTab(void);
	void ConfigTab(void);

	void DisplayEntityHandles(void);

	bool m_bOpened;
	bool m_bIgnoreInputWhenOpened;
	char m_szAdapterUtf8[128 * 4];
	char m_szOutputUtf8[32 * 4];

	BOOLEAN m_KeyboardState[256];
	BOOLEAN m_OldKeyboardState[256];
	DIMOUSESTATE2 MouseState;
	DIMOUSESTATE2 OldMouseState;

	ImColor m_Primary;
	ImColor m_PrimaryBg;

	CConfig* m_pConfig;

	struct Config_t
	{
		PWIN32_FIND_DATA_LIST pHead;
		INT iSelectedConfig;
		TCHAR szName[MAX_PATH];
	} Config;

	constexpr static const char* s_MenuTitle = "2B Hook! ~ 2B Owns Me and All :^)";
};

extern CMenu* g_pMenu;

static void ApplyModelMods(Pl0000* pEntity);

static bool BlacklistItemCallback(void* data, int idx, const char** out_text);

static bool ConfigCallback(void* data, int idx, const char** out_text);

// NPC
//Pl0000* pCurrent = (Pl0000*)GetEntityFromHandleGlobal(&hCurrent);
static const char* EntityHandlesCallback(void* user_data, int idx)
{
	static char s_Buffer[64];
	EntityHandle_t hCurrent = {};
	CBehaviorAppBase* pCurrent = nullptr;
	const char* szType = "Unknown";

	if (idx < 0)
		return nullptr;

	if (idx < g_pNPCManager->m_handles.m_Count)
	{
		hCurrent = g_pNPCManager->m_handles[idx];
		pCurrent = g_pEntityList->GetEntity<CBehaviorAppBase>(hCurrent);
		szType = "NPC";
	}
	else if (idx < g_pNPCManager->m_handles.m_Count + g_pEnemyManager->m_handles.m_Count)
	{
		hCurrent = g_pEnemyManager->m_handles[idx];
		pCurrent = g_pEntityList->GetEntity<CBehaviorAppBase>(hCurrent);
		szType = "Enemy";
	}
	else
	{
		hCurrent = g_pYorhaManager->m_handles[idx];
		pCurrent = g_pEntityList->GetEntity<CBehaviorAppBase>(hCurrent);
		szType = "Yorha";
	}

	((EntityHandle*)(user_data))[idx] = hCurrent;

	sprintf_s(s_Buffer, "%s: %s | Handle: %x | ObjectId: %x", szType,
		pCurrent ? pCurrent->m_pInfo->m_szEntityType : 0, hCurrent, pCurrent ? pCurrent->m_ObjectId : 0);

	return s_Buffer;
}

static const char* EntityCallback(void* user_data, int idx)
{
	static char s_Buffer[1024];

	CEntityInfo* pInfo = g_pEntityList->m_pItems[idx].second;

	if (pInfo)
	{
		sprintf_s(s_Buffer, "%s (ObjectId %x, SetType %x) Base %llx EHANDLE %08x\n",
			(pInfo->m_szEntityType) ? pInfo->m_szEntityType : "EntityLayout",
			pInfo->m_ObjectId, pInfo->m_uSetType, pInfo->m_pEntity, pInfo->m_hEntity);
	}


	return s_Buffer;
}