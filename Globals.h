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

#define PROTAGONIST_NONE NULL
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

#define MODELTYPE_2B_NORMAL 0x21020
#define MODELTYPE_2B_SUIT	0xA2140

#define NOP_DAMAGE_ENEMY 0
#define NOP_DAMAGE_WORLD 1

#define NOP_FRAMECAP_SLEEP 0
#define NOP_FRAMECAP_SPINLOCK 1


#define XINPUT_GAMEPAD_GUIDE 0x400

#define CREATE_ENTITY(pc) ((MakeEntityFn)(0x1404F9AA0))((void*)0x14160DFE0, pc)

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

typedef Level_t*(__fastcall* CalculateLevelFn)(ExExpInfo* pInfo, int experience);
typedef Pl0000*(__fastcall* GetEntityFromHandleFn)(EntityHandle* pHandle);
typedef BOOL(__fastcall* ObjectIdToObjectNameFn)(char* szObjectName, size_t size, int objectId); //0x140628940
typedef int(__fastcall*  GetItemIdByNameFn)(void*, const char* szItemName);			//returns a sItem* maybe?
typedef const char*(__fastcall* GetItemByIdFn)(__int64 thisrcx, int item_id);	//returns a sItem* maybe?
typedef bool(__fastcall* AddItemFn)(__int64 pItemManager, int item_id);
typedef bool(__fastcall* UseItemFn)(__int64 pItemManager, int item_id);
typedef void(__fastcall* ChangePlayerFn)(Pl0000* pEntity);
typedef __int64(__fastcall* SetLocalPlayerFn)(EntityHandle* pHandle);
typedef void(__fastcall* ResetCameraFn)(CCamera* pCamera);
typedef bool(__fastcall* DestroyBuddyFn)(Pl0000* pBuddy);
typedef __int64(__fastcall* NPC_ChangeSetTypeFollowFn)(Pl0000* pNPC);
typedef __int64(__fastcall* NPC_ChangeSetTypeIdleFn)(Pl0000* pNPC);
typedef __int64(__fastcall* EmitSoundFn)(Sound* pSound, Pl0000** ppSourceEntity); // also second arg is another custom struct
typedef __int64(__fastcall* PlaySoundFn)(Sound* pSound);
typedef unsigned int(__fastcall* HashStringCRC32Fn)(const char* szName, __int64 length);
typedef unsigned int(__fastcall* FNV1HashFn)(const char* szString);
typedef __int64(__fastcall* HeapInstance_ReserveMemoryFn)(CHeapInstance* pThis, HeapAlloc_t* pMemory, __int64 nReserveBytes, void* pUnknown, unsigned int flags, void* pStruct);
typedef void*(__fastcall* FindSceneStateFn)(CRITICAL_SECTION* pCriticalSection, unsigned int crc, const char* szName, __int64 length); // not sure if it actually finds the heap (it might just find the scene state) 0x1400538A0
typedef bool(__fastcall* SceneStateSystem_SetFn)(/*hap::scene_state::SceneStateSystem* */void* pThis, SceneState** ppSceneState);
typedef bool(__fastcall* SceneStateSystem_SetInternalFn)(/*hap::scene_state::SceneStateSystem* */void* pThis, SceneState** ppSceneState);
typedef BOOL(__fastcall* SceneStateUnkFn)(void* unused, void* pSceneState);	//SceneStateUnkFn = 0x140053B00
typedef __int64(__fastcall* CallTutorialDialogFn)(__int64, unsigned int dialogId); //callTutorialDialog address = 0x1401B1F30
typedef bool(__fastcall* QuestState_RequestStateInternalFn)(DWORD *pQuestId);
typedef ConstructionInfo<void>*(__fastcall* GetConstructorFn)(int objectId); //0x1401A2C20  templates are shit tbh
typedef EntityInfo*(__fastcall* MakeEntityFn)(void*, Create_t* pCreate);
typedef void* (__fastcall* AllocHeapMemoryFn)(QWORD size, CHeapInstance** ppHeap);

typedef CpkEntry*(__fastcall* CpkMountFn)(int iLoadOrder, char* szPath); // 0x140956D70 
typedef BOOL(__fastcall* CpkMount2Fn)(CpkMountInfo* pCpk); // 0x140644000

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
extern GetConstructorFn GetConstructionInfo;
extern FindSceneStateFn FindSceneState;
extern HashStringCRC32Fn HashStringCRC32;
extern FNV1HashFn FNV1Hash;
extern CpkMountFn CpkMount;

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

extern Pl0000* g_pLocalPlayer;
extern EntityHandle* g_pLocalPlayerHandle;
extern EntityInfoList* g_pEntityInfoList;
extern YorhaManager* g_pYorhaManager;
extern CUserManager* g_pUserManager;
extern NPCManager* g_pNPCManager;
extern EmBaseManager* g_pEnemyManager;
extern CCamera* g_pCamera;
extern CSceneStateSystem* g_pSceneStateSystem;
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
extern ID3D11Buffer* g_pVertexBuffers;
extern D3D11_BUFFER_DESC g_VertexBufferDesc;
extern UINT g_VertexBuffersOffset;
extern UINT g_Stride;
extern ID3D11Buffer* g_pIndexBuffer;
extern D3D11_BUFFER_DESC g_IndexBufferDesc;
extern DXGI_FORMAT g_IndexFormat;
extern UINT g_IndexOffset;
extern UINT g_StartSlot;

extern IDXGISwapChain* g_pSwapChain;
extern IDXGISwapChain* g_pSecondarySwapChain;
extern IDXGIFactory* g_pFactory;
extern CGraphicDeviceDx11* g_pGraphicDevice;

extern VirtualTableHook* g_pFactoryHook;
extern VirtualTableHook* g_pSwapChainHook;
extern VirtualTableHook* g_pDeviceContextHook;
extern VirtualTableHook* g_pMouseHook;
extern VirtualTableHook* g_pKeyboardHook;

extern ImportTableHook* g_pQueryPerformanceCounterHook;
extern ImportTableHook* g_pClipCursorHook;
extern ImportTableHook* g_pXInputGetStateHook;
extern ImportTableHook* g_pSetUnhandledExceptionFilterHook;

extern BYTE_PATCH_MEMORY bp_save_file_io;
extern BYTE_PATCH_MEMORY bp_UpdateModelParts;
extern BYTE_PATCH_MEMORY bp_CreateEntity[2];
extern BYTE_PATCH_MEMORY bp_query_performance_counter;
extern BYTE_PATCH_MEMORY bp_AntiVSync;
extern BYTE_PATCH_MEMORY bp_Framecap;
extern BYTE_PATCH_MEMORY bp_NoTutorialDialogs;
extern NOP_MEMORY nop_HairColor;
extern NOP_MEMORY nop_Health[2];
extern NOP_MEMORY nop_neon_scenery;
extern NOP_MEMORY nop_Framecap[2];
