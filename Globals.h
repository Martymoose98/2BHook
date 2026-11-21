#pragma once
#include "WinInclude.h"
#include <d3d11.h>
#include <dinput.h>
#include <xinput.h>

#include <vector> //should be temp

#include <Fluorine/ImportTableHook.h>
#include <Fluorine/VirtualTableHook.h>

#include "ReversedStructs.h"
#include "MemoryDeviceHook.h"

#define MAX_LEVELS 99
#define MIN_LEVELS 1
#define MAX_EXPERIENCE 9999999
#define MAX_MONEY 9999999

#define PROTAGONIST_NONE -1
#define PROTAGONIST_2B 0
#define PROTAGONIST_A2 1
#define PROTAGONIST_9S 2

#define POD_A 0
#define POD_B 1
#define POD_C 2

#define SZPROTAGONIST_NONE NULL
#define SZPROTAGONIST_2B "PL/2B"
#define SZPROTAGONIST_A2 "PL/A2"
#define SZPROTAGONIST_9S "PL/9S"

#define PRELOAD_TYPE_USER_INTERFACE	1
#define PRELOAD_TYPE_SHADER_INFO	1
#define PRELOAD_TYPE_BXM			1
#define PRELOAD_TYPE_QUEST			2
#define PRELOAD_TYPE_EFFECT			3
#define PRELOAD_TYPE_MODELDATA		3
#define PRELOAD_TYPE_NOVEL			4
#define PRELOAD_TYPE_GAMEDATA		5
#define PRELOAD_TYPE_FONT			5
#define PRELOAD_TYPE_CUBEMAP		7

#define MODELTYPE_2B_NORMAL 0x21020
#define MODELTYPE_2B_SUIT	0xA2140

#define OBJECTID_2B 0x10000
#define OBJECTID_A2 0x10100
#define OBJECTID_9S 0x10200

#define NOP_DAMAGE_ENEMY 0
#define NOP_DAMAGE_WORLD 1

#define NOP_FRAMECAP_SLEEP 0
#define NOP_FRAMECAP_SPINLOCK 1

#define XINPUT_GAMEPAD_GUIDE 0x400

#define KEYDOWN(name, key) (name[key] & 0x80)
#define KEYPRESSED(c, o, key) (KEYDOWN(c, key) && !KEYDOWN(o, key))

// This is for d3d11
enum eDepthState
{
	ENABLED,
	DISABLED,
	READ_NO_WRITE,
	NO_READ_NO_WRITE,
	_DEPTH_COUNT
};

struct CpkEntry;

typedef ULONGLONG QWORD;

