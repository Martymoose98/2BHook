#include "Globals.h"

CalculateLevelFn CalculateLevel;
GetEntityFromHandleFn GetEntityFromHandle;
SetLocalPlayerFn SetLocalPlayer;
ChangePlayerFn ChangePlayer;
DestroyBuddyFn DestroyBuddy;
GetConstructorFn GetConstructionInfo;
FindSceneStateFn FindSceneState;
HashStringCRC32Fn HashStringCRC32;
FNV1HashFn FNV1Hash;

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

Entity_t* g_pLocalPlayer;
EntityHandle* g_pLocalPlayerHandle;
CUserManager* g_pUserManager;
YorhaManager* g_pYorhaManager;
NPCManager* g_pNPCManager;
EmBaseManager* g_pEnemyManager;
CCamera* g_pCamera;
CSceneStateSystem* g_pSceneStateSystem;
BYTE* g_pDecreaseHealth[2];
BYTE* g_pAntiVSync; 
BYTE* g_pAntiFramerateCap_Sleep;
BYTE* g_pAntiFramerateCap_Spinlock;
BYTE* g_pAntiFramerateCap_Test4;
IDirectInput8A* g_pDirectInput8;
IDirectInputDevice8A* g_pKeyboard;
Mouse_t* g_pMouse;
CGraphics* g_pGraphics;
ID3D11Device* g_pDevice;
ID3D11DeviceContext* g_pDeviceContext;
ID3D11RenderTargetView* g_pRenderTargetView;
ID3D11RasterizerState* g_pRenderWireframeState;
ID3D11RasterizerState* g_pRenderSolidState;
ID3D11DepthStencilState* g_pDepthStencilStates[_DEPTH_COUNT];
ID3D11PixelShader* g_pPixelShaderRed;
ID3D11PixelShader* g_pPixelShaderGreen;
ID3D11Buffer* g_pVertexBuffers;
D3D11_BUFFER_DESC g_VertexBufferDesc;
UINT g_VertexBuffersOffset;
UINT g_Stride;
ID3D11Buffer* g_pIndexBuffer;
D3D11_BUFFER_DESC g_IndexBufferDesc;
DXGI_FORMAT g_IndexFormat;
UINT g_IndexOffset;
UINT g_StartSlot;

IDXGISwapChain* g_pSwapChain;
IDXGISwapChain* g_pSecondarySwapChain;
IDXGIFactory* g_pFactory;
CGraphicDeviceDx11* g_pCGraphicDevice;

VirtualTableHook* g_pFactoryHook;
VirtualTableHook* g_pSwapChainHook;
VirtualTableHook* g_pDeviceContextHook;
VirtualTableHook* g_pMouseHook;
VirtualTableHook* g_pKeyboardHook;

ImportTableHook* g_pQueryPerformanceCounterHook;
ImportTableHook* g_pClipCursorHook;
ImportTableHook* g_pXInputGetStateHook;

BYTE_PATCH_MEMORY bp_save_file_io;
BYTE_PATCH_MEMORY bp_CreateEntity[2];
BYTE_PATCH_MEMORY bp_query_performance_counter;
BYTE_PATCH_MEMORY bp_AntiVSync;
BYTE_PATCH_MEMORY bp_Framecap;
BYTE_PATCH_MEMORY bp_NoTutorialDialogs;
BYTE_PATCH_MEMORY bp_HairColor;
NOP_MEMORY nop_Health[2];
NOP_MEMORY nop_neon_scenery;
NOP_MEMORY nop_Framecap[2];