#include "Configuration.h"

#pragma comment(lib, "xmllite.lib")
#pragma comment(lib, "shlwapi.lib")

Variables Vars;

//CConfig* g_pConfig = new CConfig;

// MSXML vs XmlLite

// https://learn.microsoft.com/en-us/previous-versions/windows/desktop/ms752822(v=vs.85)
// https://learn.microsoft.com/en-us/windows/win32/stg/stgm-constants
// https://learn.microsoft.com/en-us/previous-versions/windows/desktop/ms752878(v=vs.85)

// pWriter->SetProperty(XmlWriterProperty_Indent, TRUE)

void CConfigXml::Load(LPCWSTR szFilename)
{
	IStream* pStream;
	IXmlReaderInput* pInput;

	XmlNodeType NodeType;

	SHCreateStreamOnFileEx(szFilename, STGM_READWRITE, FILE_ATTRIBUTE_NORMAL, FALSE, NULL, &pStream);

	//CreateXmlReaderInputWithEncodingName()

	//CreateXmlReaderInputWithEncodingCodePage()

	HRESULT hr = CreateXmlReader(IID_IXmlReader, (void**) & m_pReader, NULL);

	m_pReader->SetProperty(XmlReaderProperty_DtdProcessing, DtdProcessing_Prohibit);

	m_pReader->SetInput(pStream);

	while (!m_pReader->IsEOF())
	{
		m_pReader->Read(&NodeType);



	}


}

void CConfigXml::ReadNode(XmlNodeType Type)
{
	switch (Type)
	{
	case XmlNodeType_XmlDeclaration:
		//pReader->
		break;
	case XmlNodeType_Element:
		break;
	}
}

CConfig::CConfig()
{
	ZeroMemory(m_szFilename, MAX_PATH);

	InitializeConfig();
}

CConfig::~CConfig()
{
	PurgeConfig();
}

bool CConfig::CreateConfig(LPTSTR szFilename)
{
	if (!SetFilename(szFilename))
		return false;

	if (FileExists(m_szFilename))
		Load(szFilename);
	else if (_tcsstr(m_szFilename, CONFIG_DEFAULT_INI))
		Save(CONFIG_DEFAULT_INI);
	else
		return false;

	return true;
}

void CConfig::Load(LPCTSTR szFilename)
{
	if (!SetFilename(szFilename))
		return;

	if (!FileExists(m_szFilename))
		return;

	for (auto& it : m_items)
		it->Read(m_szFilename);
}

void CConfig::Load(LPCWSTR szFilename)
{
	size_t cchConverted = 0;

	char szBuf[MAX_PATH];

	wcstombs_s(&cchConverted, szBuf, szFilename, _TRUNCATE);

	Load(szBuf);
}

void CConfig::Save(LPCTSTR szFilename)
{
	if (!SetFilename(szFilename))
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

	if (!m_keybinds.empty())
	{
		for (auto& it : m_keybinds)
		{
			if (it)
			{
				delete it;
				it = NULL;
			}
		}
		m_keybinds.clear();
	}
}

void CConfig::ResetConfig()
{
	LoadDefault();
}

