#pragma once
#include <Windows.h>
#include <d3d11.h>
#include <dinput.h>
#include <xinput.h>

#include <vector> //should be temp

#include "ReversedStructs.h"
#include <Fluorine\VirtualTableHook.h>
#include <Fluorine\ImportTableHook.h>
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
typedef CBehaviorAppBase*(*GetEntityFromHandleFn)(EntityHandle* pHandle);
typedef CEntityInfo*(*GetEntityInfoFromHandleFn)(EntityHandle* pHandle);
typedef BOOL(*ObjectIdToObjectNameFn)(char* szObjectName, size_t size, int objectId); //0x140628940
typedef int(*CItemManager_GetItemIdByNameFn)(CItemManager* pThis, const char* szItemName);			//returns a sItem* maybe?
typedef const char*(*CItemManager_GetItemNameByIdFn)(CItemManager* pThis, int item_id);	//returns a sItem* maybe?
typedef bool(*CItemManager_AddItemFn)(CItemManager* pThis, int item_id, int iQuantity);
typedef bool(*CItemManager_UseItemFn)(CItemManager* pThis, const char* szItemName);
typedef bool(*CItemManager_UseItemByIdFn)(CItemManager* pThis, int item_id);
typedef void(*ChangePlayerFn)(CBehaviorAppBase* pEntity);
typedef __int64(*SetLocalPlayerFn)(EntityHandle* pHandle);
typedef BOOL(*UnlockAchievementFn)(__int64, __int64, unsigned int uAchievement);
typedef void(*CSaveDataDevice_SaveFileIOFn)(CSaveDataDevice* pSave); // NOTE: not a real function points to CUserManager::SaveFileIO's switch
typedef void(*CSaveDataDevice_ReadSaveSlotsFn)(CSaveDataDevice* pSave);
typedef void(*CSaveDataDevice_ReadSaveDataFn)(CSaveDataDevice* pSave);
typedef void(*CSaveDataDevice_WriteSaveDataFn)(CSaveDataDevice* pSave);
typedef void(*CSaveDataDevice_DeleteSaveDataFn)(CSaveDataDevice* pSave);
typedef void(*CWetObjectManager_AddLocalEntityFn)(CWetObjManager* pThis, CEntityInfo* pInfo);
typedef void(*CWetObjectManager_SetWetFn)(CWetObjManager* pThis, byte wet_level, int index);
typedef void(*CWetObjectManager_SetDryFn)(CWetObjManager* pThis, CEntityInfo *pInfo);
typedef void(*CCameraGame_SetLookAtFn)(CCameraGame* pThis);
typedef void(*ResetCameraFn)(CCameraGame* pCamera);
typedef bool(*DestroyBuddyFn)(CBehaviorAppBase* pBuddy);
typedef __int64(*NPC_ChangeSetTypeFollowFn)(CBehaviorAppBase* pNPC);
typedef __int64(*NPC_ChangeSetTypeIdleFn)(CBehaviorAppBase* pNPC);
typedef __int64(*EmitSoundFn)(Sound* pSound, CBehavior** ppSourceEntity); // also second arg is another custom struct
typedef __int64(*PlaySoundFn)(Sound* pSound);
typedef unsigned int(*HashStringCRC32Fn)(const char* szName, __int64 length);
typedef unsigned int(*FNV1HashFn)(const char* szString);
typedef __int64(*HeapInstance_ReserveMemoryFn)(CHeapInstance* pThis, HeapAlloc_t* pMemory, __int64 nReserveBytes, void* pUnknown, unsigned int flags, void* pStruct);
typedef void*(*CSceneEntitySystem_FindSceneStateFn)(CSceneEntitySystem* pThis, unsigned int crc, const char* szName, __int64 length); // not sure if it actually finds the heap (it might just find the scene state) 0x1400538A0
typedef bool(*CSceneStateSystem_SetFn)(/*hap::scene_state::SceneStateSystem* */void* pThis, SceneState** ppSceneState);
typedef BOOL(*SceneStateUnkFn)(void* unused, void* pSceneState);	//SceneStateUnkFn = 0x140053B00
typedef void(*SetSceneEntityFn)(const char*, CEntityInfo*);
typedef __int64(*CallTutorialDialogFn)(__int64, unsigned int dialogId); //callTutorialDialog address = 0x1401B1F30
typedef bool(*QuestState_RequestStateInternalFn)(DWORD *pQuestId);
typedef CConstructionInfo<void>*(*GetConstructorFn)(int objectId); //0x1401A2C20  templates are shit tbh
typedef CEntityInfo*(*CSceneEntitySystem_CreateEntityFn)(CSceneEntitySystem* pThis, Create_t* pCreate);
typedef void*(*AllocHeapMemoryFn)(QWORD size, CHeapInstance** ppHeap);

