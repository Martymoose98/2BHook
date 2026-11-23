#include "Appearance.h"

#include <algorithm>
#include <vector>

// Configuration.h before Features.h: it is how the circle Features.h -> Console.h ->
// Configuration.h -> Features.h is meant to be entered (see Configuration.cpp)
#include "Configuration.h"
#include "Features.h"
#include "MaterialTint.h"

namespace
{
	// A model the saved colours were written to (or that an editor changed), by handle, never
	// by pointer: the entity can be gone by the next frame, and GetEntityFromHandleGlobal
	// rejects a handle whose list slot is free or reused.
	struct Applied
	{
		EntityHandle_t m_hEntity;
		std::vector<const CModelShader*> m_Tints;	// MaterialTint slots this model holds
	};

	CAppearanceTable s_Table;
	std::vector<Applied> s_Applied;
	bool s_bDirty = false;
	bool s_bWarnedNoSlots = false;

	ImVec4 ToImVec4(const Vector4& v)
	{
		return ImVec4(v.x, v.y, v.z, v.w);
	}

	Vector4 ToVector4(const ImVec4& v)
	{
		return Vector4(v.x, v.y, v.z, v.w);
	}

	// The entity's object name, "pl0000" for 2B: what the saved colours are keyed by
	bool GetModelName(const CBehaviorAppBase* pEntity, char (&szModel)[16])
	{
		if (!pEntity || !pEntity->m_pInfo)
			return false;

		return ObjectIdToObjectName(szModel, sizeof(szModel), pEntity->m_pInfo->m_ObjectId, NULL) != FALSE;
	}

	const char* GetMaterialName(const CModelShader* pShader)
	{
		return (pShader && pShader->m_pMaterial) ? pShader->m_pMaterial->m_szName : NULL;
	}

	Applied* FindApplied(EntityHandle_t hEntity)
	{
		for (Applied& Entry : s_Applied)
		{
			if (Entry.m_hEntity.m_uValue == hEntity.m_uValue)
				return &Entry;
		}

		return NULL;
	}

	// The record for a model an editor just changed. It counts as applied from then on, so
	// Update doesn't write the saved colours over it again.
	Applied& EnsureApplied(CBehaviorAppBase* pEntity)
	{
		const EntityHandle_t hEntity = pEntity->m_pInfo->m_hEntity;

		if (Applied* pApplied = FindApplied(hEntity))
			return *pApplied;

		s_Applied.push_back({ hEntity, {} });

		return s_Applied.back();
	}

	void WarnNoTintSlots(void)
	{
		if (s_bWarnedNoSlots)
			return;

		s_bWarnedNoSlots = true;

		WARN("Appearance: every material tint slot is taken, some saved material tints were not applied\n");
		g_pConsole->Warn("Material tint: every slot is taken, so some saved material tints were not applied. "
			"Clear material tints you don't need.");
	}

	// The colour to write into a mesh. While first person hides the mesh (alpha 0, held every
	// frame) only its colour changes, and the alpha it will put back is what counts as the
	// mesh's alpha.
	ImVec4 MeshColorToWrite(const CBehaviorAppBase* pEntity, const CMeshPart* pMesh, const ImVec4& vColor)
	{
		float flRestoreAlpha;

		if (Features::GetFirstPersonRestoreAlpha(pEntity, pMesh->m_szMeshName, flRestoreAlpha))
			return ImVec4(vColor.x, vColor.y, vColor.z, pMesh->m_vColor.w);

		return vColor;
	}

	ImVec4 CurrentMeshColor(const CBehaviorAppBase* pEntity, const CMeshPart* pMesh)
	{
		ImVec4 vColor = ToImVec4(pMesh->m_vColor);
		float flRestoreAlpha;

		if (Features::GetFirstPersonRestoreAlpha(pEntity, pMesh->m_szMeshName, flRestoreAlpha))
			vColor.w = flRestoreAlpha;

		return vColor;
	}

	void WriteMeshColor(CMeshPart* pMesh, const ImVec4& vColor)
	{
		pMesh->m_vColor = ToVector4(vColor);
		pMesh->m_bUpdate = TRUE;
	}

