#include "Menu.h"

CMenu* g_pMenu;

CMenu::CMenu(const CAdapterOutputPair& AdapterOutput)
	: m_bOpened(false), m_bIgnoreInputWhenOpened(true), m_pConfig(new CConfigXml())
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
	m_pConfig->m_Items.emplace(IConfigItemXml::MakePair(CATEGORY_MENU, L"i_theme_fg", m_Primary));
	m_pConfig->m_Items.emplace(IConfigItemXml::MakePair(CATEGORY_MENU, L"i_theme_bg", m_PrimaryBg));

	m_pConfig->CreateConfig(NULL);
	m_pConfig->EnumerateConfigs(NULL, &Config.pHead);

	// Set selected config to head.
	Config.iSelectedConfig = 0;

	// filename copy.
	_tcscpy_s(Config.szName, Config.pHead->m_Data.cFileName);

	ApplyStyle(m_Primary, m_PrimaryBg);
}

CMenu::~CMenu(void)
{
	FindDataListFree(Config.pHead);

	delete m_pConfig;
}

void CMenu::Draw(const ImVec2 vSize)
{
	if (!m_bOpened)
		return;

	ImGui::SetNextWindowSize(vSize, ImGuiCond_FirstUseEver);

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

	if (ImGui::Begin(s_MenuTitle, &m_bOpened, ImGuiWindowFlags_NoResize))
	{
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

			if (ImGui::BeginTabItem("Config"))
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

	return (m_bOpened && m_bIgnoreInputWhenOpened) ? DIERR_INPUTLOST : Result;
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

// https://www.htmlcsscolor.com/hex/B3C4D8
ImGuiStyle* CMenu::ApplyStyle(ImColor& Primary, ImColor& PrimaryBg)
{
	ImGuiStyle* pStyle = &ImGui::GetStyle(); // IM_NEW(ImGuiStyle)();

	pStyle->WindowRounding = 5.0f;
	pStyle->ChildRounding = 3.0f;

	ImVec4* colors = pStyle->Colors;

	// Greeen
	//ImColor Primary = ImColor(7, 74, 25, 255);		// ImColor(0.26f, 0.59f, 0.98f);
	//ImColor PrimaryBg = ImColor(7, 17, 74, 255);	// ImColor(0.16f, 0.29f, 0.48f);
	ImColor Secondary, Tertiary;
	ImColor SecondaryBg, TertiaryBg;

	ImColorScheme::Analogous(Primary, Secondary, Tertiary, 0.85f);
	ImColorScheme::Analogous(PrimaryBg, SecondaryBg, TertiaryBg, 0.75f);


	colors[ImGuiCol_Text] = ImColor(0.86f, 0.86f, 0.86f, 1.0f);
	colors[ImGuiCol_TextDisabled] = ImColor(0.66f, 0.66f, 0.66f, 1.0f);
	colors[ImGuiCol_WindowBg] = PrimaryBg;		// ImColor(0.06f, 0.06f, 0.06f, 0.94f);
	colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
	colors[ImGuiCol_Border] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
	colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_FrameBg] = ImColorScheme::AlphaModulate(SecondaryBg, 0.54f);		// ImVec4(0.16f, 0.29f, 0.48f, 0.54f);
	colors[ImGuiCol_FrameBgHovered] = ImColorScheme::AlphaModulate(Primary, 0.40f);		// ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
	colors[ImGuiCol_FrameBgActive] = ImColorScheme::AlphaModulate(Primary, 0.67f);		// ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
	colors[ImGuiCol_TitleBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
	colors[ImGuiCol_TitleBgActive] = ImColorScheme::AlphaModulate(SecondaryBg, 1.00f);	// ImVec4(0.16f, 0.29f, 0.48f, 1.00f);
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
	colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
	colors[ImGuiCol_CheckMark] = ImColorScheme::AlphaModulate(Primary, 1.00f);			// ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	colors[ImGuiCol_SliderGrab] = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);
	colors[ImGuiCol_SliderGrabActive] = ImColorScheme::AlphaModulate(Primary, 1.00f);	// ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	colors[ImGuiCol_Button] = ImColorScheme::AlphaModulate(Primary, 0.40f);				// ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
	colors[ImGuiCol_ButtonHovered] = ImColorScheme::AlphaModulate(Primary, 1.00f);		// ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
	colors[ImGuiCol_Header] = ImColorScheme::AlphaModulate(Primary, 0.31f);				// ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
	colors[ImGuiCol_HeaderHovered] = ImColorScheme::AlphaModulate(Primary, 0.80f);		// ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
	colors[ImGuiCol_HeaderActive] = ImColorScheme::AlphaModulate(Primary, 1.00f);		// ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	colors[ImGuiCol_Separator] = colors[ImGuiCol_Border];
	colors[ImGuiCol_SeparatorHovered] = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
	colors[ImGuiCol_SeparatorActive] = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
	colors[ImGuiCol_ResizeGrip] = ImColorScheme::AlphaModulate(Primary, 0.20f);			// ImVec4(0.26f, 0.59f, 0.98f, 0.20f);
	colors[ImGuiCol_ResizeGripHovered] = ImColorScheme::AlphaModulate(Primary, 0.67f);	// ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
	colors[ImGuiCol_ResizeGripActive] = ImColorScheme::AlphaModulate(Primary, 0.95f);	// ImVec4(0.26f, 0.59f, 0.98f, 0.95f);

	colors[ImGuiCol_Tab] = ImLerp(colors[ImGuiCol_Header], colors[ImGuiCol_TitleBgActive], 0.80f);
	colors[ImGuiCol_TabHovered] = colors[ImGuiCol_HeaderHovered];
	colors[ImGuiCol_TabActive] = ImLerp(colors[ImGuiCol_HeaderActive], colors[ImGuiCol_TitleBgActive], 0.60f);
	colors[ImGuiCol_TabUnfocused] = ImLerp(colors[ImGuiCol_Tab], colors[ImGuiCol_TitleBg], 0.80f);
	colors[ImGuiCol_TabUnfocusedActive] = ImLerp(colors[ImGuiCol_TabActive], colors[ImGuiCol_TitleBg], 0.40f);

	colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
	colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
	colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);

	colors[ImGuiCol_TableHeaderBg] = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
	colors[ImGuiCol_TableBorderStrong] = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);   // Prefer using Alpha=1.0 here
	colors[ImGuiCol_TableBorderLight] = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);   // Prefer using Alpha=1.0 here
	colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);

	colors[ImGuiCol_TextSelectedBg] = ImColorScheme::AlphaModulate(Primary, 0.35f);		// ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
	colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);

	colors[ImGuiCol_NavHighlight] = ImColorScheme::AlphaModulate(Primary, 1.00f);		//ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
	colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);

	colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);

	return pStyle;
}

