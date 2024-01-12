#pragma once
#include <Windows.h>

#include <string>
#include <string_view>

#include "ImGui/imgui_internal.h"

#include "Console.h"
#include "Utils.h"


class CConfig;

class CMenu
{
public:
	CMenu(const CAdapter* pAdapter);

	~CMenu(void);

	void Draw(const ImVec2 vSize);

	void DisplayEntityHandles(void);

	HRESULT KeyboardHandler(HRESULT Result, LPVOID lpvData, DWORD cbData);
	HRESULT MouseHandler(LPVOID lpvData, DWORD cbData);

	bool IsOpen(void) const;

	ImGuiStyle* ApplyStyle(ImColor& Primary = ImColor(7, 74, 25, 255),
		ImColor& PrimaryBg = ImColor(7, 17, 74, 220));

private:
	void GameplayTab(Pl0000* pCameraEnt);
	void VisualsTab(CBehaviorAppBase* pCameraEnt);
	void MiscTab(void);
	void ConfigTab(void);
	
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