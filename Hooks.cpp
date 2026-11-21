#include "Hooks.h"
#include "EspRenderer.h"
#include "MaterialTint.h"


PresentFn oPresent;
ResizeBuffersFn oResizeBuffers;
CreateSwapChainFn oCreateSwapChain;
DrawIndexedFn oDrawIndexed;
DrawFn oDraw;
PSSetShaderResourcesFn oPSSetShaderResources;
ClearRenderTargetViewFn oClearRenderTargetView;
QueryPerformaceCounterFn oQueryPerformanceCounter;
SetUnhandledExceptionFilterFn oSetUnhandledExceptionFilter;
ReleaseFn oKeyboardRelease;
AcquireFn oKeyboardAcquire;
GetDeviceStateFn oKeyboardGetDeviceState;
ReleaseFn oMouseRelease;
AcquireFn oMouseAcquire;
GetDeviceStateFn oMouseGetDeviceState;
SetCursorPosFn oSetCursorPos;
XInputGetStateFn oXInputGetState;
UpdateModelPartsFn oUpdateModelParts;
CreateEntityFn oCreateEntity;
MRubyLoadScriptFn oMRubyLoadScript;
CCameraGame_SetViewAnglesFn oCCameraGameSetViewAngles;
CCameraGame_MoveFn oCCameraGameMove;
CModelAnalyzer_CreateModelShaderModuleFn oCreateModelShaderModule;
WNDPROC oWndProc;


static ID3D11RasterizerState* pNorm, * pBias;
//static bool hookupdate = false;
//VirtualTableHook m_ctxhook;
//VirtualTableHook m_shaderhook[MAX_MATERIAL_TEXTURES];

void CModelShaderModule_Draw(CModelShaderModule* pThis, CModelEntryData* pData);

class CPl0000Hook
{
public:
	// Visibility is tracked per mesh in a fixed-size array; meshes past this are left alone.
	static constexpr int MAX_TRACKED_MESHES = 32;

	CPl0000Hook(CObj* pEntity)
		: m_Hook((ULONG_PTR**)pEntity), m_pShaderHooks{}
	{
		if (pEntity->m_pModelInfo)
			m_vTint = pEntity->m_pModelInfo->m_vTint;

		for (int i = 0; i < TrackedMeshes(&pEntity->m_Work); ++i)
		{
			m_bMeshShow[i] = pEntity->m_Work.m_pMeshes[i].m_bShow;
		}


		/*	for (int i = 0; i < pEntity->m_Work.m_nMaterialShaders; ++i)
			{
				m_pShaderHooks[i] = new VirtualTableHook((ULONG_PTR**)pEntity->m_Work.m_pModelShaders->m_pShader);
				m_pShaderHooks[i]->HookFunction((ULONG_PTR)&CModelShaderModule_Draw, 3);
			}*/

		m_Hook.HookFunction((ULONG_PTR)&hkPl0000Destructor, 0);
		m_Hook.HookFunction((ULONG_PTR)&hkPl0000Update, 8);
	}

	~CPl0000Hook(void)
	{
	}

	// The mesh loops used to run to m_nMeshes - 1, which skipped the last mesh - the one the
	// menu's mesh slider can still select - and ran past m_bMeshShow on models with more
	// than MAX_TRACKED_MESHES + 1 meshes.
	static int TrackedMeshes(const CModelWork* pWork)
	{
		return min(pWork->m_nMeshes, MAX_TRACKED_MESHES);
	}

	VirtualTableHook m_Hook;
	VirtualTableHook* m_pShaderHooks[MAX_MATERIAL_TEXTURES];
	Vector4 m_vTint;
	bool m_bMeshShow[MAX_TRACKED_MESHES];

};

std::unordered_map<void*, CPl0000Hook*> g_pHookedEntities;

// WIP
void CModelShaderModule_Draw(CModelShaderModule* pThis, CModelEntryData* pData)
{
	UNREFERENCED_PARAMETER(pThis);

	typedef __int64 (*OriginalFn)(CModelShaderModule*, CModelEntryData*);
	typedef __int64 (*Original2Fn)(CModelShaderModule*, CModelShader*, CModelExtendWork*);
	//((OriginalFn)m_shaderhook.GetFunctionAddress(3))(pThis, pData);
	//pThis->Draw(pData);

	//pThis->ApplyExternalForces(pData->m_pWork->m_pModelShaders, pData->m_pWork->m_pNext->m_pModelExtend2);
	//pThis->Update(pData->m_pWork->m_pModelShaders, pData->m_pWork->m_pNext->m_pModelExtend2, 2);
	//pThis->ApplyExternalForces(pData->m_pWork->m_pModelShaders, pData->m_pWork->m_pNext->m_pModelExtend2);
	CModel* pModel = pData->m_pWork->m_pModelExtend2->m_pParent;

	Matrix4x4 RotM;

	//CModelMatrixTable* pTbl = pData->m_pMatrices[0];

	//for (int i = 0; i < pTbl->m_nMatrixCount; ++i)
	//{

	//	Matrix4x4* pBoneMat = (Matrix4x4*)pData->m_pConstBuffer[i]->m_pResources;

	//	pBoneMat->Transpose();

	//	RotM.InitAxisAngle(pModel->m_matTransform.GetAxis(RIGHT), M_PI_F / 2.0f);

	//	*pBoneMat = RotM * (*pBoneMat);

	//	pBoneMat->Transpose();
	//}

	CPl0000Hook* pEntityHook = g_pHookedEntities[pModel];//g_pHookedEntities[CONTAINING_RECORD(pData->m_pWork, CObj, m_Work)];

	if (pEntityHook)
	{
		//pEntityHook->m_vTint = Vector4(0.0f, 1.0f, 1.0f, 1.0f);
		//pEntityHook->m_pShaderHooks[0]->Call<void, CModelShaderModule, CModelEntryData*>(3, pData);

		// Always Render
		pData->m_pWork->m_dwRenderFlags &= (~CModelWork::RF_DONT_RENDER);
		pEntityHook->m_pShaderHooks[0]->Call<void, CModelShaderModule, CModelEntryData*>(3, pData);
	}

	//pThis->Draw(pData);

	// Exception thrown at 0x000000014500537C in NieRAutomata.exe: 
	// 0xC0000005: Access violation reading location 0xFFFFFFFFFFFFFFFF.
	//if (pData && GetAsyncKeyState(VK_END) & 1)
	//{
	//	//14093FF50
	//	typedef __int64(*R_DrawModelFn)(CModelDrawContext* a1,
	//		CGraphics* pGraphics, CModelMatrixTable* pModelMatrixTable,
	//		CModelInstanceParam* pModelInstanceParam, CConstantBufferInfo* a5);

	//	((R_DrawModelFn)(0x14093FF50))(*pData->m_pCtx, g_pGraphics, *pData->m_pMatrices,
	//		(pData->m_pParams) ? (*pData->m_pParams) : NULL, (CConstantBufferInfo*)&pData->m_iStartIndex);
	//}
}

// Chams, an ESP: enemies (Chams) and NPCs (NPC Chams) drawn in one colour where something is in front
// of them and another where they can be seen, red and green for enemies, blue and cyan for NPCs.
// Glow (and NPC Glow) puts a glow of the hidden colour around them, through walls too. hkDrawIndexed
// draws each of their colour draws into EspRenderer's mask, and with chams draws it again in the seen
// colour, with the game's own depth state, in place of the game's draw. EspRenderer::Composite fills
// in the hidden colour after the frame, wherever the game drew something nearer: drawn in place with
// the depth test off, it stayed only where the wall in front had been drawn before the target, and
// whatever the game drew afterwards covered it. "Use Shader" colours the seen pass with a flat pixel
// shader; without it the game's shader runs with a colour texture in its first texture slot, so the
// model is still lit. The camera entity is left alone.
//
// Their draws are told apart by their models' index buffers, which UpdateChamsTargets gathers once
// a frame. Each vertex group of a model has one (CModelDrawData::m_pVertexGroups), and
// CGraphicContextDx11_SetIndexBuffer (0x1402563C0) binds its D3D buffer with m_uIndexStart added to
// every draw's start index, so a draw is the model's when it reads that buffer within
// [m_uIndexStart, m_uIndexStart + m_nIndices). Enemies of one kind share a model, so its ranges are
// gathered once. A model's non-instanced draws go through
// CGraphicContextDx11_DrawIndexed (0x140256DD0, context slot 46; R_DrawMesh in ExcitedDelirium),
// which flushes the game's state cache and then calls ID3D11DeviceContext::DrawIndexed, so
// hkDrawIndexed sees the state the game draws with.
//
// This used to replace slot 46 of the game's context and call Denuvo-build addresses, and was only
// installed in Debug builds of that version, so the checkboxes did nothing on v1.02 and 26534214.
enum ChamsType_t
{
	CHAMS_NONE = -1,
	CHAMS_ENEMY,
	CHAMS_NPC,
};

