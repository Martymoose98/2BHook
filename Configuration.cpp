#include "Configuration.h"

#pragma comment(lib, "xmllite.lib")
#pragma comment(lib, "shlwapi.lib")

Variables Vars;

// MSXML vs XmlLite

// https://learn.microsoft.com/en-us/previous-versions/windows/desktop/ms752822(v=vs.85)
// https://learn.microsoft.com/en-us/windows/win32/stg/stgm-constants
// https://learn.microsoft.com/en-us/previous-versions/windows/desktop/ms752878(v=vs.85)
// https://learn.microsoft.com/en-us/previous-versions//ms752896(v=vs.85)?redirectedfrom=MSDN
// https://github.com/microsoft/Windows-classic-samples/blob/main/Samples/Win7Samples/web/xml/xmllite/xmllitereader/cpp/XmlLiteReader.cpp

// pWriter->SetProperty(XmlWriterProperty_Indent, TRUE)

template<>
void CConfigItemXml<bool&>::SetValue(LPCWSTR szValue, uint32_t cchValue)
{
	m_Value = !_wcsicmp(szValue, L"TRUE");
}

template<>
void CConfigItemXml<int32_t&>::SetValue(LPCWSTR szValue, uint32_t cchValue)
{
	m_Value = _wtoi(szValue); // wcstol
}

template<>
void CConfigItemXml<uint32_t&>::SetValue(LPCWSTR szValue, uint32_t cchValue)
{
	m_Value = wcstoul(szValue, NULL, 16);
}

template<>
void CConfigItemXml<float&>::SetValue(LPCWSTR szValue, uint32_t cchValue)
{
	m_Value = (float)_wtof(szValue);
}

template<>
void CConfigItemXml<ImColor&>::SetValue(LPCWSTR szValue, uint32_t cchValue)
{
	m_Value = wcstoul(szValue, NULL, 16);
}

template<>
void CConfigItemXml<CKeybindToggleable>::SetValue(LPCWSTR szValue, uint32_t cchValue)
{
	m_Value.SetKeycode(wcstol(szValue, NULL, 16));
}

template<>
void CConfigItemXml<CKeybindFunctional<void>>::SetValue(LPCWSTR szValue, uint32_t cchValue)
{
	m_Value.SetKeycode(wcstol(szValue, NULL, 16));
}

template<>
void CConfigItemXml<CKeybindFunctional<void, eTransformMatrix, float>>::SetValue(
	LPCWSTR szValue, uint32_t cchValue)
{
	m_Value.SetKeycode(wcstol(szValue, NULL, 16));
}

template<>
void CConfigItemXml<CKeybindDynamicToggleable<decltype(&Features::GetModelGravity)>>::SetValue(
	LPCWSTR szValue, uint32_t cchValue)
{
	m_Value.SetKeycode(wcstol(szValue, NULL, 16));
}

template<>
void CConfigItemXml<CKeybindDynamicIncremental<float>>::SetValue(LPCWSTR szValue, uint32_t cchValue)
{
	m_Value.SetKeycode(wcstol(szValue, NULL, 16));
}

template<>
void CConfigItemXml<CKeybindDynamicDecremental<float>>::SetValue(LPCWSTR szValue, uint32_t cchValue)
{
	m_Value.SetKeycode(wcstol(szValue, NULL, 16));
}

template<>
HRESULT CConfigItemXml<bool&>::Write(IXmlWriter* pWriter)
{
	HRESULT hr = S_OK;

	hr = pWriter->WriteElementString(NULL, m_szName, NULL, (m_Value) ? L"TRUE" : L"FALSE");
	// BUG: IDK where I was told to write this but this fucks with indentation
	//hr = pWriter->WriteWhitespace(L"\r\n");

	return hr;
}

template<>
HRESULT CConfigItemXml<int32_t&>::Write(IXmlWriter* pWriter)
{
	HRESULT hr = S_OK;
	wchar_t szBuffer[33] = { 0 };

	//swprintf_s(szBuffer, L"%i", m_Value);
	_itow_s(m_Value, szBuffer, 16);
	
	hr = pWriter->WriteElementString(NULL, m_szName, NULL, szBuffer);
	// BUG: IDK where I was told to write this but this fucks with indentation
	//hr = pWriter->WriteWhitespace(L"\r\n");

	return hr;
}

template<>
HRESULT CConfigItemXml<uint32_t&>::Write(IXmlWriter* pWriter)
{
	HRESULT hr = S_OK;
	wchar_t szBuffer[33] = { 0 };

	_ultow_s(m_Value, szBuffer, 16);
	
	hr = pWriter->WriteElementString(NULL, m_szName, NULL, szBuffer);
	// BUG: IDK where I was told to write this but this fucks with indentation
	//hr = pWriter->WriteWhitespace(L"\r\n");

	return hr;
}

template<>
HRESULT CConfigItemXml<float&>::Write(IXmlWriter* pWriter)
{
	HRESULT hr = S_OK;
	wchar_t szBuffer[33] = { 0 };

	// _scwprintf(L"%f", m_Value)
	swprintf_s(szBuffer, L"%f", m_Value);

	hr = pWriter->WriteElementString(NULL, m_szName, NULL, szBuffer);

	return hr;
}

HRESULT WriteKeybind(IXmlWriter* pWriter, const CKeybind& Keybind)
{
	HRESULT hr = S_OK;
	wchar_t szBuffer[33] = { 0 };

	_itow_s(Keybind.GetKeycode(), szBuffer, 16);

	hr = pWriter->WriteElementString(NULL, Keybind.GetName(), NULL, szBuffer);

	return hr;
}