typedef Level_t*(*CalculateLevelFn)(ExExpInfo* pInfo, int experience);
typedef CBehaviorAppBase*(*GetEntityFromHandleFn)(EntityHandle_t* pHandle);
typedef CEntityInfo*(*GetEntityInfoFromHandleFn)(EntityHandle_t* pHandle);
typedef BOOL(*ObjectIdToObjectNameFn)(char* szObjectName, size_t size, uint32_t ObjectId); //0x140628940
typedef int(*CItemManager_GetItemIdByNameFn)(CItemManager* pThis, const char* szItemName);			//returns a sItem* maybe?
typedef const char*(*CItemManager_GetItemNameByIdFn)(CItemManager* pThis, int item_id);	//returns a sItem* maybe?
typedef bool(*CItemManager_AddItemFn)(CItemManager* pThis, int item_id, int iQuantity);
typedef bool(*CItemManager_UseItemFn)(CItemManager* pThis, const char* szItemName);
typedef bool(*CItemManager_UseItemByIdFn)(CItemManager* pThis, int item_id);
typedef void(*ChangePlayerFn)(CBehaviorAppBase* pEntity);
typedef __int64(*SetLocalPlayerFn)(EntityHandle_t* pHandle);
typedef BOOL(*UnlockAchievementFn)(__int64, __int64, unsigned int uAchievement);
typedef uint32_t(*CUserManager_GiveAchievementFn)(uint32_t uAchievement);
typedef bool(*DestroyBuddyFn)(CBehaviorAppBase* pBuddy);
typedef __int64(*NPC_ChangeSetTypeFollowFn)(CBehaviorAppBase* pNPC);
typedef __int64(*NPC_ChangeSetTypeIdleFn)(CBehaviorAppBase* pNPC);
typedef __int64(*EmitSoundFn)(Sound* pSound, CBehavior** ppSourceEntity); // also second arg is another custom struct
typedef __int64(*PlaySoundFn)(Sound* pSound);
typedef uint32_t(*HashStringCRC32Fn)(const char* szName, __int64 length);
typedef uint32_t(*FNV1HashFn)(const char* szString);
typedef __int64(*HeapInstance_ReserveMemoryFn)(CHeapInstance* pThis, HeapAlloc_t* pMemory, __int64 nReserveBytes, void* pUnknown, unsigned int flags, void* pStruct);
typedef void*(*CSceneEntitySystem_FindSceneStateFn)(CSceneEntitySystem* pThis, uint32_t crc, const char* szName, __int64 length);
typedef bool(*CSceneStateSystem_SetFn)(/*hap::scene_state::SceneStateSystem* */void* pThis, SceneState** ppSceneState);
typedef BOOL(*SceneStateUnkFn)(void* unused, void* pSceneState);	//SceneStateUnkFn = 0x140053B00
typedef void(*SetSceneEntityFn)(const char* szHashName, CEntityInfo* pInfo);
typedef __int64(*CallTutorialDialogFn)(__int64, uint32_t dialogId); //callTutorialDialog address = 0x1401B1F30
typedef bool(*QuestState_RequestStateInternalFn)(DWORD *pQuestId);
typedef CConstructionInfo<void>*(*GetConstructorFn)(uint32_t ObjectId); //0x1401A2C20  templates are shit tbh
typedef CEntityInfo*(*CSceneEntitySystem_CreateEntityFn)(CSceneEntitySystem* pThis, Create_t* pCreate);
typedef void*(*AllocHeapMemoryFn)(QWORD size, CHeapInstance** ppHeap);

typedef void(*ExCollision_GetOBBMaxFn)(ExCollision* pThis, Vector3Aligned* pvMax);
typedef void(*ExCollision_ResetOriginFn)(ExCollision* pThis);

// render funcs
typedef bool(*COtManager_SetTextureFn)(void* pThis, int nTextures, int iTextureIndex, CTargetTexture* pTexture, void* pSamplerState, unsigned int iShaderResourceView);
typedef CGraphicCommand*(*COtManager_GetGraphicCommandFn)(__int64 uIndex);

typedef __int64(*CreateUIFromIdFn)(int id);

// Create UI Element: a game text by name hash, shown in a black banner dialog
typedef uint32_t(*HashLowercaseStringNoLenCRC32Fn)(const char* szString);
typedef const wchar_t*(*DialogUI_System_GetTextFn)(void* pFileSlots, int iNameHash, uint32_t uSlotIndex);
typedef __int64(*DialogUI_System_CreateBlackBannerFn)(void* pThis, int iOwner, int iNameHash, const wchar_t* szMessage, const wchar_t* szText, int iType, int iStyle, int iKind);

typedef void(*Pl0000_SetFlightUnitFn)(Pl0000* pThis);

// First person: turning the player toward the look, and the camera's near plane
typedef void(*Pl0000_TurnTowardHeadingFn)(CObj* pThis, float flHeading, float flFraction, float flMaxStep);
typedef CCameraInstance*(*CCameraInstance_SetZNearFn)(CCameraInstance** ppInstance, float flZNear);

typedef BOOL(*CreateTextureFn)(CGraphics* pThis, CTargetTexture *pTexture, CTextureDescription *pDesc);
typedef CTextureResource*(*CTextureResourceManager_FindResourceFn)(unsigned int texid);

typedef HeapAlloc_t*(*QueryHeapFn)(HeapAlloc_t* pResult, int objectid, int a3); // or void i guess

