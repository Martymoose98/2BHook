#pragma once
#include <Windows.h>

#include <string>
#include <string_view>

#include "ImGui/imgui_internal.h"

#include "Console.h"
#include "Utils.h"
#include "Variables.h"

#include "SDK/Entity/CObj.h"
#include "Theme.h"
#include "Version.h"


class CConfig;

class CMenu
{
public:
	CMenu(const CAdapterOutputPair& AdapterOutput);

	~CMenu(void);

	void Draw(const ImVec2 vSize);

	HRESULT KeyboardHandler(HRESULT Result, LPVOID lpvData, DWORD cbData);
	HRESULT MouseHandler(LPVOID lpvData, DWORD cbData);

	bool IsOpen(void) const;

	// The real keyboard state from the last DirectInput poll (DIK_ codes), even while the game
	// is being fed an empty one (menu open, or the free camera flying)
	bool IsKeyDown(BYTE uDik) const { return (m_KeyboardState[uDik] & 0x80) != 0; }

	// Rebuilds the ImGui style when the theme or the UI scale changed. Call it between
	// frames (before ImGui::NewFrame): the editor only marks the style dirty, so a change
	// made while a frame is being built never swaps the style under it.
	void UpdateStyle(void);

	void LoadConfig(LPCWSTR szConfig);
	void SaveConfig(LPCWSTR szConfig);

private:
	void GameplayTab(Pl0000* pCameraEnt);
	void VisualsTab(CBehaviorAppBase* pCameraEnt);
	void SpawnTab(Pl0000* pCameraEnt);
	void MiscTab(void);
	void ConfigTab(void);
	void ThemeEditor(void);

	void DisplayEntityHandles(void);

	// Migrates a pre-version-2 theme, repairs invisible colours and resets the "unsaved"
	// baseline after a config is loaded.
	void OnConfigLoaded(void);

	bool m_bOpened;
	bool m_bIgnoreInputWhenOpened;
	char m_szAdapterUtf8[128 * 4];
	char m_szOutputUtf8[32 * 4];

	BOOLEAN m_KeyboardState[256];		// c_dfDIKeyboard
	BOOLEAN m_OldKeyboardState[256];
	DIMOUSESTATE2 MouseState;			// c_dfDIMouse2
	DIMOUSESTATE2 OldMouseState;

	ThemeSettings m_Theme;
	ThemeSettings m_SavedTheme;		// as last loaded or saved, for the "not saved yet" notice
	bool m_bStyleDirty;
	float m_flStyleScale;			// UI scale the current style was built with, 0 before the first build
	float m_flWindowRescale;		// pending menu window resize after a scale change, 0 for none
	ImVec2 m_vWindowSize;
	float m_flEditScale;			// the manual scale slider's value while it is being dragged
	bool m_bEditingScale;

	// TODO: 
	// IConfig* m_pConfig;
	CConfigXml* m_pConfig;

	struct Config_t
	{
		PWIN32_FIND_DATA_LIST pHead;
		INT iSelectedConfig;
		TCHAR szName[MAX_PATH];
	} Config;

	// "###2BHook" keeps the window ID (and its saved position) the same whatever the version
	constexpr static const char* s_MenuTitle = "2B Hook v" HOOK_VERSION_STRING " ~ 2B Owns Me and All :^)###2BHook";
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

// Entity View rows. user_data is CMenu::DisplayEntityHandles' list of the entity list slots that
// are live this frame. A slot that had no entity used to return the previous row's text.
static const char* EntityCallback(void* user_data, int idx)
{
	static char s_Buffer[256];
	const std::vector<uint32_t>* pSlots = (const std::vector<uint32_t>*)user_data;

	if (!pSlots || idx < 0 || idx >= (int)pSlots->size())
		return nullptr;

	const CEntityInfo* pInfo = g_pEntityList->m_pItems[(*pSlots)[idx]].second;

	if (!pInfo)
		return nullptr;

	// m_szEntityType is a 32-character array, not a pointer, so it was never NULL
	sprintf_s(s_Buffer, "%.32s (ObjectId %x, SetType %x) Base %p EHANDLE %08x",
		pInfo->m_szEntityType[0] ? pInfo->m_szEntityType : "EntityLayout",
		pInfo->m_ObjectId, pInfo->m_uSetType, (void*)pInfo->m_pEntity, pInfo->m_hEntity.m_uValue);

	return s_Buffer;
}