	// CModelInfo_SetColor (0x140198C00) raises the dirty flag with every change, and
	// CModelWork_Update (0x140196620) needs it to pass the colour on to the draw entries
	void WriteModelTint(CModelInfo* pInfo, const ImVec4& vColor)
	{
		pInfo->m_vTint = ToVector4(vColor);
		pInfo->m_uFlags |= MODELINFO_FLAG_TINT_DIRTY;
	}

	// MaterialTint::Set on every material of pEntity named szMaterial (the editor lists them
	// per mesh), each owned by Record so the slot is freed with the model
	void SetMaterialTintByName(CBehaviorAppBase* pEntity, Applied& Record, const char* szMaterial,
		const ImVec4& vColor, float flIntensity)
	{
		CModelWork* pWork = &pEntity->m_Work;

		if (!MaterialTint::IsAvailable() || !pWork->m_pModelShaders || !szMaterial)
			return;

		for (int32_t i = 0; i < pWork->m_nMaterialShaders; ++i)
		{
			const CModelShader* pShader = &pWork->m_pModelShaders[i];
			const char* szName = GetMaterialName(pShader);

			if (!szName || strcmp(szName, szMaterial))
				continue;

			if (!MaterialTint::Set(pShader, vColor, flIntensity))
			{
				WarnNoTintSlots();
				continue;
			}

			if (std::find(Record.m_Tints.begin(), Record.m_Tints.end(), pShader) == Record.m_Tints.end())
				Record.m_Tints.push_back(pShader);
		}
	}

	void ClearMaterialTintByName(CBehaviorAppBase* pEntity, Applied& Record, const char* szMaterial)
	{
		CModelWork* pWork = &pEntity->m_Work;

		if (!pWork->m_pModelShaders || !szMaterial)
			return;

		for (int32_t i = 0; i < pWork->m_nMaterialShaders; ++i)
		{
			const CModelShader* pShader = &pWork->m_pModelShaders[i];
			const char* szName = GetMaterialName(pShader);

			if (!szName || strcmp(szName, szMaterial))
				continue;

			MaterialTint::Clear(pShader);
			Record.m_Tints.erase(std::remove(Record.m_Tints.begin(), Record.m_Tints.end(), pShader), Record.m_Tints.end());
		}
	}

	// Writes every saved colour of szModel into a model that just appeared
	void ApplyTo(CBehaviorAppBase* pEntity, EntityHandle_t hEntity, const char* szModel)
	{
		Applied Record = { hEntity, {} };
		CModelWork* pWork = &pEntity->m_Work;

		for (AppearanceEntry& Entry : s_Table.GetEntries())
		{
			if (Entry.m_Model != szModel)
				continue;

			switch (Entry.m_Kind)
			{
			case APPEARANCE_MESH:
			{
				CMeshPart* pMesh = GetModelMesh(pWork, Entry.m_Name.c_str());

				if (!pMesh)
					break;

				if (!Entry.m_bHaveOriginal)
				{
					Entry.m_vOriginal = CurrentMeshColor(pEntity, pMesh);
					Entry.m_bHaveOriginal = true;
				}

				WriteMeshColor(pMesh, MeshColorToWrite(pEntity, pMesh, Entry.m_vColor));
				break;
			}

			case APPEARANCE_MODEL_TINT:
				if (!pEntity->m_pModelInfo)
					break;

				if (!Entry.m_bHaveOriginal)
				{
					Entry.m_vOriginal = ToImVec4(pEntity->m_pModelInfo->m_vTint);
					Entry.m_bHaveOriginal = true;
				}

				WriteModelTint(pEntity->m_pModelInfo, Entry.m_vColor);
				break;

			case APPEARANCE_MATERIAL:
				SetMaterialTintByName(pEntity, Record, Entry.m_Name.c_str(), Entry.m_vColor, Entry.m_flIntensity);
				break;
			}
		}

		s_Applied.push_back(Record);
	}