struct ChamsRange_t
{
	ID3D11Buffer* m_pBuffer;	// only compared, never used
	UINT m_uFirst;
	UINT m_uEnd;
	ChamsType_t m_Type;
	uint32_t m_uFlags;			// EspRenderer::ESPMASK_CHAMS and _GLOW
};

static ChamsRange_t s_ChamsRanges[256];
static int s_nChamsRanges;

// The hidden colours, which the glow takes too: red for enemies, blue for NPCs (g_pRed, g_pBlue)
static const float s_EnemyHiddenColor[3] = { 1.0f, 0.0f, 0.0f };
static const float s_NPCHiddenColor[3] = { 0.0f, 0.3f, 1.0f };

// The index ranges of one entity's model, once each
static void AddChamsTargets(const CModelWork* pWork, ChamsType_t Type, uint32_t uFlags)
{
	if (!pWork || !pWork->m_pModelData || !uFlags)
		return;

	const CModelDrawData* pDrawData = pWork->m_pModelData->m_pDrawData;

	if (!pDrawData || !pDrawData->m_pVertexGroups)
		return;

	for (int i = 0; i < pDrawData->m_nVertexGroups; ++i)
	{
		const CIndexBuffer* pIndices = pDrawData->m_pVertexGroups[i].m_pIndexBuffer;

		if (!pIndices || !pIndices->m_pInfo || !pIndices->m_pInfo->m_pBuffer || pIndices->m_nIndices <= 0)
			continue;

		ChamsRange_t Range = { pIndices->m_pInfo->m_pBuffer, pIndices->m_uIndexStart,
			pIndices->m_uIndexStart + (UINT)pIndices->m_nIndices, Type, uFlags };

		bool bKnown = false;

		for (int j = 0; j < s_nChamsRanges && !bKnown; ++j)
			bKnown = s_ChamsRanges[j].m_pBuffer == Range.m_pBuffer && s_ChamsRanges[j].m_uFirst == Range.m_uFirst &&
				s_ChamsRanges[j].m_uEnd == Range.m_uEnd;

		if (!bKnown && s_nChamsRanges < (int)ARRAYSIZE(s_ChamsRanges))
			s_ChamsRanges[s_nChamsRanges++] = Range;
	}
}

// Once a frame from hkPresent, on the thread that draws: the living enemies (through the game's
// enemy lookup, as Display Enemy Info), the NPCs and Emil (through the untyped one, as Display
// NPC Info)
static void UpdateChamsTargets(void)
{
	s_nChamsRanges = 0;

	const uint32_t uEnemyFlags = (Vars.Visuals.bChams ? EspRenderer::ESPMASK_CHAMS : 0) | (Vars.Visuals.bGlow ? EspRenderer::ESPMASK_GLOW : 0);
	const uint32_t uNPCFlags = (Vars.Visuals.bNPCChams ? EspRenderer::ESPMASK_CHAMS : 0) | (Vars.Visuals.bNPCGlow ? EspRenderer::ESPMASK_GLOW : 0);

	if (uEnemyFlags && g_pEnemyManager && GetEntityFromHandle)
	{
		for (size_t i = 0; i < g_pEnemyManager->m_handles.m_Count; ++i)
		{
			CBehaviorAppBase* pEnemy = GetEntityFromHandle(&g_pEnemyManager->m_handles.m_pItems[i]);

			if (pEnemy && pEnemy->m_iHealth > 0)
				AddChamsTargets(&pEnemy->m_Work, CHAMS_ENEMY, uEnemyFlags);
		}
	}

	if (uNPCFlags && g_pNPCManager)
	{
		for (size_t i = 0; i < g_pNPCManager->m_handles.m_Count; ++i)
		{
			CBehaviorAppBase* pNPC = GetEntityFromHandleGlobal(&g_pNPCManager->m_handles.m_pItems[i]);

			if (pNPC)
				AddChamsTargets(&pNPC->m_Work, CHAMS_NPC, uNPCFlags);
		}

		CBehaviorAppBase* pEmil = g_pEmilHandle ? GetEntityFromHandleGlobal(g_pEmilHandle) : NULL;

		if (pEmil)
			AddChamsTargets(&pEmil->m_Work, CHAMS_NPC, uNPCFlags);
	}
}

// The target a draw belongs to, from the index buffer and range it reads, or NULL
static const ChamsRange_t* FindChamsRange(ID3D11DeviceContext* pContext, UINT IndexCount, UINT StartIndexLocation)
{
	ID3D11Buffer* pIndexBuffer = NULL;
	DXGI_FORMAT Format;
	UINT uOffset;

	pContext->IAGetIndexBuffer(&pIndexBuffer, &Format, &uOffset);

	if (!pIndexBuffer)
		return NULL;

	pIndexBuffer->Release();	// only its address is compared

	for (int i = 0; i < s_nChamsRanges; ++i)
	{
		const ChamsRange_t* pRange = &s_ChamsRanges[i];

		if (pRange->m_pBuffer == pIndexBuffer && StartIndexLocation >= pRange->m_uFirst &&
			StartIndexLocation + IndexCount <= pRange->m_uEnd)
			return pRange;
	}

	return NULL;
}

// One of a target's draws: into EspRenderer's mask, then with chams in the seen colour in place of
// the game's draw. False when the game's own draw is still to be made: a draw without a pixel
// shader or a colour target (the shadow and depth passes, left to the game), or a glow-only target.
static bool DrawChams(ID3D11DeviceContext* pContext, const ChamsRange_t* pRange, UINT IndexCount, UINT StartIndexLocation, INT BaseVertexLocation)
{
	ID3D11PixelShader* pShader = NULL;
	ID3D11RenderTargetView* pTarget = NULL;

	pContext->PSGetShader(&pShader, NULL, NULL);
	pContext->OMGetRenderTargets(1, &pTarget, NULL);

	const bool bColourPass = pShader && pTarget;

	if (pTarget)
		pTarget->Release();

	if (!bColourPass)
	{
		if (pShader)
			pShader->Release();

		return false;
	}

	// Only the draws into the game's main depth buffer. The same models are drawn into shadow maps
	// and other buffers too, where the seen colour would be wrong and the silhouette (from the
	// light's view, or at another size) landed somewhere else on the screen.
	if (!EspRenderer::IsMainPass(pContext))
	{
		pShader->Release();
		return false;
	}

	const bool bNPC = pRange->m_Type == CHAMS_NPC;

	// The silhouette, for the hidden colour and the glow
	if (EspRenderer::BeginMaskDraw(pContext, bNPC ? s_NPCHiddenColor : s_EnemyHiddenColor, pRange->m_uFlags))
	{
		oDrawIndexed(pContext, IndexCount, StartIndexLocation, BaseVertexLocation);
		EspRenderer::EndMaskDraw(pContext);
	}

	if (!(pRange->m_uFlags & EspRenderer::ESPMASK_CHAMS))
	{
		pShader->Release();
		return false;
	}

	ID3D11ShaderResourceView* pTexture = NULL;
	const bool bUseShader = Vars.Visuals.iChamType != 0;
	ID3D11PixelShader* pSeenShader = bNPC ? g_pCyan : g_pGreen;
	ID3D11ShaderResourceView* pSeenTexture = bNPC ? g_pCyanSRV : g_pGreenSRV;

	pContext->PSGetShaderResources(0, 1, &pTexture);

	// Seen: the game's own depth state
	if (bUseShader)
		pContext->PSSetShader(pSeenShader, NULL, 0);
	else
		pContext->PSSetShaderResources(0, 1, &pSeenTexture);

	oDrawIndexed(pContext, IndexCount, StartIndexLocation, BaseVertexLocation);

	// Put back what the game bound. Its state cache (CGraphicsRenderContext_PreRender, 0x1402554F0)
	// only binds what it has changed, so anything left here would carry over to its next draws.
	pContext->PSSetShader(pShader, NULL, 0);
	pContext->PSSetShaderResources(0, 1, &pTexture);

	if (pTexture)
		pTexture->Release();

	pShader->Release();

	return true;
}