//template<>
//HRESULT CConfigItemXml<CKeybind>::Write(IXmlWriter* pWriter)
//{
//	return WriteKeybind(pWriter, m_Value);
//}

// T::CKeybind
template<>
HRESULT CConfigItemXml<CKeybindToggleable>::Write(IXmlWriter* pWriter)
{
	return WriteKeybind(pWriter, m_Value);
}

template<>
HRESULT CConfigItemXml<CKeybindFunctional<void>>::Write(IXmlWriter* pWriter)
{
	return WriteKeybind(pWriter, m_Value);
}

template<>
HRESULT CConfigItemXml<CKeybindFunctional<void, eTransformMatrix, float>>::Write(IXmlWriter* pWriter)
{
	return WriteKeybind(pWriter, m_Value);
}

template<>
HRESULT CConfigItemXml<CKeybindDynamicToggleable<decltype(&Features::GetModelGravity)>>::Write(IXmlWriter* pWriter)
{
	return WriteKeybind(pWriter, m_Value);
}

template<>
HRESULT CConfigItemXml<CKeybindDynamicIncremental<float>>::Write(IXmlWriter* pWriter)
{
	return WriteKeybind(pWriter, m_Value);
}

template<>
HRESULT CConfigItemXml<CKeybindDynamicDecremental<float>>::Write(IXmlWriter* pWriter)
{
	return WriteKeybind(pWriter, m_Value);
}

CConfigXml::CConfigXml(void)
{
	ZeroMemory(m_szFilename, MAX_PATH);

	InitializeConfig();
}

CConfigXml::~CConfigXml(void)
{
	PurgeConfig();
}

bool CConfigXml::CreateConfig(LPCWSTR szFilename)
{
	if (!SetFilename(szFilename))
		return false;

	if (FileExists(m_szFilename))
		Load(szFilename);
	else if (wcsstr(m_szFilename, _CRT_WIDE(CONFIG_DEFAULT_NAME)))
		Save(szFilename);
	else
		return false;

	return true;
}

void CConfigXml::PurgeConfig(void)
{
	if (!m_Items.empty())
	{
		for (auto& it : m_Items)
		{
			if (it.second)
			{
				delete it.second;
				it.second = NULL;
			}
		}
		m_Items.clear();
	}
}

void CConfigXml::ResetConfig(void)
{
}

HRESULT CConfigXml::Load(LPCWSTR szFilename)
{
	HRESULT hr = S_OK;

	ComPtr<IStream> pStream;

	XmlNodeType NodeType;
	uint32_t nAttrCount;

	if (!SetFilename(szFilename))
		return E_FAIL;

	// https://learn.microsoft.com/en-us/windows/win32/api/shlwapi/nf-shlwapi-shcreatestreamonfileex
	if (FAILED(hr = SHCreateStreamOnFileEx(m_szFilename, STGM_READWRITE, 
		FILE_ATTRIBUTE_NORMAL, FALSE, NULL, &pStream)))
		return hr;

	//CreateXmlReaderInputWithEncodingName()

	//CreateXmlReaderInputWithEncodingCodePage()

	if (FAILED(hr = CreateXmlReader(IID_IXmlReader, (void**)&m_pReader, NULL)))
		return hr;

	if (FAILED(hr = m_pReader->SetProperty(XmlReaderProperty_DtdProcessing, DtdProcessing_Prohibit)))
		return hr;

	if (FAILED(hr = m_pReader->SetInput(pStream.Get())))
		return hr;

	while (!m_pReader->IsEOF())
	{
		hr = m_pReader->Read(&NodeType);

		hr = ReadNode(NodeType);
	}

	return hr;
}

HRESULT CConfigXml::Save(LPCWSTR szFilename)
{
	HRESULT hr = S_OK;

	ComPtr<IStream> pStream;

	if (!SetFilename(szFilename))
		return E_FAIL;

	if (FAILED(hr = SHCreateStreamOnFileEx(m_szFilename, STGM_CREATE | STGM_READWRITE,
		FILE_ATTRIBUTE_NORMAL, TRUE, NULL, &pStream)))
		return hr;

	if (FAILED(hr = CreateXmlWriter(IID_IXmlWriter, (void**)&m_pWriter, NULL)))
		return hr;

	if (FAILED(hr = m_pWriter->SetOutput(pStream.Get())))
		return hr;

	if (FAILED(hr = m_pWriter->SetProperty(XmlWriterProperty_Indent, TRUE)))
		return hr;

	if (FAILED(hr = m_pWriter->WriteStartDocument(XmlStandalone_Omit)))
		return hr;

	{
		CConfigItemXmlNamespace RootNamespace(m_pWriter, L"Root", m_Items.size());

		for (auto& It = m_Items.begin(); It != m_Items.end();)
		{
			uint32_t uChildren = m_Items.count(It->first);
			CConfigItemXmlNamespace Namespace(m_pWriter, It->first.c_str(), uChildren);

			auto Sector = m_Items.equal_range(It->first);

			for (auto i = Sector.first; i != Sector.second; ++i)
			{
 				i->second->Write(m_pWriter.Get());
			}

			std::advance(It, uChildren);
		}
	}

	if (FAILED(hr = m_pWriter->WriteEndDocument()))
		return hr;

	hr = m_pWriter->Flush();

	return hr;
}