typedef void(*ExCollision_GetOBBMaxFn)(ExCollision* pThis, Vector3Aligned* pvMax);

// render funcs
typedef bool(*COtManager_SetTextureFn)(void* pThis, int nTextures, int iTextureIndex, CTargetTexture* pTexture, void* pSamplerState, unsigned int iShaderResourceView);
typedef CGraphicCommand*(*COtManager_GetGraphicCommandFn)(__int64 uIndex);

typedef __int64(*CreateUIFromIdFn)(int id);

typedef BOOL(*CreateTextureFn)(CGraphics* pThis, CTargetTexture *pTexture, CTextureDescription *pDesc);
typedef CTextureResource*(*CTextureResourceManager_FindResourceFn)(unsigned int texid);

typedef HeapAlloc_t*(*QueryHeapFn)(HeapAlloc_t* pResult, int objectid, int a3); // or void i guess
typedef CObjReadSystem::Work*(*GetWorkFn)(int objectid);
typedef CObjReadSystem::Work::Desc*(*PreloadFileFn)(__int64 thisrcx, int filetype, const char* szFilename, void* pHeap, byte flag, CObjReadSystem::Work* pWork);
typedef void(*ObjReadSystem_RequestEndFn)(__int64 a1, int objectId);
typedef BOOL(*ObjReadSystem_PreloadModelFn)(CObjReadSystem::Work* pWork);
typedef CpkEntry*(*CpkMountFn)(int iLoadOrder, char* szPath); // 0x140956D70 
typedef BOOL(*CpkMount2Fn)(CpkMountInfo* pCpk); // 0x140644000

typedef void(*CRILogCallbackFn)(const char* szFormat, unsigned int, unsigned int, void*);

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
extern ResetCameraFn ResetCamera;
extern ChangePlayerFn ChangePlayer;
extern DestroyBuddyFn DestroyBuddy;
extern CSaveDataDevice_SaveFileIOFn SaveFileIO;
extern CSaveDataDevice_ReadSaveSlotsFn ReadSaveSlots;
extern CSaveDataDevice_ReadSaveDataFn ReadSaveData;
extern CSaveDataDevice_WriteSaveDataFn WriteSaveData;
extern CSaveDataDevice_DeleteSaveDataFn DeleteSaveData;
extern CWetObjectManager_SetWetFn WetObjectManager_SetWet;
extern CWetObjectManager_SetDryFn WetObjectManager_SetDry;
extern CWetObjectManager_AddLocalEntityFn WetObjectManager_AddLocalEntity;
extern CSceneEntitySystem_CreateEntityFn SceneEntitySystem_CreateEntity;
extern CSceneStateSystem_SetFn SceneStateSystem_Set;
extern CCameraGame_SetLookAtFn CameraGame_SetLookAt;
extern CCameraGame_SetLookAtFn CameraGame_SetLookAtNoDistance;
extern ExCollision_GetOBBMaxFn ExCollision_GetOBBMax;
extern SetSceneEntityFn SetSceneEntity;
extern GetConstructorFn GetConstructionInfo;
extern CSceneEntitySystem_FindSceneStateFn SceneEntitySystem_FindSceneState;
extern HashStringCRC32Fn HashStringCRC32;
extern UnlockAchievementFn UnlockAchievement;
extern FNV1HashFn FNV1Hash;
extern QueryHeapFn QueryHeap;
extern GetWorkFn GetWork;
extern PreloadFileFn PreloadFile;
extern ObjReadSystem_RequestEndFn RequestEnd;
extern ObjReadSystem_PreloadModelFn PreloadModel;
extern COtManager_GetGraphicCommandFn GetGraphicCommand;
extern CreateTextureFn CreateTexture;
extern CTextureResourceManager_FindResourceFn TextureResourceManager_FindResource;
extern CreateUIFromIdFn CreateUIFromId;
extern CpkMountFn CpkMount;
extern CRILogCallbackFn CRILogCallback;