// The old Chams, kept for reference: it replaced slot 46 of the game's graphics context with
// Denuvo-build addresses and is only installed in Debug builds of that version (hkPresent).
BOOL DrawIndexedPrimitive(CGraphicContextDx11* pThis, RenderInfo* pInfo)
{
	ID3D11RasterizerState* pRS;
	ID3D11DepthStencilState* pDSS;
	UINT uStencilRef;
	CModelDrawData* pDrawData;
	BOOLEAN bChams = FALSE;

	BOOL b = (*(BOOL(*)(void*, ID3D11DeviceContext*))(0x140941130))(&pThis->m_RenderContext, pThis->m_pContext);

	if (g_pCamera && g_pCamera->m_pCamEntity && Vars.Visuals.bChams)
	{
		pDrawData = g_pCamera->m_pCamEntity->m_Work.m_pModelData->m_pDrawData;

		for (int i = 0; i < pDrawData->m_nBatches; ++i)
		{
			CBatch* pBatch = &pDrawData->m_pBatches[i];

			if (pInfo->m_uVertexCount == (UINT)pBatch->m_nVertices && pInfo->m_uIndexCountPerInstance == (UINT)pBatch->m_nPrimitives)
			{
				bChams = TRUE;
				break;
			}
		}
	}

	if (bChams)
	{
		pThis->m_pContext->OMGetDepthStencilState(&pDSS, &uStencilRef);
		pDSS->Release();
		pThis->m_pContext->RSGetState(&pRS);
		pRS->Release();

		pThis->m_pContext->OMSetDepthStencilState(g_pDepthStencilStates[DISABLED], 1);

		if (Vars.Visuals.iChamType)
		{
			pThis->m_pContext->PSSetShader(g_pRed, NULL, 0);
		}
		else
		{
			pThis->m_pContext->PSSetShaderResources(0, 1, &g_pRedSRV);
			//pThis->m_pContext->PSSetSamplers(0, 1, &g_pSamplerState);
		}

		(*(BOOL(*)(void*, ID3D11DeviceContext*, RenderInfo*))(0x140941380))(&pThis->m_RenderContext, pThis->m_pContext, pInfo);

		pThis->m_pContext->OMSetDepthStencilState(g_pDepthStencilStates[READ_NO_WRITE], 1);
		pThis->m_pContext->RSSetState(pBias);

		if (Vars.Visuals.iChamType)
		{
			pThis->m_pContext->PSSetShader(g_pGreen, NULL, 0);
		}
		else
		{
			pThis->m_pContext->PSSetShaderResources(0, 1, &g_pGreenSRV);
			//pThis->m_pContext->PSSetSamplers(0, 1, &g_pSamplerState);
		}

		(*(BOOL(*)(void*, ID3D11DeviceContext*, RenderInfo*))(0x140941380))(&pThis->m_RenderContext, pThis->m_pContext, pInfo);
		pThis->m_pContext->RSSetState(pRS);
		//pThis->m_pContext->RSSetState(pNorm);
		pThis->m_pContext->OMSetDepthStencilState(pDSS, uStencilRef);

		//b = (*(BOOL(*)(void*, ID3D11DeviceContext*))(0x140941130))(&pThis->m_RenderContext, pThis->m_pContext);
	}

	return (b) ? (*(BOOL(*)(void*, ID3D11DeviceContext*, RenderInfo*))(0x140941380))(&pThis->m_RenderContext, pThis->m_pContext, pInfo) : b;
}

HRESULT hkPresent(IDXGISwapChain* pThis, UINT SyncInterval, UINT Flags)
{
	ImGui::GetIO().MouseDrawCursor = g_pMenu->IsOpen();

	//g_pGraphics->m_iTimeStep = 0x100;

	// Between frames: theme edits, config loads and UI scale changes take effect here
	g_pMenu->UpdateStyle();

	// The QueryPerformanceCounter hook used to be taken out around these, so that ImGui's frame
	// timing saw the real counter. It never had to be: the hook is in NieRAutomata.exe's import
	// table, and ImGui, in this DLL, already calls the real one through ours. Taking it out only
	// let the game's other threads read the real counter meanwhile, which jumps from the
	// speedhack's.
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();

	ImGui::NewFrame();

	// FIXME: change when other Pl entity's are reversed
	g_pLocalPlayer = static_cast<Pl0000*>(GetEntityFromHandle2(g_pLocalPlayerHandle));

	Pl0000* pCameraEnt = static_cast<Pl0000*>(g_pCamera->m_pCamEntity);

	UpdateChamsTargets();

#if defined(_DEBUG) && defined(DENUVO_STEAM_BUILD)

	if (!hookupdate)
	{
		hookupdate = true;
		m_ctxhook.Initialize((ULONG_PTR**)g_pGraphics->m_pContext);
		m_ctxhook.HookFunction((ULONG_PTR)DrawIndexedPrimitive, 46);

		D3D11_RASTERIZER_DESC nrasterizer_desc;
		ZeroMemory(&nrasterizer_desc, sizeof(nrasterizer_desc));
		nrasterizer_desc.FillMode = D3D11_FILL_SOLID;
		//nrasterizer_desc.CullMode = D3D11_CULL_BACK; //flickering
		nrasterizer_desc.CullMode = D3D11_CULL_NONE;
		nrasterizer_desc.FrontCounterClockwise = false;
		nrasterizer_desc.DepthBias = 0;
		nrasterizer_desc.SlopeScaledDepthBias = 0.0f;
		nrasterizer_desc.DepthBiasClamp = 0.0f;
		nrasterizer_desc.DepthClipEnable = true;
		nrasterizer_desc.ScissorEnable = false;
		nrasterizer_desc.MultisampleEnable = false;
		nrasterizer_desc.AntialiasedLineEnable = false;
		g_pDevice->CreateRasterizerState(&nrasterizer_desc, &pNorm);

		D3D11_RASTERIZER_DESC rasterizer_desc;
		ZeroMemory(&rasterizer_desc, sizeof(rasterizer_desc));
		rasterizer_desc.FillMode = D3D11_FILL_SOLID;
		rasterizer_desc.CullMode = D3D11_CULL_FRONT; // D3D11_CULL_NONE;
		rasterizer_desc.FrontCounterClockwise = false;
		float bias = 1000.0f;
		float bias_float = -bias;
		bias_float /= 10000.0f;
		rasterizer_desc.DepthBias = (INT)(*(UINT*)&bias_float / (1.f / 8388608.f));
		rasterizer_desc.SlopeScaledDepthBias = 0.0f;
		rasterizer_desc.DepthBiasClamp = 0.0f;
		rasterizer_desc.DepthClipEnable = true;
		rasterizer_desc.ScissorEnable = false;
		rasterizer_desc.MultisampleEnable = false;
		rasterizer_desc.AntialiasedLineEnable = false;
		g_pDevice->CreateRasterizerState(&rasterizer_desc, &pBias);
	}

	if (GetAsyncKeyState(VK_F8) & 1)
		m_ctxhook.Unhook();

	if (GetAsyncKeyState(VK_F9) & 1)
		m_ctxhook.Rehook();

	if (GetAsyncKeyState(VK_OEM_3) & 1)
	{
		for (size_t i = 0; i < g_pEntityInfoList->m_dwSize; ++i)
		{
			CEntityInfo* pInfo = g_pEntityInfoList->m_pItems[i].second;

			if (pInfo)
				LOG("Entity: %s Handle: %x Address: %llx\n", pInfo->m_szEntityType, pInfo->m_hEntity, pInfo->m_pEntity);
		}
	}

	if (GetAsyncKeyState(VK_F1) & 1)
		LOG("Animation ID: %d\n", ++Vars.Gameplay.iAnimation);

	if (GetAsyncKeyState(VK_F2) & 1)
		LOG("Animation ID: %d\n", --Vars.Gameplay.iAnimation);

	if (GetAsyncKeyState(VK_F4) & 1)
	{
		if (g_pLocalPlayer)
		{
			m_shaderhook.Initialize((ULONG_PTR**)g_pLocalPlayer->m_Work.m_pModelShaders[10].m_pShader);
			m_shaderhook.HookFunction((ULONG_PTR)CModelShaderModule_Draw, 3);
		}
#if 0
		typedef BOOL(*CreateModelShaderModuleFn)(CModelShader* pEnt_390, CMaterialInfo* pInfo, void* pModelWork);
		static bool bInit = false;
		static CMaterial* pReplace;

		CMaterialInfo info = { 0 };

		if (!bInit)
		{
			bInit = true;
			pReplace = LoadMaterial("X:\\Documents\\Visual Studio 2017\\Projects\\2B Hook\\2B Hook\\Hair.dds");
		}

		info.m_pModelAnalyzer = (void*)0x141101C60;
		info.m_pMaterial = pReplace;
		info.m_bUnknownsExist = *MakePtr(int*, g_pLocalPlayer->m_pModelData, 0xE0) > 0;
		((CreateModelShaderModuleFn)(0x140871EF0))(&g_pLocalPlayer->m_pModelShaders[10], &info, &g_pLocalPlayer->m_ModelExtendWork);
		g_pLocalPlayer->m_pModelShaders[10].m_pMaterial = info.m_pMaterial;
#endif
	}

	if (GetAsyncKeyState(VK_F11) & 1)
	{
		Features::TeleportAllEnemiesToEncirclePoint(pCameraEnt->m_vPosition, 50);
	}
#endif

	// wip color isn't updating
#if 0
	if (Vars.Gameplay.bRainbowModel)
	{
		Vector3 rgb;

		Vars.Gameplay.flModelTintHue += 0.00005f;

		if (Vars.Gameplay.flModelTintHue > 1.f)
			Vars.Gameplay.flModelTintHue = 0.0f;

		ImGui::ColorConvertHSVtoRGB(Vars.Gameplay.flModelTintHue, 1.0f, 1.f, rgb[0], rgb[1], rgb[2]);
		g_pLocalPlayer->m_pModelInfo->m_vTint = Vector4(rgb);
	}
#endif

	//bool bDerived = RTTI::DerivesFrom(pCameraEnt, typeid(Pl0000));
	// (The Home/End camera type counter that used to be here stepped Vars.Misc.nSlot, the save
	// slot "Backup Save" copies. It now lives in Overlay::Render's debug readout.)

	if (pCameraEnt)
	{
		Features::ApplyHiddenMeshes(&pCameraEnt->m_Work);

		// This ran in hkPl0000Update until 90c09d7, but that hook is only installed in Debug
		// builds with EXP_ENT_HOOK defined, which it never is - so Rainbow Hair did nothing.
		if (Vars.Gameplay.bRainbowHair)
		{
			Vars.Gameplay.flModelTintHue += 0.005f;

			if (Vars.Gameplay.flModelTintHue > 1.f)
				Vars.Gameplay.flModelTintHue = 0.0f;

			CMeshPart* pHair = GetModelMesh(&pCameraEnt->m_Work, "Hair");

			if (pHair)
			{
				// Only the colour: the alpha is how first person hides the hair, and the mesh
				// "Enabled" checkbox owns m_bShow (forcing it every frame undid hiding the hair).
				Vector4 vHue = Color::FromHSB(Vars.Gameplay.flModelTintHue, 1.f, 1.f);

				pHair->m_vColor = Vector4(vHue.x, vHue.y, vHue.z, pHair->m_vColor.w);
				pHair->m_bUpdate = TRUE;
			}
		}

		if (Vars.Gameplay.bRainbowPod)
		{
			// The active pod of the camera entity, if that is a Pl0000. GetEntityFromHandle only
			// returns enemies (it ends in CEntityInfo_GetEmBase, 0x140308550), so this never found
			// the pod, and it always read pod slot 0.
			CBehaviorAppBase* pPod = GetActivePod(GetPl0000(pCameraEnt));

			if (pPod)
			{
				Vars.Gameplay.flPodTintHue += 0.005f;

				if (Vars.Gameplay.flPodTintHue > 1.f)
					Vars.Gameplay.flPodTintHue = 0.0f;

				// Look the mesh up on the pod's own model. This used to search the camera
				// entity's model for "Hair" and then index the pod's mesh array with the
				// result - an index into a different, much larger mesh list. It tinted
				// whichever pod mesh happened to share 2B's hair slot, or wrote past the end
				// of the pod's meshes when that slot didn't exist on the pod.
				int idx = GetModelMeshIndex(&pPod->m_Work, "Body");

				if (idx != -1)
				{
					// Only tint. Visibility belongs to the pod mesh "Enabled" checkbox; forcing
					// m_bShow here every frame undid hiding the Body mesh.
					pPod->m_Work.m_pMeshes[idx].m_vColor = Color::FromHSB(Vars.Gameplay.flPodTintHue, 1.f, 1.f);
					pPod->m_Work.m_pMeshes[idx].m_bUpdate = TRUE;
				}
			}
		}
	}

	if (Vars.Gameplay.bBalanceEnemyLevels)
		Features::BuffEnemies();

	Features::UpdatePendingSpawn();
	Features::ApplyCreatedEntity();
	Features::ReleaseDeadSpawnRequests();
	Features::ApplyNoCollision(g_pCamera->m_pCamEntity);
	Features::MoveSun();
	Features::ApplyHidePod(g_pCamera->m_pCamEntity);

	Features::ApplyHealthMods();
	Features::ApplyFreeCamFlags();

	MaterialTint::Update();

	if (Vars.Gameplay.bNoTutorialDialogs)
		PatchBytes(&bp_NoTutorialDialogs);
	else
		RestoreMemory(&bp_NoTutorialDialogs);

	// Anti-Framerate Cap turns the frame limiter's jnz into a jmp (bp_Framecap, see Setup). This runs
	// inside Graphics_LimitFrameAndPresent, which presents, already past that jnz, so the patch takes
	// effect from the next frame; nothing else runs that function. It used to set g_uGameFlags bit 4
	// here, after the limiter had run, and the game rebuilds that bit every window pump.
	if (Vars.Misc.bAntiFramerateCap)
		PatchBytes(&bp_Framecap);
	else
		RestoreMemory(&bp_Framecap);

	g_pOverlay->Render(true);

	g_pMenu->Draw(ImVec2(875, 600));

	ImGui::Render();

	// Bind the swapchain's back buffer before submitting ImGui's geometry.
	//
	// CreateRenderTarget() binds this once during initialization, but the game's very next
	// draw call replaces the binding - it swaps render targets all through the frame for
	// its own passes. Whatever happened to be bound at Present time is not the back
	// buffer, so ImGui was drawing perfectly valid geometry into the wrong target and
	// nothing ever reached the screen.
	if (g_pRenderTargetView)
		g_pDeviceContext->OMSetRenderTargets(1, &g_pRenderTargetView, NULL);

	// The chams' hidden colour and the glow, under the menu
	EspRenderer::Composite(g_pDeviceContext, g_pRenderTargetView);

	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	//COtManager::GetGraphicCommand(176i64);
	return oPresent(pThis, (Vars.Misc.bAntiVSync) ? 0 : SyncInterval, Flags);
}