void CMenu::LoadConfig(LPCWSTR szConfig)
{
	g_pConsole->Log(ImGui::GetStyle().Colors[ImGuiCol_TextSelectedBg], "Loading config. (%s)", szConfig);
	m_pConfig->Load(szConfig);
	ApplyStyle(m_Primary, m_PrimaryBg);
}

void CMenu::SaveConfig(LPCWSTR szConfig)
{
	g_pConsole->Log(ImGui::GetStyle().Colors[ImGuiCol_TextSelectedBg], "Saving config. (%s)", szConfig);
	m_pConfig->Save(szConfig);
}

// 40 53 48 83 EC 20 48 8B 91 80 72 - pl0000::SetStateMosaic
void CMenu::GameplayTab(Pl0000* pCameraEnt)
{
	ImGui::Checkbox("Godmode", &Vars.Gameplay.bGodmode);
	ImGui::SameLine();
	ImGui::Checkbox("No Fall Damage", &Vars.Gameplay.bNoWorldDamage);
	ImGui::SameLine();
	ImGui::Checkbox("No Enemy Damage", &Vars.Gameplay.bNoEnemyDamage);

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

	char szDesc[64];
	char* szName = (char*)ItemManager_GetItemNameById(g_pItemManager, Vars.Gameplay.iSpawnItemId);
	// FIXME: Broken on the new version this fucking function keeps crashing
	sprintf_s(szDesc, "Item Name or Id: (%s)", szName);
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

	ImGui::DragFloat("Camera Fov", &Vars.Visuals.flFov, 1.0f, 20.0f, 300.0f);

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
				typedef void (*Pl_ChangeFlightSuitFn)(void* pEnt);
				//pCameraEnt->m_bFlightSuit = TRUE;
				pCameraEnt->m_hFlightSuit = pInfo->m_hEntity;
				((Pl_ChangeFlightSuitFn)((ULONG_PTR)GetModuleHandle(NULL) + 0x4F8300))(pCameraEnt); /*0x4E9DB0*/
				//((Pl_ChangeFlightSuitFn)((ULONG_PTR)GetModuleHandle(NULL) + 0x4E0340))(pCameraEnt);
				break;
			}
		}
	}

	ImGui::ListBox("Spawn Blacklist", &Vars.Gameplay.iSelectedBlacklistItem, BlacklistItemCallback, Vars.Gameplay.SpawnBlacklist.data(), (int)Vars.Gameplay.SpawnBlacklist.size());
	ImGui::InputText("Blacklist Item", Vars.Gameplay.szBlacklistName, ARRAYSIZE(Vars.Gameplay.szBlacklistName));

	if (ImGui::Button("Add Item"))
	{
		Vars.Gameplay.SpawnBlacklist.emplace_back(Vars.Gameplay.szBlacklistName);
	}

	ImGui::SameLine();

	if (ImGui::Button("Delete Item"))
	{
		Vars.Gameplay.SpawnBlacklist.erase(std::find(Vars.Gameplay.SpawnBlacklist.cbegin(), Vars.Gameplay.SpawnBlacklist.cend(), Vars.Gameplay.SpawnBlacklist.data()[Vars.Gameplay.iSelectedBlacklistItem]));
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

	//FIXME: DEFO CRASH
	if (ImGui::Button("Emit Sound"))
	{
		g_pLocalPlayer->EmitSound(Vars.Misc.szSoundName, g_pLocalPlayer, 5, 0x5000000, 0x200007F);
		//Sound sound = { Vars.Misc.szSoundName, FNV1Hash(Vars.Misc.szSoundName), 0 };
		//QWORD rax = ((EmitSoundFn)(0x14081FD80))(&sound, &g_pLocalPlayer);
		//LOG("RAX = %x\n", rax);
	}

	ImGui::InputFloat3("Viewangles", (float*)&g_pCamera->m_vViewangles);

	ImGui::Checkbox("Speedhack", &Vars.Gameplay.bSpeedhack);
	ImGui::SameLine();
	ImGui::SliderFloat("Speed Multiplier", &Vars.Gameplay.flSpeedMultiplier, -10.0f, 10.0f); // when the framerate is uncapped this can go higher, else x5 will freeze the game because the game thinks the frame rate is too high so it sleeps the threads
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
	ImGui::SameLine();
	ImGui::Checkbox("Anti-VSync", &Vars.Misc.bAntiVSync);
	ImGui::SameLine();
	ImGui::Checkbox("Anti-Framerate Cap", &Vars.Misc.bAntiFramerateCap);
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

	if (ImGui::Button("Create UI Element"))
	{
		// TODO: Move function typedefs to globals!
		typedef unsigned int (*HashLowercaseStringCRC32Fn)(const char* szString);
		// Retreives game text from crc32
		typedef wchar_t* (*DialogUI_System_GetTextFn)(void* pThis, int crc32, unsigned int index);
		// Creates a white dialog
		typedef __int64(*DialogUI_System_CreateYesNoDialogFn)(wchar_t* szMessage);
		// Creates a white dialog w/ options
		typedef __int64(*DialogUI_System_CreateDialogFn)(void* pThis, wchar_t* szMessage, int bWhiteDialog, DialogUIWhiteType type, int darkness);
		// Creates a black banner
		typedef __int64(*DialogUI_System_CreateBlackBannerFn)(void* pThis, int a1, int crc32, wchar_t* szMessage, wchar_t* szText, DialogUIBlackType type, unsigned int a8, int darkness);


		ULONG_PTR uNier = (ULONG_PTR)GetModuleHandle(NULL);

		unsigned int crc = ((HashLowercaseStringCRC32Fn)(uNier + 0x14C4C0))("CORE_QUEST_EMP_TXT_MONEY");// CORE_MAP_VOICECNG_01
		wchar_t* szMsg = ((DialogUI_System_GetTextFn)(uNier + 0x9BD090))((void*)(uNier + 0x14207A0), crc, 24);
		// e8a92ebf crc32 | CORE_ITEM_NAME_935
		//((DialogUI_System_CreateDialogFn)(uNier + 0x2F4F60))((void*)(uNier + 0xF04018), L"Cazzone", (DialogUIWhiteType)ui_ids[0], ui_ids[1], ui_ids[2]);
		((DialogUI_System_CreateBlackBannerFn)(uNier + 0x2F5040))((void*)(uNier + 0xF04018), -1, 0, szMsg, L"It wasn't supposed to be like this!\nCazzone", (DialogUIBlackType)ui_ids[0], ui_ids[1], ui_ids[2]);
		//((DialogUI_System_CreateYesNoDialogFn)(uNier + 0x2F4760))(L"Bad News!");
		//CreateUIFromId(ui_ids[0]);
	}

	// new test shit
	ImGui::InputInt("Debug Flags", (int*)g_pDebugFlags, 1, 100, ImGuiInputTextFlags_CharsHexadecimal);
	ImGui::InputInt("Camera Flags", (int*)g_pCameraFlags, 1, 100, ImGuiInputTextFlags_CharsHexadecimal);
	ImGui::InputInt("Game Flags", (int*)g_pGameFlags, 1, 100, ImGuiInputTextFlags_CharsHexadecimal);
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

	ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.3f);

	ImGui::ColorPicker4("ThemePrimary", (float*)&m_Primary.Value, ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_AlphaBar);
	ImGui::SameLine();
	ImGui::ColorPicker4("ThemePrimaryBg", (float*)&m_PrimaryBg.Value, ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_AlphaBar);

	ImGui::PopItemWidth();

	if (ImGui::Button("Apply Theme"))
		ApplyStyle(m_Primary, m_PrimaryBg);

	ImGui::ListBox("Configs", &Config.iSelectedConfig, ConfigCallback, Config.pHead, (INT)FindDataListCount(Config.pHead));
	ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.5f);
	ImGui::InputText("Config Name", Config.szName, _ARRAYSIZE(Config.szName));
	ImGui::PopItemWidth();
	ImGui::SameLine();

	PWIN32_FIND_DATA_LIST pSelected = FindDataListNav(Config.pHead, Config.iSelectedConfig);
	szConfig = Config.szName[0] ? Config.szName : (pSelected) ? pSelected->m_Data.cFileName : NULL;

	if (ImGui::Button("Refresh"))
	{
		FindDataListFree(Config.pHead);
		m_pConfig->EnumerateConfigs(NULL, &Config.pHead);
	}

	wchar_t cfgname[MAX_PATH];
	mbstowcs_s(nullptr, cfgname, szConfig, _TRUNCATE);

	if (ImGui::Button("Load"))
		LoadConfig(cfgname);

	ImGui::SameLine();

	if (ImGui::Button("Save"))
		SaveConfig(cfgname);
}

