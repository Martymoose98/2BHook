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
	CMenu(const CAdapterOutputPair& AdapterOutput);

	~CMenu(void);

	void Draw(const ImVec2 vSize);

	HRESULT KeyboardHandler(HRESULT Result, LPVOID lpvData, DWORD cbData);
	HRESULT MouseHandler(LPVOID lpvData, DWORD cbData);

	bool IsOpen(void) const;

	ImGuiStyle* ApplyStyle(ImColor& Primary = ImColor(7, 74, 25, 255),
		ImColor& PrimaryBg = ImColor(7, 17, 74, 220));

	void LoadConfig(LPCWSTR szConfig);
	void SaveConfig(LPCWSTR szConfig);

private:
	void GameplayTab(Pl0000* pCameraEnt);
	void VisualsTab(CBehaviorAppBase* pCameraEnt);
	void SpawnTab(Pl0000* pCameraEnt);
	void MiscTab(void);
	void ConfigTab(void);

	void DisplayEntityHandles(void);

	void RepairTransparentTheme(void);

	bool m_bOpened;
	bool m_bIgnoreInputWhenOpened;
	char m_szAdapterUtf8[128 * 4];
	char m_szOutputUtf8[32 * 4];

	BOOLEAN m_KeyboardState[256];		// c_dfDIKeyboard
	BOOLEAN m_OldKeyboardState[256];
	DIMOUSESTATE2 MouseState;			// c_dfDIMouse2
	DIMOUSESTATE2 OldMouseState;

	// Theme defaults, matching the colours documented in ApplyStyle.
	static const ImColor s_DefaultPrimary;
	static const ImColor s_DefaultPrimaryBg;

	ImColor m_Primary;
	ImColor m_PrimaryBg;

	// TODO: 
	// IConfig* m_pConfig;
	CConfigXml* m_pConfig;

	struct Config_t
	{
		PWIN32_FIND_DATA_LIST pHead;
		INT iSelectedConfig;
		TCHAR szName[MAX_PATH];
	} Config;

	constexpr static const char* s_MenuTitle = "2B Hook! ~ 2B Owns Me and All :^)";
};

extern CMenu* g_pMenu;

// Not 'static': all three are defined with external linkage in Menu.cpp. Declaring them
// static here gave every translation unit including this header an internal-linkage
// declaration with no definition behind it - unreferenced-local-function warnings, and
// unlinkable had anything outside Menu.cpp ever called one.
void ApplyModelMods(Pl0000* pEntity);

const char* BlacklistItemCallback(void* data, int idx);

const char* ConfigCallback(void* data, int idx);

// NPC, Enemy and YoRHa manager handles, concatenated in that order. CMenu::DisplayEntityHandles
// fills one per frame and hands it to the handle ListBox as its user data.
struct EntityHandleSnapshot_t
{
	const EntityHandle_t* m_pHandles;
	int m_nNPC;		// [0, m_nNPC) are NPCs
	int m_nEnemy;	// [m_nNPC, m_nNPC + m_nEnemy) are enemies, the rest are YoRHa
	int m_nCount;
};

// NPC
//Pl0000* pCurrent = (Pl0000*)GetEntityFromHandleGlobal(&hCurrent);
//
// This used to pull the handle out of the managers itself, indexing g_pEnemyManager and
// g_pYorhaManager with the *combined* list index instead of subtracting the rows before
// them. Enemy rows showed the wrong enemy (shifted by the NPC count, off the end once that
// passed the enemy count), YoRHa rows were shifted by NPCs + enemies and read past the end
// of the YoRHa list, and the handle it wrote back for Set Level / Delete / Set Player was
// that same wrong entity. It now only labels the snapshot, so the ordering is decided in
// one place.
static const char* EntityHandlesCallback(void* user_data, int idx)
{
	static char s_Buffer[64];
	const EntityHandleSnapshot_t* pSnapshot = (const EntityHandleSnapshot_t*)user_data;

	if (!pSnapshot || idx < 0 || idx >= pSnapshot->m_nCount)
		return nullptr;

	EntityHandle_t hCurrent = pSnapshot->m_pHandles[idx];
	CBehaviorAppBase* pCurrent = g_pEntityList->GetEntity<CBehaviorAppBase>(hCurrent);
	const char* szType = (idx < pSnapshot->m_nNPC) ? "NPC"
		: (idx < pSnapshot->m_nNPC + pSnapshot->m_nEnemy) ? "Enemy" : "Yorha";

	sprintf_s(s_Buffer, "%s: %s | Handle: %x | ObjectId: %x", szType,
		pCurrent ? pCurrent->m_pInfo->m_szEntityType : 0, hCurrent.m_uValue, pCurrent ? pCurrent->m_ObjectId : 0);

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