typedef void(*CJobManager_YieldToNextTaskFn)(void);
typedef void(*CJobManager_SetTaskFunctionFn)(void (*pFunc)(void*), void* pParams);
typedef uint32_t(*CreateTaskFn)(void* pfnCallback, void* pParam, uint32_t uPriority, const char* szName);

typedef BOOL (*CreateWorkFn)(int Objectid);
typedef CObjReadSystem::Work* (*CObjReadSystem_FindObjectWorkFn)(int ObjectId);
typedef CObjReadSystem::Work::Desc*(*PreloadFileFn)(__int64 thisrcx, int filetype, const char* szFilename, void* pHeap, byte flag, CObjReadSystem::Work* pWork);
typedef void(*ObjReadSystem_RequestEndFn)(__int64 a1, int ObjectId);
typedef BOOL(*ObjReadSystem_PreloadModelFn)(CObjReadSystem::Work* pWork);
// CObjReadSystem::RequestStartAsscessory and ::RequestStartFilter (the IDB's names): despite the
// names both only look, and answer 1 when an object and its dependencies are ready
typedef int(*CObjReadSystem_IsObjectReadyFn)(CObjReadSystem* pThis, uint32_t uObjectId, int32_t iSetType);
// CObjReadSystem::RequestStart takes read references on an object's files and those of every
// object its dependency record names, starting the reads of any not in memory; 1 on success.
// CObjReadSystem_ReleaseRequest drops exactly the references RequestStart took.
typedef int(*CObjReadSystem_RequestStartFn)(CObjReadSystem* pThis, uint32_t uObjectId, uint32_t uSetType);
typedef void(*CObjReadSystem_ReleaseRequestFn)(CObjReadSystem* pThis, uint32_t uObjectId, uint32_t uSetType);
// HwRofsFst_MountCpk: true if the archive is bound and ready. (The Denuvo build's version at
// 0x140956D70 returned the CpkEntry* instead.)
typedef bool(*CpkMountFn)(int iLoadOrder, const char* szPath);
typedef BOOL(*CpkMount2Fn)(CpkMountInfo* pCpk); // 0x140644000

// CRI's error callback. CriLogInternal (0x140098454) passes the message (a format string), then
// the high and the low half of a pointer to two 64-bit argument slots, and returns what the
// callback returns.
typedef const char*(*CRILogCallbackFn)(const char* szFormat, unsigned int, unsigned int, void*);

// XInput Function Defs

typedef struct _XINPUT_BASE_BUS_INFORMATION
{
	WORD w0x0000;		//0x0000
	WORD a2;			//0x0002
	DWORD a3;			//0x0004
	DWORD Flags;		//0x0008 | probably
	BYTE a4;			//0x0009
	BYTE a5;			//0x000A
	BYTE a6;			//0x000B
	BYTE reserved;		//0x000C
} XINPUT_BASE_BUS_INFORMATION, *PXINPUT_BASE_BUS_INFORMATION;

typedef struct _XINPUT_CAPABILITIES_EX
{
	XINPUT_CAPABILITIES Capabilities;	//0x0000
	WORD a1;							//0x0014
	WORD a2;							//0x0016
	WORD a3;							//0x0018
	DWORD a4;							//0x001C
} XINPUT_CAPABILITIES_EX, *PXINPUT_CAPABILITIES_EX;

typedef DWORD(__stdcall* XInputGetStateExFn)(DWORD dwUserIndex, XINPUT_STATE* pState);													// Ordinal = 100
typedef DWORD(__stdcall* XInputWaitForGuideButtonFn)(DWORD dwUserIndex, DWORD dwFlags, PVOID pUnknown);									// Ordinal = 101
typedef DWORD(__stdcall* XInputCancelGuideButtonWaitFn)(DWORD dwUserIndex);																// Ordinal = 102 
typedef DWORD(__stdcall* XInputPowerOffControllerFn)(DWORD dwUserIndex);																// Ordinal = 103
typedef DWORD(__stdcall* XInputGetBaseBusInformationFn)(DWORD dwUserIndex, XINPUT_BASE_BUS_INFORMATION* pInfo);							// Ordinal = 104
typedef DWORD(__stdcall* XInputGetCapabilitiesExFn)(DWORD a1, DWORD dwUserIndex, DWORD dwFlags, XINPUT_CAPABILITIES_EX *pCapabilities);	// Ordinal = 108

