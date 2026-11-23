#include "AppearanceTable.h"

#include <stdlib.h>
#include <string.h>

#include "Theme.h"

namespace
{
	const wchar_t* const ELEMENT_MESH = L"mesh";
	const wchar_t* const ELEMENT_MODEL_TINT = L"model_tint";
	const wchar_t* const ELEMENT_MATERIAL = L"material";

	bool KindHasName(AppearanceKind Kind)
	{
		return Kind != APPEARANCE_MODEL_TINT;
	}

	std::string ToUtf8(LPCWSTR szText)
	{
		const int cb = WideCharToMultiByte(CP_UTF8, 0, szText, -1, NULL, 0, NULL, NULL);

		if (cb <= 1)
			return std::string();

		std::string Out(cb - 1, '\0');
		WideCharToMultiByte(CP_UTF8, 0, szText, -1, &Out[0], cb, NULL, NULL);

		return Out;
	}

	std::wstring ToWide(const std::string& Text)
	{
		const int cch = MultiByteToWideChar(CP_UTF8, 0, Text.c_str(), -1, NULL, 0);

		if (cch <= 1)
			return std::wstring();

		std::wstring Out(cch - 1, L'\0');
		MultiByteToWideChar(CP_UTF8, 0, Text.c_str(), -1, &Out[0], cch);

		return Out;
	}

	bool Matches(const AppearanceEntry& Entry, AppearanceKind Kind, const char* szModel, const char* szName)
	{
		return Entry.m_Kind == Kind && szModel && Entry.m_Model == szModel &&
			(!KindHasName(Kind) || (szName && Entry.m_Name == szName));
	}
}

AppearanceEntry* CAppearanceTable::Find(AppearanceKind Kind, const char* szModel, const char* szName)
{
	for (AppearanceEntry& Entry : m_Entries)
	{
		if (Matches(Entry, Kind, szModel, szName))
			return &Entry;
	}

	return NULL;
}

AppearanceEntry& CAppearanceTable::Set(AppearanceKind Kind, const char* szModel, const char* szName,
	const ImVec4& vColor, float flIntensity)
{
	AppearanceEntry* pEntry = Find(Kind, szModel, szName);

	if (!pEntry)
	{
		AppearanceEntry Entry;

		Entry.m_Kind = Kind;
		Entry.m_Model = szModel ? szModel : "";
		Entry.m_Name = (KindHasName(Kind) && szName) ? szName : "";

		m_Entries.push_back(Entry);
		pEntry = &m_Entries.back();
	}

	pEntry->m_vColor = vColor;
	pEntry->m_flIntensity = flIntensity;

	return *pEntry;
}

bool CAppearanceTable::Remove(AppearanceKind Kind, const char* szModel, const char* szName)
{
	for (auto It = m_Entries.begin(); It != m_Entries.end(); ++It)
	{
		if (Matches(*It, Kind, szModel, szName))
		{
			m_Entries.erase(It);
			return true;
		}
	}

	return false;
}

void CAppearanceTable::Clear(void)
{
	m_Entries.clear();
}

bool CAppearanceTable::HasModel(const char* szModel) const
{
	for (const AppearanceEntry& Entry : m_Entries)
	{
		if (szModel && Entry.m_Model == szModel)
			return true;
	}

	return false;
}

bool CAppearanceTable::ReadElement(LPCWSTR szElement, LPCWSTR szModel, LPCWSTR szName, LPCWSTR szIntensity,
	LPCWSTR szColor)
{
	AppearanceKind Kind;

	if (!szElement)
		return false;

	if (!wcscmp(szElement, ELEMENT_MESH))
		Kind = APPEARANCE_MESH;
	else if (!wcscmp(szElement, ELEMENT_MODEL_TINT))
		Kind = APPEARANCE_MODEL_TINT;
	else if (!wcscmp(szElement, ELEMENT_MATERIAL))
		Kind = APPEARANCE_MATERIAL;
	else
		return false;

	if (!szModel || !*szModel || (KindHasName(Kind) && (!szName || !*szName)))
		return false;

	ImVec4 vColor;

	if (!Theme::ParseColor(szColor, vColor))
		return false;

	float flIntensity = 1.0f;

	if (Kind == APPEARANCE_MATERIAL && szIntensity && *szIntensity)
	{
		flIntensity = (float)_wtof(szIntensity);
		flIntensity = (flIntensity < 0.0f) ? 0.0f : (flIntensity > MAX_INTENSITY) ? MAX_INTENSITY : flIntensity;
	}

	const std::string Model = ToUtf8(szModel);
	const std::string Name = KindHasName(Kind) ? ToUtf8(szName) : std::string();

	Set(Kind, Model.c_str(), Name.c_str(), vColor, flIntensity);

	return true;
}