void CConfig::InitializeConfig()
{
#pragma region visuals
	m_items.emplace_back(new ConfigItemBool(CATEGORY_VISUALS, "t_traceline", Vars.Visuals.bTraceLine));
	m_items.emplace_back(new ConfigItemFloat(CATEGORY_VISUALS, "fl_traceline", Vars.Visuals.flTraceLength));
	m_items.emplace_back(new ConfigItemBool(CATEGORY_VISUALS, "t_chams", Vars.Visuals.bChams));
	m_items.emplace_back(new ConfigItemInt(CATEGORY_VISUALS, "i_cham_type", Vars.Visuals.iChamType));
	m_items.emplace_back(new ConfigItemBool(CATEGORY_VISUALS, "t_espbox", Vars.Visuals.bEspBox));
	m_items.emplace_back(new ConfigItemBool(CATEGORY_VISUALS, "t_2despbox", Vars.Visuals.b2DEspBox));
	m_items.emplace_back(new ConfigItemBool(CATEGORY_VISUALS, "t_enemy_info", Vars.Visuals.bEnemyInfo));
	m_items.emplace_back(new ConfigItemBool(CATEGORY_VISUALS, "t_npc_info", Vars.Visuals.bNPCInfo));
	m_items.emplace_back(new ConfigItemBool(CATEGORY_VISUALS, "t_world_info", Vars.Visuals.bCollisionObjectInfo));
	m_items.emplace_back(new ConfigItemBool(CATEGORY_VISUALS, "t_world_debug_info", Vars.Visuals.bCollisionDebugObjectInfo));
	m_items.emplace_back(new ConfigItemBool(CATEGORY_VISUALS, "t_skeleton", Vars.Visuals.bSkeleton));
	m_items.emplace_back(new ConfigItemBool(CATEGORY_VISUALS, "t_debug_skeleton", Vars.Visuals.bDebugLocalPlayerSkeleton));
	m_items.emplace_back(new ConfigItemFloat(CATEGORY_VISUALS, "fl_fov", Vars.Visuals.flFov));
#pragma endregion

#pragma region gameplay
	m_items.emplace_back(new ConfigItemBool(CATEGORY_GAMEPLAY, "t_freeze_player", Vars.Gameplay.bFreezePlayer));
	m_items.emplace_back(new ConfigItemBool(CATEGORY_GAMEPLAY, "t_godmode", Vars.Gameplay.bGodmode));
	m_items.emplace_back(new ConfigItemBool(CATEGORY_GAMEPLAY, "t_no_enemy_damage", Vars.Gameplay.bNoEnemyDamage));
	m_items.emplace_back(new ConfigItemBool(CATEGORY_GAMEPLAY, "t_no_world_damage", Vars.Gameplay.bNoWorldDamage));
	m_items.emplace_back(new ConfigItemBool(CATEGORY_GAMEPLAY, "t_no_collision", Vars.Gameplay.bNoCollision));
	m_items.emplace_back(new ConfigItemBool(CATEGORY_GAMEPLAY, "t_instant_equip", Vars.Gameplay.bInstantEquip));
	m_items.emplace_back(new ConfigItemBool(CATEGORY_GAMEPLAY, "t_no_tutorial_dialogs", Vars.Gameplay.bNoTutorialDialogs));
	m_items.emplace_back(new ConfigItemBool(CATEGORY_GAMEPLAY, "t_rainbow_hair", Vars.Gameplay.bRainbowHair));
	//m_items.emplace_back(new ConfigItemBool(CATEGORY_GAMEPLAY, "t_rainbow_model", Vars.Gameplay.bRainbowModel)); not impl-
	m_items.emplace_back(new ConfigItemBool(CATEGORY_GAMEPLAY, "t_speedhack", Vars.Gameplay.bSpeedhack));
	m_items.emplace_back(new ConfigItemFloat(CATEGORY_GAMEPLAY, "fl_speed_multiplier", Vars.Gameplay.flSpeedMultiplier));
	//m_items.emplace_back(new ConfigItemBool(CATEGORY_GAMEPLAY, "fl_model_tint_hue", Vars.Gameplay.flModelTintHue)); not impl (rainbow model)
	m_items.emplace_back(new ConfigItemInt(CATEGORY_GAMEPLAY, "i_level", Vars.Gameplay.iLevel));
	m_items.emplace_back(new ConfigItemInt(CATEGORY_GAMEPLAY, "i_enemy_level", Vars.Gameplay.iEnemyLevel));
	m_items.emplace_back(new ConfigItemInt(CATEGORY_GAMEPLAY, "i_enemy_level_tolerance", Vars.Gameplay.iEnemyLevelTolerance));
	m_items.emplace_back(new ConfigItemBool(CATEGORY_GAMEPLAY, "b_balance_enemy_levels", Vars.Gameplay.bBalanceEnemyLevels));
	m_items.emplace_back(new ConfigItemBool(CATEGORY_GAMEPLAY, "b_enemy_level_tolerance_exclusively_positive", Vars.Gameplay.bExclusivelyPositiveTolerance));
	m_items.emplace_back(new ConfigItemBool(CATEGORY_GAMEPLAY, "b_enemy_level_absolute", Vars.Gameplay.bLevelBuffMode));

	//m_items.emplace_back(new ConfigItemVector<std::string>(CATEGORY_GAMEPLAY, "v_blacklist", Vars.Gameplay.SpawnBlacklist));
#pragma endregion

#pragma region keybinds
	
	//AddKeybind(KeybindDynamicToggleable<decltype(&CMenu::IsOpen)>("kb_open_menu", DIK_INSERT, g_pMenu, &CMenu::IsOpen));
	AddKeybind(new KeybindToggleable("kb_firstperson_camera", DIK_F1, &Vars.Misc.bFirstperson));
	AddKeybind(new KeybindFunctional<void>("kb_change_player", DIK_F6, IKeybind::KEYBIND_ON_KEYPRESSED, Features::SwapPlayer));
	AddKeybind(new KeybindFunctional<void>("kb_airstuck", DIK_F7, IKeybind::KEYBIND_ON_KEYDOWN, Features::Airstuck));
	AddKeybind(new KeybindFunctional<void>("kb_duplicate_buddy", DIK_F3, IKeybind::KEYBIND_ON_KEYPRESSED, Features::DuplicateBuddyAsNPC));
	AddKeybind(new KeybindFunctional<void>("kb_play_animation", DIK_F5, IKeybind::KEYBIND_ON_KEYPRESSED, Features::PlayAnimation));
	AddKeybind(new KeybindFunctional<void, eTransformMatrix, float>("kb_teleport_forward", DIK_F10, IKeybind::KEYBIND_ON_KEYDOWN, Features::TeleportScalar, FORWARD, 0.5f));
	AddKeybind(new KeybindFunctional<void, eTransformMatrix, float>("kb_teleport_backward", DIK_F11, IKeybind::KEYBIND_ON_KEYDOWN, Features::TeleportScalar, FORWARD, -0.5f));
	AddKeybind(new KeybindDynamicToggleable<decltype(&Features::GetModelGravity)>("kb_model_gravity", DIK_F9, &Features::GetModelGravity));
	
	AddKeybind(new KeybindDynamicIncremental<float>("kb_model_ycontrol_inc", DIK_UP, IKeybind::KEYBIND_ON_KEYDOWN, Features::GetEntityOBBY, 0.3f));
	AddKeybind(new KeybindDynamicDecremental<float>("kb_model_ycontrol_dec", DIK_DOWN, IKeybind::KEYBIND_ON_KEYDOWN, Features::GetEntityOBBY, 0.3f));

	// FIXME: can't get this dogshit to work
	//AddKeybinds(KeybindDynamicIncrement<float>("kb_model_ycontrol_inc", "kb_model_ycontrol_dec", DIK_UP, DIK_DOWN, IKeybind::KEYBIND_ON_KEYDOWN, Features::GetEntityOBBY, 0.3f));

#pragma endregion

#pragma region misc
	m_items.emplace_back(new ConfigItemBool(CATEGORY_MISC, "t_wireframe", Vars.Misc.bWireframe));
	m_items.emplace_back(new ConfigItemBool(CATEGORY_MISC, "t_anti_framecap", Vars.Misc.bAntiFramerateCap));
	m_items.emplace_back(new ConfigItemBool(CATEGORY_MISC, "t_anti_vsync", Vars.Misc.bAntiVSync));
	m_items.emplace_back(new ConfigItemBool(CATEGORY_MISC, "t_console_show_game_errors", Vars.Misc.bConsoleShowGameErrors));
#pragma endregion

#pragma region menu
	//m_items.emplace_back(new ConfigItemBool(CATEGORY_MENU, "b_ignore_input", Vars.Menu.bIgnoreInputWhenOpened));
	//m_items.emplace_back(new ConfigItemInt(CATEGORY_MENU, "i_theme_fg", g_pMenu->
#pragma endregion
}