	// Calls Fn(pCopy, Record) for every other live model of szModel the saved colours were
	// written to, so an edit or a reset shows on every copy at once and not only on the next
	// load. Copies not written to yet get the table's colour when Update reaches them. Fn must
	// not add to s_Applied.
	template<typename FnType>
	void ForEachCopy(const CBehaviorAppBase* pEntity, const char* szModel, FnType&& Fn)
	{
		for (Applied& Record : s_Applied)
		{
			EntityHandle_t hEntity = Record.m_hEntity;
			CBehaviorAppBase* pCopy = GetEntityFromHandleGlobal(&hEntity);
			char szCopy[16];

			if (!pCopy || pCopy == pEntity || !GetModelName(pCopy, szCopy) || strcmp(szCopy, szModel))
				continue;

			Fn(pCopy, Record);
		}
	}

	// The original colours back on one model, for the edits of its name
	void RestoreOriginals(CBehaviorAppBase* pEntity, const char* szModel)
	{
		for (const AppearanceEntry& Entry : s_Table.GetEntries())
		{
			if (Entry.m_Model != szModel || !Entry.m_bHaveOriginal)
				continue;

			if (Entry.m_Kind == APPEARANCE_MESH)
			{
				CMeshPart* pMesh = GetModelMesh(&pEntity->m_Work, Entry.m_Name.c_str());

				if (pMesh)
					WriteMeshColor(pMesh, MeshColorToWrite(pEntity, pMesh, Entry.m_vOriginal));
			}
			else if (Entry.m_Kind == APPEARANCE_MODEL_TINT && pEntity->m_pModelInfo)
			{
				WriteModelTint(pEntity->m_pModelInfo, Entry.m_vOriginal);
			}
		}
	}
}

CAppearanceTable& Appearance::GetTable(void)
{
	return s_Table;
}

bool Appearance::HasMeshColor(CBehaviorAppBase* pEntity, const CMeshPart* pMesh)
{
	char szModel[16];

	return pMesh && pMesh->m_szMeshName && GetModelName(pEntity, szModel) &&
		s_Table.Find(APPEARANCE_MESH, szModel, pMesh->m_szMeshName);
}

void Appearance::RecordMeshColor(CBehaviorAppBase* pEntity, CMeshPart* pMesh, const ImVec4& vColor)
{
	char szModel[16];

	if (!pMesh || !pMesh->m_szMeshName || !GetModelName(pEntity, szModel))
		return;

	// While first person hides the mesh, the picker shows (and hands back) its alpha of 0
	ImVec4 vSaved = vColor;
	float flRestoreAlpha;

	if (Features::GetFirstPersonRestoreAlpha(pEntity, pMesh->m_szMeshName, flRestoreAlpha))
		vSaved.w = flRestoreAlpha;

	const ImVec4 vBefore = CurrentMeshColor(pEntity, pMesh);
	AppearanceEntry& Entry = s_Table.Set(APPEARANCE_MESH, szModel, pMesh->m_szMeshName, vSaved);

	if (!Entry.m_bHaveOriginal)
	{
		Entry.m_vOriginal = vBefore;
		Entry.m_bHaveOriginal = true;
	}

	WriteMeshColor(pMesh, MeshColorToWrite(pEntity, pMesh, vSaved));
	EnsureApplied(pEntity);
	s_bDirty = true;

	const char* szMesh = pMesh->m_szMeshName;

	ForEachCopy(pEntity, szModel, [&](CBehaviorAppBase* pCopy, Applied&)
	{
		if (CMeshPart* pCopyMesh = GetModelMesh(&pCopy->m_Work, szMesh))
			WriteMeshColor(pCopyMesh, MeshColorToWrite(pCopy, pCopyMesh, vSaved));
	});
}

