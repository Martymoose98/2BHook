#pragma once
#include <Windows.h>
#include <OleCtl.h>
#include <d3d11.h>
#include <Xinput.h>
#include "ImGui/imgui.h"
#include "ImGui/backends/imgui_impl_dx11.h"
#include "ImGui/backends/imgui_impl_win32.h"
#include <Fluorine\VirtualTableHook.h>
#include "Console.h"
#include "Log.h"
#include "Overlay.h"
#include "Renderer.h"
#include "Math.h"
#include "Menu.h"
#include "Globals.h"
#include "Configuration.h"
#include "Utils.h"


typedef HRESULT(*PresentFn)(IDXGISwapChain* pThis, UINT SyncInterval, UINT Flags);
typedef HRESULT(*ResizeBuffersFn)(IDXGISwapChain* pThis, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags);
typedef HRESULT(*CreateSwapChainFn)(IDXGIFactory* pThis, IUnknown* pDevice, DXGI_SWAP_CHAIN_DESC* pDesc, IDXGISwapChain** ppSwapChain);
typedef void(*DrawIndexedFn)(ID3D11DeviceContext* pThis, UINT IndexCount, UINT StartIndexLocation, INT BaseVertexLocation);
typedef void(*DrawFn)(ID3D11DeviceContext* pThis, UINT VertexCount, UINT StartVertexLocation);
typedef void(*ClearRenderTargetViewFn)(ID3D11DeviceContext* pThis, ID3D11RenderTargetView* pRenderTargetView, const FLOAT ColorRGBA[4]);
typedef void(*PSSetShaderResourcesFn)(ID3D11DeviceContext* pThis, UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView* const* ppShaderResourceViews);
typedef ULONG(*ReleaseFn)(IDirectInputDevice8A* pThis);
typedef HRESULT(*AcquireFn)(IDirectInputDevice8A* pThis);
typedef HRESULT(*GetDeviceStateFn)(IDirectInputDevice8A* pThis, DWORD cbData, LPVOID lpvData);
typedef HRESULT(*OleLoadPictureFn)(LPSTREAM lpstream, LONG lSize, BOOL fRunmode, REFIID riid, LPVOID* lplpvObj);
typedef BOOL(*QueryPerformaceCounterFn)(LARGE_INTEGER* lpPerfomaceCount);
typedef LPTOP_LEVEL_EXCEPTION_FILTER(*SetUnhandledExceptionFilterFn)(LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter);
typedef BOOL(*SetCursorPosFn)(int X, int Y);
typedef DWORD(*XInputGetStateFn)(DWORD dwUserIndex, PXINPUT_STATE pState);

typedef void(*UpdateModelPartsFn)(Pl0000* pEntity);

#ifdef DENUVO_STEAM_BUILD
typedef void* (*CreateEntityFn)(void* pUnknown, CEntityInfo* pInfo, unsigned int objectId, int iGroupId, CHeapInfo* pHeapInfo);
#else
typedef void* (*CreateEntityFn)(void* pUnknown, CEntityInfo* pInfo, unsigned int objectId, int iGroupId);
#endif

typedef BOOL(*LoadWordBlacklistFn)(BannedWordChecker* pThis, __int64 thisrdx, QWORD* a3, const char* szBlacklistName);

typedef bool(*MRubyLoadScriptFn)(MrubyImpl* pThis, MrubyScript* pScript);
typedef BOOL(*CCameraGame_SetViewAnglesFn)(CCameraGame* pCamera);
typedef void* (*CCameraGame_MoveFn)(CCameraGame* pCamera);

typedef CModelShaderModule* (*CModelAnalyzer_CreateModelShaderModuleFn)(CModelAnalyzer* pThis, CModelShader* pShader, DWORD* a3, CHeapInfo* a4);

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