HRESULT CConfigXml::ReadNode(XmlNodeType Type)
{
	HRESULT hr = S_OK;
	LPCWSTR szComment;
	uint32_t chSize;

	switch (Type)
	{
	case XmlNodeType_XmlDeclaration:
		//m_pReader->
		break;
	case XmlNodeType_DocumentType:
		break;
	case XmlNodeType_Comment:
		hr = m_pReader->GetValue(&szComment, &chSize);
		break;
	case XmlNodeType_Text:
	case XmlNodeType_Whitespace:

		break;
	case XmlNodeType_Element:
		hr = BeginReadElement();
		break;
	case XmlNodeType_EndElement:
		hr = EndReadElement();
		break;
	case XmlNodeType_CDATA:
		break;
	case XmlNodeType_ProcessingInstruction:
		break;

	}
	return hr;
}

HRESULT CConfigXml::WriteNode(XmlNodeType Type)
{
	HRESULT hr = S_OK;

	return E_NOTIMPL;
}

HRESULT CConfigXml::BeginReadElement(void)
{
	HRESULT hr = S_OK;

	XmlNodeType NodeType;

	LPCWSTR szPrefix;
	uint32_t cchPrefix;

	LPCWSTR szLocalName;
	uint32_t cchLocalName;	// this is optional

	uint32_t nAttributes;

	if (m_pReader->IsEmptyElement())
		return XmlError::WC_E_NDATA;

	hr = m_pReader->GetPrefix(&szPrefix, &cchPrefix);

	if (FAILED(hr = m_pReader->GetLocalName(&szLocalName, &cchLocalName)))
		return hr;

	auto& It = m_Items.find(szLocalName);

	if (It != m_Items.end())
	{
		uint32_t uChildren = m_Items.count(It->first);
		CConfigItemXmlNamespace Namespace(m_pReader, It->first.c_str(), uChildren);

		auto Sector = m_Items.equal_range(It->first);

		for (auto i = Sector.first; i != Sector.second; ++i)
		{
			// Read Next Node
			hr = m_pReader->Read(&NodeType);

			hr = i->second->Read(m_pReader.Get());

			// Read Whitespace Node
			hr = m_pReader->Read(&NodeType);

			// Read End Element Node
			hr = m_pReader->Read(&NodeType);

			// Read Next Node
			ReadNode(NodeType);
		}
	}

	hr = m_pReader->GetAttributeCount(&nAttributes);

	return hr;
}

HRESULT CConfigXml::EndReadElement(void)
{
	return E_NOTIMPL;
}