void CMenu::DisplayEntityHandles(void)
{
	if (!g_pNPCManager)
		return;

	int count = (int)(g_pNPCManager->m_handles.m_Count + g_pYorhaManager->m_handles.m_Count + g_pEnemyManager->m_handles.m_Count);
	EntityHandle_t* pHandles = new EntityHandle_t[count];

	ImGui::ListBox("NPC, Yorha & Enemy Handles", &Vars.Gameplay.iSelectedActorEntityHandle,
		&EntityHandlesCallback, pHandles, count);

	ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.25f);

	ImGui::InputInt("Level", &Vars.Gameplay.iSelectedEntityLevel, 1, 5);

	ImGui::SameLine();

	if (ImGui::Button("Set Level"))
		Features::BalanceEnemyLevel(GetEntityFromHandleGlobal(&pHandles[Vars.Gameplay.iSelectedActorEntityHandle]), Vars.Gameplay.iSelectedEntityLevel - 1, Vars.Gameplay.iSelectedEntityLevel - 1);

	ImGui::PopItemWidth();

#if 0
	//FIXME: DEFO CRASH
	if (ImGui::Button("Set NPC Follow"))
	{
		CBehaviorAppBase* pNPC = GetEntityFromHandle(&pHandles[Vars.Gameplay.iSelectedActorEntityHandle]);

		if (pNPC)
			((NPC_ChangeSetTypeFollowFn)(0x1404B2770))(pNPC);
	}

	ImGui::SameLine();

	//FIXME: DEFO CRASH
	if (ImGui::Button("Set NPC Idle"))
	{
		CBehaviorAppBase* pNPC = GetEntityFromHandle(&pHandles[Vars.Gameplay.iSelectedActorEntityHandle]);

		if (pNPC)
			((NPC_ChangeSetTypeIdleFn)(0x1404B2770))(pNPC);
	}