extern PresentFn oPresent;
extern ResizeBuffersFn oResizeBuffers;
extern CreateSwapChainFn oCreateSwapChain;
extern DrawIndexedFn oDrawIndexed;
extern DrawFn oDraw;
extern PSSetShaderResourcesFn oPSSetShaderResources;
extern ClearRenderTargetViewFn oClearRenderTargetView;
extern ReleaseFn oKeyboardRelease;
extern AcquireFn oKeyboardAcquire;
extern GetDeviceStateFn oKeyboardGetDeviceState;
extern ReleaseFn oMouseRelease;
extern AcquireFn oMouseAcquire;
extern GetDeviceStateFn oMouseGetDeviceState;
extern QueryPerformaceCounterFn oQueryPerformanceCounter;
extern SetUnhandledExceptionFilterFn oSetUnhandledExceptionFilter;
extern SetCursorPosFn oSetCursorPos;
extern XInputGetStateFn oXInputGetState;
extern UpdateModelPartsFn oUpdateModelParts;
extern CreateEntityFn oCreateEntity;
extern LoadWordBlacklistFn oLoadWordBlacklist;
extern MRubyLoadScriptFn oMRubyLoadScript;
extern CCameraGame_SetViewAnglesFn oCCameraGameSetViewAngles;
extern CCameraGame_MoveFn oCCameraGameMove;
extern CModelAnalyzer_CreateModelShaderModuleFn oCreateModelShaderModule;
extern WNDPROC oWndProc;


HRESULT hkPresent(IDXGISwapChain* pThis, UINT SyncInterval, UINT Flags);
HRESULT hkResizeBuffers(IDXGISwapChain* pThis, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags);
HRESULT hkCreateSwapChain(IDXGIFactory* pThis, IUnknown* pDevice, DXGI_SWAP_CHAIN_DESC* pDesc, IDXGISwapChain** ppSwapChain);
void hkPSSetShaderResources(ID3D11DeviceContext* pThis, UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView* const* ppShaderResourceViews);
void hkDraw(ID3D11DeviceContext* pThis, UINT VertexCount, UINT StartVertexLocation);
void hkDrawIndexed(ID3D11DeviceContext* pThis, UINT IndexCount, UINT StartIndexLocation, INT BaseVertexLocation);
void hkClearRenderTargetView(ID3D11DeviceContext* pThis, ID3D11RenderTargetView* pRenderTargetView, const FLOAT ColorRGBA[4]);
ULONG hkKeyboardRelease(IDirectInputDevice8A* pThis);
HRESULT hkKeyboardAcquire(IDirectInputDevice8A* pThis);
HRESULT hkKeyboardGetDeviceState(IDirectInputDevice8A* pThis, DWORD cbData, LPVOID lpvData);
ULONG hkMouseRelease(IDirectInputDevice8A* pThis);
HRESULT hkMouseAcquire(IDirectInputDevice8A* pThis);
HRESULT hkMouseGetDeviceState(IDirectInputDevice8A* pThis, DWORD cbData, LPVOID lpvData);
extern "C" void hkSaveFileIOThunk(void); // idk args yet
extern "C" void hkSaveFileIO(CSaveDataDevice * pSave);
extern "C" BOOL hkLoadWordBlacklistThunk(BannedWordChecker * pThis, __int64 thisrdx, QWORD * thisr8, const char* szBlacklistName);
extern "C" BOOL hkLoadWordBlacklist(BannedWordChecker * pThis, __int64 thisrdx, QWORD * thisr8, const char* szBlacklistName);
extern "C" void hkUpdateModelPartsThunk(Pl0000 * pEntity);
extern "C" void hkUpdateModelParts(Pl0000 * pEntity); //proabably not a pl0000 must be a parent
extern "C" void* hkCreateEntityThunk(CSceneEntitySystemUnk* pUnknown, CEntityInfo * pInfo, uint32_t objectId, int32_t iGroupId, CHeapInfo * pHeapInfo);
extern "C" void* hkCreateEntity(CSceneEntitySystemUnk* pUnknown, CEntityInfo * pInfo, uint32_t uObjectId, int32_t iGroupId, CHeapInfo * pHeapInfo);
bool hkMRubyLoadScript(MrubyImpl* pThis, MrubyScript* pScript);
void hkPl0000Destructor(Pl0000* pThis);
void hkPl0000Update(Pl0000* pThis);
BOOL hkCCameraGameSetViewAngles(CCameraGame* pThis);
void* hkCCameraGameMove(CCameraGame* pThis);
CModelShaderModule* hkCreateModelShaderModule(CModelAnalyzer* pThis, CModelShader* pShader, DWORD* a3, CHeapInfo* a4);
LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
HRESULT hkOleLoadPicture(LPSTREAM lpstream, LONG lSize, BOOL fRunmode, REFIID riid, LPVOID* lplpvObj);
BOOL hkQueryPerformanceCounter(LARGE_INTEGER* lpPerfomaceCount);
LPTOP_LEVEL_EXCEPTION_FILTER hkSetUnhandledExceptionFilter(LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter);
BOOL hkSetCursorPos(int X, int Y);
DWORD hkXInputGetState(DWORD dwUserIndex, PXINPUT_STATE pState);