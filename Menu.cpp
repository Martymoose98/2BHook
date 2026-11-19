#include "Menu.h"
#include "MaterialTint.h"

CMenu* g_pMenu;

namespace
{
	// <Theme><preset>yorha</preset>: the theme preset by name
	class CThemePresetItem : public IConfigItemXml
	{
	public:
		explicit CThemePresetItem(ThemePreset& Preset)
			: m_Preset(Preset)
		{
		}

		virtual const wchar_t* GetName(void) const override
		{
			return L"preset";
		}

		virtual void ReadValue(LPCWSTR szValue, uint32_t cchValue) override
		{
			m_Preset = Theme::PresetFromKey(szValue);
		}

		virtual HRESULT Write(IXmlWriter* pWriter) override
		{
			return pWriter->WriteElementString(NULL, GetName(), NULL, Theme::GetPresetKey(m_Preset));
		}

	private:
		ThemePreset& m_Preset;
	};

	// <Theme><WindowBg>#DAD4BBF7</WindowBg>: one per ImGuiCol, named after it. Only
	// overridden colours are written; anything absent uses the preset's colour.
	class CThemeColorItem : public IConfigItemXml
	{
	public:
		CThemeColorItem(ImGuiCol Col, ThemeSettings& Settings)
			: m_Col(Col), m_Settings(Settings)
		{
			MultiByteToWideChar(CP_UTF8, 0, ImGui::GetStyleColorName(Col), -1, m_szName, ARRAYSIZE(m_szName));
		}

		virtual const wchar_t* GetName(void) const override
		{
			return m_szName;
		}

		virtual void ReadValue(LPCWSTR szValue, uint32_t cchValue) override
		{
			ImVec4 Color;

			if (Theme::ParseColor(szValue, Color))
			{
				m_Settings.m_Overrides[m_Col] = Color;
				m_Settings.m_bOverride[m_Col] = true;
			}
		}

		virtual HRESULT Write(IXmlWriter* pWriter) override
		{
			if (!m_Settings.m_bOverride[m_Col])
				return S_OK;

			wchar_t szColor[16];

			Theme::FormatColor(m_Settings.m_Overrides[m_Col], szColor);

			return pWriter->WriteElementString(NULL, m_szName, NULL, szColor);
		}

	private:
		ImGuiCol m_Col;
		ThemeSettings& m_Settings;
		wchar_t m_szName[64];
	};
}

CMenu::CMenu(const CAdapterOutputPair& AdapterOutput)
	// The theme's accent colours used to be left out of this list, so ImColor
	// default-constructed them to a fully transparent (0,0,0,0). The first Save() wrote that
	// pair out as 0, and every Load() since restored it - which is how default.xml ended up
	// with i_theme_fg/i_theme_bg of 0. ThemeSettings now initializes them itself.
	: m_bOpened(false), m_bIgnoreInputWhenOpened(true),
	m_bStyleDirty(true), m_flStyleScale(0.0f), m_flWindowRescale(0.0f), m_vWindowSize(0.0f, 0.0f),
	m_flEditScale(1.0f), m_bEditingScale(false),
	m_pConfig(new CConfigXml())
{
	//no ini file
	ImGui::GetIO().IniFilename = NULL;

	//const CAdapter* pAdapter = &g_pGraphicDevice->m_Factory.m_pAdapters[AdapterOutput.m_iAdapter];
	const CAdapter* pAdapter = g_pGraphicDevice->GetAdapter(AdapterOutput.m_iAdapter);
	const COutput* pOutput = &pAdapter->m_pOutputs[AdapterOutput.m_iOutput];

	ImTextStrToUtf8(m_szAdapterUtf8, sizeof(m_szAdapterUtf8), (const ImWchar*)pAdapter->m_szDescription, NULL);
	ImTextStrToUtf8(m_szOutputUtf8, sizeof(m_szOutputUtf8), (const ImWchar*)pOutput->m_szDeviceName, NULL);

	m_pConfig->m_Items.emplace(IConfigItemXml::MakePair(CATEGORY_KEYBINDS, L"kb_open_menu",
		CKeybindToggleable(L"kb_open_menu", DIK_INSERT, &m_bOpened)));

	m_pConfig->m_Items.emplace(IConfigItemXml::MakePair(CATEGORY_MENU, L"b_ignore_input", m_bIgnoreInputWhenOpened));
	// The Custom preset's two colours stay where the whole theme used to live, so configs from
	// before the presets (and older 2B Hook builds) keep reading them.
	m_pConfig->m_Items.emplace(IConfigItemXml::MakePair(CATEGORY_MENU, L"i_theme_fg", m_Theme.m_Accent));
	m_pConfig->m_Items.emplace(IConfigItemXml::MakePair(CATEGORY_MENU, L"i_theme_bg", m_Theme.m_AccentBg));

	m_pConfig->m_Items.emplace(CATEGORY_THEME, new CThemePresetItem(m_Theme.m_Preset));
	m_pConfig->m_Items.emplace(IConfigItemXml::MakePair(CATEGORY_THEME, L"b_auto_scale", m_Theme.m_bAutoScale));
	m_pConfig->m_Items.emplace(IConfigItemXml::MakePair(CATEGORY_THEME, L"fl_ui_scale", m_Theme.m_flScale));

	for (int i = 0; i < ImGuiCol_COUNT; ++i)
		m_pConfig->m_Items.emplace(CATEGORY_THEME, new CThemeColorItem(i, m_Theme));

	if (!m_pConfig->CreateConfig(NULL))
		LERROR("CreateConfig failed - no config was loaded or written!\n");

	Config.pHead = NULL;

	m_pConfig->EnumerateConfigs(NULL, &Config.pHead);

	// Set selected config to head.
	Config.iSelectedConfig = 0;

	// filename copy. EnumerateConfigs legitimately finds nothing when no config file
	// exists yet; this used to dereference the null head and take the whole Setup thread
	// down with it, before InitHooks() had installed the D3D Present hook.
	if (Config.pHead)
	{
		_tcscpy_s(Config.szName, Config.pHead->m_Data.cFileName);
	}
	else
	{
		_tcscpy_s(Config.szName, CONFIG_DEFAULT_NAME);
		LERROR("EnumerateConfigs found no config files - defaulting to %s\n", Config.szName);
	}

	OnConfigLoaded();

	// The Present hook isn't installed yet, so no frame can be in flight
	UpdateStyle();
}

CMenu::~CMenu(void)
{
	FindDataListFree(Config.pHead);

	delete m_pConfig;
}

void CMenu::OnConfigLoaded(void)
{
	// Version 1 configs only had the two accent colours. Keep their look: the Custom preset
	// built from them, or Default when they are the stock colours.
	if (m_pConfig->GetLoadedVersion() == 1)
	{
		const bool bStock = !memcmp(&m_Theme.m_Accent.Value, &Theme::s_DefaultAccent.Value, sizeof(ImVec4)) &&
			!memcmp(&m_Theme.m_AccentBg.Value, &Theme::s_DefaultAccentBg.Value, sizeof(ImVec4));

		m_Theme.m_Preset = bStock ? THEME_DEFAULT : THEME_CUSTOM;
	}

	// What's on disk, before any repair below - so a repaired theme shows as not saved yet
	m_SavedTheme = m_Theme;

	// A (nearly) transparent text, background or accent colour hides the whole menu, with no
	// way to fix it, since the menu is what you'd use. Configs written before the constructor
	// initialized the accents contain exactly that.
	if (const char* szRepaired = Theme::Sanitize(m_Theme))
	{
		LERROR("Config theme colours were (nearly) transparent (%s) - restored the defaults\n", szRepaired);

		// Say so in the console too. This silently rewrites what the config asked for, so the
		// user should be told rather than left wondering why their theme changed.
		g_pConsole->Warn("Config theme colours were (nearly) transparent (%s), which hides the menu. "
			"Restored the defaults - save your config to keep them.", szRepaired);
	}

	m_bStyleDirty = true;
}