void CConfigXml::InitializeConfig(void)
{
#pragma region visuals
	m_Items.emplace(IConfigItemXml::MakePair(CATEGORY_VISUALS, L"t_traceline", Vars.Visuals.bTraceLine));
	m_Items.emplace(IConfigItemXml::MakePair(CATEGORY_VISUALS, L"fl_traceline", Vars.Visuals.flTraceLength));
	m_Items.emplace(IConfigItemXml::MakePair(CATEGORY_VISUALS, L"t_chams", Vars.Visuals.bChams));
	m_Items.emplace(IConfigItemXml::MakePair(CATEGORY_VISUALS, L"i_cham_type", Vars.Visuals.iChamType));
	m_Items.emplace(IConfigItemXml::MakePair(CATEGORY_VISUALS, L"t_espbox", Vars.Visuals.bEspBox));
	m_Items.emplace(IConfigItemXml::MakePair(CATEGORY_VISUALS, L"t_2despbox", Vars.Visuals.b2DEspBox));
	m_Items.emplace(IConfigItemXml::MakePair(CATEGORY_VISUALS, L"t_enemy_info", Vars.Visuals.bEnemyInfo));
	m_Items.emplace(IConfigItemXml::MakePair(CATEGORY_VISUALS, L"t_npc_info", Vars.Visuals.bNPCInfo));
	m_Items.emplace(IConfigItemXml::MakePair(CATEGORY_VISUALS, L"t_world_info", Vars.Visuals.bCollisionObjectInfo));
	m_Items.emplace(IConfigItemXml::MakePair(CATEGORY_VISUALS, L"t_world_debug_info", Vars.Visuals.bCollisionDebugObjectInfo));
	m_Items.emplace(IConfigItemXml::MakePair(CATEGORY_VISUALS, L"t_skeleton", Vars.Visuals.bSkeleton));
	m_Items.emplace(IConfigItemXml::MakePair(CATEGORY_VISUALS, L"t_debug_skeleton", Vars.Visuals.bDebugLocalPlayerSkeleton));
	m_Items.emplace(IConfigItemXml::MakePair(CATEGORY_VISUALS, L"fl_fov", Vars.Visuals.flFov));
#pragma endregion visuals

#pragma region gameplay
	m_Items.emplace(IConfigItemXml::MakePair(CATEGORY_GAMEPLAY, L"t_freeze_player", Vars.Gameplay.bFreezePlayer));
	m_Items.emplace(IConfigItemXml::MakePair(CATEGORY_GAMEPLAY, L"t_godmode", Vars.Gameplay.bGodmode));
	m_Items.emplace(IConfigItemXml::MakePair(CATEGORY_GAMEPLAY, L"t_no_enemy_damage", Vars.Gameplay.bNoEnemyDamage));
	m_Items.emplace(IConfigItemXml::MakePair(CATEGORY_GAMEPLAY, L"t_no_world_damage", Vars.Gameplay.bNoWorldDamage));
	m_Items.emplace(IConfigItemXml::MakePair(CATEGORY_GAMEPLAY, L"t_no_collision", Vars.Gameplay.bNoCollision));
	m_Items.emplace(IConfigItemXml::MakePair(CATEGORY_GAMEPLAY, L"t_instant_equip", Vars.Gameplay.bInstantEquip));
	m_Items.emplace(IConfigItemXml::MakePair(CATEGORY_GAMEPLAY, L"t_no_tutorial_dialogs", Vars.Gameplay.bNoTutorialDialogs));
	m_Items.emplace(IConfigItemXml::MakePair(CATEGORY_GAMEPLAY, L"t_rainbow_hair", Vars.Gameplay.bRainbowHair));
	//m_Items.emplace(IConfigItemXml::MakePair(CATEGORY_GAMEPLAY, L"t_rainbow_model", Vars.Gameplay.bRainbowModel)); not impl-
	m_Items.emplace(IConfigItemXml::MakePair(CATEGORY_GAMEPLAY, L"t_speedhack", Vars.Gameplay.bSpeedhack));
	m_Items.emplace(IConfigItemXml::MakePair(CATEGORY_GAMEPLAY, L"fl_speed_multiplier", Vars.Gameplay.flSpeedMultiplier));
	//m_Items.emplace(IConfigItemXml::MakePair(XCATEGORY_GAMEPLAY, L"fl_model_tint_hue", Vars.Gameplay.flModelTintHue)); not impl (rainbow model)
	m_Items.emplace(IConfigItemXml::MakePair(CATEGORY_GAMEPLAY, L"i_level", Vars.Gameplay.iLevel));
	m_Items.emplace(IConfigItemXml::MakePair(CATEGORY_GAMEPLAY, L"i_enemy_level", Vars.Gameplay.iEnemyLevel));
	m_Items.emplace(IConfigItemXml::MakePair(CATEGORY_GAMEPLAY, L"i_enemy_level_tolerance", Vars.Gameplay.iEnemyLevelTolerance));
	m_Items.emplace(IConfigItemXml::MakePair(CATEGORY_GAMEPLAY, L"b_balance_enemy_levels", Vars.Gameplay.bBalanceEnemyLevels));
	m_Items.emplace(IConfigItemXml::MakePair(CATEGORY_GAMEPLAY, L"b_enemy_level_tolerance_exclusively_positive", Vars.Gameplay.bExclusivelyPositiveTolerance));
	m_Items.emplace(IConfigItemXml::MakePair(CATEGORY_GAMEPLAY, L"b_enemy_level_absolute", Vars.Gameplay.bLevelBuffMode));
	//m_Items.emplace(new ConfigItemVector<std::string>(CATEGORY_GAMEPLAY, L"v_blacklist", Vars.Gameplay.SpawnBlacklist));
#pragma endregion gameplay

#pragma region keybinds
	
	// TODO: FINISH AND TEST THIS!!! SO IT CAN BE PORTED TO OTHER PROJS
	//m_Items.emplace(IConfigItemXml::MakePair<CKeybind*>(
	//	XCATEGORY_KEYBINDS,  L"kb_firstperson_camera",
	//	m_Keybinds.emplace_back(CKeybindToggleable("kb_firstperson_camera", DIK_F1, Vars.Misc.bFirstperson))));

	//AddKeybind(CKeybindToggleable("kb_firstperson_camera", DIK_F1, Vars.Misc.bFirstperson));
	//AddKeybind(CKeybindFunctional<void>("kb_change_player", DIK_F6, IKeybind::KEYBIND_ON_KEYPRESSED, Features::SwapPlayer));
	//AddKeybind(CKeybindFunctional<void>("kb_airstuck", DIK_F7, IKeybind::KEYBIND_ON_KEYPRESSED, Features::Airstuck));

	m_Items.emplace(IConfigItemXml::MakePair(CATEGORY_KEYBINDS, L"kb_firstperson_camera",
		CKeybindToggleable(L"kb_firstperson_camera", DIK_F1, &Vars.Misc.bFirstperson)));

	m_Items.emplace(IConfigItemXml::MakePair(CATEGORY_KEYBINDS, L"kb_change_player",
		CKeybindFunctional<void>(L"kb_change_player", DIK_F6, IKeybind::KEYBIND_ON_KEYPRESSED, Features::SwapPlayer)));

	m_Items.emplace(IConfigItemXml::MakePair(CATEGORY_KEYBINDS, L"kb_airstuck",
		CKeybindFunctional<void>(L"kb_airstuck", DIK_F7, IKeybind::KEYBIND_ON_KEYPRESSED, Features::Airstuck)));

	m_Items.emplace(IConfigItemXml::MakePair(CATEGORY_KEYBINDS, L"kb_duplicate_buddy",
		CKeybindFunctional<void>(L"kb_duplicate_buddy", DIK_F3, IKeybind::KEYBIND_ON_KEYPRESSED, Features::DuplicateBuddyAsNPC)));

	m_Items.emplace(IConfigItemXml::MakePair(CATEGORY_KEYBINDS, L"kb_play_animation",
		CKeybindFunctional<void>(L"kb_play_animation", DIK_F5, IKeybind::KEYBIND_ON_KEYPRESSED, Features::PlayAnimation)));

	m_Items.emplace(IConfigItemXml::MakePair(CATEGORY_KEYBINDS, L"kb_teleport_forward",
		CKeybindFunctional<void, eTransformMatrix, float>(L"kb_teleport_forward", DIK_F10,
			IKeybind::KEYBIND_ON_KEYDOWN, Features::TeleportScalar, FORWARD, 0.5f)));

	m_Items.emplace(IConfigItemXml::MakePair(CATEGORY_KEYBINDS, L"kb_teleport_backward",
		CKeybindFunctional<void, eTransformMatrix, float>(L"kb_teleport_backward", DIK_F11,
			IKeybind::KEYBIND_ON_KEYDOWN, Features::TeleportScalar, FORWARD, -0.5f)));

	m_Items.emplace(IConfigItemXml::MakePair(CATEGORY_KEYBINDS, L"kb_model_gravity",
		CKeybindDynamicToggleable<decltype(&Features::GetModelGravity)>(L"kb_model_gravity", DIK_F9,
			&Features::GetModelGravity)));

	m_Items.emplace(IConfigItemXml::MakePair(CATEGORY_KEYBINDS, L"kb_model_ycontrol_inc",
		CKeybindDynamicIncremental<float>(L"kb_model_ycontrol_inc", DIK_UP,
			IKeybind::KEYBIND_ON_KEYDOWN, Features::GetEntityOBBY, 0.3f)));

	m_Items.emplace(IConfigItemXml::MakePair(CATEGORY_KEYBINDS, L"kb_model_ycontrol_dec",
		CKeybindDynamicDecremental<float>(L"kb_model_ycontrol_dec", DIK_DOWN,
			IKeybind::KEYBIND_ON_KEYDOWN, Features::GetEntityOBBY, 0.3f)));

#pragma endregion

#pragma region misc
	m_Items.emplace(IConfigItemXml::MakePair(CATEGORY_MISC, L"t_wireframe", Vars.Misc.bWireframe));
	m_Items.emplace(IConfigItemXml::MakePair(CATEGORY_MISC, L"t_anti_framecap", Vars.Misc.bAntiFramerateCap));
	m_Items.emplace(IConfigItemXml::MakePair(CATEGORY_MISC, L"t_anti_vsync", Vars.Misc.bAntiVSync));
	m_Items.emplace(IConfigItemXml::MakePair(CATEGORY_MISC, L"t_console_show_game_errors", Vars.Misc.bConsoleShowGameErrors));
#pragma endregion misc

}