void Appearance::ResetMeshColor(CBehaviorAppBase* pEntity, CMeshPart* pMesh)
{
	char szModel[16];

	if (!pMesh || !pMesh->m_szMeshName || !GetModelName(pEntity, szModel))
		return;

	const AppearanceEntry* pEntry = s_Table.Find(APPEARANCE_MESH, szModel, pMesh->m_szMeshName);

	if (!pEntry)
		return;

	const char* szMesh = pMesh->m_szMeshName;

	if (pEntry->m_bHaveOriginal)
	{
		const ImVec4 vOriginal = pEntry->m_vOriginal;

		WriteMeshColor(pMesh, MeshColorToWrite(pEntity, pMesh, vOriginal));

		ForEachCopy(pEntity, szModel, [&](CBehaviorAppBase* pCopy, Applied&)
		{
			if (CMeshPart* pCopyMesh = GetModelMesh(&pCopy->m_Work, szMesh))
				WriteMeshColor(pCopyMesh, MeshColorToWrite(pCopy, pCopyMesh, vOriginal));
		});
	}

	s_Table.Remove(APPEARANCE_MESH, szModel, szMesh);
	s_bDirty = true;
}

bool Appearance::HasModelTint(CBehaviorAppBase* pEntity)
{
	char szModel[16];

	return GetModelName(pEntity, szModel) && s_Table.Find(APPEARANCE_MODEL_TINT, szModel, NULL);
}

void Appearance::RecordModelTint(CBehaviorAppBase* pEntity, const ImVec4& vColor)
{
	char szModel[16];

	if (!GetModelName(pEntity, szModel) || !pEntity->m_pModelInfo)
		return;

	const ImVec4 vBefore = ToImVec4(pEntity->m_pModelInfo->m_vTint);
	AppearanceEntry& Entry = s_Table.Set(APPEARANCE_MODEL_TINT, szModel, NULL, vColor);

	if (!Entry.m_bHaveOriginal)
	{
		Entry.m_vOriginal = vBefore;
		Entry.m_bHaveOriginal = true;
	}

	WriteModelTint(pEntity->m_pModelInfo, vColor);
	EnsureApplied(pEntity);
	s_bDirty = true;

	ForEachCopy(pEntity, szModel, [&](CBehaviorAppBase* pCopy, Applied&)
	{
		if (pCopy->m_pModelInfo)
			WriteModelTint(pCopy->m_pModelInfo, vColor);
	});
}

void Appearance::ResetModelTint(CBehaviorAppBase* pEntity)
{
	char szModel[16];

	if (!GetModelName(pEntity, szModel))
		return;

	const AppearanceEntry* pEntry = s_Table.Find(APPEARANCE_MODEL_TINT, szModel, NULL);

	if (!pEntry)
		return;

	if (pEntry->m_bHaveOriginal)
	{
		const ImVec4 vOriginal = pEntry->m_vOriginal;

		if (pEntity->m_pModelInfo)
			WriteModelTint(pEntity->m_pModelInfo, vOriginal);

		ForEachCopy(pEntity, szModel, [&](CBehaviorAppBase* pCopy, Applied&)
		{
			if (pCopy->m_pModelInfo)
				WriteModelTint(pCopy->m_pModelInfo, vOriginal);
		});
	}

	s_Table.Remove(APPEARANCE_MODEL_TINT, szModel, NULL);
	s_bDirty = true;
}

bool Appearance::SetMaterialTint(CBehaviorAppBase* pEntity, const CModelShader* pShader, const ImVec4& vColor, float flIntensity)
{
	char szModel[16];

	if (!MaterialTint::Set(pShader, vColor, flIntensity))
		return false;

	// Owned by the model, so the slot is freed when the model goes away
	if (pEntity && pEntity->m_pInfo)
	{
		Applied& Record = EnsureApplied(pEntity);

		if (std::find(Record.m_Tints.begin(), Record.m_Tints.end(), pShader) == Record.m_Tints.end())
			Record.m_Tints.push_back(pShader);
	}

	// A material without a name, or on a model without an object name, is tinted for the
	// session but can't be saved
	const char* szMaterial = GetMaterialName(pShader);

	if (szMaterial && GetModelName(pEntity, szModel))
	{
		s_Table.Set(APPEARANCE_MATERIAL, szModel, szMaterial, vColor, flIntensity);
		s_bDirty = true;

		ForEachCopy(pEntity, szModel, [&](CBehaviorAppBase* pCopy, Applied& Record)
		{
			SetMaterialTintByName(pCopy, Record, szMaterial, vColor, flIntensity);
		});
	}

	return true;
}