extern CalculateLevelFn CalculateLevel;
extern GetEntityInfoFromHandleFn GetEntityInfoFromHandle;
extern GetEntityFromHandleFn GetEntityFromHandle;
extern GetEntityFromHandleFn GetEntityFromHandle2;
extern CItemManager_GetItemNameByIdFn ItemManager_GetItemNameById;
extern CItemManager_GetItemIdByNameFn ItemManager_GetItemIdByName;
extern CItemManager_AddItemFn ItemManager_AddItem;
extern CItemManager_UseItemFn ItemManager_UseItem;
extern SetLocalPlayerFn SetLocalPlayer;
extern CCameraGame::ResetCameraFn ResetCamera;
extern ChangePlayerFn ChangePlayer;
extern DestroyBuddyFn DestroyBuddy;
extern CSaveDataDevice::SaveFileIOFn SaveFileIO;
extern CSaveDataDevice::ReadSaveSlotsFn ReadSaveSlots;
extern CSaveDataDevice::ReadSaveDataFn ReadSaveData;
extern CSaveDataDevice::WriteSaveDataFn WriteSaveData;
extern CSaveDataDevice::DeleteSaveDataFn DeleteSaveData;
extern CWetObjectManager::SetWetFn WetObjectManager_SetWet;
extern CWetObjectManager::SetDryFn WetObjectManager_SetDry;
extern CWetObjectManager::AddLocalEntityFn WetObjectManager_AddLocalEntity;
extern CEntityInfo::DeleteFn EntityInfo_Delete;
extern CSceneEntitySystem::CreateEntityFn SceneEntitySystem_CreateEntity;
extern CSceneEntitySystem::FindSceneStateFn SceneEntitySystem_FindSceneState;
extern CSceneEntitySystem::FindByObjectIdFn SceneEntitySystem_FindByObjectId;
extern CSceneEntitySystem::RemoveNodeFn SceneEntitySystem_RemoveNode;
extern CSceneStateSystem_SetFn SceneStateSystem_Set;
extern CCameraGame::SetLookAtFn CameraGame_SetLookAt;
extern CCameraGame::SetLookAtFn CameraGame_SetLookAtNoDistance;
extern ExCollision_GetOBBMaxFn ExCollision_GetOBBMax;
extern ExCollision_ResetOriginFn ExCollision_ResetOrigin;
extern SetSceneEntityFn SetSceneEntity;
extern GetConstructorFn GetConstructionInfo;

extern HashStringCRC32Fn HashStringCRC32;
extern UnlockAchievementFn UnlockAchievement;
extern CUserManager_GiveAchievementFn UserManager_GiveAchievement;
extern FNV1HashFn FNV1Hash;
extern PlaySoundFn ExecuteSound;
extern QueryHeapFn QueryHeap;
extern CreateTaskFn CreateTask;
extern CJobManager::CreateTaskFn JobManager_CreateTask;
extern CJobManager::SuspendTaskFn JobManager_SuspendTask;

extern CJobManager::DestroyTaskFn JobManager_DestroyTask;
// Specific to ObjReadSystem ?
extern CJobManager::SuspendTaskFn JobManager_PauseTask2;

