#pragma once

#include "AppearanceTable.h"

class CBehaviorAppBase;
class CModelShader;
struct CMeshPart;

// Saved colour edits on the game's models: mesh colours, Model Tint and material tints, kept
// in CAppearanceTable (the config's <Appearance> section) by model and mesh or material name.
//
// The editors write a colour into the model they edit and record it here; copies of that model
// already given the saved colours get the edit (or reset) at the same time. Every other model
// with the same name gets the saved colours once, when it appears (Update), so they survive
// a relaunch, an area load or a respawn. After that the colours are left alone: the game,
// Rainbow Hair, first person and Hide Pod change them as they always have.
//
// Everything here runs on the thread that draws the menu (hkPresent).
namespace Appearance
{
	CAppearanceTable& GetTable(void);

	bool HasMeshColor(CBehaviorAppBase* pEntity, const CMeshPart* pMesh);
	// Writes vColor into pMesh of pEntity and saves it for every model of that name
	void RecordMeshColor(CBehaviorAppBase* pEntity, CMeshPart* pMesh, const ImVec4& vColor);
	// Puts back the colour the mesh had before it was first edited and forgets the edit
	void ResetMeshColor(CBehaviorAppBase* pEntity, CMeshPart* pMesh);

	bool HasModelTint(CBehaviorAppBase* pEntity);
	void RecordModelTint(CBehaviorAppBase* pEntity, const ImVec4& vColor);
	void ResetModelTint(CBehaviorAppBase* pEntity);

	// MaterialTint::Set / Clear for pShader of pEntity, saved by the material's name. The
	// tint is cleared again when pEntity goes away, which frees its slot. False if every
	// MaterialTint slot is taken (or material tint is unavailable on this build).
	bool SetMaterialTint(CBehaviorAppBase* pEntity, const CModelShader* pShader, const ImVec4& vColor, float flIntensity);
	void ClearMaterialTint(CBehaviorAppBase* pEntity, const CModelShader* pShader);

	// Once a frame: gives models that just appeared their saved colours, and frees the
	// material tints of models that are gone. Call it before MaterialTint::Update.
	void Update(void);

	// Before a config is loaded: puts the original colours back on the models the current
	// edits were written to and forgets them all, so the loaded config's edits replace them.
	void BeginLoad(void);

	// The edits changed since the config was last loaded or saved
	bool IsDirty(void);
	void MarkSaved(void);
}