HRESULT CAppearanceTable::Write(IXmlWriter* pWriter) const
{
	HRESULT hr = S_OK;

	for (const AppearanceEntry& Entry : m_Entries)
	{
		const wchar_t* szElement = (Entry.m_Kind == APPEARANCE_MESH) ? ELEMENT_MESH :
			(Entry.m_Kind == APPEARANCE_MODEL_TINT) ? ELEMENT_MODEL_TINT : ELEMENT_MATERIAL;
		wchar_t szColor[16];

		Theme::FormatColor(Entry.m_vColor, szColor);

		if (FAILED(hr = pWriter->WriteStartElement(NULL, szElement, NULL)) ||
			FAILED(hr = pWriter->WriteAttributeString(NULL, L"model", NULL, ToWide(Entry.m_Model).c_str())))
			return hr;

		if (KindHasName(Entry.m_Kind) &&
			FAILED(hr = pWriter->WriteAttributeString(NULL, L"name", NULL, ToWide(Entry.m_Name).c_str())))
			return hr;

		if (Entry.m_Kind == APPEARANCE_MATERIAL)
		{
			wchar_t szIntensity[32];

			swprintf_s(szIntensity, L"%.3f", Entry.m_flIntensity);

			if (FAILED(hr = pWriter->WriteAttributeString(NULL, L"intensity", NULL, szIntensity)))
				return hr;
		}

		if (FAILED(hr = pWriter->WriteString(szColor)) ||
			FAILED(hr = pWriter->WriteFullEndElement()))
			return hr;
	}

	return hr;
}

void CAppearanceItem::ReadAttributes(IXmlReader* pReader)
{
	LPCWSTR szElement = NULL;

	m_bHaveModel = m_bHaveName = m_bHaveIntensity = false;
	m_Element.clear();

	// The element's name first: on an attribute, GetLocalName gives the attribute's
	if (FAILED(pReader->GetLocalName(&szElement, NULL)) || !szElement)
		return;

	m_Element = szElement;

	for (HRESULT hr = pReader->MoveToFirstAttribute(); hr == S_OK; hr = pReader->MoveToNextAttribute())
	{
		LPCWSTR szAttribute = NULL, szValue = NULL;

		if (FAILED(pReader->GetLocalName(&szAttribute, NULL)) || FAILED(pReader->GetValue(&szValue, NULL)) ||
			!szAttribute || !szValue)
			continue;

		if (!wcscmp(szAttribute, L"model"))
		{
			m_Model = szValue;
			m_bHaveModel = true;
		}
		else if (!wcscmp(szAttribute, L"name"))
		{
			m_Name = szValue;
			m_bHaveName = true;
		}
		else if (!wcscmp(szAttribute, L"intensity"))
		{
			m_Intensity = szValue;
			m_bHaveIntensity = true;
		}
	}
}

void CAppearanceItem::ReadValue(LPCWSTR szValue, uint32_t cchValue)
{
	UNREFERENCED_PARAMETER(cchValue);

	if (!m_Element.empty())
	{
		m_Table.ReadElement(m_Element.c_str(), m_bHaveModel ? m_Model.c_str() : NULL,
			m_bHaveName ? m_Name.c_str() : NULL, m_bHaveIntensity ? m_Intensity.c_str() : NULL, szValue);
	}

	// One element's attributes are never used for the next
	m_Element.clear();
	m_bHaveModel = m_bHaveName = m_bHaveIntensity = false;
}

HRESULT CAppearanceItem::Write(IXmlWriter* pWriter)
{
	return m_Table.Write(pWriter);
}