HRESULT CreateRenderTarget(void);

HRESULT hkResizeBuffers(IDXGISwapChain* pThis, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags)
{
	LOG("pSwapChain %p | pWindowedSwapChain %p\n", g_pGraphics->m_Display.m_pSwapChain, g_pGraphics->m_Display.m_pWindowedSwapChain);

	if (g_pRenderTargetView)
	{
		g_pDeviceContext->OMSetRenderTargets(0, NULL, NULL);
		g_pRenderTargetView->Release();
	}

	HRESULT hr = oResizeBuffers(pThis, BufferCount, Width, Height, NewFormat, SwapChainFlags);

	if (SUCCEEDED(hr))
	{
		ImGui_ImplDX11_InvalidateDeviceObjects();
		CreateRenderTarget();
	}
	return hr;
}

HRESULT hkCreateSwapChain(IDXGIFactory* pThis, IUnknown* pDevice, DXGI_SWAP_CHAIN_DESC* pDesc, IDXGISwapChain** ppSwapChain)
{
	pDesc->BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	HRESULT hr = oCreateSwapChain(pThis, pDevice, pDesc, ppSwapChain);

	if (SUCCEEDED(hr))
	{
		g_pSwapChain = *ppSwapChain;
		g_pSwapChainHook->Relocate((ULONG_PTR**)g_pSwapChain);

		ImGui_ImplDX11_InvalidateDeviceObjects();
	}

	return hr;
}

HRESULT hkCreateSwapChainForHwnd(IDXGIFactory2* pThis, IUnknown* pDevice, DXGI_SWAP_CHAIN_DESC* pDesc, IDXGISwapChain** ppSwapChain)
{
	UNREFERENCED_PARAMETER(pThis);
	UNREFERENCED_PARAMETER(pDevice);
	UNREFERENCED_PARAMETER(pDesc);
	UNREFERENCED_PARAMETER(ppSwapChain);

	return E_NOTIMPL;
}

void hkPSSetShaderResources(ID3D11DeviceContext* pThis, UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView* const* ppShaderResourceViews)
{
	D3D11_SHADER_RESOURCE_VIEW_DESC Desc;

	for (UINT i = 0; i < NumViews; ++i)
	{
		ID3D11ShaderResourceView* pShaderResourceView = ppShaderResourceViews[i];

		if (pShaderResourceView)
		{
			pShaderResourceView->GetDesc(&Desc);

			if ((Desc.ViewDimension == D3D11_SRV_DIMENSION_BUFFER) || (Desc.ViewDimension == D3D11_SRV_DIMENSION_BUFFEREX))
				continue;//Skip buffer resources
		}
	}

	oPSSetShaderResources(pThis, StartSlot, NumViews, ppShaderResourceViews);
}

void hkDraw(ID3D11DeviceContext* pThis, UINT VertexCount, UINT StartVertexLocation)
{
	oDraw(pThis, VertexCount, StartVertexLocation);
}

/*
* Any stride bigger than 0x1C (28) is the ui (so far)
*/
void hkDrawIndexed(ID3D11DeviceContext* pThis, UINT IndexCount, UINT StartIndexLocation, INT BaseVertexLocation)
{
	ID3D11Buffer* pVertexBuffers[8];
	UINT Strides[8];
	UINT VertexBufferOffsets[8];

	pThis->IAGetVertexBuffers(0, 8, pVertexBuffers, Strides, VertexBufferOffsets);

	for (int i = 0; i < 8; ++i)
	{
		if (pVertexBuffers[i])
		{
			pVertexBuffers[i]->Release();
			pVertexBuffers[i] = NULL;
		}
	}

	if (Vars.Misc.bWireframe && Strides[0] == 28)
		g_pDeviceContext->RSSetState(g_pRenderWireframeState); // wireframe

	if (s_nChamsRanges)
	{
		const ChamsRange_t* pRange = FindChamsRange(pThis, IndexCount, StartIndexLocation);

		if (pRange && DrawChams(pThis, pRange, IndexCount, StartIndexLocation, BaseVertexLocation))
			return;
	}

	oDrawIndexed(pThis, IndexCount, StartIndexLocation, BaseVertexLocation);
}

