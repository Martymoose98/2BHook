#pragma once

#include "WinInclude.h"
#include <d3d11.h>
#include <stdint.h>

// The ESP effects that have to wait for the whole frame: the chams' hidden colour and the glow.
//
// Drawn in place, the hidden colour (a target's draw with the depth test off) only stayed where
// the game had drawn the wall in front of it earlier in the frame: anything it drew afterwards
// still covered it, and the game's draw order decided which walls the chams showed through. So
// each of a target's colour draws also goes into a mask (BeginMaskDraw/EndMaskDraw around the
// draw): its colour and flags in one target, its depth in a depth buffer of the same size as the
// game's. After the frame, Composite compares that depth with the game's own and fills in the
// hidden colour wherever the game drew something nearer; the glow is the mask blurred, added
// around the silhouettes, through walls too.
namespace EspRenderer
{
	// What a target's silhouette in the mask is for
	enum MaskFlags_t : uint32_t
	{
		ESPMASK_PRESENT = 1,	// set for every target
		ESPMASK_CHAMS = 2,		// the hidden colour where something is in front of it
		ESPMASK_GLOW = 4,		// a glow around it
	};

	// Compiles the shaders and makes the states. The targets are made the first time a target is
	// drawn, at the size of the game's depth buffer.
	bool Initialize(ID3D11Device* pDevice);

	void Shutdown(void);

	// Whether the draw being made goes into the game's main depth buffer, which the mask follows:
	// the one the size of the frame, multisampled or not, first drawn into this frame (the game has
	// two and can switch between them). The targets are drawn into the 2048x2048 shadow map too,
	// where their silhouettes are from the light's view; those draws are left alone. True when
	// EspRenderer couldn't start, so the seen colour still works.
	bool IsMainPass(ID3D11DeviceContext* pContext);

	// One line for the menu: the depth buffer followed last frame and the ones left out
	void GetStatus(char* szStatus, size_t cchStatus);

	// From hkDrawIndexed, around one of a target's colour draws: binds the mask, the mask's depth
	// test and a flat colour in place of the game's targets and pixel shader. False, with nothing
	// changed, when the draw can't go into the mask (not the main pass, or the targets couldn't be
	// made). The game's pixel shader constants are left alone.
	bool BeginMaskDraw(ID3D11DeviceContext* pContext, const float Color[3], uint32_t uFlags);

	// Puts back what BeginMaskDraw replaced
	void EndMaskDraw(ID3D11DeviceContext* pContext);

	// From hkPresent, before the menu: draws the hidden colour and the glow into the back buffer
	// and leaves the game's state as it was. The back buffer's size picks the game's main depth
	// buffer. The mask is cleared at its first draw of the next frame.
	void Composite(ID3D11DeviceContext* pContext, ID3D11RenderTargetView* pBackBuffer);
}
