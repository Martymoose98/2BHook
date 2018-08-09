#pragma once
#include <Windows.h>
#include <d3d11.h>
#include <Xinput.h>
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "Renderer.h"
#include "Menu.h"
#include "Variables.h"
#include "Globals.h"
#include "Log.h"
#include "Features.h"
#include "Utils.h"

typedef HRESULT(__fastcall* PresentFn)(IDXGISwapChain* pThis, UINT SyncInterval, UINT Flags);
typedef HRESULT(__fastcall* CreateSwapChainFn)(IDXGIFactory* pThis, IUnknown* pDevice, DXGI_SWAP_CHAIN_DESC* pDesc, IDXGISwapChain** ppSwapChain);
typedef void(__fastcall* DrawIndexedFn)(ID3D11DeviceContext* pThis, UINT IndexCount, UINT StartIndexLocation, INT BaseVertexLocation);
typedef void(__fastcall* ClearRenderTargetViewFn)(ID3D11DeviceContext* pThis, ID3D11RenderTargetView* pRenderTargetView, const FLOAT ColorRGBA[4]);
typedef void(__fastcall* PSSetShaderResourcesFn)(ID3D11DeviceContext* pThis, UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView *const *ppShaderResourceViews);
typedef HRESULT(__fastcall* AcquireFn)(IDirectInputDevice8A* pThis);
typedef HRESULT(__fastcall* GetDeviceStateFn)(IDirectInputDevice8A* pThis, DWORD cbData, LPVOID lpvData);
typedef BOOL(__fastcall* QueryPerformaceCounterFn)(LARGE_INTEGER* lpPerfomaceCount);
typedef BOOL(__fastcall* SetCursorPosFn)(int X, int Y);
typedef DWORD(__fastcall* XInputGetStateFn)(DWORD dwUserIndex, PXINPUT_STATE pState);

typedef void(__fastcall* ReadSaveSlotsFn)(CSaveDataDevice* pSave);
typedef void(__fastcall* ReadSaveDataFn)(CSaveDataDevice* pSave);
typedef void(__fastcall* WriteSaveDataFn)(CSaveDataDevice* pSave);
typedef void(__fastcall* DeleteSaveDataFn)(CSaveDataDevice* pSave);

typedef void*(__fastcall* CreateEntityFn)(void* pUnknown, EntityInfo* pInfo, unsigned int objectId, int flags, CHeapInstance** ppHeaps);

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

extern PresentFn oPresent;
extern CreateSwapChainFn oCreateSwapChain;
extern DrawIndexedFn oDrawIndexed;
extern PSSetShaderResourcesFn oPSSetShaderResources;
extern ClearRenderTargetViewFn oClearRenderTargetView;
extern AcquireFn oKeyboardAcquire;
extern GetDeviceStateFn oKeyboardGetDeviceState;
extern AcquireFn oMouseAcquire;
extern GetDeviceStateFn oMouseGetDeviceState;
extern QueryPerformaceCounterFn oQueryPerformanceCounter;
extern SetCursorPosFn oSetCursorPos;
extern XInputGetStateFn oXInputGetState;
extern CreateEntityFn oCreateEntity;
extern WNDPROC oWndProc;


HRESULT __fastcall hkPresent(IDXGISwapChain* pThis, UINT SyncInterval, UINT Flags);
HRESULT __fastcall hkCreateSwapChain(IDXGIFactory* pThis, IUnknown* pDevice, DXGI_SWAP_CHAIN_DESC* pDesc, IDXGISwapChain** ppSwapChain);
void __fastcall hkPSSetShaderResources(ID3D11DeviceContext* pThis, UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView *const *ppShaderResourceViews);
void __fastcall hkDrawIndexed(ID3D11DeviceContext* pThis, UINT IndexCount, UINT StartIndexLocation, INT BaseVertexLocation);
void __fastcall hkClearRenderTargetView(ID3D11DeviceContext* pThis, ID3D11RenderTargetView* pRenderTargetView, const FLOAT ColorRGBA[4]);
HRESULT __fastcall hkKeyboardAcquire(IDirectInputDevice8A* pThis);
HRESULT __fastcall hkKeyboardGetDeviceState(IDirectInputDevice8A* pThis, DWORD cbData, LPVOID lpvData);
HRESULT __fastcall hkMouseAcquire(IDirectInputDevice8A* pThis);
HRESULT __fastcall hkMouseGetDeviceState(IDirectInputDevice8A* pThis, DWORD cbData, LPVOID lpvData);
void __fastcall hkSaveFileIO(CSaveDataDevice* pSave);
extern "C" void __fastcall hkModelParts(Pl0000* pEntity);
extern "C" void* __fastcall hkCreateEntityThunk(void* pUnknown, EntityInfo* pInfo, unsigned int objectId, int flags, CHeapInstance** ppHeaps);
extern "C" void* __fastcall hkCreateEntity(void* pUnknown, EntityInfo* pInfo, unsigned int objectId, int flags, CHeapInstance** ppHeaps);
LRESULT __fastcall WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL __fastcall hkQueryPerformanceCounter(LARGE_INTEGER* lpPerfomaceCount);
BOOL __fastcall hkSetCursorPos(int X, int Y);
DWORD hkXInputGetState(DWORD dwUserIndex, PXINPUT_STATE pState);