std::pair<
	std::multimap<std::wstring, IConfigItemXml*>::const_iterator,
	std::multimap<std::wstring, IConfigItemXml*>::const_iterator
> CConfigXml::GetKeybinds(void) const
{
	return m_Items.equal_range(CATEGORY_KEYBINDS);
}

bool CConfigXml::SetFilename(LPCWSTR szFilename)
{
	WCHAR szDLLFilename[MAX_PATH];
	LPCWSTR	szExtension;
	SIZE_T uPathSize;

	ZeroMemory(m_szFilename, sizeof(m_szFilename));
	GetModuleFileNameW(g_hInstance, szDLLFilename, MAX_PATH);

	if (SanitizePath(L"\\", szDLLFilename, MAX_PATH, m_szFilename, MAX_PATH))
		return false;

	if (szFilename)
	{
		wcscat_s(m_szFilename, szFilename);

		szExtension = wcsrchr(szFilename, '.');

		if (szExtension && _wcsicmp(szExtension, _CRT_WIDE(CONFIG_EXTENSION)))
			wcscat_s(m_szFilename, _CRT_WIDE(CONFIG_EXTENSION));
	}
	else
	{
		wcscat_s(m_szFilename, _CRT_WIDE(CONFIG_DEFAULT_NAME));
	}

	return true;
}

// TODO: make this more generic 
BOOL CConfigXml::EnumerateConfigs(OPTIONAL IN LPCTSTR szDirectory, OUT PWIN32_FIND_DATA_LIST* ppData) const
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
	{
		*ppData = NULL;
		return GetLastError();
	}	

	do
	{
		if (!bHeadConstructed)
		{
			bHeadConstructed = TRUE;
			pEntry = (PWIN32_FIND_DATA_LIST)malloc(sizeof(WIN32_FIND_DATA_LIST));
			*ppData = pEntry;
		}
		else
		{
			pEntry = (PWIN32_FIND_DATA_LIST)malloc(sizeof(WIN32_FIND_DATA_LIST));
		}

		if (!pEntry)
		{
			FindDataListFree(*ppData);
			FindClose(hFind);
			return GetLastError();
		}

		if (pPrev)
			pPrev->m_pNext = pEntry;

		pEntry->m_pNext = NULL;
		pEntry->m_pPrevious = pPrev;
		memcpy(&pEntry->m_Data, &Find, sizeof(WIN32_FIND_DATA));
		pPrev = pEntry;
	} while (FindNextFile(hFind, &Find));

	dwStatus = GetLastError();

	FindClose(hFind);

	return (dwStatus == ERROR_NO_MORE_FILES) ? ERROR_SUCCESS : dwStatus;
}

CConfig::CConfig(void)
{
	ZeroMemory(m_szFilename, MAX_PATH);

	InitializeConfig();
}

CConfig::~CConfig(void)
{
	PurgeConfig();
}

bool CConfig::CreateConfig(LPCWSTR szFilename)
{
	//if (!SetFilename(szFilename))
	//	return false;

	if (FileExists(m_szFilename))
		Load(szFilename);
	else if (_tcsstr(m_szFilename, CONFIG_DEFAULT_NAME))
		Save(CONFIG_DEFAULT_NAME);
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

	for (auto& it : m_Items)
		it->Read(m_szFilename);
}

HRESULT CConfig::Load(LPCWSTR szFilename)
{
	size_t cchConverted = 0;

	char szBuf[MAX_PATH];

	wcstombs_s(&cchConverted, szBuf, szFilename, _TRUNCATE);

	Load(szBuf);

	return S_OK;
}

HRESULT CConfig::Save(LPCWSTR szFilename)
{
	size_t cchConverted = 0;

	char szBuf[MAX_PATH];

	wcstombs_s(&cchConverted, szBuf, szFilename, _TRUNCATE);

	Save(szBuf);

	return S_OK;
}

