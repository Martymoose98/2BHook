#pragma once

#include "WinInclude.h"
#include <xmllite.h>

#include <stdint.h>
#include <utility>

// The interface of one item of the XML config. It lives apart from Configuration.h, which
// pulls in the game headers, so items that don't touch the game (AppearanceTable.h) can be
// built and tested without them.

template<typename T>
class CConfigItemXml;	// Configuration.h

class IConfigItemXml
{
public:
	virtual ~IConfigItemXml(void) = default;

	// The element name this item is stored under, e.g. L"t_godmode"
	virtual const wchar_t* GetName(void) const = 0;

	// Takes the text of this item's element. CConfigXml::Load finds the item by its section
	// and name, so items can be added, removed or reordered without breaking old configs.
	virtual void ReadValue(LPCWSTR szValue, uint32_t cchValue) = 0;

	// Called when CConfigXml::Load finds this item's element, before its text, with the
	// reader on the element, so an item that keeps data in attributes can read them. Most
	// items have none.
	virtual void ReadAttributes(IXmlReader* pReader)
	{
		UNREFERENCED_PARAMETER(pReader);
	}

	// True for an item that takes every element of its section, whatever its name: a list
	// such as <Appearance>, whose entries all share a few element names. CConfigXml::FindItem
	// falls back to it when no item has the element's name.
	virtual bool MatchesAnyName(void) const
	{
		return false;
	}

	virtual HRESULT Write(IXmlWriter* pWriter) = 0;

	//template<typename T>
	//static constexpr std::pair<const wchar_t*, IConfigItemXml*> MakePair(const wchar_t* szSector,
	//	const wchar_t* szName, T& Value) noexcept
	//{
	//	return std::make_pair(szSector, new CConfigItemXml<std::add_lvalue_reference_t<T>>(szSector, szName, Value));
	//}

	template<typename T>
	static constexpr std::pair<const wchar_t*, IConfigItemXml*> MakePair(
		const wchar_t* szSector,
		const wchar_t* szName, T&& Value) noexcept
	{
		return std::make_pair(szSector,
			new CConfigItemXml<T>(szSector, szName, std::forward<T>(Value)));
	}
};