void hkClearRenderTargetView(ID3D11DeviceContext* pThis, ID3D11RenderTargetView* pRenderTargetView, const FLOAT ColorRGBA[4])
{
	oClearRenderTargetView(pThis, pRenderTargetView, ColorRGBA);
}

ULONG hkKeyboardRelease(IDirectInputDevice8A* pThis)
{
	g_pKeyboardHook->Unhook();

	ULONG uReferences = oKeyboardRelease(pThis);

	g_pKeyboardHook->Rehook();

	return uReferences;
}

HRESULT hkKeyboardAcquire(IDirectInputDevice8A* pThis)
{
	g_pKeyboardHook->Unhook();

	HRESULT hr = oKeyboardAcquire(pThis);

	g_pKeyboardHook->Rehook();

	return hr;
}

HRESULT hkKeyboardGetDeviceState(IDirectInputDevice8A* pThis, DWORD cbData, LPVOID lpvData)
{
	g_pKeyboardHook->Unhook();

	HRESULT hr = oKeyboardGetDeviceState(pThis, cbData, lpvData);

	g_pKeyboardHook->Rehook();

	return g_pMenu->KeyboardHandler(hr, lpvData, cbData);
}

ULONG hkMouseRelease(IDirectInputDevice8A* pThis)
{
	g_pMouseHook->Unhook();

	ULONG uReferences = oMouseRelease(pThis);

	g_pMouseHook->Rehook();

	return uReferences;
}

HRESULT hkMouseAcquire(IDirectInputDevice8A* pThis)
{
	g_pMouseHook->Unhook();

	HRESULT hr = oMouseAcquire(pThis);

	g_pMouseHook->Rehook();

	return hr;
}

HRESULT hkMouseGetDeviceState(IDirectInputDevice8A* pThis, DWORD cbData, LPVOID lpvData)
{
	//if (Vars.Menu.bOpened && Vars.Menu.bIgnoreInputWhenOpened)
	//	return DIERR_INPUTLOST;

	if (g_pMenu->MouseHandler(lpvData, cbData) == DIERR_INPUTLOST)
	{
		return DIERR_INPUTLOST;
	}

	g_pMouseHook->Unhook();

	HRESULT hr = oMouseGetDeviceState(pThis, cbData, lpvData);

	LPDIMOUSESTATE2 pState = (LPDIMOUSESTATE2)lpvData;

	if (SUCCEEDED(hr) && pState && (Vars.Misc.uCameraFlags & Variables_t::Misc_t::CameraFlg::CAMERA_FREE))
	{
		// The free camera looks with these (FreeCamKeyboardMouse consumes them once a camera
		// update, so polls in between add up), and the game gets a still mouse with no buttons
		// so 2B doesn't turn or attack.
		Vars.Misc.flDeltaX += pState->lX;
		Vars.Misc.flDeltaY += pState->lY;

		memset(pState, 0, min(cbData, (DWORD)sizeof(DIMOUSESTATE2)));
	}
	else if (pState)
	{
		Vars.Misc.flDeltaX = (float)pState->lX;
		Vars.Misc.flDeltaY = (float)pState->lY;
	}

	g_pMouseHook->Rehook();

	//memcpy(&Vars.Menu.Input.MouseState, lpvData, cbData);

	return hr;
}

bool hkMRubyLoadScript(MrubyImpl* pThis, MrubyScript* pScript)
{
	CCONSOLE_DEBUG_LOG(ImColor(0xff8f20d4), "FUCK");
	g_pConsole->Log(ImColor(0.5f, 0.f, 0.7f), "Script %x loaded!", pScript->m_dwHash);
	return oMRubyLoadScript(pThis, pScript);
	//if (pThis->m_pNext)
	//{
	//	g_pConsole->Warn("Next is valid!");
	//
	//	return oMRubyLoadScript(pThis, pScript);
	//}
	//return false;
}

// ORIG FUNCTION SIG: 40 53 48 81 EC ? ? ? ? F6 05
// Keyboard and mouse for the game's free camera, which reads only the gamepad
// (CameraFreeControl_MoveFly, g_MergedPadState). Called from CCameraGame::Update's tail, after
// CCameraGame_Freecam moved the camera with the pad and published it; the pose is written back
// too, so the next frame's Freecam starts from here. While the free camera is on, CMenu's
// keyboard handler and hkMouseGetDeviceState keep these keys and the mouse away from the game,
// so 2B doesn't walk or turn (the game itself only locks her gamepad).
//   W/S forward/back, A/D strafe, Space/Left Ctrl up/down, Left Shift x4, Left Alt x0.25,
//   mouse to look (pitch stops 5 degrees short of straight up or down, like the game's).
static void FreeCamKeyboardMouse(CCameraGame* pThis)
{
	// The camera shouldn't move while the menu is up and the keys are typing into it
	const bool bInput = !g_pMenu->IsOpen();
	const float flMouseX = bInput ? Vars.Misc.flDeltaX : 0.0f;
	const float flMouseY = bInput ? Vars.Misc.flDeltaY : 0.0f;

	// Consumed: hkMouseGetDeviceState accumulates them while the free camera is on
	Vars.Misc.flDeltaX = 0.0f;
	Vars.Misc.flDeltaY = 0.0f;

	Vector3 vLook = pThis->m_vTarget - pThis->m_vSource;

	if (vLook.LengthSqr() < 1e-8f)
		return;

	vLook.Normalize();

	const Vector3 vUp(0.0f, 1.0f, 0.0f);
	constexpr float SENSITIVITY = 0.0025f;	// radians per mouse count

	// Yaw about the world up axis
	const float flYaw = -flMouseX * SENSITIVITY;
	const float flCosYaw = cosf(flYaw), flSinYaw = sinf(flYaw);

	vLook = Vector3(vLook.x * flCosYaw + vLook.z * flSinYaw, vLook.y, -vLook.x * flSinYaw + vLook.z * flCosYaw);

	// Pitch about the view's right axis (Rodrigues; the axis is perpendicular to the look)
	Vector3 vRight = vLook.Cross(vUp);

	if (vRight.LengthSqr() > 1e-8f)
	{
		vRight.Normalize();

		const float flPitch = -flMouseY * SENSITIVITY;
		const Vector3 vPitched = vLook * cosf(flPitch) + vRight.Cross(vLook) * sinf(flPitch);
		const float flCosToUp = vPitched.Dot(vUp);

		if (fabsf(flCosToUp) < cosf(DEGTORAD(5.0f)))
			vLook = vPitched;

		vRight = vLook.Cross(vUp);
		vRight.Normalize();
	}

	Vector3 vMove(0.0f, 0.0f, 0.0f);

	if (bInput)
	{
		if (g_pMenu->IsKeyDown(DIK_W)) vMove += vLook;
		if (g_pMenu->IsKeyDown(DIK_S)) vMove -= vLook;
		if (g_pMenu->IsKeyDown(DIK_D)) vMove += vRight;
		if (g_pMenu->IsKeyDown(DIK_A)) vMove -= vRight;
		if (g_pMenu->IsKeyDown(DIK_SPACE)) vMove += vUp;
		if (g_pMenu->IsKeyDown(DIK_LCONTROL)) vMove -= vUp;
	}

	float flSpeed = 0.1f;	// metres per frame, close to the game's own full-stick speed

	if (g_pMenu->IsKeyDown(DIK_LSHIFT))
		flSpeed *= 4.0f;

	if (g_pMenu->IsKeyDown(DIK_LMENU))
		flSpeed *= 0.25f;

	const Vector3 vSource = Vector3(pThis->m_vSource) + vMove * flSpeed;

	pThis->m_vSource = vSource;
	pThis->m_vTarget = vSource + vLook;
	pThis->m_vUp = vUp;

	CameraGame_SetLookAt(pThis);

	// The live pose, which CCameraGame_Freecam copies into the camera at the start of its next run
	pThis->m_vPosition = pThis->m_vSource;
	pThis->m_vTarget2 = pThis->m_vTarget;
}