void CConfig::Save(LPCTSTR szFilename)
{
	if (!SetFilename(szFilename))
		return;

	for (auto& it : m_Items)
		it->Write(m_szFilename);
}

void CConfig::PurgeConfig()
{
	if (!m_Items.empty())
	{
		for (auto& it : m_Items)
		{
			if (it)
			{
				delete it;
				it = NULL;
			}
		}
		m_Items.clear();
	}

	if (!m_Keybinds.empty())
	{
		for (auto& it : m_Keybinds)
		{
			if (it)
			{
				delete it;
				it = NULL;
			}
		}
		m_Keybinds.clear();
	}
}

void CConfig::ResetConfig()
{
	LoadDefault();
}

void CConfig::InitializeConfig()
{
#ifndef CONFIG_XML

#pragma region visuals
	m_Items.emplace_back(new ConfigItemBool(CATEGORY_VISUALS, "t_traceline", Vars.Visuals.bTraceLine));
	m_Items.emplace_back(new ConfigItemFloat(CATEGORY_VISUALS, "fl_traceline", Vars.Visuals.flTraceLength));
	m_Items.emplace_back(new ConfigItemBool(CATEGORY_VISUALS, "t_chams", Vars.Visuals.bChams));
	m_Items.emplace_back(new ConfigItemInt(CATEGORY_VISUALS, "i_cham_type", Vars.Visuals.iChamType));
	m_Items.emplace_back(new ConfigItemBool(CATEGORY_VISUALS, "t_espbox", Vars.Visuals.bEspBox));
	m_Items.emplace_back(new ConfigItemBool(CATEGORY_VISUALS, "t_2despbox", Vars.Visuals.b2DEspBox));
	m_Items.emplace_back(new ConfigItemBool(CATEGORY_VISUALS, "t_enemy_info", Vars.Visuals.bEnemyInfo));
	m_Items.emplace_back(new ConfigItemBool(CATEGORY_VISUALS, "t_npc_info", Vars.Visuals.bNPCInfo));
	m_Items.emplace_back(new ConfigItemBool(CATEGORY_VISUALS, "t_world_info", Vars.Visuals.bCollisionObjectInfo));
	m_Items.emplace_back(new ConfigItemBool(CATEGORY_VISUALS, "t_world_debug_info", Vars.Visuals.bCollisionDebugObjectInfo));
	m_Items.emplace_back(new ConfigItemBool(CATEGORY_VISUALS, "t_skeleton", Vars.Visuals.bSkeleton));
	m_Items.emplace_back(new ConfigItemBool(CATEGORY_VISUALS, "t_debug_skeleton", Vars.Visuals.bDebugLocalPlayerSkeleton));
	m_Items.emplace_back(new ConfigItemFloat(CATEGORY_VISUALS, "fl_fov", Vars.Visuals.flFov));
#pragma endregion

#pragma region gameplay
	m_Items.emplace_back(new ConfigItemBool(CATEGORY_GAMEPLAY, "t_freeze_player", Vars.Gameplay.bFreezePlayer));
	m_Items.emplace_back(new ConfigItemBool(CATEGORY_GAMEPLAY, "t_godmode", Vars.Gameplay.bGodmode));
	m_Items.emplace_back(new ConfigItemBool(CATEGORY_GAMEPLAY, "t_no_enemy_damage", Vars.Gameplay.bNoEnemyDamage));
	m_Items.emplace_back(new ConfigItemBool(CATEGORY_GAMEPLAY, "t_no_world_damage", Vars.Gameplay.bNoWorldDamage));
	m_Items.emplace_back(new ConfigItemBool(CATEGORY_GAMEPLAY, "t_no_collision", Vars.Gameplay.bNoCollision));
	m_Items.emplace_back(new ConfigItemBool(CATEGORY_GAMEPLAY, "t_instant_equip", Vars.Gameplay.bInstantEquip));
	m_Items.emplace_back(new ConfigItemBool(CATEGORY_GAMEPLAY, "t_no_tutorial_dialogs", Vars.Gameplay.bNoTutorialDialogs));
	m_Items.emplace_back(new ConfigItemBool(CATEGORY_GAMEPLAY, "t_rainbow_hair", Vars.Gameplay.bRainbowHair));
	//m_items.emplace_back(new ConfigItemBool(CATEGORY_GAMEPLAY, "t_rainbow_model", Vars.Gameplay.bRainbowModel)); not impl-
	m_Items.emplace_back(new ConfigItemBool(CATEGORY_GAMEPLAY, "t_speedhack", Vars.Gameplay.bSpeedhack));
	m_Items.emplace_back(new ConfigItemFloat(CATEGORY_GAMEPLAY, "fl_speed_multiplier", Vars.Gameplay.flSpeedMultiplier));
	//m_items.emplace_back(new ConfigItemBool(CATEGORY_GAMEPLAY, "fl_model_tint_hue", Vars.Gameplay.flModelTintHue)); not impl (rainbow model)
	m_Items.emplace_back(new ConfigItemInt(CATEGORY_GAMEPLAY, "i_level", Vars.Gameplay.iLevel));
	m_Items.emplace_back(new ConfigItemInt(CATEGORY_GAMEPLAY, "i_enemy_level", Vars.Gameplay.iEnemyLevel));
	m_Items.emplace_back(new ConfigItemInt(CATEGORY_GAMEPLAY, "i_enemy_level_tolerance", Vars.Gameplay.iEnemyLevelTolerance));
	m_Items.emplace_back(new ConfigItemBool(CATEGORY_GAMEPLAY, "b_balance_enemy_levels", Vars.Gameplay.bBalanceEnemyLevels));
	m_Items.emplace_back(new ConfigItemBool(CATEGORY_GAMEPLAY, "b_enemy_level_tolerance_exclusively_positive", Vars.Gameplay.bExclusivelyPositiveTolerance));
	m_Items.emplace_back(new ConfigItemBool(CATEGORY_GAMEPLAY, "b_enemy_level_absolute", Vars.Gameplay.bLevelBuffMode));

	//m_items.emplace_back(new ConfigItemVector<std::string>(CATEGORY_GAMEPLAY, "v_blacklist", Vars.Gameplay.SpawnBlacklist));
#pragma endregion

#pragma region keybinds

	//AddKeybind(KeybindDynamicToggleable<decltype(&CMenu::IsOpen)>("kb_open_menu", DIK_INSERT, g_pMenu, &CMenu::IsOpen));
	AddKeybind(new CKeybindToggleable("kb_firstperson_camera", DIK_F1, &Vars.Misc.bFirstperson));
	AddKeybind(new CKeybindFunctional<void>("kb_change_player", DIK_F6, IKeybind::KEYBIND_ON_KEYPRESSED, Features::SwapPlayer));
	AddKeybind(new CKeybindFunctional<void>("kb_airstuck", DIK_F7, IKeybind::KEYBIND_ON_KEYDOWN, Features::Airstuck));
	AddKeybind(new CKeybindFunctional<void>("kb_duplicate_buddy", DIK_F3, IKeybind::KEYBIND_ON_KEYPRESSED, Features::DuplicateBuddyAsNPC));
	AddKeybind(new CKeybindFunctional<void>("kb_play_animation", DIK_F5, IKeybind::KEYBIND_ON_KEYPRESSED, Features::PlayAnimation));
	AddKeybind(new CKeybindFunctional<void, eTransformMatrix, float>("kb_teleport_forward", DIK_F10, IKeybind::KEYBIND_ON_KEYDOWN, Features::TeleportScalar, FORWARD, 0.5f));
	AddKeybind(new CKeybindFunctional<void, eTransformMatrix, float>("kb_teleport_backward", DIK_F11, IKeybind::KEYBIND_ON_KEYDOWN, Features::TeleportScalar, FORWARD, -0.5f));
	AddKeybind(new CKeybindDynamicToggleable<decltype(&Features::GetModelGravity)>("kb_model_gravity", DIK_F9, &Features::GetModelGravity));

	AddKeybind(new CKeybindDynamicIncremental<float>("kb_model_ycontrol_inc", DIK_UP, IKeybind::KEYBIND_ON_KEYDOWN, Features::GetEntityOBBY, 0.3f));
	AddKeybind(new CKeybindDynamicDecremental<float>("kb_model_ycontrol_dec", DIK_DOWN, IKeybind::KEYBIND_ON_KEYDOWN, Features::GetEntityOBBY, 0.3f));

	// FIXME: can't get this dogshit to work
	//AddKeybinds(KeybindDynamicIncrement<float>("kb_model_ycontrol_inc", "kb_model_ycontrol_dec", DIK_UP, DIK_DOWN, IKeybind::KEYBIND_ON_KEYDOWN, Features::GetEntityOBBY, 0.3f));

#pragma endregion

#pragma region misc
	m_Items.emplace_back(new ConfigItemBool(CATEGORY_MISC, "t_wireframe", Vars.Misc.bWireframe));
	m_Items.emplace_back(new ConfigItemBool(CATEGORY_MISC, "t_anti_framecap", Vars.Misc.bAntiFramerateCap));
	m_Items.emplace_back(new ConfigItemBool(CATEGORY_MISC, "t_anti_vsync", Vars.Misc.bAntiVSync));
	m_Items.emplace_back(new ConfigItemBool(CATEGORY_MISC, "t_console_show_game_errors", Vars.Misc.bConsoleShowGameErrors));
#pragma endregion

#pragma region menu
	//m_items.emplace_back(new ConfigItemBool(CATEGORY_MENU, "b_ignore_input", Vars.Menu.bIgnoreInputWhenOpened));
	//m_items.emplace_back(new ConfigItemInt(CATEGORY_MENU, "i_theme_fg", g_pMenu->
#pragma endregion

#endif
}

