#pragma once

class Pl0000;

typedef struct Variables_t
{
	Variables_t() {}

	struct Visuals_t
	{	
		bool bTraceLine;
		float flTraceLength;
		bool bEspBox;
		bool bEnemyInfo;
		bool bNPCInfo;
		bool bSkeleton;
		bool bDebugLocalPlayerSkeleton;
	} Visuals;

	struct Gameplay_t
	{
		bool bGodmode;
		bool bNoEnemyDamage;
		bool bNoWorldDamage;
		bool bNoCollision;
		int iLevel;
		bool bTemporaryLevel;
		bool bBalanceEnemyLevels;
		bool bExclusivelyPositiveTolerance;
		int iEnemyLevelTolerance;
		int iSpawnObjectId;
		int iSpawnItemId;
		char szItemName[64];
		bool bInstantEquip;
		bool bRainbowModel;
		bool bRainbowHair;
		float flModelTintHue;
		int iSelectedModelPart;
		bool bRainbowPod;
		bool bHidePod;
		float flPodTintHue;
		int iSelectedPodModelPart;
		int iAnimation;
		Vector3Aligned vSpawnEntityScale;
		int iSelectedEntityType;
		int iSelectedAnimation;
		int iSelectedEntityHandle;
		int iSelectedEntityLevel;
		bool bGhostModel;
		bool bGhostModelOld;
		int nBones;
		bool bFreezePlayer;
		bool bNoTutorialDialogs;
		bool bSpeedMeister;
		float flSpeedMultiplier;
		int iSelectedBlacklistItem;
		char szBlacklistName[32];
		std::vector<std::string> SpawnBlacklist;
	} Gameplay;

	struct Misc_t
	{
		char szCpkName[32];
		bool bCpkLoaded;
		char szSoundName[128];
		bool bWireframe;
		bool bAntiVSync;
		bool bAntiFramerateCap;
		bool bBackupSave;
		bool bConsoleShowGameErrors;
		bool bLoading;
		int nSlot;
	} Misc;

	struct Menu_t
	{
		bool bOpened;
		bool bIgnoreInputWhenOpened;

		struct Input_t
		{
			XINPUT_STATE state;
			XINPUT_STATE emulate;
			LPDIDATAFORMAT pKeyboardFormat;	// c_dfDIKeyboard
			LPDIDATAFORMAT pMouseFormat;	// c_dfDIMouse2
			BOOLEAN KeyboardState[256];
			BOOLEAN OldKeyboardState[256];
			DIMOUSESTATE2 MouseState;
			DIMOUSESTATE2 OldMouseState;
		} Input;

		struct Config_t
		{
			char szName[MAX_PATH];
		} Config;
	} Menu;

	struct Keybinds_t
	{
		Keybinds_t() {}
		
		KeybindFunctional<void> ChangePlayer;
		KeybindFunctional<void> Airstuck;
		KeybindFunctional<void> DuplicateBuddy;
		KeybindFunctional<void> PlayAnimation;
		KeybindFunctional<void> TeleportForward;
		KeybindDynamicToggleable ModelGravity;
		KeybindDynamicIncrement<float> ModelYControl;
	} Keybinds;

	struct Animation
	{
		int id;
		const char* szDescription;
	};

	constexpr static Animation AnimationList[58] =
	{
		{ 0, "None" },
		{ 1, "Step Forward" },
		{ 2, "One Foot Lounge" },
		{ 3, "One Foot Lounge Fast" },
		{ 4, "Falling" },
		{ 5, "Jump" },
		{ 6, "Squat (Absorb Jump)" },
		{ 7, "Absorb Jump With Small Damage And Camera Shake" },
		{ 8, "Squat (Absorb Jump)" },
		{ 9, "Squat (Absorb Jump)" },
		{ 10, "Pod Magnet (Freezes Player)" },
		{ 11, "Squat (Absorb Jump)" },
		{ 12, "Ground Backflip (Recovery)" },
		{ 13, "Ground Backflip (Recovery)" },
		{ 14, "Vault" },
		{ 15, "Lay On Ground/Die (Freezes Player)" },
		{ 16, "Attack (Left Mouse)" },
		{ 17, "Roundhouse Kick" },
		{ 18, "Backward Dash" },
		{ 19, "Perfect Dodge" },
		{ 20, "A2 Small Sword Taunt" },
		{ 21, "Counter Chip Effect (Block/Parry)" },
		{ 22, "Volt Slave (Shock, Lingers)" },
		{ 23, "A2 Leg Grab Taunt" },
		{ 24, "Sword Lateral Spin" },
		{ 25, "Slow Motion Sword Play" },
		{ 26, "Unknown" },
		{ 27, "[Access Violation]" },
		{ 28, "Obtain Item" },
		{ 29, "Obtain Item With Dialog" },
		{ 30, "Obtain Item (Small Flair)" },
		{ 31, "Pick Up Item" },
		{ 32, "Pervert/Hentai/Stop Looking Up My Dress" },
		{ 33, "Pick Up Item Squat" },
		{ 34, "Stand Up Straight" },
		{ 35, "Stand Up Straight" },
		{ 36, "Spin And Shoot Pod" },
		{ 37, "Pod Boost/Swing"},
		{ 38, "Flight Suit Movement" },
		{ 39, "Flight Suit Animation?" },
		{ 40, "Flight Suit Backwards Dash" },
		{ 41, "Stand Up Straight" },
		{ 42, "Flinch" },
		{ 43, "Flinch" },
		{ 44, "Flinch And Stumble" },
		{ 45, "Flinch And Knockback" },
		{ 46, "Collaspe Forward And Backflip Up" },
		{ 47, "Float Up" },
		{ 48, "Push" },
		{ 49, "Flinch" },
		{ 50, "Seizure Flinch" },
		{ 51, "Ragdoll In Air" },
		{ 52, "Collapse Backward And Backflip Up" },
		{ 53, "Collapse And Die" },
		{ 54, "Die" },
		{ 55, "Self Destruct Start" },
		{ 56, "Self Destruct" },
		{ 57, "Self Destruct Effect" },
	};

	struct Spawn
	{
		const char* m_szName;
		const char* m_szClass;
		int m_ObjectId;
	};

	constexpr static Spawn SpawnEntities[] =
	{
		{ "2B", "partner", 0x10000 },
		{ "A2", "partner", 0x10100 },
		{ "9S", "partner", 0x10200 },
		{ "Emil", "\0", 0x11010 },
		{ "Moose", "regenerate_animal", 0x2A010 },
		{ "White Moose", "regenerate_animal", 0x2A011 },
		{ "Boar", "regenerate_animal", 0x2A000 },
		{ "White Boar", "regenerate_animal", 0x2A001 },
		{ "Grun", "Em4000Core", 0x31700 },
		{ "Eve", "em5000", 0x42000 },
		{ "Missle", "missile", 0x35000 },
		{ "Flight Suit", "flightUnit", 0x10010 },
		{ "Director", "et0009", 0x40009 },
		{ "Corpse1", "Corpse", 0x21080 },
		{ "Corpse2", "Corpse", 0x21081 },
		{ "Enemy Drop", "EmBase", 0x70001 },
		{ "Enemy Drop 2", "EmBase", 0x20030 } //0xA2180 c, 0x21060 c, 0x20030,  0x40006, 0xF2014, 0x20030, 0x30610, 0x30240, 0x30450, 0x30050, 0x30260, 0x30261, 0x30240, 0x30070, 0x30071, 0x30080, 0x30610, 0x30450, 0x30240, 0x30050, 0x31000, 0x40006, 0x40002,  0x31000
		//{ "Director2", "Et0024", 0x40012 }, CRASHES
		//{ "BG", "Bg4440", 0xC4440}, CRASHES
		//{ "Enemy1", "Em2101", 0x22101 }, CRASHES
		//{ "Enemy2", "Em2101", 0xA21C1 }, CRASHES
		//{ "Kago", "kago", 0xF0431 }, CRASHES
		//{ "Baketsu", "baketsu", 0xF0113 } CRASHES
	};

	constexpr static const char* EntityTypeList[] =
	{
		"2B",
		"A2",
		"9S",
		"Emil",
		"Moose",
		"White Moose",
		"Boar",
		"White Boar",
		"Grun",
		"Eve",
		"Missle",
		"Flight Suit (Dummy)",
		"Director",
		"Corpse1 (Invisible)",
		"Corpse2 (Invisible)",
		"Enemy Drop",
		"Enemy Drop 2"
		//"BG", CRASHES
		//"Enemy1",CRASHES
		//"Enemy2", CRASHES
		//"Kago", CRASHES
		//"Baketsu" CRASHES
	};

	constexpr static const char* AnimationListBoxList[58] =
	{
		"[0] ~ None" ,
		"[1] ~ Step Forward" ,
		"[2] ~ One Foot Lounge",
		"[3] ~ One Foot Lounge Fast",
		"[4] ~ Falling",
		"[5] ~ Jump",
		"[6] ~ Squat (Absorb Jump)",
		"[7] ~ Absorb Jump With Small Damage And Camera Shake",
		"[8] ~ Squat (Absorb Jump)",
		"[9] ~ Squat (Absorb Jump)",
		"[10] ~ Pod Magnet (Freezes Player)",
		"[11] ~ Squat (Absorb Jump)",
		"[12] ~ Ground Backflip (Recovery)",
		"[13] ~ Ground Backflip (Recovery)",
		"[14] ~ Vault",
		"[15] ~ Lay On Ground/Die (Freezes Player)",
		"[16] ~ Attack (Left Mouse)",
		"[17] ~ Roundhouse Kick",
		"[18] ~ Backward Dash",
		"[19] ~ Perfect Dodge",
		"[20] ~ A2 Small Sword Taunt",
		"[21] ~ Counter Chip Effect (Block/Parry)",
		"[22] ~ Volt Slave(Shock, Lingers)",
		"[23] ~ A2 Leg Grab Taunt",
		"[24] ~ Sword Lateral Spin",
		"[25] ~ Slow Motion Sword Play",
		"[26] ~ Kick (Finsher)",
		"[27] ~ [Access Violation]",
		"[28] ~ Obtain Item",
		"[29] ~ Obtain Item With Dialog",
		"[30] ~ Obtain Item (Small Flair)",
		"[31] ~ Pick Up Item",
		"[32] ~ Pervert/Hentai/Stop Looking Up My Dress",
		"[33] ~ Pick Up Item Squat",
		"[34] ~ Stand Up Straight",
		"[35] ~ Stand Up Straight",
		"[36] ~ Spin And Shoot Pod",
		"[37] ~ Pod Boost/Swing",
		"[38] ~ Flight Suit Movement",
		"[39] ~ Flight Suit Animation?",
		"[40] ~ Flight Suit Backwards Dash",
		"[41] ~ Stand Up Straight",
		"[42] ~ Flinch",
		"[43] ~ Flinch",
		"[44] ~ Flinch And Stumble",
		"[45] ~ Flinch And Knockback",
		"[46] ~ Collaspe Forward And Backflip Up",
		"[47] ~ Float Up",
		"[48] ~ Push",
		"[49] ~ Flinch",
		"[50] ~ Seizure Flinch",
		"[51] ~ Ragdoll In Air",
		"[52] ~ Collapse Backward And Backflip Up",
		"[53] ~ Collapse And Die",
		"[54] ~ Die",
		"[55] ~ Self Destruct Start",
		"[56] ~ Self Destruct",
		"[57] ~ Self Destruct Effect"
	};

	constexpr static const char* GameSounds[]
	{
		"core_noise",
		"core_new_game",
		"core_level_up",
		"core_novel_click_nomal",
		"core_novel_click_loading",
		"core_novel_click_pod",
		"core_novel_ward",
		"core_pl_dress_effect",
		"core_pl_curtain_close",
		"core_pl_AS_hold_spark",
		"core_pl_gameover_noize"
		"core_pl_low_life_loop"
		"core_pl_AS_hologram_loop"
		"wp3000_blackbox_sonar_hand_start",
		"wp3000_blackbox_sonar_hand_end",
		"wp3000_valcan_shot_em",
		"M1040_Park_Guide_1st",
		"M1040_Park_Guide_2nd",
		"bg3020_c4bomb_exp"
	};

} Variables;

extern Variables Vars;