BOOL hkCCameraGameSetViewAngles(CCameraGame* pThis)
{
	Vector3Aligned vForward, vRight, vUp, vRotX, vRotY;

	/* failed aimbot code

	int iTarget = -1;
	float best = FLT_MAX;

for (int i = 0; i < g_pEnemyManager->m_handles.m_count; ++i)
{
	Pl0000* pCur = GetEntityFromHandle(&g_pEnemyManager->m_handles.m_pItems[i]);

	if (pCur && pCur->m_iHealth > 0)
	{
		float fov = g_pCamera->m_vPosition.DistTo(pCur->m_vPosition);

		if (fov < best)
		{
			iTarget = i;
			best = fov;
		}
	}
}

if (iTarget != -1)
{
	Pl0000* pTarget = GetEntityFromHandle(&g_pEnemyManager->m_handles.m_pItems[iTarget]);
	Pl0000* pPod = GetEntityFromHandle(&g_pLocalPlayer->m_hPod);
	pThis->m_Transform.m_vSource = pTarget->m_pBones[GetBoneIndex(pTarget->m_pModelData, BONE_SPINE1)].m_vPosition;
	pThis->m_Transform.m_vTarget = g_pCamera->m_vPosition;
	pThis->m_Transform.m_vUp = pPod->m_matTransform.GetAxis(UP) * -1;

	((void(*)(CCameraGame*))(0x140832210))(pThis);
}
return TRUE;*/
	// Camera Fov. CCameraGame_Update (0x14073DC40) has already eased m_Pose.m_flFov toward
	// m_GoalPose.m_flFov and handed it to the camera (CCameraInstance::SetFov, 0x14073E36D) by the
	// time it calls this (0x14073E4BB), and next frame CCameraGame::Move resets the goal FOV from
	// the camera rig (CCameraGame__ApplyRig, 0x1407279C7) before the next ease. So with only the
	// live pose written here, what reached the camera was a frame late and mixed with the rig's FOV.
	// The game's own FOV override goes in before that ease: while m_flFovOverrideTime is above 0
	// (Move counts it down first, 0x1407224D5), every camera type's Move ends by setting the goal
	// FOV to m_flFovOverride (CCameraGame__NormalCameraMove at 0x14073179C), the way
	// CCameraGame_SetFovOverride (0x140728D10) sets it for scripts. The time is in seconds (Move
	// takes the camera entity's tick coefficient / 60 off it), held half a second ahead so a slow
	// frame doesn't let it lapse. The goal is written too, for the rest of this frame. Below 180
	// degrees, where the projection's tan(fov / 2) blows up.
	if (Vars.Visuals.flFov == 0.0f)
		Vars.Visuals.flFov = RADTODEG(pThis->m_flFov);

	const float flFov = DEGTORAD(min(max(Vars.Visuals.flFov, 1.0f), 179.0f));

	pThis->m_flFov = pThis->m_flGoalFov = pThis->m_flFovOverride = flFov;
	pThis->m_flFovOverrideTime = 0.5f;

	// First person, after the port's CFirstPerson (ExcitedDelirium FirstPerson.cpp). The game
	// has already moved its live pose (m_vPosition/m_vTarget2/m_vUp2, CCameraGame::m_Pose at
	// 0xF0) this frame; only the published eye and target are replaced, from the head, looking
	// the way the normal camera looks, within the limits below. The camera's own angles are
	// left alone, so the mouse, the stick and movement relative to the camera all work as
	// usual. Only for the default follow camera on the local player: events, lock-on rigs and
	// other camera types keep their own view. The old version aimed from m_vViewangles with a
	// broken clamp and indexed the bones with an unchecked head index.
	// The first-person keybind (kb_firstperson_camera) toggles Vars.Misc.bFirstperson, which
	// nothing read; the camera goes by the CAMERA_FIRSTPERSON flag the menu checkbox sets. Each
	// press of the key now flips that flag.
	if (Vars.Misc.bFirstperson != Vars.Misc.bFirstpersonOld)
	{
		Vars.Misc.uCameraFlags ^= Variables_t::Misc_t::CameraFlg::CAMERA_FIRSTPERSON;
		Vars.Misc.bFirstpersonOld = Vars.Misc.bFirstperson;
	}

	CBehaviorAppBase* pCamEntity = pThis->m_pCamEntity;
	const bool bFirstPerson = (Vars.Misc.uCameraFlags & Variables_t::Misc_t::CameraFlg::CAMERA_FIRSTPERSON) &&
		!(Vars.Misc.uCameraFlags & Variables_t::Misc_t::CameraFlg::CAMERA_FREE) &&
		pCamEntity && pCamEntity == g_pLocalPlayer && pThis->m_CamType == CAM_DEFAULT;
	bool bPlaced = false;

	Features::UpdateFirstPersonHead(bFirstPerson ? pCamEntity : NULL);

	if (bFirstPerson)
	{
		Vector3Aligned vLook = pThis->m_vTarget2 - pThis->m_vPosition;

		if (vLook.LengthSqr() > 1e-6f)
		{
			// Looking down or back used to put the eye inside her: it was moved 12 cm along the look,
			// so down into her neck and chest, or back into her shoulders. The look is now held
			// within FIRSTPERSON_PITCH_LIMIT of level and FIRSTPERSON_YAW_LIMIT of her heading, she
			// turns to it past a threshold (Features::TurnFirstPersonBody), the eye moves forward
			// along the level look only, and the near plane comes in (ApplyFirstPersonNearPlane).
			// Only the published view is held: the camera's own pose and angles are the game's, so
			// the mouse and the stick work as usual. The input's look basis is built from this
			// published eye, target and up (PlayerKeyboardInput_UpdateLookBasis, 0x140511970, looks
			// at pvFocusPosition from pvTranslation, g_CameraGame +0x20 and +0x10), so she moves the
			// way the view looks.
			constexpr float FIRSTPERSON_PITCH_LIMIT = DEGTORAD(80.0f);
			constexpr float FIRSTPERSON_YAW_LIMIT = DEGTORAD(100.0f);
			constexpr float FIRSTPERSON_EYE_FORWARD = 0.10f;	// metres, ahead of the head bone

			// pCamEntity is g_pLocalPlayer, which comes from the Pl0000-only GetEntityFromHandle2
			Pl0000* pPlayer = static_cast<Pl0000*>(pCamEntity);

			vLook.Normalize();

			// The game's heading convention, atan2(x, z): the camera and move headings
			// (PlayerInput_ResolveStick, port PlayerController.cpp:265-284) and the yaw
			// Pl0000_TurnTowardHeading turns to
			const float flLookYaw = atan2f(vLook.x, vLook.z);
			const float flLookPitch = asinf(min(max(vLook.y, -1.0f), 1.0f));

			Features::TurnFirstPersonBody(pPlayer, flLookYaw);

			// Her yaw after the turn, m_vRotation.y (CObj +0x94, m_matModelToWorld row 3), the
			// field Pl0000_TurnTowardHeading (0x1404FE750) writes
			const float flBodyYaw = pPlayer->m_matModelToWorld.GetAxis(3).y;
			const float flRelativeYaw = remainderf(flLookYaw - flBodyYaw, 2.0f * M_PI_F);
			const float flViewYaw = flBodyYaw + min(max(flRelativeYaw, -FIRSTPERSON_YAW_LIMIT), FIRSTPERSON_YAW_LIMIT);
			const float flViewPitch = min(max(flLookPitch, -FIRSTPERSON_PITCH_LIMIT), FIRSTPERSON_PITCH_LIMIT);
			const Vector3 vLevel(sinf(flViewYaw), 0.0f, cosf(flViewYaw));
			const Vector3 vView(vLevel.x * cosf(flViewPitch), sinf(flViewPitch), vLevel.z * cosf(flViewPitch));

			// Head bone 5, 8 cm up; with no head bone, 1.5 m above the feet (the port's fallback)
			const int16_t sHead = GetBoneIndex(pCamEntity->m_Work.m_pModelData, BONE_HEAD);
			Vector3Aligned vHead = (sHead >= 0 && sHead < pCamEntity->m_nBones) ?
				pCamEntity->m_pBones[sHead].m_vPosition + Vector3(0.0f, 0.08f, 0.0f) :
				pCamEntity->m_vPosition + Vector3(0.0f, 1.5f, 0.0f);

			// The eye height eases (0.35 a frame) so footsteps don't shake the view
			static float s_flEyeHeight = 0.0f;
			static const CBehaviorAppBase* s_pEyeOwner = NULL;

			if (s_pEyeOwner != pCamEntity)
				s_flEyeHeight = vHead.y;

			s_pEyeOwner = pCamEntity;
			s_flEyeHeight += (vHead.y - s_flEyeHeight) * 0.35f;
			vHead.y = s_flEyeHeight;

			// In front of her face the way the view faces, level, so looking down doesn't drop the
			// eye into her neck. Along the look rather than her heading, as the port has it
			// (FirstPerson.cpp:24-30): an eye in front of her face looking past her shoulder sat
			// inside her head, and A2's skull is part of her Body mesh, which stays drawn.
			pThis->m_vSource = vHead + vLevel * FIRSTPERSON_EYE_FORWARD;
			pThis->m_vTarget = pThis->m_vSource + vView;
			pThis->m_vUp = pThis->m_vUp2;

			CameraGame_SetLookAt(pThis);
			bPlaced = true;
		}
	}

	if (!bPlaced)
		Features::TurnFirstPersonBody(NULL, 0.0f);

	Features::ApplyFirstPersonNearPlane(pThis, bPlaced);

	if (bPlaced)
	{
		Vars.Misc.bCameraFlagsOld = Vars.Misc.uCameraFlags;
		return TRUE;
	}

	// Freecamera: the game's own debug free camera runs instead of Move (Features::ApplyFreeCamFlags
	// sets its flag) and reads only the gamepad; keyboard and mouse fly it from here, in
	// CCameraGame::Update's tail, after CCameraGame_Freecam has applied the pad.
	if ((Vars.Misc.uCameraFlags & Variables_t::Misc_t::CameraFlg::CAMERA_FREE))
		FreeCamKeyboardMouse(pThis);

	Vars.Misc.bCameraFlagsOld = Vars.Misc.uCameraFlags;

	// First person returned above when it applied; otherwise (an event or other camera type)
	// the game's own view goes through instead of being skipped.
	return (Vars.Misc.uCameraFlags & Variables_t::Misc_t::CameraFlg::CAMERA_FREE) ?
		TRUE : oCCameraGameSetViewAngles(pThis);
}