extern CJobManager::YieldFn JobManager_Yield;
// Specific to ObjReadSyste
extern CJobManager_YieldToNextTaskFn YieldToNextTask;
extern CJobManager_SetTaskFunctionFn SetTaskFunction;
extern CreateWorkFn CreateWork;
extern CObjReadSystem_FindObjectWorkFn FindObjectWork;
extern PreloadFileFn PreloadFile;
extern ObjReadSystem_RequestEndFn RequestEnd;
extern ObjReadSystem_PreloadModelFn PreloadModel;
extern CObjReadSystem_IsObjectReadyFn ObjReadSystem_RequestStartAsscessory;
extern CObjReadSystem_IsObjectReadyFn ObjReadSystem_RequestStartFilter;
extern CObjReadSystem_RequestStartFn ObjReadSystem_RequestStart;
extern CObjReadSystem_ReleaseRequestFn ObjReadSystem_ReleaseRequest;
extern COtManager_GetGraphicCommandFn GetGraphicCommand;
extern CreateTextureFn CreateTexture;
extern CTextureResourceManager_FindResourceFn TextureResourceManager_FindResource;
extern CreateUIFromIdFn CreateUIFromId;
extern HashLowercaseStringNoLenCRC32Fn HashLowercaseStringNoLenCRC32;
extern DialogUI_System_GetTextFn DialogUI_System_GetText;
extern DialogUI_System_CreateBlackBannerFn DialogUI_System_CreateBlackBanner;
extern Pl0000_SetFlightUnitFn Pl0000_SetFlightUnit;
extern Pl0000_TurnTowardHeadingFn Pl0000_TurnTowardHeading;
extern CCameraInstance_SetZNearFn CameraInstance_SetZNear;
extern CpkMountFn CpkMount;
extern CRILogCallbackFn* CRIMainCallbackLog;	// CRI's main error callback slot, which the game leaves null

// XInput Functions

extern XInputGetStateExFn XInputGetStateEx;
extern XInputWaitForGuideButtonFn InputWaitForGuideButton;
extern XInputCancelGuideButtonWaitFn XInputCancelGuideButtonWait;
extern XInputPowerOffControllerFn XInputPowerOffController;
extern XInputGetBaseBusInformationFn XInputGetBaseBusInformation;
extern XInputGetCapabilitiesExFn InputGetCapabilitiesEx;

extern CreateStub_t g_CreateCtx;

extern uint32_t* g_pDebugFlags;
extern uint32_t* g_pCameraFlags;
extern uint32_t* g_pGameFlags;
extern uint32_t* g_pGameFlags2;


extern int32_t* g_piMoney;
extern int32_t* g_piExperience;

extern HWND g_hWnd;
extern HINSTANCE g_hInstance;
extern HANDLE* g_phHeaps;
extern LPSTR g_szDataDirectoryPath;
extern CpkEntry* g_pCpkEntries;		// g_HwRofsFstMounts, the game's CPK mount table
extern CpkEntry* g_pCpkEntriesEnd;	// one past its last entry
extern std::vector<LPTOP_LEVEL_EXCEPTION_FILTER> g_pExceptionHandlers;
extern std::vector<MrubyImpl*> g_pRubyInstances;

//extern void* g_pTextureVtbl;

extern Pl0000* g_pLocalPlayer;
extern EntityHandle_t* g_pLocalPlayerHandle;
extern EntityHandle_t* g_pEmilHandle;
extern CEntityList* g_pEntityList;
extern CItemManager* g_pItemManager;		// the item database (IDA: g_ItemManager)
extern CItemManager* g_pGameItemManager;	// what the game passes CItemManager::AddItem/UseItem (IDA: g_ITEM_MANAGER)
extern CYorhaManager* g_pYorhaManager;
extern CUserManager* g_pUserManager;
extern CNPCManager* g_pNPCManager;
extern CLightManager* g_pLightManager;
extern CEmBaseManager* g_pEnemyManager;
extern void* g_pUiFileSlots;		// g_UiFileSlots, what DialogUI::System::GetText looks texts up in
extern void* g_pDialogUISystem;		// g_DialogUISystem, the dialog system CreateBlackBanner works on
extern CWetObjManager* g_pWetObjectManager;
extern CCollisionDataObjectManager* g_pCollisionDataObjectManager;
extern CTextureResourceManager* g_pTextureResourceManager;
extern COtManager* g_pOtManager;
extern CModelManager* g_pModelManager;
extern CModelAnalyzer* g_pModelAnalyzer;
extern CCameraGame* g_pCamera;
extern VMatrix* g_pViewMatrix;
extern CJobManager* g_pJobManager;
extern CSceneStateSystem* g_pSceneStateSystem;
extern CSceneEntitySystem* g_pSceneEntitySystem;
extern CObjReadSystem* g_pObjReadSystem;						// g_ObjReadSystem, what the two checks above take
extern CConstructionInfo<void>* g_pConstructionInfoList;	// g_ConstructionInfoList, one row per object id
extern uint32_t g_uConstructionInfoRows;					// the rows GetConstructionInfo matches whole ids over
extern CNetworkDevice* g_pNetworkDevice;
extern CMemoryDevice* g_pMemoryDevice;
extern BYTE* g_pDecreaseHealth[2];
extern BYTE* g_pAntiVSync;
extern BYTE* g_pAntiFramerateCap_Sleep;
extern BYTE* g_pAntiFramerateCap_Spinlock;
extern BYTE* g_pAntiFramerateCap_Test4;
extern IDirectInput8A* g_pDirectInput8;
extern Keyboard_t* g_pKeyboard;
extern Mouse_t* g_pMouse;
extern CGraphics* g_pGraphics;