void CConfig::LoadDefault()
{
	ZeroMemory(&Vars.Visuals, sizeof(Variables_t::Visuals_t));
	ZeroMemory(&Vars.Misc, sizeof(Variables_t::Misc_t));
	ZeroMemory(&Vars.Gameplay, UFIELD_OFFSET(Variables_t::Gameplay_t, SpawnBlacklist));
	ZeroMemory(&Vars.Menu, UFIELD_OFFSET(Variables_t::Menu_t, bIgnoreInputWhenOpened));

	PurgeConfig();
	InitializeConfig();
}

void CConfig::AddKeybind(CKeybind* pKeybind)
{
	if (!pKeybind)
		return;

	m_items.emplace_back(new ConfigItemKeybind(CATEGORY_KEYBINDS, *pKeybind));
	m_keybinds.emplace_back(pKeybind);
}

BOOL CConfig::EnumerateConfigs(OPTIONAL IN LPCTSTR szDirectory, OUT PWIN32_FIND_DATA_LIST* ppData) const
{
	PWIN32_FIND_DATA_LIST pEntry, pPrev = NULL;
	WIN32_FIND_DATA Find;
	DWORD dwStatus;
	HANDLE hFind;
	TCHAR szSearchDirectory[MAX_PATH];
	BOOLEAN bHeadConstructed = FALSE;

	if (!ppData)
		return ERROR_INVALID_PARAMETER;

	if (szDirectory)
	{
		_tcscpy_s(szSearchDirectory, szDirectory);
	}
	else
	{
		GetModuleFileName(g_hInstance, szSearchDirectory, MAX_PATH);

		if (SanitizePath(TEXT("\\"), szSearchDirectory, MAX_PATH, szSearchDirectory, MAX_PATH))
			return ERROR_INVALID_DATA;
	}

	if (_tcslen(szSearchDirectory) > (MAX_PATH - _ARRAYSIZE(CONFIG_SEARCH_WILDCARD)))
		return ERROR_DIRECTORY;

	//prep the directory string for FindFirstFile
	_tcscat_s(szSearchDirectory, MAX_PATH, CONFIG_SEARCH_WILDCARD);

	hFind = FindFirstFile(szSearchDirectory, &Find);

	if (hFind == INVALID_HANDLE_VALUE)
		return GetLastError();

	do
	{
		if (!bHeadConstructed)
		{
			bHeadConstructed = TRUE;
			pEntry = (PWIN32_FIND_DATA_LIST)LocalAlloc(LPTR, sizeof(WIN32_FIND_DATA_LIST));
			*ppData = pEntry;
		}
		else
		{
			pEntry = (PWIN32_FIND_DATA_LIST)LocalAlloc(LPTR, sizeof(WIN32_FIND_DATA_LIST));
		}

		if (!pEntry)
		{
			FindDataListFree(*ppData);
			FindClose(hFind);
			return GetLastError();
		}

		if (pPrev)
			pPrev->m_pNext = pEntry;

		pEntry->m_pPrevious = pPrev;
		memcpy(&pEntry->m_Data, &Find, sizeof(WIN32_FIND_DATA));
		pPrev = pEntry;
	} while (FindNextFile(hFind, &Find));

	dwStatus = GetLastError();

	FindClose(hFind);

	return (dwStatus == ERROR_NO_MORE_FILES) ? ERROR_SUCCESS : dwStatus;
}