// XInput Functions

extern XInputGetStateExFn XInputGetStateEx;
extern XInputWaitForGuideButtonFn InputWaitForGuideButton;
extern XInputCancelGuideButtonWaitFn XInputCancelGuideButtonWait;
extern XInputPowerOffControllerFn XInputPowerOffController;
extern XInputGetBaseBusInformationFn XInputGetBaseBusInformation;
extern XInputGetCapabilitiesExFn InputGetCapabilitiesEx;

extern unsigned int* g_pDebugFlags;
extern unsigned int* g_pCameraFlags;
extern unsigned int* g_pGameFlags;

extern int* g_piMoney;
extern int* g_piExperience;

extern HWND g_hWnd;
extern HINSTANCE g_hInstance;
extern HANDLE* g_phHeaps;
extern LPSTR g_szDataDirectoryPath;
extern LPSTR g_szCRILogBuffer;
extern std::vector<LPTOP_LEVEL_EXCEPTION_FILTER> g_pExceptionHandlers;
extern std::vector<MrubyImpl*> g_pRubyInstances;

//extern void* g_pTextureVtbl;

extern Pl0000* g_pLocalPlayer;
extern EntityHandle* g_pLocalPlayerHandle;
extern EntityHandle* g_pEmilHandle;
extern CEntityList* g_pEntityList;
extern CItemManager* g_pItemManager;
extern CYorhaManager* g_pYorhaManager;
extern CUserManager* g_pUserManager;
extern CNPCManager* g_pNPCManager;
extern CEmBaseManager* g_pEnemyManager;
extern CWetObjManager* g_pWetObjectManager;
extern CCollisionDataObjectManager* g_pCollisionDataObjectManager;
extern CTextureResourceManager* g_pTextureResourceManager;
extern COtManager* g_pOtManager;
extern CModelManager* g_pModelManager;
extern CModelAnalyzer* g_pModelAnalyzer;
extern CCameraGame* g_pCamera;
extern VMatrix* g_pViewMatrix;
extern CSceneStateSystem* g_pSceneStateSystem;
extern CSceneEntitySystem* g_pSceneEntitySystem;
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
extern ID3D11RenderTargetView* g_pRenderTargetView;
extern ID3D11RasterizerState* g_pRenderWireframeState;
extern ID3D11RasterizerState* g_pRenderSolidState;
extern ID3D11DepthStencilState* g_pDepthStencilStates[_DEPTH_COUNT];
extern ID3D11SamplerState* g_pSamplerState;
extern ID3D11Texture2D* g_pTexRed;
extern ID3D11Texture2D* g_pTexGreen;
extern ID3D11ShaderResourceView* g_pRedSRV;
extern ID3D11ShaderResourceView* g_pGreenSRV;
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
extern NOP_MEMORY nop_Framecap[2];
extern HOOK_FUNC g_CreateEntityHook;
extern HOOK_FUNC g_LoadWordBlacklist;
extern HOOK_FUNC g_UserManagerSaveFileIO;