#endif

	if (ImGui::Button("Delete"))
		Features::DeleteEntity(pHandles[Vars.Gameplay.iSelectedActorEntityHandle]);

#ifdef  _DEBUG
	ImGui::SameLine();

	if (ImGui::Button("Set Player"))
	{
		Features::SetPlayer(pHandles[Vars.Gameplay.iSelectedActorEntityHandle]);
	}
#endif

	delete[] pHandles;

	ImGui::SeparatorEx(ImGuiSeparatorFlags_::ImGuiSeparatorFlags_Horizontal);

	ImGui::ListBox("Entity View", &Vars.Gameplay.iSelectedEntityHandle, &EntityCallback, NULL, g_pEntityList->m_uSize);

	if (ImGui::Button("Delete Entity"))
		Features::DeleteEntity(g_pEntityList->m_pItems[Vars.Gameplay.iSelectedEntityHandle].first);

	ImGui::SameLine();

	if (ImGui::Button("Set Player"))
	{
		Features::SetPlayer(g_pEntityList->m_pItems[Vars.Gameplay.iSelectedEntityHandle].first);
	}
}

void ApplyPodMods(Pl0000* pOwner)
{
	if (!pOwner)
		return;

	CBehaviorAppBase* pPod = GetEntityFromHandle(&pOwner->m_hPod);

	if (!pPod)
		return;

	ImGui::Checkbox("Rainbow Pod", &Vars.Gameplay.bRainbowPod);
	ImGui::SameLine();
	ImGui::Checkbox("Hide Pod", &Vars.Gameplay.bHidePod);

	pPod->m_pModelInfo->m_vTint.w = Vars.Gameplay.bHidePod ? 0.f : 1.f;

	char szModelPart[64];
	sprintf_s(szModelPart, "Pod Mesh: (%s)", pPod->m_Work.m_pMeshes[Vars.Gameplay.iSelectedPodMesh].m_szMeshName);
	ImGui::SliderInt(szModelPart, &Vars.Gameplay.iSelectedPodMesh, 0, pPod->m_Work.m_nMeshes - 1);

	sprintf_s(szModelPart, "Pod Mesh Color: (%s)", pPod->m_Work.m_pMeshes[Vars.Gameplay.iSelectedPodMesh].m_szMeshName);

	if (ImGui::Checkbox("Enabled", (bool*)&pPod->m_Work.m_pMeshes[Vars.Gameplay.iSelectedPodMesh].m_bShow))
		pPod->m_Work.m_pMeshes[Vars.Gameplay.iSelectedPodMesh].m_bUpdate = TRUE;

	ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.3f);

	ImGui::ColorPicker4(szModelPart, (float*)&pPod->m_Work.m_pMeshes[Vars.Gameplay.iSelectedPodMesh].m_vColor);

	ImGui::PopItemWidth();

	pPod->m_Work.m_pMeshes[Vars.Gameplay.iSelectedModelMesh].m_bUpdate = TRUE;
	pPod->m_Work.m_pMeshes[Vars.Gameplay.iSelectedModelMesh].m_bShow = TRUE;
}