void CConfig::LoadDefault()
{
	ZeroMemory(&Vars.Visuals, sizeof(Variables_t::Visuals_t));
	ZeroMemory(&Vars.Misc, sizeof(Variables_t::Misc_t));
	ZeroMemory(&Vars.Gameplay, UFIELD_OFFSET(Variables_t::Gameplay_t, SpawnBlacklist));

	PurgeConfig();
	InitializeConfig();
}

void CConfig::AddKeybind(CKeybind* pKeybind)
{
	if (!pKeybind)
		return;

	//m_Items.emplace_back(new ConfigItemKeybind("Keybinds"/*CATEGORY_KEYBINDS*/, *pKeybind));
	m_Keybinds.emplace_back(pKeybind);
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
	{
		*ppData = NULL;
		return GetLastError();
	}

	do
	{
		if (!bHeadConstructed)
		{
			bHeadConstructed = TRUE;
			pEntry = (PWIN32_FIND_DATA_LIST)malloc(sizeof(WIN32_FIND_DATA_LIST));
			*ppData = pEntry;
		}
		else
		{
			pEntry = (PWIN32_FIND_DATA_LIST)malloc(sizeof(WIN32_FIND_DATA_LIST));
		}

		if (!pEntry)
		{
			FindDataListFree(*ppData);
			FindClose(hFind);
			return GetLastError();
		}

		if (pPrev)
			pPrev->m_pNext = pEntry;

		pEntry->m_pNext = NULL;
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
		_tcscat_s(m_szFilename, CONFIG_DEFAULT_NAME);
	}

	return true;
}

