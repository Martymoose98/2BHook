#include "Globals.h"

CalculateLevelFn CalculateLevel;
GetEntityFromHandleFn GetEntityFromHandle;
SetLocalPlayerFn SetLocalPlayer;
ResetCameraFn ResetCamera;
ChangePlayerFn ChangePlayer;
DestroyBuddyFn DestroyBuddy;
SetSceneEntityFn SetSceneEntity;
WetObjectManager_SetWetFn WetObjectManager_SetWet;
WetObjectManager_SetDryFn WetObjectManager_SetDry;
WetObjectManager_AddLocalEntityFn WetObjectManager_AddLocalEntity;
GetConstructorFn GetConstructionInfo;
FindSceneStateFn FindSceneState;
HashStringCRC32Fn HashStringCRC32;
FNV1HashFn FNV1Hash;
CpkMountFn CpkMount;

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
HANDLE* g_pHeaps;
LPSTR g_szDataDirectoryPath;
std::vector<LPTOP_LEVEL_EXCEPTION_FILTER> g_pExceptionHandlers;


Pl0000* g_pLocalPlayer;
EntityHandle* g_pLocalPlayerHandle;
EntityHandle* g_pEmilHandle;
EntityInfoList* g_pEntityInfoList;
CUserManager* g_pUserManager;
YorhaManager* g_pYorhaManager;
NPCManager* g_pNPCManager;
EmBaseManager* g_pEnemyManager;
WetObjManager* g_pWetObjectManager;
CCameraGame* g_pCamera;
VMatrix* g_pViewMatrix;
CSceneStateSystem* g_pSceneStateSystem;
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
ID3D11Buffer* g_pVertexBuffers[8];
D3D11_BUFFER_DESC g_VertexBufferDesc[8];
UINT g_VertexBuffersOffset[8];
UINT g_Stride[8];
ID3D11Buffer* g_pIndexBuffer;
D3D11_BUFFER_DESC g_IndexBufferDesc;
DXGI_FORMAT g_IndexFormat;
UINT g_IndexOffset;
UINT g_StartSlot;
ID3D11Buffer* g_pPixelShaderBuffer;
D3D11_BUFFER_DESC g_PixelShaderBufferDesc;
UINT g_PixelShaderStartSlot;
D3D11_SHADER_RESOURCE_VIEW_DESC g_ShaderResViewDesc;

IDXGISwapChain* g_pSwapChain;
IDXGISwapChain* g_pSecondarySwapChain;
IDXGIFactory* g_pFactory;
CGraphicDeviceDx11* g_pGraphicDevice;

VirtualTableHook* g_pFactoryHook;
VirtualTableHook* g_pSwapChainHook;
VirtualTableHook* g_pDeviceContextHook;
VirtualTableHook* g_pKeyboardHook;
VirtualTableHook* g_pMouseHook;

ImportTableHook* g_pQueryPerformanceCounterHook;
ImportTableHook* g_pClipCursorHook;
ImportTableHook* g_pXInputGetStateHook;
ImportTableHook* g_pSetUnhandledExceptionFilterHook;

BYTE_PATCH_MEMORY bp_save_file_io;
BYTE_PATCH_MEMORY bp_UpdateModelParts;
BYTE_PATCH_MEMORY bp_CreateEntity[2];
BYTE_PATCH_MEMORY bp_query_performance_counter;
BYTE_PATCH_MEMORY bp_Framecap;
BYTE_PATCH_MEMORY bp_NoTutorialDialogs;
NOP_MEMORY nop_HairColor;
NOP_MEMORY nop_Health[2];
NOP_MEMORY nop_neon_scenery;
NOP_MEMORY nop_Framecap[2];