void CMenu::UpdateStyle(void)
{
	const float flScale = Theme::GetTargetScale(m_Theme, g_hWnd);

	if (!m_bStyleDirty && flScale == m_flStyleScale)
		return;

	// Keep the menu window the same size relative to its contents when the scale changes.
	// Accumulated, because the window may be closed through several changes.
	if (m_flStyleScale > 0.0f && flScale != m_flStyleScale)
		m_flWindowRescale = ((m_flWindowRescale > 0.0f) ? m_flWindowRescale : 1.0f) * (flScale / m_flStyleScale);

	// Built from scratch every time, so ScaleAllSizes never compounds
	ImGuiStyle Style;

	Theme::Build(m_Theme, Style);
	Style.ScaleAllSizes(flScale);
	Style.FontScaleDpi = flScale;

	ImGui::GetStyle() = Style;

	m_flStyleScale = flScale;
	m_bStyleDirty = false;
}

void CMenu::Draw(const ImVec2 vSize)
{
	if (!m_bOpened)
		return;

	const float flScale = (m_flStyleScale > 0.0f) ? m_flStyleScale : 1.0f;

	if (m_flWindowRescale > 0.0f && m_vWindowSize.x > 0.0f)
		ImGui::SetNextWindowSize(ImVec2(m_vWindowSize.x * m_flWindowRescale, m_vWindowSize.y * m_flWindowRescale), ImGuiCond_Always);
	else
		ImGui::SetNextWindowSize(ImVec2(vSize.x * flScale, vSize.y * flScale), ImGuiCond_FirstUseEver);

	m_flWindowRescale = 0.0f;

	// Resizable now; just don't let it shrink past usable
	ImGui::SetNextWindowSizeConstraints(ImVec2(480.0f * flScale, 320.0f * flScale), ImVec2(FLT_MAX, FLT_MAX));

	//static size_t title_off = 0;
	//static size_t last_move = 0;
	//static std::string_view title = "Burn slow like blunts with yayo... peel more skins the Idaho potato";
	//
	//std::string_view view = title.substr(title_off);
	//
	//if (ImGui::GetCurrentContext()->FrameCount - last_move >= 50)
	//{
	//	last_move = ImGui::GetCurrentContext()->FrameCount;
	//	++title_off %= title.size() - 1;
	//}

	if (ImGui::Begin(s_MenuTitle, &m_bOpened))
	{
		m_vWindowSize = ImGui::GetWindowSize();

		ImGui::Text("%s (%s)\tAverage %.3f ms / frame(%.1f FPS)", m_szAdapterUtf8, m_szOutputUtf8, 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

		CBehaviorAppBase* pCameraEnt = g_pCamera->m_pCamEntity;

		if (ImGui::BeginTabBar("MainTabs"))
		{
			if (ImGui::BeginTabItem("Gameplay"))
			{
				GameplayTab(static_cast<Pl0000*>(pCameraEnt));
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Visuals"))
			{
				VisualsTab(pCameraEnt);
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Spawn"))
			{
				SpawnTab(static_cast<Pl0000*>(pCameraEnt));
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Miscellaneous"))
			{
				MiscTab();
				ImGui::EndTabItem();
			}

			// The tab shows ImGui's unsaved-document dot while the theme differs from the config
			if (ImGui::BeginTabItem("Config", NULL, (m_Theme != m_SavedTheme) ? ImGuiTabItemFlags_UnsavedDocument : 0))
			{
				ConfigTab();
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Console"))
			{
				g_pConsole->Draw("Console");
				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}
	}
	ImGui::End();
}

HRESULT CMenu::KeyboardHandler(HRESULT Result, LPVOID lpvData, DWORD cbData)
{
	memcpy(&m_OldKeyboardState, &m_KeyboardState, cbData);
	memcpy(&m_KeyboardState, lpvData, cbData);

	auto [begin, end] = m_pConfig->GetKeybinds();

	for (auto& it = begin; it != end; ++it)
	{
		CConfigItemXml<CKeybind>* pItem = static_cast<CConfigItemXml<CKeybind>*>(it->second);
		CKeybind& Keybind = pItem->GetValue();

		if (Keybind.GetMode() == IKeybind::KEYBIND_ON_KEYPRESSED)
		{
			if (KEYPRESSED(m_KeyboardState, m_OldKeyboardState, Keybind.GetKeycode()))
				Keybind.OnPressed();
		}
		else if (Keybind.GetMode() == IKeybind::KEYBIND_ON_KEYDOWN)
		{
			if (KEYDOWN(m_KeyboardState, Keybind.GetKeycode()))
				Keybind.OnPressed();
		}
	}

	//for (CKeybind* pKeybind : m_pConfig->GetKeybinds())
	//{
	//	if (pKeybind->GetMode() == IKeybind::KEYBIND_ON_KEYPRESSED)
	//	{
	//		if (KEYPRESSED(m_KeyboardState, m_OldKeyboardState, pKeybind->GetKeycode()))
	//			pKeybind->OnPressed();
	//	}
	//	else if (pKeybind->GetMode() == IKeybind::KEYBIND_ON_KEYDOWN)
	//	{
	//		if (KEYDOWN(m_KeyboardState, pKeybind->GetKeycode()))
	//			pKeybind->OnPressed();
	//	}
	//}

	if (m_bOpened && m_bIgnoreInputWhenOpened)
		return DIERR_INPUTLOST;

	// The free camera flies on these keys (FreeCamKeyboardMouse); the game would walk 2B with
	// them, since it only locks her gamepad. Escape still goes through, so the game can pause.
	if (SUCCEEDED(Result) && (Vars.Misc.uCameraFlags & Variables_t::Misc_t::CameraFlg::CAMERA_FREE) && lpvData)
	{
		BYTE* pKeys = (BYTE*)lpvData;
		const BYTE uEscape = (cbData > DIK_ESCAPE) ? pKeys[DIK_ESCAPE] : 0;

		memset(pKeys, 0, cbData);

		if (cbData > DIK_ESCAPE)
			pKeys[DIK_ESCAPE] = uEscape;
	}

	return Result;
}

HRESULT CMenu::MouseHandler(LPVOID lpvData, DWORD cbData)
{
	if (m_bOpened && m_bIgnoreInputWhenOpened)
		return DIERR_INPUTLOST;

	memcpy(&MouseState, lpvData, cbData);

	return S_OK;
}

bool CMenu::IsOpen(void) const
{
	return m_bOpened;
}

void CMenu::LoadConfig(LPCWSTR szConfig)
{
	g_pConsole->Log(ImGui::GetStyle().Colors[ImGuiCol_TextSelectedBg], "Loading config. (%ls)", szConfig);

	// Loading only sets the items the file has, and an absent colour override means "use the
	// preset's colour" - not "keep the previous config's override".
	memset(m_Theme.m_bOverride, 0, sizeof(m_Theme.m_bOverride));

	HRESULT hr = m_pConfig->Load(szConfig);

	if (FAILED(hr))
		g_pConsole->Warn("Loading config %ls failed (%08X)", szConfig, hr);

	// Applied by UpdateStyle before the next frame, not in the middle of this one
	OnConfigLoaded();
}

void CMenu::SaveConfig(LPCWSTR szConfig)
{
	g_pConsole->Log(ImGui::GetStyle().Colors[ImGuiCol_TextSelectedBg], "Saving config. (%ls)", szConfig);
	HRESULT hr = m_pConfig->Save(szConfig);

	if (SUCCEEDED(hr))
		m_SavedTheme = m_Theme;
	else
		g_pConsole->Warn("Saving config %ls failed (%08X)", szConfig, hr);
}

// 40 53 48 83 EC 20 48 8B 91 80 72 - pl0000::SetStateMosaic
void CMenu::GameplayTab(Pl0000* pCameraEnt)
{
	ImGui::Checkbox("Godmode", &Vars.Gameplay.bGodmode);
	ImGui::SameLine();
	ImGui::Checkbox("No Fall Damage", &Vars.Gameplay.bNoWorldDamage);
	ImGui::SameLine();
	ImGui::Checkbox("No Enemy Damage", &Vars.Gameplay.bNoEnemyDamage);
	ImGui::SameLine();
	ImGui::Checkbox("Invincible Hacking Ship", &Vars.Gameplay.bInvincibleHackingShip);

	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("The ship in hacking and in the Ending E credits shooter takes no hits. Godmode covers this too.");

	if (!pCameraEnt)
		return;

	ImGui::SliderFloat("Tick Base", &pCameraEnt->m_pInfo->m_pUnknown->m_flTickBase, 0.0f, 32.0f);

	ImGui::Columns(2);
	ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.2f);

	ImGui::InputInt("Experience:", g_piExperience, 1, 5);
	ImGui::InputInt("Level:", &Vars.Gameplay.iLevel, 1, 5);
	ImGui::SameLine();

	if (ImGui::Button("Apply Level"))
	{
		if (g_piExperience)
		{
			Vars.Gameplay.iLevel = min(MAX_LEVELS, max(MIN_LEVELS, Vars.Gameplay.iLevel));
			*g_piExperience = g_pLocalPlayer->m_LevelsContainer.m_levels[Vars.Gameplay.iLevel - 1].m_iMinimumExperience;
		}
	}

	ImGui::InputInt("Money:", g_piMoney, 1000, 10000);
	ImGui::SameLine();

	if (ImGui::Button("Max Money"))
		*g_piMoney = MAX_MONEY;

	ImGui::PopItemWidth();
	ImGui::NextColumn();
	ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.2f);

	ImGui::InputInt((Vars.Gameplay.bLevelBuffMode) ? "Enemy Level (1-99):" : "Enemy Level Tolerance (+/-):",
		(Vars.Gameplay.bLevelBuffMode) ? &Vars.Gameplay.iEnemyLevel : &Vars.Gameplay.iEnemyLevelTolerance, 1, 5);

	ImGui::Checkbox("Balance Enemy Levels", &Vars.Gameplay.bBalanceEnemyLevels);
	ImGui::SameLine();
	ImGui::Checkbox("Exclusively Positive", &Vars.Gameplay.bExclusivelyPositiveTolerance);
	ImGui::Checkbox("Absolute Level", &Vars.Gameplay.bLevelBuffMode);

	ImGui::PopItemWidth();
	ImGui::Columns();

	// "###ItemName" keeps the input's ID fixed while the resolved name in its label changes;
	// otherwise the field lost focus whenever a keystroke matched a different item. %.48s keeps
	// a long item name inside the buffer (sprintf_s would hit the invalid parameter handler).
	char szDesc[96];
	const char* szName = ItemManager_GetItemNameById(g_pItemManager, Vars.Gameplay.iSpawnItemId);
	// FIXME: Broken on the new version this fucking function keeps crashing
	sprintf_s(szDesc, "Item Name or Id: (%.48s)###ItemName", szName ? szName : "?");
	ImGui::InputText(szDesc, Vars.Gameplay.szItemName, _ARRAYSIZE(Vars.Gameplay.szItemName));
	int id = ItemManager_GetItemIdByName(g_pItemManager, Vars.Gameplay.szItemName);
	Vars.Gameplay.iSpawnItemId = (id != -1) ? id : atoi(Vars.Gameplay.szItemName);
	ImGui::InputInt("Quantity", &Vars.Gameplay.iSpawnItemQuantity);
	ImGui::Checkbox("Instant Equip", &Vars.Gameplay.bInstantEquip);
	ImGui::SameLine();

	if (ImGui::Button("Spawn Item"))
		Features::AddItem(Vars.Gameplay.iSpawnItemId, Vars.Gameplay.iSpawnItemQuantity);

	ImGui::Separator();

	ImGui::ListBox("Animations", &Vars.Gameplay.iSelectedAnimation, Vars.AnimationListBoxList, _ARRAYSIZE(Vars.AnimationListBoxList));

	if (ImGui::Button("Play Animation"))
	{
		pCameraEnt->Animate(Vars.Gameplay.iSelectedAnimation, 0, 1, 0);
	}

	ImGui::SameLine();

	if (ImGui::Button("Change Player"))
	{
		Features::SwapPlayer();
	}

	ImGui::SameLine();

	if (ImGui::Button("Destroy Buddy") && g_pLocalPlayer)
	{
		Pl0000* pBuddy = static_cast<Pl0000*>(GetEntityFromHandle2(&g_pLocalPlayer->m_hBuddy));

		if (pBuddy)
			DestroyBuddy(pBuddy);
	}

	if (ImGui::Button("Duplicate Buddy As NPC") && g_pLocalPlayer)
	{
		//Features::DuplicateBuddyAsNPC()
		g_pLocalPlayer->m_hBuddy.m_uValue = NULL;
	}

	ImGui::SameLine();

	if (ImGui::Button("A2 As Buddy"))
	{
		Features::ChangeBuddy(PROTAGONIST_A2);
	}

	ImGui::SameLine();

	if (ImGui::Button("2B As Buddy"))
	{
		Features::ChangeBuddy(PROTAGONIST_2B);
	}

	ImGui::SameLine();

	if (ImGui::Button("9S As Buddy"))
	{
		Features::ChangeBuddy(PROTAGONIST_9S);
	}

	ImGui::Checkbox("No Model Collision", &Vars.Gameplay.bNoCollision);

	Vars.Gameplay.bNoCollision ? Features::RemoveHorizontalCollision(pCameraEnt) : Features::ApplyHorizontalCollision(pCameraEnt);

	ImGui::SameLine();

	if (pCameraEnt)
		ImGui::Checkbox("Model Gravity", (bool*)&pCameraEnt->m_VerticalCollision.m_bEnabled);
}

void CMenu::VisualsTab(CBehaviorAppBase* pCameraEnt)
{
	ImGui::Checkbox("Wireframe", &Vars.Misc.bWireframe);

	ImGui::SameLine();

	ImGui::Checkbox("Chams", &Vars.Visuals.bChams);

	ImGui::SameLine();

	ImGui::Checkbox("Use Shader", (bool*)&Vars.Visuals.iChamType);

	ImGui::SameLine();

	ImGui::Checkbox("Display Enemy Info", &Vars.Visuals.bEnemyInfo);

	ImGui::SameLine();

	ImGui::Checkbox("Display NPC Info", &Vars.Visuals.bNPCInfo);

	ImGui::Separator();

	ImGui::Checkbox("Display Collision Object Info", &Vars.Visuals.bCollisionObjectInfo);
	ImGui::Checkbox("Display Collision Object Info 2", &Vars.Visuals.bCollisionObjectInfo2);
	ImGui::Checkbox("Display Collision Debug Object Info", &Vars.Visuals.bCollisionDebugObjectInfo);


	ImGui::Separator();

	// Degrees, below 180 (hkCCameraGameSetViewAngles clamps a config's value the same way)
	ImGui::DragFloat("Camera Fov", &Vars.Visuals.flFov, 1.0f, 20.0f, 179.0f, "%.1f", ImGuiSliderFlags_AlwaysClamp);

	ImGui::Separator();
	//ImGui::Checkbox("Display Emil Info", &Vars.Visuals.bEmilInfo);

	ApplyModelMods(static_cast<Pl0000*>(pCameraEnt));
}

void CMenu::SpawnTab(Pl0000* pCameraEnt)
{
	if (ImGui::Button("Spawn Pod A"))
		Features::AddPod(POD_A);

	ImGui::SameLine();

	if (ImGui::Button("Spawn Pod B"))
		Features::AddPod(POD_B);

	ImGui::SameLine();

	if (ImGui::Button("Spawn Pod C"))
		Features::AddPod(POD_C);

	DisplayEntityHandles();

	if (!pCameraEnt)
		return;

	ImGui::ListBox("Entity", &Vars.Gameplay.iSelectedEntityType,
		[](void* pUserData, int idx) -> const char* { return ((Variables_t::Spawn*)pUserData)[idx].m_szName; },
		(void*)Vars.SpawnEntities, _ARRAYSIZE(Vars.SpawnEntities));

	ImGui::InputFloat3("Entity Scale", (float*)&Vars.Gameplay.vSpawnEntityScale);

	if (ImGui::Button("Create Entity"))
	{
		set_info_t set_info;

		set_info.m_mat = Matrix4x4();
		set_info.m_vPosition = pCameraEnt->m_vPosition + (pCameraEnt->m_matTransform.GetAxis(FORWARD) * 2.f);
		set_info.m_vRotation = pCameraEnt->m_matModelToWorld.GetAxis(POSITION); //  Vector3Aligned(0, 1, 0);
		set_info.m_vScale = Vars.Gameplay.vSpawnEntityScale;
		set_info.m_i0x080 = -1;
		set_info.m_i0x084 = 0;
		set_info.m_i0x088 = 1;
		set_info.m_iGroupId = -1;
		set_info.m_i0x07C = 1; //flags
		set_info.m_dwSetType = 1;
		set_info.m_dw0x74 = 0;
		set_info.m_i0x078 = 0;

		const char* szClass = Vars.SpawnEntities[Vars.Gameplay.iSelectedEntityType].m_szClass;
		uint32_t ObjectId = Vars.SpawnEntities[Vars.Gameplay.iSelectedEntityType].m_ObjectId;

		//CObjReadSystem::Work* pWork = FindObjectWork(ObjectId);

		//if (!pWork)
		//{
		//	if (CreateWork(ObjectId))
		//	{
		//		static CObjReadSystem* pObjReadSystem = (CObjReadSystem*)FindPatternPtr(NULL, "8B 15 ? ? ? ? 83 FA FF 74 4D", 2);
		//		typedef __int64 (*CObjReadSystem_RequestStartFn)(CObjReadSystem* pThis, unsigned int uObjectId, int a3); // a3 usually 0
		//		static CObjReadSystem_RequestStartFn RequestStart = (CObjReadSystem_RequestStartFn)FindPattern(NULL, "40 57 41 54 41 55 41 56 41 57 48 83 EC 30 48 C7 44 24 20 FE FF FF FF 48 89 5C 24 68 48 89 6C 24 70 48 89 74 24 78 41");
		//		__int64 r = RequestStart(pObjReadSystem, ObjectId, 1); // bool return
		//			
		//		pWork = FindObjectWork(ObjectId);
		//
		//		typedef CObjReadSystem::Work* (*ReadWorkFn)(int ObjectId);
		//		//static ReadWorkFn ReadWork = (ReadWorkFn)FindPattern(NULL, "48 89 5C 24 10 57 48 81 EC D0 00");
		//		//pWork = ReadWork(ObjectId);
		//	}
		//}

		CEntityInfo* pInfo = Features::CreateEntity(szClass, ObjectId, &set_info);

		if (pInfo)
		{
			switch (pInfo->m_ObjectId)
			{
			case OBJECTID_2B:
				pCameraEnt->m_hBuddy = pInfo->m_hEntity;
				//SetSceneEntity("buddy_2B", pInfo); // FIXME: !!!
				//SetSceneEntity("buddy", pInfo);
				break;
			case OBJECTID_A2:
				pCameraEnt->m_hBuddy = pInfo->m_hEntity;
				//SetSceneEntity("buddy_A2", pInfo);
				//SetSceneEntity("buddy", pInfo);
				break;
			case OBJECTID_9S:
				pCameraEnt->m_hBuddy = pInfo->m_hEntity;
				//SetSceneEntity("buddy_9S", pInfo);
				//SetSceneEntity("buddy", pInfo);
				break;
			case 0x11010:
				*g_pEmilHandle = pInfo->m_hEntity;
				break;
			case 0x10010:
				// Pl0000::SetFlightUnit (0x1404F8400) puts the player in the flight unit m_hFlightSuit
				// names. It was called at v1.02's RVA, Pl0000_EnterFighterForm+0x40 on build 26534214.
				if (Pl0000_SetFlightUnit)
				{
					pCameraEnt->m_hFlightSuit = pInfo->m_hEntity;
					Pl0000_SetFlightUnit(pCameraEnt);
				}
				break;
			}
		}
	}

	ImGui::ListBox("Spawn Blacklist", &Vars.Gameplay.iSelectedBlacklistItem, BlacklistItemCallback, Vars.Gameplay.SpawnBlacklist.data(), (int)Vars.Gameplay.SpawnBlacklist.size());
	ImGui::InputText("Blacklist Item", Vars.Gameplay.szBlacklistName, ARRAYSIZE(Vars.Gameplay.szBlacklistName));

	if (ImGui::Button("Add Item"))
	{
		Vars.Gameplay.SpawnBlacklist.emplace_back(Vars.Gameplay.szBlacklistName);

		// Select the new row so Delete Item works on it straight away. Otherwise, after the
		// last row was deleted, the selection stayed at -1 and Delete did nothing.
		Vars.Gameplay.iSelectedBlacklistItem = (int)Vars.Gameplay.SpawnBlacklist.size() - 1;
	}

	ImGui::SameLine();

	if (ImGui::Button("Delete Item"))
	{
		std::vector<std::string>& Blacklist = Vars.Gameplay.SpawnBlacklist;
		int& iSelected = Vars.Gameplay.iSelectedBlacklistItem;

		// Erase by position. This used to read data()[iSelected] unchecked - null with an
		// empty list, past the end once the last row had been deleted - and then erase
		// whatever std::find returned, which is end() when nothing matched and the *first*
		// duplicate rather than the selected row when something did.
		if (iSelected >= 0 && iSelected < (int)Blacklist.size())
		{
			Blacklist.erase(Blacklist.begin() + iSelected);

			// Keep the selection on a row that still exists.
			if (iSelected >= (int)Blacklist.size())
				iSelected = (int)Blacklist.size() - 1;
		}
	}
}

void CMenu::MiscTab(void)
{
	if (ImGui::Button("Teleport Emil To Me"))
		Features::TeleportEntityToOther(*g_pLocalPlayerHandle, *g_pEmilHandle);

	ImGui::SameLine();

	if (ImGui::Button("Teleport Me To Emil"))
		Features::TeleportEntityToOther(*g_pEmilHandle, *g_pLocalPlayerHandle);

	ImGui::InputText("Sound Name", Vars.Misc.szSoundName, _ARRAYSIZE(Vars.Misc.szSoundName));

	if (ImGui::Button("Play Sound"))
	{
		Sound sound = { Vars.Misc.szSoundName, FNV1Hash(Vars.Misc.szSoundName), 0 };
		ExecuteSound(&sound);
	}

	ImGui::SameLine();

	if (ImGui::Button("Emit Sound"))
	{
		g_pLocalPlayer->EmitSound(Vars.Misc.szSoundName, g_pLocalPlayer, 5, 0x5000000, 0x200007F);
		//Sound sound = { Vars.Misc.szSoundName, FNV1Hash(Vars.Misc.szSoundName), 0 };
		//QWORD rax = ((EmitSoundFn)(0x14081FD80))(&sound, &g_pLocalPlayer);
		//LOG("RAX = %x\n", rax);
	}

	ImGui::SameLine();

	// TODO: EXPERIMENTAL
	if (ImGui::Button("Emit Particle"))
		Features::CreateParticle(g_pCamera->m_pCamEntity, 15);

	ImGui::InputFloat3("Viewangles", (float*)&g_pCamera->m_vViewangles);

	ImGui::Checkbox("Speedhack", &Vars.Gameplay.bSpeedhack);
	ImGui::SameLine();
	// Above 0: the game's QueryPerformanceCounter is scaled by it (hkQueryPerformanceCounter), and
	// a counter that stands still or runs backwards stalls its frame pacing
	ImGui::SliderFloat("Speed Multiplier", &Vars.Gameplay.flSpeedMultiplier, 0.1f, 10.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
	ImGui::Separator();
	ImGui::InputText("Mount Custom Cpk", Vars.Misc.szCpkName, _ARRAYSIZE(Vars.Misc.szCpkName));

	if (ImGui::Button("Mount"))
	{
		CpkEntry* pEntry = Features::LoadCpk(Vars.Misc.szCpkName);
		Vars.Misc.bCpkLoaded = pEntry != NULL;
	}
	ImGui::SameLine();
	ImGui::Text((Vars.Misc.bCpkLoaded) ? "%s successfully mounted!" : "%s failed to mount!", Vars.Misc.szCpkName);

	if (ImGui::Button("Unlock All Achievements"))
		Features::UnlockAllAchievements();

	ImGui::Checkbox("No Tutorial Dialogs", &Vars.Gameplay.bNoTutorialDialogs);
	ImGui::InputInt("Frame Budget (3ms step)", &g_pGraphics->m_iFrameBudget);
	ImGui::Checkbox("Anti-VSync", &Vars.Misc.bAntiVSync);
	ImGui::SameLine();
	ImGui::Checkbox("Anti-Framerate Cap", &Vars.Misc.bAntiFramerateCap);
	//ImGui::CheckboxFlagsT<uint32_t>("Anti-Framerate Cap", g_pGameFlags2, 0x04);
	ImGui::SameLine();

	// The slot Backup Save copies. Loading a save also moves it to that slot (hkSaveFileIO),
	// but injecting after the load used to leave it on slot 0 with no way to change it.
	ImGui::SetNextItemWidth(ImGui::GetFontSize() * 7.0f);
	ImGui::SliderInt("##BackupSlot", &Vars.Misc.nSlot, 0, 2, "SlotData_%d", ImGuiSliderFlags_AlwaysClamp);
	ImGui::SameLine();

	if (ImGui::Button("Backup Save"))
	{
		Vars.Misc.bBackupSave = !BackupSave((CSaveDataDevice::Slot)Vars.Misc.nSlot);
	}

	ImGui::SameLine();
	ImGui::Text(Vars.Misc.bBackupSave ? "Succeeded!" : "Failed!");

	static int ui_ids[3];

	ImGui::InputInt3("UI Id", ui_ids);
	ImGui::SameLine();

	// The game text CORE_QUEST_EMP_TXT_MONEY from UI file slot 24, in a black banner with the type,
	// style and kind above. These were v1.02 RVAs that land mid-function on build 26534214, so they
	// are found by signature now (FindSteamOffsets), and without them the button is off.
	const bool bUIElementAvailable = HashLowercaseStringNoLenCRC32 && DialogUI_System_GetText && g_pUiFileSlots &&
		DialogUI_System_CreateBlackBanner && g_pDialogUISystem;

	ImGui::BeginDisabled(!bUIElementAvailable);

	if (ImGui::Button("Create UI Element"))
	{
		uint32_t uNameHash = HashLowercaseStringNoLenCRC32("CORE_QUEST_EMP_TXT_MONEY");
		const wchar_t* szMsg = DialogUI_System_GetText(g_pUiFileSlots, (int)uNameHash, 24);

		// The banner keeps the pointer, so a text that isn't loaded gets an empty string, not NULL
		DialogUI_System_CreateBlackBanner(g_pDialogUISystem, -1, 0, szMsg ? szMsg : L"",
			L"It wasn't supposed to be like this!\nCazzone", ui_ids[0], ui_ids[1], ui_ids[2]);
	}

	ImGui::EndDisabled();

	if (!bUIElementAvailable)
		ImGui::SetItemTooltip("Not available on this game build.");

	// new test shit
	ImGui::InputInt("Debug Flags", (int*)g_pDebugFlags, 1, 100, ImGuiInputTextFlags_CharsHexadecimal);
	ImGui::InputInt("Camera Flags", (int*)g_pCameraFlags, 1, 100, ImGuiInputTextFlags_CharsHexadecimal);
	ImGui::InputInt("Game Flags", (int*)g_pGameFlags, 1, 100, ImGuiInputTextFlags_CharsHexadecimal);
	ImGui::InputInt("Game Flags2", (int*)g_pGameFlags2, 1, 100, ImGuiInputTextFlags_CharsHexadecimal);
}

void CMenu::ConfigTab(void)
{
	LPCTSTR szConfig;

	ImGui::Checkbox("Ignore Input", &m_bIgnoreInputWhenOpened);

	auto [begin, end] = m_pConfig->GetKeybinds();

	for (auto& it = begin; it != end; ++it)
	{
		CConfigItemXml<CKeybind>* pItem = static_cast<CConfigItemXml<CKeybind>*>(it->second);
		CKeybind& Keybind = pItem->GetValue();
		const KeyOrdinal* pKey = FindKeyOrdinal(Keybind.GetKeycode());

		if (pKey)
		{
			char name[256];
			wcstombs_s(nullptr, name, Keybind.GetName(), _TRUNCATE);

			if (ImGui::BeginCombo(name, pKey->m_szName))
			{
				for (int i = 0; i < ARRAYSIZE(s_Keycodes); ++i)
					if (ImGui::Selectable(s_Keycodes[i].m_szName))
						Keybind.SetKeycode(s_Keycodes[i].m_uKeyCode);

				ImGui::EndCombo();
			}
		}
	}

	ThemeEditor();

	ImGui::SeparatorText("Configs");

	ImGui::ListBox("Configs", &Config.iSelectedConfig, ConfigCallback, Config.pHead, (INT)FindDataListCount(Config.pHead));
	ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.5f);
	ImGui::InputText("Config Name", Config.szName, _ARRAYSIZE(Config.szName));
	ImGui::PopItemWidth();
	ImGui::SameLine();

	if (ImGui::Button("Refresh"))
	{
		FindDataListFree(Config.pHead);
		Config.pHead = NULL;
		m_pConfig->EnumerateConfigs(NULL, &Config.pHead);

		// The list the selection indexed into is gone; a stale index would walk off the
		// rebuilt one.
		Config.iSelectedConfig = 0;
	}

	// Resolve the name *after* Refresh. This used to be read above it, so clicking Refresh
	// freed the node szConfig pointed into and the mbstowcs_s below read freed memory.
	PWIN32_FIND_DATA_LIST pSelected = FindDataListNav(Config.pHead, Config.iSelectedConfig);

	szConfig = (Config.szName[0]) ? Config.szName : (pSelected) ? pSelected->m_Data.cFileName : NULL;

	// FindDataListNav returns NULL past the end, and the name box can be empty, so szConfig
	// is genuinely reachable as NULL - which trips the CRT invalid parameter handler and
	// takes the process down.
	if (!szConfig)
	{
		ImGui::TextDisabled("No config selected");
		return;
	}

	wchar_t cfgname[MAX_PATH];
	mbstowcs_s(nullptr, cfgname, szConfig, _TRUNCATE);

	if (ImGui::Button("Load"))
		LoadConfig(cfgname);

	ImGui::SameLine();

	// "###Save" keeps the button's ID stable while its label changes
	if (ImGui::Button((m_Theme != m_SavedTheme) ? "Save *###Save" : "Save###Save"))
		SaveConfig(cfgname);
}

void CMenu::ThemeEditor(void)
{
	bool bChanged = false;

	ImGui::SeparatorText("Theme");

	int iPreset = (int)m_Theme.m_Preset;

	ImGui::SetNextItemWidth(ImGui::GetFontSize() * 9.0f);

	if (ImGui::Combo("Preset", &iPreset, "Default\0YoRHa\0Custom\0"))
	{
		m_Theme.m_Preset = (ThemePreset)iPreset;
		bChanged = true;
	}

	ImGui::SameLine();

	if (ImGui::Button("Reset"))
	{
		m_Theme.Reset();
		bChanged = true;
	}

	ImGui::SetItemTooltip((m_Theme.m_Preset == THEME_CUSTOM)
		? "Clear the colour overrides and put the Custom colours back to the default"
		: "Clear the colour overrides");

	if (m_Theme.m_Preset == THEME_CUSTOM)
	{
		const ImGuiColorEditFlags Flags = ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf;

		ImGui::SetNextItemWidth(ImGui::GetFontSize() * 16.0f);
		bChanged |= ImGui::ColorEdit4("Accent", (float*)&m_Theme.m_Accent.Value, Flags);
		ImGui::SetNextItemWidth(ImGui::GetFontSize() * 16.0f);
		bChanged |= ImGui::ColorEdit4("Background", (float*)&m_Theme.m_AccentBg.Value, Flags);
	}

	bChanged |= ImGui::Checkbox("Auto UI scale", &m_Theme.m_bAutoScale);
	ImGui::SetItemTooltip("Follow the display's DPI and the game's resolution (1080p = 1x)");
	ImGui::SameLine();

	if (m_Theme.m_bAutoScale)
	{
		ImGui::TextDisabled("%.2fx", m_flStyleScale);
	}
	else
	{
		// Applied when the slider is let go. Rescaling while dragging would move the slider
		// out from under the mouse.
		if (!m_bEditingScale)
			m_flEditScale = m_Theme.m_flScale;

		ImGui::SetNextItemWidth(ImGui::GetFontSize() * 9.0f);
		ImGui::SliderFloat("UI scale", &m_flEditScale, Theme::MIN_SCALE, 3.0f, "%.2fx", ImGuiSliderFlags_AlwaysClamp);
		m_bEditingScale = ImGui::IsItemActive();

		if (ImGui::IsItemDeactivatedAfterEdit())
		{
			m_Theme.m_flScale = m_flEditScale;
			bChanged = true;
		}
	}

	if (ImGui::TreeNode("Colours"))
	{
		ImGui::TextDisabled("Pick a colour to override the preset's. x puts it back.");

		const ImGuiStyle& Style = ImGui::GetStyle();
		const ImGuiColorEditFlags Flags = ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar |
			ImGuiColorEditFlags_AlphaPreviewHalf;

		if (ImGui::BeginChild("##ThemeColours", ImVec2(0.0f, ImGui::GetFontSize() * 16.0f), ImGuiChildFlags_Borders))
		{
			for (int i = 0; i < ImGuiCol_COUNT; ++i)
			{
				ImGui::PushID(i);

				// The live style already has this colour, override included
				ImVec4 Color = m_Theme.m_bOverride[i] ? m_Theme.m_Overrides[i] : Style.Colors[i];

				if (ImGui::ColorEdit4("##Colour", (float*)&Color, Flags))
				{
					m_Theme.m_Overrides[i] = Color;
					m_Theme.m_bOverride[i] = true;
					bChanged = true;
				}

				ImGui::SameLine();

				if (m_Theme.m_bOverride[i])
				{
					if (ImGui::SmallButton("x"))
					{
						m_Theme.m_bOverride[i] = false;
						bChanged = true;
					}

					ImGui::SetItemTooltip("Use the preset's colour again");
					ImGui::SameLine();
					ImGui::Text("%s *", ImGui::GetStyleColorName(i));
				}
				else
				{
					ImGui::TextUnformatted(ImGui::GetStyleColorName(i));
				}

				ImGui::PopID();
			}
		}

		ImGui::EndChild();
		ImGui::TreePop();
	}

	// Live preview: rebuilt before the next frame
	if (bChanged)
		m_bStyleDirty = true;

	if (m_Theme != m_SavedTheme)
	{
		// A fixed amber that reads on both the dark presets and YoRHa's beige
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.80f, 0.40f, 0.00f, 1.00f));
		ImGui::TextWrapped("Theme changes are live but not saved. Press Save below to keep them.");
		ImGui::PopStyleColor();
	}
}

void CMenu::DisplayEntityHandles(void)
{
	if (!g_pNPCManager || !g_pEnemyManager || !g_pYorhaManager)
		return;

	const int nYorha = (int)g_pYorhaManager->m_handles.m_Count;

	EntityHandleSnapshot_t Snapshot;
	Snapshot.m_nNPC = (int)g_pNPCManager->m_handles.m_Count;
	Snapshot.m_nEnemy = (int)g_pEnemyManager->m_handles.m_Count;
	Snapshot.m_nCount = Snapshot.m_nNPC + Snapshot.m_nEnemy + nYorha;

	// Filled here, in full, in the NPC / Enemy / YoRHa order the ListBox labels. This used to
	// be filled by the ListBox getter as a side effect, but ImGui only calls the getter for
	// rows that are on screen, so a selection scrolled out of view was never written and the
	// buttons below acted on an invalid handle.
	std::vector<EntityHandle_t> Handles(Snapshot.m_nCount);
	int i = 0;

	for (int j = 0; j < Snapshot.m_nNPC; ++j)
		Handles[i++] = g_pNPCManager->m_handles.m_pItems[j];

	for (int j = 0; j < Snapshot.m_nEnemy; ++j)
		Handles[i++] = g_pEnemyManager->m_handles.m_pItems[j];

	for (int j = 0; j < nYorha; ++j)
		Handles[i++] = g_pYorhaManager->m_handles.m_pItems[j];

	Snapshot.m_pHandles = Handles.data();

	ImGui::ListBox("NPC, Yorha & Enemy Handles", &Vars.Gameplay.iSelectedActorEntityHandle,
		&EntityHandlesCallback, &Snapshot, Snapshot.m_nCount);

	// The lists shrink as entities die and despawn, which leaves the selection past the end
	// (and with nothing listed at all, index 0 is already out of range).
	const bool bActorSelected = (Vars.Gameplay.iSelectedActorEntityHandle >= 0 &&
		Vars.Gameplay.iSelectedActorEntityHandle < Snapshot.m_nCount);

	EntityHandle_t hActor = (bActorSelected) ? Handles[Vars.Gameplay.iSelectedActorEntityHandle] : EntityHandle_t();

	ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.25f);

	ImGui::InputInt("Level", &Vars.Gameplay.iSelectedEntityLevel, 1, 5);

	ImGui::SameLine();

	// The game's SetLevel takes the level itself (1..99), not level - 1
	if (ImGui::Button("Set Level") && bActorSelected)
	{
		Vars.Gameplay.iSelectedEntityLevel = min(MAX_LEVELS, max(MIN_LEVELS, Vars.Gameplay.iSelectedEntityLevel));
		Features::BalanceEnemyLevel(GetEntityFromHandleGlobal(&hActor), Vars.Gameplay.iSelectedEntityLevel, Vars.Gameplay.iSelectedEntityLevel);
	}

	ImGui::PopItemWidth();

#if 0
	//FIXME: DEFO CRASH
	if (ImGui::Button("Set NPC Follow") && bActorSelected)
	{
		CBehaviorAppBase* pNPC = GetEntityFromHandle(&hActor);

		if (pNPC)
			((NPC_ChangeSetTypeFollowFn)(0x1404B2770))(pNPC);
	}

	ImGui::SameLine();

	//FIXME: DEFO CRASH
	if (ImGui::Button("Set NPC Idle") && bActorSelected)
	{
		CBehaviorAppBase* pNPC = GetEntityFromHandle(&hActor);

		if (pNPC)
			((NPC_ChangeSetTypeIdleFn)(0x1404B2770))(pNPC);
	}
#endif

	if (ImGui::Button("Delete") && bActorSelected)
		Features::DeleteEntity(hActor);

#ifdef  _DEBUG
	ImGui::SameLine();

	// "##Actor": the Entity View below has its own "Set Player" button in the same window,
	// and two widgets with one ID means only one of them ever responds.
	if (ImGui::Button("Set Player##Actor") && bActorSelected)
	{
		Features::SetPlayer(hActor);
	}
#endif

	ImGui::SeparatorEx(ImGuiSeparatorFlags_::ImGuiSeparatorFlags_Horizontal);

	// Every live slot of the entity list. This listed slots 0 to m_uSize - 1, but m_uSize (+4) is
	// the live count (CEntityInfo::Release, 0x140745DB0, takes one off) and m_uItems (+0) the
	// capacity that GetEntityInfoFromHandle (0x140745F20) bounds by. Slots are reused out of order
	// (CSceneEntitySystem::CreateEntityInternal, 0x1407453C0, takes the next free one after a
	// rolling index), so entities past the first gaps were missed and empty slots were listed.
	// An entry counts as live the way GetEntityInfoFromHandle tests it.
	std::vector<uint32_t> EntitySlots;

	for (uint32_t i = 0; i < g_pEntityList->m_uItems; ++i)
	{
		const CEntityInfo* pInfo = g_pEntityList->m_pItems[i].second;

		if (pInfo && !(pInfo->m_Flags & 3))
			EntitySlots.push_back(i);
	}

	ImGui::ListBox("Entity View", &Vars.Gameplay.iSelectedEntityHandle, &EntityCallback, &EntitySlots, (int)EntitySlots.size());

	// Same as above: the entity list changes under a selection made in an earlier frame.
	const bool bEntitySelected = (Vars.Gameplay.iSelectedEntityHandle >= 0 &&
		Vars.Gameplay.iSelectedEntityHandle < (int)EntitySlots.size());

	const EntityHandle_t hEntity = (bEntitySelected) ?
		g_pEntityList->m_pItems[EntitySlots[Vars.Gameplay.iSelectedEntityHandle]].first : EntityHandle_t();

	if (ImGui::Button("Delete Entity") && bEntitySelected)
		Features::DeleteEntity(hEntity);

	ImGui::SameLine();

	if (ImGui::Button("Set Player") && bEntitySelected)
	{
		Features::SetPlayer(hEntity);
	}
}

void ApplyPodMods(Pl0000* pOwner)
{
	// pOwner is the camera entity, cast without a check, so it is tested before its pod slots are
	// read. The pod used to come from GetEntityFromHandle (0x140491240), which ends in
	// CEntityInfo_GetEmBase (0x140308550) and so only returns enemies, never a pod; and it was
	// always slot 0, not the pod that is out.
	CBehaviorAppBase* pPod = GetActivePod(GetPl0000(pOwner));

	if (!pPod)
		return;

	ImGui::Checkbox("Rainbow Pod", &Vars.Gameplay.bRainbowPod);
	ImGui::SameLine();
	// Held hidden while ticked; unticking shows it again once, rather than forcing alpha 1 over the
	// game's every frame this tab is open
	if (ImGui::Checkbox("Hide Pod", &Vars.Gameplay.bHidePod) || Vars.Gameplay.bHidePod)
		Features::SetModelAlpha(pPod, Vars.Gameplay.bHidePod ? 0.f : 1.f);

	if (pPod->m_Work.m_nMeshes <= 0)
		return;

	// The selection outlives the pod it was made on (pods get swapped and respawned), so pull
	// it back into this pod's range before it is used to index m_pMeshes below.
	Vars.Gameplay.iSelectedPodMesh = min(max(Vars.Gameplay.iSelectedPodMesh, 0), pPod->m_Work.m_nMeshes - 1);

	char szModelPart[64];
	// "###" keeps the widget ID fixed while the mesh name in the label changes. Otherwise the
	// ID changes with every step and a drag stops after moving one mesh. AlwaysClamp because
	// Ctrl+Click text entry is otherwise allowed past the bounds, straight into m_pMeshes.
	// %.40s keeps a long mesh name from overflowing the buffer (sprintf_s would hit the
	// invalid parameter handler) and from cutting off the ### ID.
	sprintf_s(szModelPart, "Pod Mesh: (%.40s)###PodMesh", pPod->m_Work.m_pMeshes[Vars.Gameplay.iSelectedPodMesh].m_szMeshName);
	ImGui::SliderInt(szModelPart, &Vars.Gameplay.iSelectedPodMesh, 0, pPod->m_Work.m_nMeshes - 1, "%d", ImGuiSliderFlags_AlwaysClamp);

	CMeshPart* pPodMesh = &pPod->m_Work.m_pMeshes[Vars.Gameplay.iSelectedPodMesh];

	sprintf_s(szModelPart, "Pod Mesh Color: (%s)", pPodMesh->m_szMeshName);

	// Needs its own ID: ApplyModelMods draws an "Enabled" checkbox in the same window just
	// above this one, and the duplicate ID made this one swallow clicks.
	if (ImGui::Checkbox("Enabled##PodMesh", (bool*)&pPodMesh->m_bShow))
		pPodMesh->m_bUpdate = TRUE;

	ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.3f);

	ImGui::ColorPicker4(szModelPart, (float*)&pPodMesh->m_vColor);

	ImGui::PopItemWidth();

	// Push the colour edit through. This used to index the pod's meshes with the *player's*
	// mesh selection (iSelectedModelMesh) - another model's index - so it touched an unrelated
	// pod mesh, or wrote past the end of the pod's much shorter mesh array. It also forced
	// that mesh visible every frame; pointed at the right mesh, that would undo the Enabled
	// checkbox above as soon as it was unticked, so only the update flag is set now.
	pPodMesh->m_bUpdate = TRUE;
}

