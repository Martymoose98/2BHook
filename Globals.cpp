#include "Globals.h"

CalculateLevelFn CalculateLevel;
GetEntityInfoFromHandleFn GetEntityInfoFromHandle;
GetEntityFromHandleFn GetEntityFromHandle;
GetEntityFromHandleFn GetEntityFromHandle2;
CItemManager_GetItemNameByIdFn ItemManager_GetItemNameById;
CItemManager_GetItemIdByNameFn ItemManager_GetItemIdByName;
CItemManager_AddItemFn ItemManager_AddItem;
CItemManager_UseItemFn ItemManager_UseItem;
SetLocalPlayerFn SetLocalPlayer;
ResetCameraFn ResetCamera;
ChangePlayerFn ChangePlayer;
DestroyBuddyFn DestroyBuddy;
SetSceneEntityFn SetSceneEntity;
CSaveDataDevice_SaveFileIOFn SaveFileIO;
CSaveDataDevice_ReadSaveSlotsFn ReadSaveSlots;
CSaveDataDevice_ReadSaveDataFn ReadSaveData;
CSaveDataDevice_WriteSaveDataFn WriteSaveData;
CSaveDataDevice_DeleteSaveDataFn DeleteSaveData;
CWetObjectManager_SetWetFn WetObjectManager_SetWet;
CWetObjectManager_SetDryFn WetObjectManager_SetDry;
CWetObjectManager_AddLocalEntityFn WetObjectManager_AddLocalEntity;
CSceneEntitySystem_CreateEntityFn SceneEntitySystem_CreateEntity;
CSceneStateSystem_SetFn SceneStateSystem_Set;
CCameraGame_SetLookAtFn CameraGame_SetLookAt;
GetConstructorFn GetConstructionInfo;
CSceneEntitySystem_FindSceneStateFn SceneEntitySystem_FindSceneState;
HashStringCRC32Fn HashStringCRC32;
UnlockAchievementFn UnlockAchievement;
FNV1HashFn FNV1Hash;
QueryHeapFn QueryHeap;
GetWorkFn GetWork;
PreloadFileFn PreloadFile;
ObjReadSystem_RequestEndFn RequestEnd;
ObjReadSystem_PreloadModelFn PreloadModel; 
COtManager_GetGraphicCommandFn GetGraphicCommand;
CreateTextureFn CreateTexture;
CTextureResourceManager_FindResourceFn TextureResourceManager_FindResource;
CreateUIFromIdFn CreateUIFromId;
ExCollision_GetOBBMaxFn ExCollision_GetOBBMax;
CpkMountFn CpkMount;
CRILogCallbackFn CRILogCallback;

XInputGetStateExFn XInputGetStateEx;
XInputWaitForGuideButtonFn InputWaitForGuideButton;
XInputCancelGuideButtonWaitFn XInputCancelGuideButtonWait;
XInputPowerOffControllerFn XInputPowerOffController;
XInputGetBaseBusInformationFn XInputGetBaseBusInformation;
XInputGetCapabilitiesExFn InputGetCapabilitiesEx;

int* g_piMoney;
int* g_piExperience;

HWND g_hWnd;
HINSTANCE g_hInstance;
HANDLE* g_phHeaps;
LPSTR g_szDataDirectoryPath;
LPSTR g_szCRILogBuffer;
std::vector<LPTOP_LEVEL_EXCEPTION_FILTER> g_pExceptionHandlers;
std::vector<MrubyImpl*> g_pRubyInstances;

Pl0000* g_pLocalPlayer;
EntityHandle* g_pLocalPlayerHandle;
EntityHandle* g_pEmilHandle;
CEntityList* g_pEntityList;
CItemManager* g_pItemManager;
CUserManager* g_pUserManager;
CYorhaManager* g_pYorhaManager;
CNPCManager* g_pNPCManager;
CEmBaseManager* g_pEnemyManager;
CWetObjManager* g_pWetObjectManager;
CCollisionDataObjectManager* g_pCollisionDataObjectManager;
CTextureResourceManager* g_pTextureResourceManager;
COtManager* g_pOtManager;
CModelManager* g_pModelManager;
CModelAnalyzer* g_pModelAnalyzer;
CCameraGame* g_pCamera;
VMatrix* g_pViewMatrix;
CSceneStateSystem* g_pSceneStateSystem;
CSceneEntitySystem* g_pSceneEntitySystem;
CNetworkDevice* g_pNetworkDevice;
CMemoryDevice* g_pMemoryDevice;
BYTE* g_pDecreaseHealth[2];
BYTE* g_pAntiVSync; 
BYTE* g_pAntiFramerateCap_Sleep;
BYTE* g_pAntiFramerateCap_Spinlock;
BYTE* g_pAntiFramerateCap_Test4;
IDirectInput8A* g_pDirectInput8;
Keyboard_t* g_pKeyboard;
Mouse_t* g_pMouse;
CGraphics* g_pGraphics;

ID3D11Device* g_pDevice;
ID3D11DeviceContext* g_pDeviceContext;
ID3D11RenderTargetView* g_pRenderTargetView;
ID3D11PixelShader* g_pRed;
ID3D11PixelShader* g_pGreen;
ID3D11RasterizerState* g_pRenderWireframeState;
ID3D11RasterizerState* g_pRenderSolidState;
ID3D11DepthStencilState* g_pDepthStencilStates[_DEPTH_COUNT];
ID3D11SamplerState* g_pSamplerState;
ID3D11Texture2D* g_pTexRed;
ID3D11Texture2D* g_pTexGreen;
ID3D11ShaderResourceView* g_pRedSRV;
ID3D11ShaderResourceView* g_pGreenSRV;

IDXGISwapChain* g_pSwapChain;
IDXGISwapChain* g_pSecondarySwapChain;
IDXGIFactory* g_pFactory;
CGraphicDeviceDx11* g_pGraphicDevice;

VirtualTableHook* g_pFactoryHook;
VirtualTableHook* g_pSwapChainHook;
VirtualTableHook* g_pDeviceContextHook;
VirtualTableHook* g_pKeyboardHook;
VirtualTableHook* g_pMouseHook;
VirtualTableHook* g_pCameraHook;
VirtualTableHook* g_pModelAnalyzerHook;
MemoryDeviceHook* g_pMemoryDeviceHook;
std::vector<VirtualTableHook*> g_pRubyInstancesHooks;

MemoryDeviceHook* g_pMemoryDevHook;

ImportTableHook* g_pQueryPerformanceCounterHook;
ImportTableHook* g_pClipCursorHook;
ImportTableHook* g_pXInputGetStateHook;
ImportTableHook* g_pSetUnhandledExceptionFilterHook;
ImportTableHook* g_pOleLoadPictureHook;

BYTE_PATCH_MEMORY bp_UpdateModelParts;
BYTE_PATCH_MEMORY bp_query_performance_counter;
BYTE_PATCH_MEMORY bp_Framecap;
BYTE_PATCH_MEMORY bp_NoTutorialDialogs;
NOP_MEMORY nop_HairColor;
NOP_MEMORY nop_Health[2];
NOP_MEMORY nop_neon_scenery;
NOP_MEMORY nop_Framecap[2];
HOOK_FUNC g_CreateEntityHook;
HOOK_FUNC g_LoadWordBlacklist;
HOOK_FUNC g_UserManagerSaveFileIO;