extern ID3D11Device* g_pDevice;
extern ID3D11DeviceContext* g_pDeviceContext;
extern ID3D11PixelShader* g_pRed;
extern ID3D11PixelShader* g_pGreen;
extern ID3D11PixelShader* g_pBlue;
extern ID3D11PixelShader* g_pCyan;
extern ID3D11RenderTargetView* g_pRenderTargetView;
extern ID3D11RasterizerState* g_pRenderWireframeState;
extern ID3D11RasterizerState* g_pRenderSolidState;
extern ID3D11DepthStencilState* g_pDepthStencilStates[_DEPTH_COUNT];
extern ID3D11SamplerState* g_pSamplerState;
extern ID3D11Texture2D* g_pTexRed;
extern ID3D11Texture2D* g_pTexGreen;
extern ID3D11ShaderResourceView* g_pRedSRV;
extern ID3D11ShaderResourceView* g_pGreenSRV;
extern ID3D11ShaderResourceView* g_pBlueSRV;
extern ID3D11ShaderResourceView* g_pCyanSRV;
extern IDXGISwapChain* g_pSwapChain;
extern IDXGISwapChain* g_pSecondarySwapChain;
extern IDXGIFactory* g_pFactory;
extern CGraphicDeviceDx11* g_pGraphicDevice;

extern VirtualTableHook* g_pFactoryHook;
extern VirtualTableHook* g_pSwapChainHook;
extern VirtualTableHook* g_pDeviceContextHook;
extern VirtualTableHook* g_pMouseHook;
extern VirtualTableHook* g_pKeyboardHook;
extern VirtualTableHook* g_pCameraHook;
extern VirtualTableHook* g_pModelAnalyzerHook;
extern MemoryDeviceHook* g_pMemoryDeviceHook;
extern std::vector<VirtualTableHook*> g_pRubyInstancesHooks;

extern MemoryDeviceHook* g_pMemoryDevHook;

extern ImportTableHook* g_pQueryPerformanceCounterHook;
extern ImportTableHook* g_pClipCursorHook;
extern ImportTableHook* g_pXInputGetStateHook;
extern ImportTableHook* g_pSetUnhandledExceptionFilterHook;
extern ImportTableHook* g_pOleLoadPictureHook;

extern BYTE_PATCH_MEMORY bp_save_file_io;
extern BYTE_PATCH_MEMORY bp_UpdateModelParts;
extern BYTE_PATCH_MEMORY bp_query_performance_counter;
extern BYTE_PATCH_MEMORY bp_Framecap;
extern BYTE_PATCH_MEMORY bp_NoTutorialDialogs;
extern NOP_MEMORY nop_HairColor;
extern NOP_MEMORY nop_Health[2];
extern NOP_MEMORY nop_neon_scenery;
extern HOOK_FUNC g_CreateEntityHook;
extern HOOK_FUNC g_LoadWordBlacklist;
extern HOOK_FUNC g_UserManagerSaveFileIO;