// Issue #16: the mesh colour tints every material of a mesh at once (2B's "Body" mesh holds the
// dress, tights, skin and boots), so each material can get its own colour here.
static void MaterialTintEditor(Pl0000* pEntity, CMeshPart* pMesh)
{
	ImGui::SeparatorText("Material tint");

	if (!MaterialTint::IsAvailable())
	{
		ImGui::TextDisabled("Not available on this game build.");
		return;
	}

	if (!pMesh->m_pShaderInfo || pMesh->m_nShaderInfo <= 0 || !pEntity->m_Work.m_pModelShaders)
	{
		ImGui::TextDisabled("%s has no materials.", pMesh->m_szMeshName);
		return;
	}

	ImGui::TextWrapped("A tint multiplies the mesh colour wherever that material is used on this model "
		"(W_Skin also covers the face, for example), so mesh colours and Rainbow Hair still show through. "
		"Intensity above 1 brightens dark materials such as 2B's boots.");

	for (int i = 0; i < pMesh->m_nShaderInfo; ++i)
	{
		const int iShader = pMesh->m_pShaderInfo[i].m_iShader;

		if (iShader < 0 || iShader >= pEntity->m_Work.m_nMaterialShaders)
			continue;

		// The same pointer FindModelEntryData stores in each draw entry
		const CModelShader* pShader = &pEntity->m_Work.m_pModelShaders[iShader];
		const char* szMaterial = (pShader->m_pMaterial && pShader->m_pMaterial->m_szName)
			? pShader->m_pMaterial->m_szName : "(unnamed)";

		ImVec4 vColor(1.0f, 1.0f, 1.0f, 1.0f);
		float flIntensity = 1.0f;
		bool bTinted = MaterialTint::Get(pShader, vColor, flIntensity);

		ImGui::PushID(i);

		if (ImGui::Checkbox(szMaterial, &bTinted))
		{
			if (!bTinted)
				MaterialTint::Clear(pShader);
			else if (!MaterialTint::Set(pShader, vColor, flIntensity))
				g_pConsole->Warn("Material tint: no free slots - clear another material first");
		}

		if (bTinted)
		{
			ImGui::SameLine();
			bool bChanged = ImGui::ColorEdit3("##Colour", (float*)&vColor, ImGuiColorEditFlags_NoInputs);

			ImGui::SameLine();
			ImGui::SetNextItemWidth(ImGui::GetFontSize() * 8.0f);
			bChanged |= ImGui::SliderFloat("##Intensity", &flIntensity, 0.0f, 50.0f, "x%.2f", ImGuiSliderFlags_Logarithmic);
			ImGui::SetItemTooltip("Intensity");

			if (bChanged)
				MaterialTint::Set(pShader, vColor, flIntensity);
		}

		ImGui::PopID();
	}
}

