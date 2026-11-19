#pragma once

#include <Windows.h>

#include "ImGui/imgui.h"

class CModelShader;

// Per-material tint (issue #16).
//
// A mesh's colour (CMeshPart::m_vColor) multiplies the whole mesh, and 2B's "Body" mesh holds
// the dress, tights, skin and boots as separate materials. The engine uploads that colour into
// a 16-byte constant buffer (the pixel shader's cb4, g_MeshColor; CModelWork::m_pSamplerParams,
// shared by consecutive meshes of the same colour), and CModelWork::FindModelEntryData copies
// a pointer to it into every draw entry it builds (CModelEntryData::m_pSamplerParam, +0x30).
// Detouring that function and pointing the entries of one material at
// our own buffer gives that material its own colour without touching the rest of the mesh.
// The tint stacks with the mesh colour: each engine colour buffer the material is drawn with
// gets a buffer of ours holding that colour x tint, with its alpha, so Rainbow Hair, the mesh
// colour picker and first person's hiding still show through a tinted material.
namespace MaterialTint
{
	// Finds the engine functions and installs the detour. False (and the feature stays off)
	// if a signature is missing on this build.
	bool Init(void);
	void Shutdown(void);

	bool IsAvailable(void);

	// pShader is the material's CModelShader in its model's CModelWork::m_pModelShaders, the
	// same pointer the engine stores at CModelEntryData+0x00. The final colour is the mesh
	// colour * vColor * flIntensity with the mesh's alpha; an intensity above 1 can lift
	// near-black materials.
	bool Get(const CModelShader* pShader, ImVec4& vColor, float& flIntensity);
	bool Set(const CModelShader* pShader, const ImVec4& vColor, float flIntensity);
	void Clear(const CModelShader* pShader);
	void ClearAll(void);

	// Uploads changed colours. Call once per frame; the engine accepts one update per
	// buffer per frame, so a refused upload is retried on the next call.
	void Update(void);
}