void Appearance::ClearMaterialTint(CBehaviorAppBase* pEntity, const CModelShader* pShader)
{
	char szModel[16];

	MaterialTint::Clear(pShader);

	if (pEntity && pEntity->m_pInfo)
	{
		if (Applied* pRecord = FindApplied(pEntity->m_pInfo->m_hEntity))
			pRecord->m_Tints.erase(std::remove(pRecord->m_Tints.begin(), pRecord->m_Tints.end(), pShader), pRecord->m_Tints.end());
	}

	const char* szMaterial = GetMaterialName(pShader);

	if (!szMaterial || !GetModelName(pEntity, szModel))
		return;

	if (s_Table.Remove(APPEARANCE_MATERIAL, szModel, szMaterial))
		s_bDirty = true;

	ForEachCopy(pEntity, szModel, [&](CBehaviorAppBase* pCopy, Applied& Record)
	{
		ClearMaterialTintByName(pCopy, Record, szMaterial);
	});
}

void Appearance::Update(void)
{
	// Models that are gone give their material tint slots back. Their shader pointers are
	// stale, but MaterialTint::Clear only compares them against its own slots.
	for (size_t i = 0; i < s_Applied.size(); )
	{
		EntityHandle_t hEntity = s_Applied[i].m_hEntity;

		if (GetEntityFromHandleGlobal(&hEntity))
		{
			++i;
			continue;
		}

		for (const CModelShader* pShader : s_Applied[i].m_Tints)
			MaterialTint::Clear(pShader);

		s_Applied.erase(s_Applied.begin() + i);
	}

	if (s_Table.GetEntries().empty() || !g_pEntityList || !g_pEntityList->m_pItems)
		return;

	// Every live slot, tested the way GetEntityInfoFromHandle (0x140745F20) does (see the
	// entity view in Menu.cpp). Slots are reused out of order, so all of them are walked.
	for (uint32_t uSlot = 0; uSlot < g_pEntityList->m_uItems; ++uSlot)
	{
		const CEntityInfo* pInfo = g_pEntityList->m_pItems[uSlot].second;

		if (!pInfo || (pInfo->m_Flags & 3))
			continue;

		EntityHandle_t hEntity = pInfo->m_hEntity;

		if (FindApplied(hEntity))
			continue;

		char szModel[16];

		if (!ObjectIdToObjectName(szModel, sizeof(szModel), pInfo->m_ObjectId, NULL) || !s_Table.HasModel(szModel))
			continue;

		CBehaviorAppBase* pEntity = GetEntityFromHandleGlobal(&hEntity);

		// Not set up yet: tried again next frame. CModelWork::Setup (0x140195620) sets
		// m_pModelData last, after CModelWork_CreateMeshParts (0x140194FC0) has named every
		// mesh and CMeshPart_Reset (0x140193D00) has set its colour to white.
		if (!pEntity || pEntity->m_pInfo != pInfo || !pEntity->m_Work.m_pModelData ||
			pEntity->m_Work.m_nMeshes <= 0 || !pEntity->m_Work.m_pMeshes)
			continue;

		ApplyTo(pEntity, hEntity, szModel);
	}
}

void Appearance::BeginLoad(void)
{
	for (Applied& Record : s_Applied)
	{
		for (const CModelShader* pShader : Record.m_Tints)
			MaterialTint::Clear(pShader);

		EntityHandle_t hEntity = Record.m_hEntity;
		CBehaviorAppBase* pEntity = GetEntityFromHandleGlobal(&hEntity);
		char szModel[16];

		if (pEntity && GetModelName(pEntity, szModel))
			RestoreOriginals(pEntity, szModel);
	}

	s_Applied.clear();
	s_Table.Clear();
	s_bDirty = false;
}

bool Appearance::IsDirty(void)
{
	return s_bDirty;
}

void Appearance::MarkSaved(void)
{
	s_bDirty = false;
}