void ApplyModelMods(Pl0000* pEntity)
{
	char szModelPart[64];

	if (!pEntity)
		return;

	if (ImGui::Button("Wet Entity"))
		Features::WetEntity(pEntity, 127U);

	ImGui::SameLine();

	ImGui::CheckboxFlagsT("Firstperson", &Vars.Misc.uCameraFlags,
		(uint32_t)Variables::Misc_t::CameraFlg::CAMERA_FIRSTPERSON);

	ImGui::SameLine();

	ImGui::CheckboxFlagsT("Freecamera", &Vars.Misc.uCameraFlags,
		(uint32_t)Variables::Misc_t::CameraFlg::CAMERA_FREE);

	ImGui::SameLine();

	ImGui::Checkbox("Draw Trace Line", &Vars.Visuals.bTraceLine);

	ImGui::SameLine();

	ImGui::Checkbox("Draw ESP Box", &Vars.Visuals.bEspBox);

	ImGui::SameLine();

	ImGui::Checkbox("Draw 2D Esp Box", &Vars.Visuals.b2DEspBox);

	ImGui::Checkbox("Draw Skeleton", &Vars.Visuals.bSkeleton);

	ImGui::SameLine();

	ImGui::Checkbox("Debug Skeleton", &Vars.Visuals.bDebugLocalPlayerSkeleton);

	ImGui::DragFloat("Trace Line Length", &Vars.Visuals.flTraceLength, 0.00625f, 0.4f, 5.0f);

	ImGui::InputFloat3("Position (X,Y,Z)", (float*)&pEntity->m_vPosition);
	ImGui::InputFloat3("Model Scale (X,Y,Z)", (float*)&pEntity->m_matModelToWorld.GetAxis(1));
	ImGui::InputFloat3("Model Rotation (Pitch, Yaw, Roll)", (float*)&pEntity->m_matModelToWorld.GetAxis(3));

	ImGui::Checkbox("Rainbow Hair", &Vars.Gameplay.bRainbowHair);

	if (pEntity->m_Work.m_nMeshes <= 0)
	{
		ApplyPodMods(pEntity);
		return;
	}

	// The selection outlives the model it was made on - the camera moves between 2B, 9S, A2
	// and anything spawned - so pull it back into this model's range before it indexes
	// m_pMeshes. (The old commented-out "&= m_nMeshes - 1" only worked for power-of-two
	// mesh counts.)
	Vars.Gameplay.iSelectedModelMesh = min(max(Vars.Gameplay.iSelectedModelMesh, 0), pEntity->m_Work.m_nMeshes - 1);

	// "###" keeps the widget ID fixed while the mesh name in the label changes, so a drag
	// isn't dropped after one step; AlwaysClamp stops Ctrl+Click entry going out of range.
	// %.40s: see the pod mesh slider above.
	sprintf_s(szModelPart, "Mesh: %.40s###ModelMesh", pEntity->m_Work.m_pMeshes[Vars.Gameplay.iSelectedModelMesh].m_szMeshName);

	ImGui::SliderInt(szModelPart, &Vars.Gameplay.iSelectedModelMesh, 0, pEntity->m_Work.m_nMeshes - 1, "%d", ImGuiSliderFlags_AlwaysClamp);

	// Taken after the slider, so the rest of this frame edits the mesh the slider now shows
	// rather than the one it showed before it moved.
	CMeshPart* pSelectedMesh = &pEntity->m_Work.m_pMeshes[Vars.Gameplay.iSelectedModelMesh];

	sprintf_s(szModelPart, "Mesh %s Color", pSelectedMesh->m_szMeshName);

	// Shows the mesh's real visibility, which the game changes too. See Features::SetMeshHidden.
	bool bMeshShown = pSelectedMesh->m_bShow != FALSE;

	if (ImGui::Checkbox("Enabled", &bMeshShown))
		Features::SetMeshHidden(&pEntity->m_Work, Vars.Gameplay.iSelectedModelMesh, !bMeshShown);

#if defined(_DEBUG)
	ImGui::InputText("Custom Texture (DDS)", Vars.Gameplay.szModelTextureName, MAX_PATH);
	ImGui::SameLine();

	if (ImGui::Button("Load"))
	{
		CTextureDescription desc;
		CTargetTexture* pTexture = CreateTextureR(Vars.Gameplay.szModelTextureName, desc);

		if (pTexture)
		{
			// The selection indexes pEntity's mesh list. This used g_pLocalPlayer's, which is a
			// different model whenever the camera isn't on the local player.
			CMeshPart* pMesh = pSelectedMesh;

			for (int i = 0; i < pMesh->m_nShaderInfo; ++i)
			{
				CMaterial* pMat = pMesh->m_pShaderInfo[i].m_pShader->m_pMaterial;

				SwapTexture(pMat->m_TextureIds[0], pTexture->m_pTexture);

				/*
				for (int k = 0; k < MAX_MATERIAL_TEXTURES - 1; ++k)
					SwapTexture(pMat->m_TextureIds[k], pTexture->m_pTexture);
				*/
			}
		}
	}
#endif

	ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.3f);

	ImGui::ColorPicker4(szModelPart, (float*)&pSelectedMesh->m_vColor);
	pSelectedMesh->m_bUpdate = TRUE;

	ImGui::SameLine();

	if (!Vars.Gameplay.bRainbowModel)
	{
		ImGui::ColorPicker4("Model Tint Color", (float*)&Vars.Gameplay.vModelTint);
	}

	ImGui::PopItemWidth();

	MaterialTintEditor(pEntity, pSelectedMesh);

	ApplyPodMods(pEntity);
}

// ImGui 1.92 changed the ListBox getter from bool(void*, int, const char**) to
// const char*(void*, int), signalling "no item" with NULL instead of false.
const char* BlacklistItemCallback(void* data, int idx)
{
	return ((std::string*)data)[idx].c_str();
}

/*
this might be needed
	for (int i = 0; i < idx; pEntry = pEntry->m_pNext)
		if (!pEntry->m_pNext)
			return false;
*/
const char* ConfigCallback(void* data, int idx)
{
	PWIN32_FIND_DATA_LIST pEntry = FindDataListNav((PCWIN32_FIND_DATA_LIST)data, idx);

	return (pEntry) ? pEntry->m_Data.cFileName : NULL;
}