void ApplyModelMods(Pl0000* pEntity)
{
	char szModelPart[64];

	if (!pEntity)
		return;

	if (ImGui::Button("Wet Entity"))
		Features::WetEntity(pEntity, 127);

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

	//Vars.Gameplay.iSelectedModelMesh &= (pEntity->m_Work.m_nMeshes - 1);

	CMeshPart* pSelectedMesh = &pEntity->m_Work.m_pMeshes[Vars.Gameplay.iSelectedModelMesh];

	if (pSelectedMesh)
	{
		sprintf_s(szModelPart, "Mesh: %s", pSelectedMesh->m_szMeshName);
		ImGui::SliderInt(szModelPart, &Vars.Gameplay.iSelectedModelMesh, 0, pEntity->m_Work.m_nMeshes - 1);

		sprintf_s(szModelPart, "Mesh %s Color", pSelectedMesh->m_szMeshName);
	}


	ImGui::Checkbox("Enabled", &Vars.Gameplay.bSelectedMeshEnable);

#if defined(_DEBUG)
	ImGui::InputText("Custom Texture (DDS)", Vars.Gameplay.szModelTextureName, MAX_PATH);
	ImGui::SameLine();

	if (ImGui::Button("Load"))
	{
		CTextureDescription desc;
		CTargetTexture* pTexture = CreateTextureR(Vars.Gameplay.szModelTextureName, desc);

		if (pTexture)
		{
			CMeshPart* pMesh = &g_pLocalPlayer->m_Work.m_pMeshes[Vars.Gameplay.iSelectedModelMesh];

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

	ApplyPodMods(pEntity);
}

bool BlacklistItemCallback(void* data, int idx, const char** out_text)
{
	if (out_text)
		*out_text = ((std::string*)data)[idx].c_str();

	return true;
}

/*
this might be needed
	for (int i = 0; i < idx; pEntry = pEntry->m_pNext)
		if (!pEntry->m_pNext)
			return false;
*/
bool ConfigCallback(void* data, int idx, const char** out_text)
{
	PWIN32_FIND_DATA_LIST pEntry = FindDataListNav((PCWIN32_FIND_DATA_LIST)data, idx);

	if (pEntry)
	{
		if (out_text)
			*out_text = pEntry->m_Data.cFileName;

		return true;
	}
	else return false;
}