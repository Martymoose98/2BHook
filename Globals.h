#pragma once
#include <Windows.h>
#include <d3d11.h>
#include <dinput.h>
#include <xinput.h>

#include <vector> //should be temp

#include "ReversedStructs.h"
#include "VirtualTableHook.h"
#include "ImportTableHook.h"

#define MAX_LEVELS 99
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

#define CREATE_ENTITY(pc) ((CSceneSceneSystem__CreateEntityFn)(0x1404F9AA0))((void*)0x14160DFE0, pc)

#define KEYDOWN(name, key) (name[key] & 0x80)
#define KEYPRESSED(c, o, key) (KEYDOWN(c, key) && !KEYDOWN(o, key))

// This is for d3d11
enum eDepthState
{
	ENABLED,
	DISABLED,
	READ_NO_WRITE,
	NO_READ_NO_WRITE,
	ENABLED1,
	ENABLED2,
	ENABLED3,
	ENABLED4,
	ENABLED5,
	ENABLED6,
	ENABLED7,
	ENABLED8,
	READ_NO_WRITE1,
	READ_NO_WRITE2,
	READ_NO_WRITE3,
	READ_NO_WRITE4,
	READ_NO_WRITE5,
	READ_NO_WRITE6,
	READ_NO_WRITE7,
	READ_NO_WRITE8,
	_DEPTH_COUNT
};

struct CpkEntry;

typedef ULONGLONG QWORD;

typedef Level_t*(*CalculateLevelFn)(ExExpInfo* pInfo, int experience);
typedef Pl0000*(*GetEntityFromHandleFn)(EntityHandle* pHandle);
typedef BOOL(*ObjectIdToObjectNameFn)(char* szObjectName, size_t size, int objectId); //0x140628940
typedef int(*GetItemIdByNameFn)(void*, const char* szItemName);			//returns a sItem* maybe?
typedef const char*(*GetItemByIdFn)(__int64 thisrcx, int item_id);	//returns a sItem* maybe?
typedef bool(*AddItemFn)(__int64 pItemManager, int item_id, int quantity);
typedef bool(*UseItemFn)(__int64 pItemManager, int item_id);
typedef void(*ChangePlayerFn)(Pl0000* pEntity);
typedef __int64(*SetLocalPlayerFn)(EntityHandle* pHandle);
typedef BOOL(*UnlockAchievementFn)(__int64, __int64, unsigned int uAchievement);
typedef void(*WetObjectManager_AddLocalEntityFn)(__int64, EntityInfo* pInfo);
typedef void(*WetObjectManager_SetWetFn)(__int64 pThis, byte wet_level, int index);
typedef void(*WetObjectManager_SetDryFn)(__int64 pThis, EntityInfo *pInfo);
typedef void(*ResetCameraFn)(CCameraGame* pCamera);
typedef bool(*DestroyBuddyFn)(Pl0000* pBuddy);
typedef __int64(*NPC_ChangeSetTypeFollowFn)(Pl0000* pNPC);
typedef __int64(*NPC_ChangeSetTypeIdleFn)(Pl0000* pNPC);
typedef __int64(*EmitSoundFn)(Sound* pSound, Pl0000** ppSourceEntity); // also second arg is another custom struct
typedef __int64(*PlaySoundFn)(Sound* pSound);
typedef unsigned int(*HashStringCRC32Fn)(const char* szName, __int64 length);
typedef unsigned int(*FNV1HashFn)(const char* szString);
typedef __int64(*HeapInstance_ReserveMemoryFn)(CHeapInstance* pThis, HeapAlloc_t* pMemory, __int64 nReserveBytes, void* pUnknown, unsigned int flags, void* pStruct);
typedef void*(*FindSceneStateFn)(CRITICAL_SECTION* pCriticalSection, unsigned int crc, const char* szName, __int64 length); // not sure if it actually finds the heap (it might just find the scene state) 0x1400538A0
typedef bool(*SceneStateSystem_SetFn)(/*hap::scene_state::SceneStateSystem* */void* pThis, SceneState** ppSceneState);
typedef bool(*SceneStateSystem_SetInternalFn)(/*hap::scene_state::SceneStateSystem* */void* pThis, SceneState** ppSceneState);
typedef BOOL(*SceneStateUnkFn)(void* unused, void* pSceneState);	//SceneStateUnkFn = 0x140053B00
typedef void(*SetSceneEntityFn)(const char*, EntityInfo*);
typedef __int64(*CallTutorialDialogFn)(__int64, unsigned int dialogId); //callTutorialDialog address = 0x1401B1F30
typedef bool(*QuestState_RequestStateInternalFn)(DWORD *pQuestId);
typedef ConstructionInfo<void>*(*GetConstructorFn)(int objectId); //0x1401A2C20  templates are shit tbh
typedef EntityInfo*(*SceneEntitySystem_CreateEntityFn)(CSceneEntitySystem* pThis, Create_t* pCreate);
typedef void*(*AllocHeapMemoryFn)(QWORD size, CHeapInstance** ppHeap);

typedef void(*ExCollision_GetOBBMaxFn)(ExCollision* pThis, Vector3Aligned* pvMax);