bool CConfig::SetFilename(LPCTSTR szFilename)
{
	TCHAR szDLLFilename[MAX_PATH];
	LPCTSTR	szExtension;

	ZeroMemory(m_szFilename, sizeof(m_szFilename));
	GetModuleFileName(g_hInstance, szDLLFilename, MAX_PATH);

	if (SanitizePath(TEXT("\\"), szDLLFilename, MAX_PATH, m_szFilename, MAX_PATH))
		return false;

	if (szFilename)
	{
		_tcscat_s(m_szFilename, szFilename);

		szExtension = _tcsrchr(szFilename, '.');

		if (szExtension && _tcsicmp(szExtension, CONFIG_EXTENSION))
			_tcscat_s(m_szFilename, CONFIG_EXTENSION);
	}
	else
	{
		_tcscat_s(m_szFilename, CONFIG_DEFAULT_INI);
	}

	return true;
}

BOOL IConfig::FileExists(LPCTSTR szFilename)
{
	return GetFileAttributes(szFilename) != INVALID_FILE_ATTRIBUTES;
}

BOOL IConfig::SanitizePath(IN LPCTSTR szDelimiter, IN LPTSTR szOriginalPath, IN SIZE_T cchOriginalPath,
	OUT LPTSTR szSanitizedPath, IN SIZE_T cchSanitizedPath)
{
	LPTSTR szToken;
	LPTSTR szNextToken = NULL;

	LPTSTR szTemporary = NULL;
	LPTSTR szDirPath = (LPTSTR)LocalAlloc(LPTR, (cchOriginalPath + 1) * sizeof(TCHAR));
	BOOL Status = ERROR_DIRECTORY;

	if (szDirPath)
	{
		_tcscpy_s(szDirPath, cchOriginalPath, szOriginalPath);
		szToken = _tcstok_s(szOriginalPath, szDelimiter, &szNextToken);

		while (szToken)
		{
			szTemporary = szToken;
			szToken = _tcstok_s(NULL, szDelimiter, &szNextToken);
		}

		if (szTemporary)
		{
			szDirPath[szTemporary - szOriginalPath] = (TCHAR)0;

			if (szSanitizedPath)
				_tcscpy_s(szSanitizedPath, cchSanitizedPath, szDirPath);

			Status = ERROR_SUCCESS;
		}
		LocalFree((HLOCAL)szDirPath);
	}
	else
	{
		Status = ERROR_OUTOFMEMORY;
	}

	return Status;
}

