#include "Configuration.h"

Variables Vars;
CConfig* g_pConfig = new CConfig(NULL);

CConfig::CConfig(LPTSTR szFilename)
	: m_hFile(INVALID_HANDLE_VALUE)
{
	ZeroMemory(m_szFilename, MAX_PATH);

	CreateConfig(szFilename);
}

CConfig::~CConfig()
{
	if (m_hFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hFile);
	}

	if (!m_items.empty())
	{
		for (auto& it : m_items)
		{
			if (it)
			{
				delete it;
				it = NULL;
			}
		}
	}
}

BOOL CConfig::FileExists(LPTSTR szFilename)
{
	return GetFileAttributes(szFilename) != INVALID_FILE_ATTRIBUTES;
}

bool CConfig::CreateConfig(LPTSTR szFilename)
{
	if (!SetFilename(szFilename))
		return false;

	BOOL bConfigExists = FileExists(m_szFilename);

	m_hFile = CreateFile(m_szFilename, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (m_hFile == INVALID_HANDLE_VALUE)
		return false;

	InitializeConfig();

	if (bConfigExists)
		Load();
	else if (_tcsstr(m_szFilename, CONFIG_DEFAULT_INI))
		Save();

	return true;
}

void CConfig::Load() 
{
	if (m_hFile == INVALID_HANDLE_VALUE)
		return;

	for (auto& it : m_items)
		it->Read(m_szFilename);
}

void CConfig::Save() 
{
	if (m_hFile == INVALID_HANDLE_VALUE)
		return;

	for (auto& it : m_items)
		it->Write(m_szFilename);
}

void CConfig::PurgeConfig()
{
	if (!m_items.empty())
	{
		for (auto& it : m_items)
		{
			if (it)
			{
				delete it;
				it = NULL;
			}
		}
		m_items.clear();
	}
}

void CConfig::ResetConfig()
{
	if (!m_items.empty())
	{
		for (auto& it : m_items)
		{
			if (it)
			{
				delete it;
				it = NULL;
			}
		}
	}

	LoadDefault();
}

//todo make keybinds work
void CConfig::InitializeConfig()
{
	Vars.Keybinds.ChangePlayer = KeybindFunctional<void, void()>("kb_change_player", DIK_F6, IKeybind::KEYBIND_ON_KEYPRESSED, Features::SwapPlayer);
	Vars.Keybinds.Airstuck = KeybindFunctional<void, void()>("kb_airstuck", DIK_F7, IKeybind::KEYBIND_ON_KEYDOWN, Features::Airstuck);
	Vars.Keybinds.DuplicateBuddy = KeybindFunctional<void, void()>("kb_duplicate_buddy", DIK_F3, IKeybind::KEYBIND_ON_KEYPRESSED, Features::DuplicateBuddyAsNPC);
	Vars.Keybinds.TeleportForward = KeybindFunctional<void, void()>("kb_teleport_forward", DIK_F10, IKeybind::KEYBIND_ON_KEYDOWN, Features::TeleportForward);
	Vars.Keybinds.ModelGravity = KeybindDynamicToggleable("kb_model_gravity", DIK_F9, GetModelGravity);
	Vars.Keybinds.ModelYControl = KeybindDynamicIncrement<float>("kb_model_ycontrol_inc", "kb_model_ycontrol_dec", DIK_UP, DIK_DOWN, IKeybind::KEYBIND_ON_KEYPRESSED, GetOBBY, 1.f);

#pragma region visuals
	m_items.emplace_back(new ConfigItemBool(CATEGORY_VISUALS, "t_traceline", Vars.Visuals.bTraceLine));
	m_items.emplace_back(new ConfigItemFloat(CATEGORY_VISUALS, "fl_traceline", Vars.Visuals.flTraceLength));
	m_items.emplace_back(new ConfigItemBool(CATEGORY_VISUALS, "t_2despbox", Vars.Visuals.bEspBox));
	m_items.emplace_back(new ConfigItemBool(CATEGORY_VISUALS, "t_skeleton", Vars.Visuals.bSkeleton));
	m_items.emplace_back(new ConfigItemBool(CATEGORY_VISUALS, "t_debug_skeleton", Vars.Visuals.bDebugLocalPlayerSkeleton));
#pragma endregion

#pragma region gameplay
	m_items.emplace_back(new ConfigItemBool(CATEGORY_GAMEPLAY, "t_freeze_player", Vars.Gameplay.bFreezePlayer));
	m_items.emplace_back(new ConfigItemBool(CATEGORY_GAMEPLAY, "t_ghostmodel", Vars.Gameplay.bGhostModel));
	m_items.emplace_back(new ConfigItemBool(CATEGORY_GAMEPLAY, "t_godmode", Vars.Gameplay.bGodmode));
	m_items.emplace_back(new ConfigItemBool(CATEGORY_GAMEPLAY, "t_no_enemy_damage", Vars.Gameplay.bNoEnemyDamage));
	m_items.emplace_back(new ConfigItemBool(CATEGORY_GAMEPLAY, "t_no_world_damage", Vars.Gameplay.bNoWorldDamage));
	m_items.emplace_back(new ConfigItemBool(CATEGORY_GAMEPLAY, "t_instant_equip", Vars.Gameplay.bInstantEquip));
	m_items.emplace_back(new ConfigItemBool(CATEGORY_GAMEPLAY, "t_no_tutorial_dialogs", Vars.Gameplay.bNoTutorialDialogs));
	m_items.emplace_back(new ConfigItemBool(CATEGORY_GAMEPLAY, "t_rainbow_hair", Vars.Gameplay.bRainbowHair));
	//m_items.emplace_back(new ConfigItemBool(CATEGORY_GAMEPLAY, "t_rainbow_model", Vars.Gameplay.bRainbowModel)); not impl
	m_items.emplace_back(new ConfigItemBool(CATEGORY_GAMEPLAY, "t_speedmeister", Vars.Gameplay.bSpeedMeister));
	m_items.emplace_back(new ConfigItemFloat(CATEGORY_GAMEPLAY, "fl_speed_multiplier", Vars.Gameplay.flSpeedMultiplier));
	m_items.emplace_back(new ConfigItemBool(CATEGORY_GAMEPLAY, "t_temporary_level", Vars.Gameplay.bTemporaryLevel));
	//m_items.emplace_back(new ConfigItemBool(CATEGORY_GAMEPLAY, "fl_model_tint_hue", Vars.Gameplay.flModelTintHue)); not impl (rainbow model)
	m_items.emplace_back(new ConfigItemInt(CATEGORY_GAMEPLAY, "i_level", Vars.Gameplay.iLevel));
#pragma endregion

#pragma region keybinds
	m_items.emplace_back(new ConfigItemKeybind(CATEGORY_KEYBINDS, Vars.Keybinds.Airstuck));
	m_items.emplace_back(new ConfigItemKeybind(CATEGORY_KEYBINDS, Vars.Keybinds.ChangePlayer));
	m_items.emplace_back(new ConfigItemKeybind(CATEGORY_KEYBINDS, Vars.Keybinds.DuplicateBuddy));
	m_items.emplace_back(new ConfigItemKeybind(CATEGORY_KEYBINDS, Vars.Keybinds.TeleportForward));
	//m_items.emplace_back(new ConfigItemKeybind(CATEGORY_KEYBINDS, Vars.Keybinds.PlayAnimation));
	m_items.emplace_back(new ConfigItemKeybind(CATEGORY_KEYBINDS, Vars.Keybinds.ModelGravity));
	m_items.emplace_back(new ConfigItemKeybind(CATEGORY_KEYBINDS, Vars.Keybinds.ModelYControl.m_inc));
	m_items.emplace_back(new ConfigItemKeybind(CATEGORY_KEYBINDS, Vars.Keybinds.ModelYControl.m_dec));

	m_keybinds.emplace_back(&Vars.Keybinds.ChangePlayer);
	m_keybinds.emplace_back(&Vars.Keybinds.Airstuck);
	m_keybinds.emplace_back(&Vars.Keybinds.DuplicateBuddy);
	m_keybinds.emplace_back(&Vars.Keybinds.TeleportForward);
	m_keybinds.emplace_back(&Vars.Keybinds.ModelGravity);
	m_keybinds.emplace_back(&Vars.Keybinds.ModelYControl.m_inc);
	m_keybinds.emplace_back(&Vars.Keybinds.ModelYControl.m_dec);
#pragma endregion

#pragma region misc
	m_items.emplace_back(new ConfigItemBool(CATEGORY_MISC, "t_wireframe", Vars.Misc.bWireframe));
	m_items.emplace_back(new ConfigItemBool(CATEGORY_MISC, "t_anti_framecap", Vars.Misc.bAntiFramerateCap));
	m_items.emplace_back(new ConfigItemBool(CATEGORY_MISC, "t_anti_vsync", Vars.Misc.bAntiVSync));
#pragma endregion
}

void CConfig::LoadDefault()
{
	Vars.Menu.bOpened = false;

	Vars.Keybinds.ChangePlayer = KeybindFunctional<void, void()>("kb_change_player", DIK_F6, IKeybind::KEYBIND_ON_KEYPRESSED, Features::SwapPlayer);
	Vars.Keybinds.Airstuck = KeybindFunctional<void, void()>("kb_airstuck", DIK_F7, IKeybind::KEYBIND_ON_KEYDOWN, Features::Airstuck);
	Vars.Keybinds.DuplicateBuddy = KeybindFunctional<void, void()>("kb_duplicate_buddy", DIK_F3, IKeybind::KEYBIND_ON_KEYPRESSED, Features::DuplicateBuddyAsNPC);
	Vars.Keybinds.TeleportForward = KeybindFunctional<void, void()>("kb_teleport_forward", DIK_F10, IKeybind::KEYBIND_ON_KEYDOWN, Features::TeleportForward);
	Vars.Keybinds.ModelGravity = KeybindDynamicToggleable("kb_model_gravity", DIK_F9, GetModelGravity);
	Vars.Keybinds.ModelYControl = KeybindDynamicIncrement<float>("kb_model_ycontrol_inc", "kb_model_ycontrol_dec", DIK_UP, DIK_DOWN, IKeybind::KEYBIND_ON_KEYPRESSED, GetOBBY, 1.f);

	ZeroMemory(&Vars.Visuals, sizeof(Variables_t::Visuals_t));
	ZeroMemory(&Vars.Misc, sizeof(Variables_t::Misc_t));
	ZeroMemory(&Vars.Gameplay, UFIELD_OFFSET(Variables_t::Gameplay_t, SpawnBlacklist));

	InitializeConfig();
}

bool CConfig::SetFilename(LPTSTR szFilename) 
{
	TCHAR szDLLFilename[MAX_PATH];

	GetModuleFileName(g_hInstance, szDLLFilename, MAX_PATH);

	if (SanitizePath("\\", szDLLFilename, MAX_PATH, m_szFilename, MAX_PATH))
		return false;

	_tcscat_s(m_szFilename, (szFilename) ? szFilename : CONFIG_DEFAULT);
	_tcscat_s(m_szFilename, TEXT(".ini"));

	return true;
}

BOOL CConfig::SanitizePath(IN LPCTSTR szDelimiter, IN LPTSTR szOriginalPath, IN SIZE_T cchOriginalPath, OUT LPTSTR szSanitizedPath, IN SIZE_T cchSanitizedPath)
{
	LPTSTR szToken;
	LPTSTR szNextToken;

	LPTSTR szTemporary = NULL;
	LPTSTR szDirPath = (LPTSTR)LocalAlloc(LPTR, (cchOriginalPath + 1) * sizeof(TCHAR));

	_tcscpy_s(szDirPath, cchOriginalPath, szOriginalPath);
	szToken = _tcstok_s(szOriginalPath, szDelimiter, &szNextToken);

	if (!szToken)
	{
		LocalFree((HLOCAL)szDirPath);
		return ERROR_DIRECTORY;
	}

	while (szToken)
	{
		szTemporary = szToken;
		szToken = _tcstok_s(NULL, szDelimiter, &szNextToken);
	}

	if (!szTemporary)
	{
		LocalFree((HLOCAL)szDirPath);
		return ERROR_DIRECTORY;
	}

	szDirPath[szTemporary - szOriginalPath] = (TCHAR)0;

	if (szSanitizedPath)
		_tcscpy_s(szSanitizedPath, cchSanitizedPath, szDirPath);

	LocalFree((HLOCAL)szDirPath);
	return ERROR_SUCCESS;
}