// ORIG FUNCTION SIG: 40 53 57 48 83 EC 68 48 8B D9
// move happens before setviewangles it seems
void* hkCCameraGameMove(CCameraGame* pThis)
{
	// Free camera used to be done here: 2BHook rotated m_pInstance->m_World by hand, called
	// CCameraGame_Freecam and then the original Move anyway, so the game's normal camera update
	// overwrote the free camera every frame. With the game's free-camera flag set, the game calls
	// CCameraGame_Freecam instead of Move (port Camera.cpp:10817, CAM_THINK_UPDATE), so nothing is
	// needed here.
	return oCCameraGameMove(pThis);
}

CModelShaderModule* hkCreateModelShaderModule(CModelAnalyzer* pThis, CModelShader* pShader, DWORD* a3, CHeapInfo* a4)
{
	// Changes the shader vvv causes artifacts
	//pShader->m_pMaterial->m_szShaderName = "CLT00_XXXXX";
	[[maybe_unused]] CTextureDescription Desc;

#if 0
	static CTargetTexture* pTex = CreateTextureR("C:\\Users\\marty\\source\\repos\\2B Hook\\2B Hook\\Kim Jong Un Flood.dds", Desc);
	int nTexture = pThis->FindTextureIndexByName("g_AlbedoMap");

	uint32 crc = pShader->m_pMaterial->m_TextureIds[0];//HashStringCRC32("g_AlbedoMap", strlen("g_AlbedoMap"));
	pShader->m_pMaterial->m_TextureIds[0] = pShader->m_pMaterial->m_TextureIds[1];
	pShader->m_pMaterial->m_TextureIds[1] = crc;

	CModelShaderModule* pModelShader = oCreateModelShaderModule(pThis, pShader, a3, a4);

	g_pConsole->Log(ImVec4(0.0f, 0.525f, 0.0f, 1.0f), "Created Shader:\n\t Material: %s\n\t Shader: %s\n\t Technique: %s\n",
		pShader->m_pMaterial->m_szName, pShader->m_pMaterial->m_szShaderName, pShader->m_pMaterial->m_szTechniqueName);

#else
	CModelShaderModule* pModelShader = oCreateModelShaderModule(pThis, pShader, a3, a4);
#endif
	return pModelShader;
}

void hkPl0000Destructor(Pl0000* pThis)
{
	CPl0000Hook* pHook = g_pHookedEntities[pThis];

	pHook->m_Hook.Call<void, Pl0000>(0);
	g_pHookedEntities.erase(pThis);
}

static bool bInit = false;
void hkPl0000Update(Pl0000* pThis)
{
	CPl0000Hook* pHook = g_pHookedEntities[pThis];

	pHook->m_Hook.Call<void, Pl0000>(8);

	if (g_pCamera->m_pCamEntity != pThis)
		return;

	if (!bInit)
	{
		bInit = true;

		// m_pShaderHooks is sized by MAX_MATERIAL_TEXTURES - a per-material texture count,
		// not a shader count - so a model with more shaders than that overran it into the
		// rest of the hook object.
		for (int i = 0; i < pThis->m_Work.m_nMaterialShaders && i < (int)ARRAYSIZE(pHook->m_pShaderHooks); ++i)
		{
			pHook->m_pShaderHooks[i] = new VirtualTableHook((ULONG_PTR**)pThis->m_Work.m_pModelShaders[i].m_pShader);
			pHook->m_pShaderHooks[i]->HookFunction((ULONG_PTR)&CModelShaderModule_Draw, 3);
		}

		for (int i = 0; i < CPl0000Hook::TrackedMeshes(&pThis->m_Work); ++i)
		{
			pHook->m_bMeshShow[i] = pThis->m_Work.m_pMeshes[i].m_bShow;
		}
	}

	// The selection comes from the menu slider, which is bounded by the mesh count of
	// whatever the camera was on when it was drawn - not by m_bMeshShow.
	if (Vars.Gameplay.iSelectedModelMesh >= 0 && Vars.Gameplay.iSelectedModelMesh < CPl0000Hook::TrackedMeshes(&pThis->m_Work))
		pHook->m_bMeshShow[Vars.Gameplay.iSelectedModelMesh] = Vars.Gameplay.bSelectedMeshEnable;

	//CGraphicCommand* pCmd = GetGraphicCommand(176);
	//pThis->m_Work.m_pModelShaders->m_pShader->Draw(pThis->m_Work.m_pModelData->m_pDrawData);

	for (int i = 0; i < CPl0000Hook::TrackedMeshes(&pThis->m_Work); ++i)
	{
		pThis->m_Work.m_pMeshes[i].m_bShow = pHook->m_bMeshShow[i];
		pThis->m_Work.m_pMeshes[i].m_bUpdate = TRUE;
	}
}

//#define EXP_ENT_HOOK

// TODO: new hook doesn't need last param and shadow space in thunk
void* hkCreateEntity(CSceneEntitySystemUnk* pUnknown, CEntityInfo* pInfo, uint32_t uObjectId, int32_t iGroupId, CHeapInfo* pHeapInfo)
{
	UNREFERENCED_PARAMETER(pHeapInfo);

	CConstructionInfo<void>* pConstruct = GetConstructionInfo(uObjectId);
	void* pEntity = NULL;

	//CObjReadSystem::Work* pWork = FindObjectWork(uObjectId);

	// pEntity = oCreateEntity(pUnknown, pInfo, uObjectId, iGroupId, pHeapInfo); //0x1401A2B40 old denuvo
	if (Vars.Gameplay.SpawnBlacklist.empty() || std::find(Vars.Gameplay.SpawnBlacklist.cbegin(), Vars.Gameplay.SpawnBlacklist.cend(), pConstruct->m_szName) == Vars.Gameplay.SpawnBlacklist.cend())
		pEntity = oCreateEntity(pUnknown, pInfo, uObjectId, iGroupId);

	if (!pEntity)
		g_pConsole->Warn("Failed to create %s -> %s (ObjectId %x, SetType %x)\n", pInfo->m_szEntityType, pConstruct->m_szName, uObjectId, pInfo->m_uSetType);
	else
	{
#if defined(_DEBUG) && defined(EXP_ENT_HOOK)
		if (uObjectId == OBJECTID_2B || uObjectId == OBJECTID_A2 || uObjectId == OBJECTID_9S)
		{
			g_pHookedEntities.insert(std::make_pair(pEntity, new CPl0000Hook(static_cast<CObj*>(pEntity))));
		}
#endif

		g_pConsole->Log(ImVec4(0.0f, 0.525f, 0.0f, 1.0f), "Created %s -> %s (ObjectId %x, SetType %x) Base %llx\n", (pInfo->m_szEntityType) ? pInfo->m_szEntityType : "EntityLayout", pConstruct->m_szName, uObjectId, pInfo->m_uSetType, pEntity);
}


	return pEntity;
}

