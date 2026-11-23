#pragma once

#include "WinInclude.h"
#include <xmllite.h>

#include <stdint.h>
#include <string>
#include <vector>

#include "imgui/imgui.h"

#include "ConfigItemXml.h"

// The saved colour edits (the <Appearance> section of the config), keyed by the model's
// object name ("pl0000" is 2B) and the mesh or material name. Pure data: nothing here touches
// the game. Appearance.h puts the entries on models.
//
//	<mesh model="pl0000" name="Hair">#FF4040FF</mesh>
//	<model_tint model="pl0000">#FFE0E0FF</model_tint>
//	<material model="pl0000" name="W_Skin" intensity="1.500">#FFD0C0FF</material>

enum AppearanceKind : int32_t
{
	APPEARANCE_MESH,		// CMeshPart::m_vColor
	APPEARANCE_MODEL_TINT,	// CModelInfo::m_vTint
	APPEARANCE_MATERIAL		// MaterialTint, with an intensity
};

struct AppearanceEntry
{
	AppearanceKind m_Kind = APPEARANCE_MESH;
	std::string m_Model;			// object name, "pl0000"
	std::string m_Name;				// mesh or material name; empty for a model tint
	ImVec4 m_vColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	float m_flIntensity = 1.0f;		// materials only

	// Runtime only, never saved: the colour the model had before the first write, which
	// Reset and loading another config put back
	ImVec4 m_vOriginal = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	bool m_bHaveOriginal = false;
};

class CAppearanceTable
{
public:
	// The intensity range of the material tint editor's slider
	static constexpr float MAX_INTENSITY = 50.0f;

	// szName is ignored (may be NULL) for a model tint
	AppearanceEntry* Find(AppearanceKind Kind, const char* szModel, const char* szName);

	// Adds the entry or updates its colour; an existing entry keeps its original
	AppearanceEntry& Set(AppearanceKind Kind, const char* szModel, const char* szName, const ImVec4& vColor,
		float flIntensity = 1.0f);

	bool Remove(AppearanceKind Kind, const char* szModel, const char* szName);
	void Clear(void);

	bool HasModel(const char* szModel) const;

	std::vector<AppearanceEntry>& GetEntries(void) { return m_Entries; }
	const std::vector<AppearanceEntry>& GetEntries(void) const { return m_Entries; }

	// One element of the section: its name, its attributes (NULL when absent) and its text.
	// False, with the table unchanged, for an unknown element, a missing model or name, or
	// a colour that isn't #RRGGBBAA.
	bool ReadElement(LPCWSTR szElement, LPCWSTR szModel, LPCWSTR szName, LPCWSTR szIntensity, LPCWSTR szColor);

	// Every entry as an element, in the order they were added
	HRESULT Write(IXmlWriter* pWriter) const;

private:
	std::vector<AppearanceEntry> m_Entries;
};

// The config item for the whole <Appearance> section: it takes every element in it
// (MatchesAnyName), reads the names from the attributes, and writes every entry.
class CAppearanceItem : public IConfigItemXml
{
public:
	explicit CAppearanceItem(CAppearanceTable& Table)
		: m_Table(Table)
	{
	}

	virtual const wchar_t* GetName(void) const override
	{
		return L"appearance";
	}

	virtual bool MatchesAnyName(void) const override
	{
		return true;
	}

	virtual void ReadAttributes(IXmlReader* pReader) override;
	virtual void ReadValue(LPCWSTR szValue, uint32_t cchValue) override;
	virtual HRESULT Write(IXmlWriter* pWriter) override;

private:
	CAppearanceTable& m_Table;

	// The element ReadAttributes saw, until its text arrives
	std::wstring m_Element;
	std::wstring m_Model;
	std::wstring m_Name;
	std::wstring m_Intensity;
	bool m_bHaveModel = false;
	bool m_bHaveName = false;
	bool m_bHaveIntensity = false;
};