BOOL IConfig::FileExists(LPCTSTR szFilename)
{
	return GetFileAttributes(szFilename) != INVALID_FILE_ATTRIBUTES;
}

BOOL CConfigXml::FileExists(LPCWSTR szFilename)
{
	return GetFileAttributesW(szFilename) != INVALID_FILE_ATTRIBUTES;
}

BOOL IConfig::SanitizePath(IN LPCTSTR szDelimiter, IN LPTSTR szOriginalPath, IN SIZE_T cchOriginalPath,
	OUT LPTSTR szSanitizedPath, IN SIZE_T cchSanitizedPath)
{
	LPTSTR szToken;
	LPTSTR szNextToken = NULL;

	LPTSTR szTemporary = NULL;
	LPTSTR szDirPath = (LPTSTR)malloc((cchOriginalPath + 1) * sizeof(TCHAR));
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
		free(szDirPath);
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
	OUT LPTSTR* pszSanitizedPath, OUT SIZE_T* pcchSanitizedPath)
{
	LPTSTR szToken;
	LPTSTR szNextToken = NULL;

	BOOL Status = ERROR_DIRECTORY;

	if (pszSanitizedPath && pcchSanitizedPath)
	{
		LPTSTR szTemporary = NULL;
		LPTSTR szDirPath = (LPTSTR)malloc((cchOriginalPath + 1) * sizeof(TCHAR));

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
				*pcchSanitizedPath = cchOriginalPath - (szTemporary - szOriginalPath + 1);
				*pszSanitizedPath = (LPTSTR)malloc((*pcchSanitizedPath) * sizeof(TCHAR));

				if (*pszSanitizedPath)
				{
					Status = ERROR_SUCCESS;
					_tcscpy_s(*pszSanitizedPath, *pcchSanitizedPath, szDirPath);
				}
				else
					Status = ERROR_OUTOFMEMORY;
			}
			free(szDirPath);
		}
		else
			Status = ERROR_OUTOFMEMORY;
	}
	else
		Status = ERROR_INVALID_PARAMETER;

	return Status;
}

BOOL IConfig::SanitizePath(IN LPCWSTR szDelimiter, IN LPWSTR szOriginalPath, IN SIZE_T cchOriginalPath,
	OUT LPWSTR szSanitizedPath, IN SIZE_T cchSanitizedPath)
{
	LPWSTR szToken;
	LPWSTR szNextToken = NULL;

	LPWSTR szTemporary = NULL;
	LPWSTR szDirPath = (LPWSTR)malloc((cchOriginalPath + 1) * sizeof(WCHAR));
	BOOL Status = ERROR_DIRECTORY;

	if (szDirPath)
	{
		wcscpy_s(szDirPath, cchOriginalPath, szOriginalPath);
		szToken = wcstok_s(szOriginalPath, szDelimiter, &szNextToken);

		while (szToken)
		{
			szTemporary = szToken;
			szToken = wcstok_s(NULL, szDelimiter, &szNextToken);
		}

		if (szTemporary)
		{
			szDirPath[szTemporary - szOriginalPath] = (TCHAR)0;

			if (szSanitizedPath)
				wcscpy_s(szSanitizedPath, cchSanitizedPath, szDirPath);

			Status = ERROR_SUCCESS;
		}
		free(szDirPath);
	}
	else
	{
		Status = ERROR_OUTOFMEMORY;
	}

	return Status;
}

BOOL IConfig::SanitizePath(IN LPCWSTR szDelimiter, IN LPWSTR szOriginalPath, IN SIZE_T cchOriginalPath,
	OUT LPWSTR* pszSanitizedPath, OUT SIZE_T* pcchSanitizedPath)
{
	LPWSTR szToken;
	LPWSTR szNextToken = NULL;

	BOOL Status = ERROR_DIRECTORY;

	if (pszSanitizedPath && pcchSanitizedPath)
	{
		LPWSTR szTemporary = NULL;
		LPWSTR szDirPath = (LPWSTR)malloc((cchOriginalPath + 1) * sizeof(WCHAR));

		if (szDirPath)
		{
			wcscpy_s(szDirPath, cchOriginalPath, szOriginalPath);
			szToken = wcstok_s(szOriginalPath, szDelimiter, &szNextToken);

			while (szToken)
			{
				szTemporary = szToken;
				szToken = wcstok_s(NULL, szDelimiter, &szNextToken);
			}

			if (szTemporary)
			{
				szDirPath[szTemporary - szOriginalPath] = (WCHAR)0;
				*pcchSanitizedPath = cchOriginalPath - (szTemporary - szOriginalPath + 1);
				*pszSanitizedPath = (LPWSTR)malloc((*pcchSanitizedPath) * sizeof(WCHAR));

				if (*pszSanitizedPath)
				{
					Status = ERROR_SUCCESS;
					wcscpy_s(*pszSanitizedPath, *pcchSanitizedPath, szDirPath);
				}
				else
					Status = ERROR_OUTOFMEMORY;
			}
			free(szDirPath);
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
	PWIN32_FIND_DATA_LIST pNext;

	while (it)
	{
		pNext = it->m_pNext;
		free(it);
		it = pNext;
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