BOOL hkLoadWordBlacklist(CBannedWordChecker* pThis, __int64 thisrdx, QWORD* thisr8, const char* szBlacklistName)
{
#if 0
	QWORD size;
	LPCVOID src;
	LPVOID dst;
	BYTE current_byte;
	PBYTE pData, lpDataEnd, lpDataStart;
	UINT  dwDataEndOffset, dwDataStartOffset, length, dwWordByteLength, Index;
	UINT dwNextDataStartOffset = 4;
	DWORD i = 0;

	if (*thisr8)
		size = (*(QWORD(*)(QWORD*, const char*))(0x1409291C0))(thisr8, szBlacklistName);
	else
		size = 0;

	pThis->m_qwBufferSize = size;

	if (size < 1)
	{
		(*(void(*)(BannedWordChecker*))(0x140606AC0))(pThis);  //FreeBannedWordChecker
		return FALSE;
	}

	src = (*(LPCVOID(*)(QWORD*, const char*))(0x140928FD0))(thisr8, szBlacklistName);

	if (!src)
	{
		(*(void(*)(BannedWordChecker*))(0x140606AC0))(pThis);  //FreeBannedWordChecker
		return FALSE;
	}

	dst = (*(AllocHeapMemoryFn)(0x140920200))(size, (CHeapInstance**)0x14147B490);

	pThis->m_pBuffer = dst;

	if (!dst)
	{
		(*(void(*)(BannedWordChecker*))(0x140606AC0))(pThis);  //FreeBannedWordChecker
		return FALSE;
	}

	memcpy(dst, src, size);

	pThis->m_dwWordCount = *(DWORD*)pThis->m_pBuffer;

	if (pThis->m_dwWordCount < 1)
	{
		(*(void(*)(BannedWordChecker*))(0x140606AC0))(pThis);  //FreeBannedWordChecker
		return FALSE;
	}

	pThis->m_pEntries = (BannedWordChecker::WordEntry*)(*(AllocHeapMemoryFn)(0x140920200))(16 * pThis->m_dwWordCount, (CHeapInstance**)0x14147B490);

	if (!pThis->m_pEntries)
	{
		(*(void(*)(BannedWordChecker*))(0x140606AC0))(pThis);  //FreeBannedWordChecker
		return FALSE;
	}

	if (pThis->m_dwWordCount > 0)
	{
		do
		{
			dwDataStartOffset = dwNextDataStartOffset;
			dwDataEndOffset = dwNextDataStartOffset + 4;
			lpDataStart = (BYTE*)pThis->m_pBuffer + dwDataStartOffset;
			lpDataEnd = (BYTE*)pThis->m_pBuffer + dwDataEndOffset;
			pThis->m_pEntries[i].dwLength = *(DWORD*)lpDataStart;
			dwWordByteLength = pThis->m_pEntries[i].dwLength * sizeof(WCHAR);
			dwNextDataStartOffset = dwWordByteLength + dwDataEndOffset;

			if (dwWordByteLength)
			{
				pData = lpDataStart;
				Index = lpDataEnd - lpDataStart;
				length = dwWordByteLength;
				do
				{
					current_byte = (pData++)[Index];
					*(pData - 1) = current_byte - 19;
				} while (--length);
			}
			*((WCHAR*)&lpDataStart[dwWordByteLength]) = 0; 	//*((WCHAR*)(lpDataStart + dwWordByteLength)) = 0;
			pThis->m_pEntries[i++].lpszBannedWord = (LPWSTR)lpDataStart;
		} while (i < pThis->m_dwWordCount);
	}

	Features::DisableWordBlacklist(pThis, NULL);

	return TRUE;
#endif

	UNREFERENCED_PARAMETER(thisrdx);
	UNREFERENCED_PARAMETER(thisr8);
	UNREFERENCED_PARAMETER(szBlacklistName);

	Features::DisableWordBlacklist(pThis, NULL);
	return TRUE;
}

// for replacing the the startup picture but needs to be run really early to work
// doesn't get run in time
// g_pOleLoadPictureHook = new ImportTableHook("oleaut32.dll", "OleLoadPicture", (LPCVOID)hkOleLoadPicture);
// if you find a way to make it work you need this code ^^^^ (proxy dll)
HRESULT hkOleLoadPicture(LPSTREAM lpStream, LONG lSize, BOOL fRunmode, REFIID riid, LPVOID* lplpvObj)
{
	DWORD dwBytesRead;
	HANDLE hFile = CreateFileA("X:\\Pictures\\Anime\\4c165dbb0f55958a27312bff6c2d1b8b86d506d8.jpg", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile != INVALID_HANDLE_VALUE)
	{
		DWORD dwSize = GetFileSize(hFile, NULL);
		HGLOBAL hPicture = GlobalAlloc(GMEM_ZEROINIT, dwSize);

		if (hPicture)
		{
			ReadFile(hFile, hPicture, dwSize, &dwBytesRead, NULL);

			if (lpStream)
			{
				lpStream->Release();
				lpStream = NULL;
			}

			if (SUCCEEDED(CreateStreamOnHGlobal(hPicture, FALSE, &lpStream)))
			{
				HGLOBAL hOriginalPicture = (HGLOBAL*)0x141584E78; // FIXME: Untested and old version offset 

				if (hOriginalPicture)
				{
					GlobalFree(hOriginalPicture);
					hOriginalPicture = hPicture;
				}
			}
		}
		CloseHandle(hFile);
	}

	return ((OleLoadPictureFn)(g_pOleLoadPictureHook->GetOriginalFunction()))(lpStream, lSize, fRunmode, riid, lplpvObj);
}

void hkSaveFileIO(CSaveDataDevice* pSavedata)
{
	if (!pSavedata)
		return;

	switch (pSavedata->m_fFlags)
	{
	case SAVE_FLAGS_READ_SLOTS:
		ReadSaveSlots(pSavedata);
		return;
	case SAVE_FLAGS_READ:
		Vars.Misc.bLoading = true;
		ReadSaveData(pSavedata);

		// Follow the slot being played, for the Backup Save slot picker. GameData and
		// SystemData reads (negative slots) aren't something Backup Save can copy.
		if (IS_SAVE_SLOTDATA(pSavedata->m_nSlot))
			Vars.Misc.nSlot = pSavedata->m_nSlot;
		return;
	case SAVE_FLAGS_WRITE:
		WriteSaveData(pSavedata);
		return;
	case SAVE_FLAGS_DELETE:
		DeleteSaveData(pSavedata);
		return;
	}
	Vars.Misc.bLoading = false;
}

// Speedhack: the game's counter runs flSpeedMultiplier times as fast as the real one. The old
// formula, fake += cur + delta * mult, added the whole counter on every call, so the multiplier
// hardly mattered, and switching it off jumped back to the real counter, running time backwards.
// The game's counter is its own clock instead: seeded from the real one on the first call, it
// advances by every real delta, scaled while the speedhack is on and 1:1 while it is off, so it
// never jumps when it is switched either way (it stays ahead or behind the real counter by
// whatever it gained or lost). Many threads call this, so it is locked, and the real counter is
// read inside the lock so no thread can step the clock back with an older reading.
BOOL hkQueryPerformanceCounter(LARGE_INTEGER* lpPerformanceCount)
{
	constexpr double MIN_SPEED_MULTIPLIER = 0.01;
	static SRWLOCK s_Lock = SRWLOCK_INIT;
	static bool s_bSeeded = false;
	static LONGLONG s_llPrevious, s_llFake;
	static double s_dFraction;	// the scaled ticks below one, carried to the next call

	if (!lpPerformanceCount)
		return oQueryPerformanceCounter(lpPerformanceCount);

	LARGE_INTEGER liCurrent;

	AcquireSRWLockExclusive(&s_Lock);

	BOOL bResult = oQueryPerformanceCounter(&liCurrent);

	if (bResult)
	{
		if (!s_bSeeded)
		{
			s_llPrevious = s_llFake = liCurrent.QuadPart;
			s_bSeeded = true;
		}

		const LONGLONG llDelta = liCurrent.QuadPart - s_llPrevious;

		if (Vars.Gameplay.bSpeedhack)
		{
			const double dScaled = (double)llDelta * max((double)Vars.Gameplay.flSpeedMultiplier, MIN_SPEED_MULTIPLIER) + s_dFraction;
			const LONGLONG llWhole = (LONGLONG)dScaled;

			s_dFraction = dScaled - (double)llWhole;
			s_llFake += llWhole;
		}
		else
		{
			s_dFraction = 0.0;
			s_llFake += llDelta;
		}

		s_llPrevious = liCurrent.QuadPart;
		lpPerformanceCount->QuadPart = s_llFake;
	}

	ReleaseSRWLockExclusive(&s_Lock);

	return bResult;
}

LPTOP_LEVEL_EXCEPTION_FILTER hkSetUnhandledExceptionFilter(LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter)
{
	if (lpTopLevelExceptionFilter != &UnhandledExceptionHandler)
	{
		auto it = std::find(g_pExceptionHandlers.cbegin(), g_pExceptionHandlers.cend(), lpTopLevelExceptionFilter);

		if (it == g_pExceptionHandlers.cend())
			g_pExceptionHandlers.push_back(lpTopLevelExceptionFilter);
	}
	return &UnhandledExceptionHandler;
}

BOOL hkSetCursorPos(int X, int Y)
{
	if (g_pMenu->IsOpen())
		return FALSE;

	return oSetCursorPos(X, Y);
}

DWORD hkXInputGetState(DWORD dwUserIndex, PXINPUT_STATE pState)
{
	// Pretty much obselete code that was dumb
	//ZeroMemory(&Vars.Menu.Input.emulate, sizeof(XINPUT_STATE)); // need to zero out the input state before emulating new inputs

	//if (Vars.Menu.Input.emulate.Gamepad.wButtons & XINPUT_GAMEPAD_A)
	//	pState->Gamepad.wButtons |= XINPUT_GAMEPAD_A;

	//if (Vars.Menu.Input.emulate.Gamepad.wButtons & XINPUT_GAMEPAD_X)
	//	pState->Gamepad.wButtons |= XINPUT_GAMEPAD_X;

	//if (Vars.Menu.Input.emulate.Gamepad.wButtons & XINPUT_GAMEPAD_Y)
	//	pState->Gamepad.wButtons |= XINPUT_GAMEPAD_Y;

	//if (Vars.Menu.Input.emulate.Gamepad.wButtons & XINPUT_GAMEPAD_B)
	//	pState->Gamepad.wButtons |= XINPUT_GAMEPAD_B;

	return oXInputGetState(dwUserIndex, pState);
}

LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);

	// SetWindowLongPtr can hand back a handle rather than the procedure itself, which only
	// CallWindowProc knows how to call
	return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}