/// <summary>
/// 
/// </summary>
/// <param name="szDelimiter"></param>
/// <param name="szOriginalPath"></param>
/// <param name="cchOriginalPath"></param>
/// <param name="pszSanitizedPath"></param>
/// <param name="pcchSanitizedPath">The value in this SIZE_T is added to the sanitized file path length</param>
/// <returns></returns>
BOOL IConfig::SanitizePath(IN LPCTSTR szDelimiter, IN LPTSTR szOriginalPath, IN SIZE_T cchOriginalPath,
	OUT LPTSTR* pszSanitizedPath, IN OUT SIZE_T* pcchSanitizedPath)
{
	LPTSTR szToken;
	LPTSTR szNextToken = NULL;

	BOOL Status = ERROR_DIRECTORY;

	if (pszSanitizedPath && pcchSanitizedPath)
	{
		LPTSTR szTemporary = NULL;
		LPTSTR szDirPath = (LPTSTR)LocalAlloc(LPTR, (cchOriginalPath + 1) * sizeof(TCHAR));

		if (szDirPath)
		{
			_tcscpy_s(szDirPath, cchOriginalPath, szOriginalPath);
			szToken = _tcstok_s(szOriginalPath, szDelimiter, &szNextToken);

			while (szToken)
			{
				szTemporary = szToken;
				szToken = _tcstok_s(NULL, szDelimiter, &szNextToken);
			}

			if (szTemporary)
			{
				szDirPath[szTemporary - szOriginalPath] = (TCHAR)0;
				*pcchSanitizedPath = szTemporary - szOriginalPath + *pcchSanitizedPath + 1;
				*pszSanitizedPath = (LPTSTR)LocalAlloc(LPTR, (*pcchSanitizedPath) * sizeof(TCHAR));

				if (*pszSanitizedPath)
				{
					Status = ERROR_SUCCESS;
					_tcscpy_s(*pszSanitizedPath, *pcchSanitizedPath, szDirPath);
				}
				else
					Status = ERROR_OUTOFMEMORY;
			}
			LocalFree((HLOCAL)szDirPath);
		}
		else
			Status = ERROR_OUTOFMEMORY;
	}
	else
		Status = ERROR_INVALID_PARAMETER;

	return Status;
}

const KeyOrdinal* FindKeyOrdinal(USHORT uKeycode)
{
	int iLeft = 0;
	int iRight = ARRAYSIZE(s_Keycodes) - 1;

	while (iLeft <= iRight)
	{
		int iMiddle = (iLeft + iRight) >> 1;
		const KeyOrdinal* pMiddle = &s_Keycodes[iMiddle];

		if (pMiddle->m_uKeyCode == uKeycode)
			return pMiddle;

		(pMiddle->m_uKeyCode > uKeycode) ? iRight = iMiddle - 1 : iLeft = iMiddle + 1;
	}
	return NULL;
}

VOID FindDataListFree(PCWIN32_FIND_DATA_LIST pList)
{
	PWIN32_FIND_DATA_LIST it = pList;
	PWIN32_FIND_DATA_LIST next;

	while (it)
	{
		next = it->m_pNext;
		LocalFree(it);
		it = next;
	}
}

SIZE_T FindDataListCount(PCWIN32_FIND_DATA_LIST pList)
{
	SIZE_T count = 0;

	for (PWIN32_FIND_DATA_LIST it = pList; it; it = it->m_pNext, ++count);

	return count;
}

PWIN32_FIND_DATA_LIST FindDataListNav(PCWIN32_FIND_DATA_LIST pList, INT iIndex)
{
	PWIN32_FIND_DATA_LIST it = pList;
	INT i = 0;

	for (; i < iIndex && it; ++i)
		it = it->m_pNext;

	return i == iIndex ? it : NULL;
}