typedef HeapThing*(*QueryHeapFn)(HeapThing* pResult, int objectid, int a3); // or void i guess
typedef ObjReadSystem::Work*(*GetWorkFn)(int objectid);
typedef ObjReadSystem::Work::Desc*(*PreloadFileFn)(__int64 thisrcx, int filetype, const char* szFilename, void* pHeap, byte flag, ObjReadSystem::Work* pWork);
typedef void(*ObjReadSystem_RequestEndFn)(__int64 a1, int objectId);
typedef BOOL(*ObjReadSystem_PreloadModelFn)(ObjReadSystem::Work* pWork);
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
extern GetEntityFromHandleFn GetEntityFromHandle;
extern SetLocalPlayerFn SetLocalPlayer;
extern ResetCameraFn ResetCamera;
extern ChangePlayerFn ChangePlayer;
extern DestroyBuddyFn DestroyBuddy;
extern WetObjectManager_SetWetFn WetObjectManager_SetWet;
extern WetObjectManager_SetDryFn WetObjectManager_SetDry;
extern WetObjectManager_AddLocalEntityFn WetObjectManager_AddLocalEntity;
extern ExCollision_GetOBBMaxFn GetOBBMax;
extern SetSceneEntityFn SetSceneEntity;
extern GetConstructorFn GetConstructionInfo;
extern FindSceneStateFn FindSceneState;
extern HashStringCRC32Fn HashStringCRC32;
extern UnlockAchievementFn UnlockAchievement;
extern FNV1HashFn FNV1Hash;
extern QueryHeapFn QueryHeap;
extern GetWorkFn GetWork;
extern PreloadFileFn PreloadFile;
extern ObjReadSystem_RequestEndFn RequestEnd;
extern ObjReadSystem_PreloadModelFn PreloadModel;
extern CpkMountFn CpkMount;
extern CRILogCallbackFn CRILogCallback;

// XInput Functions

extern XInputGetStateExFn XInputGetStateEx;
extern XInputWaitForGuideButtonFn InputWaitForGuideButton;
extern XInputCancelGuideButtonWaitFn XInputCancelGuideButtonWait;
extern XInputPowerOffControllerFn XInputPowerOffController;
extern XInputGetBaseBusInformationFn XInputGetBaseBusInformation;
extern XInputGetCapabilitiesExFn InputGetCapabilitiesEx;

extern int* g_piMoney;
extern int* g_piExperience;

extern HWND g_hWnd;
extern HINSTANCE g_hInstance;
extern HANDLE* g_pHeaps;
extern LPSTR g_szDataDirectoryPath;
extern std::vector<LPTOP_LEVEL_EXCEPTION_FILTER> g_pExceptionHandlers;
extern std::vector<MrubyImpl*> g_pRubyInstances;

extern Pl0000* g_pLocalPlayer;
extern EntityHandle* g_pLocalPlayerHandle;
extern EntityHandle* g_pEmilHandle;
extern EntityInfoList* g_pEntityInfoList;
extern YorhaManager* g_pYorhaManager;
extern CUserManager* g_pUserManager;
extern NPCManager* g_pNPCManager;
extern EmBaseManager* g_pEnemyManager;
extern WetObjManager* g_pWetObjectManager;
extern CCollisionDataObjectManager* g_pCollisionDataObjectManager;
extern CCameraGame* g_pCamera;
extern VMatrix* g_pViewMatrix;
extern CSceneStateSystem* g_pSceneStateSystem;
extern CSceneEntitySystem* g_pSceneEntitySystem;
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
extern ID3D11Buffer* g_pVertexBuffers[8];
extern D3D11_BUFFER_DESC g_VertexBufferDesc[8];
extern UINT g_VertexBuffersOffset[8];
extern UINT g_Stride[8];
extern ID3D11Buffer* g_pIndexBuffer;
extern D3D11_BUFFER_DESC g_IndexBufferDesc;
extern DXGI_FORMAT g_IndexFormat;
extern UINT g_IndexOffset;
extern UINT g_StartSlot;
extern ID3D11Buffer* g_pPixelShaderBuffer;
extern D3D11_BUFFER_DESC g_PixelShaderBufferDesc;
extern UINT g_PixelShaderStartSlot;
extern D3D11_SHADER_RESOURCE_VIEW_DESC g_ShaderResViewDesc;
extern IDXGISwapChain* g_pSwapChain;
extern IDXGISwapChain* g_pSecondarySwapChain;
extern IDXGIFactory* g_pFactory;
extern CGraphicDeviceDx11* g_pGraphicDevice;

extern VirtualTableHook* g_pFactoryHook;
extern VirtualTableHook* g_pSwapChainHook;
extern VirtualTableHook* g_pDeviceContextHook;
extern VirtualTableHook* g_pMouseHook;
extern VirtualTableHook* g_pKeyboardHook;
extern std::vector<VirtualTableHook*> g_pRubyInstancesHooks;

extern ImportTableHook* g_pQueryPerformanceCounterHook;
extern ImportTableHook* g_pClipCursorHook;
extern ImportTableHook* g_pXInputGetStateHook;
extern ImportTableHook* g_pSetUnhandledExceptionFilterHook;

extern BYTE_PATCH_MEMORY bp_save_file_io;
extern BYTE_PATCH_MEMORY bp_UpdateModelParts;
extern BYTE_PATCH_MEMORY bp_CreateEntity[2];
extern BYTE_PATCH_MEMORY bp_query_performance_counter;
extern BYTE_PATCH_MEMORY bp_Framecap;
extern BYTE_PATCH_MEMORY bp_NoTutorialDialogs;
extern NOP_MEMORY nop_HairColor;
extern NOP_MEMORY nop_Health[2];
extern NOP_MEMORY nop_neon_scenery;
extern NOP_MEMORY nop_Framecap[2];
