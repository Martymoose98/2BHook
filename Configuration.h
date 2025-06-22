#pragma once
#include <Windows.h>
#include <Shlwapi.h>
//#include <Wsdxml.h>
#include <xmllite.h>
#include <wrl.h>
#include <tchar.h>

#include <vector>
#include <map>
#include <stack>
#include <functional>
#include <string>

#include "Globals.h"

// c++ btw (just a friendly reminder to myself)
//class ConfigItem
//{
//public:
//	enum eType
//	{
//		CIT_BOOL,
//		CIT_INT,
//		CIT_UINT,
//		CIT_FLOAT
//	};
//	typedef unsigned int ConfigItemType;
//
//	ConfigItem(const char* szFile, const char* szCategory, const char* szName, ConfigItemType type)
//		: m_szFile(szFile), m_szCategory(szCategory), m_szName(szName), m_type(type)
//	{
//		switch (m_type)
//		{
//		case CIT_BOOL:
//			this->Write = WriteBool;
//			this->Read = ReadBool;
//			break;
//		case CIT_INT:
//			this->Write = WriteInt;
//			this->Read = ReadInt;
//			break;
//		case CIT_UINT:
//			this->Write = WriteUInt;
//			this->Read = ReadUInt;
//			break;
//		case CIT_FLOAT:
//			this->Write = WriteFloat;
//			this->Read = ReadFloat;
//		default:
//			break;
//		}
//	}
//
//	const char* m_szFile;
//	const char* m_szCategory;
//	const char* m_szName;
//	ConfigItemType m_type;
//
//	union
//	{
//		bool m_bValue;		
//		int m_iValue;
//		unsigned int m_uValue;
//		float m_flValue;
//	};
//
//	void(*Write)(const ConfigItem* pItem);
//	void(*Read)(ConfigItem* pItem);
//
//private:
//	static void ReadBool(ConfigItem* pItem);
//	static void ReadInt(ConfigItem* pItem);
//	static void ReadUInt(ConfigItem* pItem);
//	static void ReadFloat(ConfigItem* pItem);
//
//	static void WriteBool(const ConfigItem* pItem);
//	static void WriteInt(const ConfigItem* pItem);
//	static void WriteUInt(const ConfigItem* pItem);
//	static void WriteFloat(const ConfigItem* pItem);
//};

//#define CATEGORY_VISUALS "Visuals"
//#define CATEGORY_GAMEPLAY "Gameplay"
//#define CATEGORY_KEYBINDS "Keybinds"
//#define CATEGORY_MISC "Misc"
//#define CATEGORY_MENU "Menu"

#define CONFIG_XML

#if defined(CONFIG_XML)
#define CATEGORY_VISUALS _CRT_WIDE("Visuals")
#define CATEGORY_GAMEPLAY _CRT_WIDE("Gameplay")
#define CATEGORY_KEYBINDS _CRT_WIDE("Keybinds")
#define CATEGORY_MISC _CRT_WIDE("Misc")
#define CATEGORY_MENU _CRT_WIDE("Menu")
#define CONFIG_EXTENSION TEXT(".xml")
#else
#define CATEGORY_VISUALS "Visuals"
#define CATEGORY_GAMEPLAY "Gameplay"
#define CATEGORY_KEYBINDS "Keybinds"
#define CATEGORY_MISC "Misc"
#define CATEGORY_MENU "Menu"

#define CONFIG_EXTENSION TEXT(".ini")
#endif // CONFIG_XML


#define CONFIG_DEFAULT TEXT("default")

#define CONFIG_SEARCH_WILDCARD _CRT_CONCATENATE(TEXT("*"), CONFIG_EXTENSION)
#define CONFIG_DEFAULT_NAME _CRT_CONCATENATE(CONFIG_DEFAULT, CONFIG_EXTENSION)

using namespace Microsoft::WRL;

class IKeybind
{
public:
	typedef void(*OnPressedFn)(const IKeybind* pThis);

	enum Type
	{
		KEYBIND_TOGGLE,
		KEYBIND_INCREMENT,
		KEYBIND_FUNCTION,
		KEYBIND_VIRTUAL_FUNCTION
	};

	enum Mode
	{
		KEYBIND_INVALID = 0,
		KEYBIND_ON_KEYDOWN = 1,
		KEYBIND_ON_KEYPRESSED = 2
	};

	virtual void OnPressed(void) = 0;
};

// TODO: maybe remove and revert to IKeybind only
class CKeybind : public IKeybind
{
public:
	CKeybind(void) : m_szName(NULL), m_Keycode(-1), m_Mode(KEYBIND_INVALID) {}

	CKeybind(const wchar_t* szName, int32_t Keycode, Mode Mode)
		: m_szName(szName), m_Keycode(Keycode), m_Mode(Mode)
	{
	}

	// Move constructor
	CKeybind(CKeybind&& other) noexcept
		: m_szName(std::move(other.m_szName)),
		m_Keycode(other.m_Keycode),
		m_Mode(other.m_Mode)
	{
		// Optional: reset source
		other.m_Keycode = -1;
		other.m_Mode = KEYBIND_INVALID;
	}

	CKeybind(const CKeybind&) = default;

	virtual void OnPressed(void) {}//= 0;

	const wchar_t* GetName(void) const { return m_szName; }
	int GetKeycode(void) const { return m_Keycode; }
	void SetKeycode(int keycode) { m_Keycode = keycode; }
	Mode GetMode(void) const { return m_Mode; }

private:
	const wchar_t* m_szName;
	int32_t m_Keycode;
	Mode m_Mode;
};

class CKeybindToggleable : public CKeybind
{
public:
	CKeybindToggleable(void) : CKeybind(), m_pbToggle(NULL) {}

	CKeybindToggleable(const wchar_t* szName, int keycode, bool* pToggle)
		: CKeybind(szName, keycode, IKeybind::KEYBIND_ON_KEYPRESSED), m_pbToggle(pToggle)
	{
	}

	virtual void OnPressed(void) { if (m_pbToggle) *m_pbToggle = !(*m_pbToggle); }

private:
	bool* m_pbToggle;
};

template<typename GetterFn>
class CKeybindDynamicToggleable;

//template<typename R, class Base>
template<class Base>
class CKeybindDynamicToggleable<bool (Base::*)(void) const> : public CKeybind
{
public:
	typedef bool (Base::* GetterFn)(void) const;

	constexpr explicit CKeybindDynamicToggleable(void) : CKeybind(), m_pGetter(NULL)
	{
	}

	constexpr explicit CKeybindDynamicToggleable(const wchar_t* szName, int keycode, Base* pBase, GetterFn pGetter)
		: CKeybind(szName, keycode, IKeybind::KEYBIND_ON_KEYPRESSED), m_pBase(pBase), m_pGetter(pGetter)
	{
	}

	virtual void OnPressed(void)
	{
		if (m_pGetter)
		{
			bool pbToggle = (m_pBase->*m_pGetter)();

			//if (pbToggle)
			pbToggle = !(pbToggle);
		}
	}

private:
	Base* m_pBase;
	GetterFn m_pGetter;
};

template<>
class CKeybindDynamicToggleable<bool* (*)(void)> : public CKeybind
{
public:
	typedef bool* (*GetterFn)(void);

	explicit CKeybindDynamicToggleable(void) : CKeybind(), m_pGetter(NULL)
	{
	}

	explicit CKeybindDynamicToggleable(const wchar_t* szName, int keycode, GetterFn pGetter)
		: CKeybind(szName, keycode, IKeybind::KEYBIND_ON_KEYPRESSED), m_pGetter(pGetter)
	{
	}

	virtual void OnPressed(void)
	{
		if (m_pGetter)
		{
			bool* pbToggle = m_pGetter();

			if (pbToggle)
				*pbToggle = !(*pbToggle);
		}
	}

private:
	GetterFn m_pGetter;
};

template<typename T>
class KeybindIncremental : public CKeybind
{
public:
	KeybindIncremental(void) : CKeybind(), m_pIncremental(NULL) {}

	KeybindIncremental(const wchar_t* szName, int keycode, Mode mode, T* pIncremental, T step)
		: CKeybind(szName, keycode, mode), m_pIncremental(pIncremental), m_step(step)
	{
	}

	virtual void OnPressed(void) { if (m_pIncremental) *m_pIncremental += m_step; }

private:
	T m_step;
	T* m_pIncremental;
};

template<typename T>
class CKeybindDynamicIncremental : public CKeybind
{
public:
	typedef T* (*GetterFn)(void);

	CKeybindDynamicIncremental(void) : CKeybind(), m_step(), m_pGetter(NULL) {}

	constexpr CKeybindDynamicIncremental(const wchar_t* szName, int keycode, Mode mode, GetterFn pGetter, T step)
		: CKeybind(szName, keycode, mode), m_pGetter(pGetter), m_step(step)
	{
	}

	virtual void OnPressed(void)
	{
		if (m_pGetter)
		{
			T* pIncremental = m_pGetter();

			if (pIncremental)
				*pIncremental += m_step;
		}
	}

private:
	T m_step;
	GetterFn m_pGetter;
};


template<typename T>
class KeybindDecremental : public CKeybind
{
public:
	KeybindDecremental(void) : CKeybind(), m_pDecremental(NULL) {}

	KeybindDecremental(const wchar_t* szName, int keycode, Mode mode, T* pDecremental, T step)
		: CKeybind(szName, keycode, mode), m_pDecremental(pDecremental), m_step(step)
	{
	}

	virtual void OnPressed(void) { if (m_pDecremental) *m_pDecremental -= m_step; }

private:
	T m_step;
	T* m_pDecremental;
};

template<typename T>
class CKeybindDynamicDecremental : public CKeybind
{
public:
	typedef T* (*GetterFn)(void);

	CKeybindDynamicDecremental(void) : CKeybind(), m_pGetter(NULL) {}

	constexpr CKeybindDynamicDecremental(const wchar_t* szName, int keycode, Mode mode, GetterFn pGetter, T step)
		: CKeybind(szName, keycode, mode), m_pGetter(pGetter), m_step(step)
	{
	}

	virtual void OnPressed(void)
	{
		if (m_pGetter)
		{
			T* pDecremental = m_pGetter();

			if (pDecremental)
				*pDecremental -= m_step;
		}
	}

private:
	T m_step;
	GetterFn m_pGetter;
};

template<typename T>
class KeybindIncrement
{
public:
	typedef IKeybind::Mode Mode;

	KeybindIncrement(void) : m_inc(), m_dec() {}

	KeybindIncrement(const wchar_t* szNameInc, const wchar_t* szNameDec, 
		int keycodeInc, int keycodeDec, Mode mode, T* pVariable, T step)
		: m_inc(szNameInc, keycodeInc, mode, pVariable, step),
		m_dec(szNameDec, keycodeDec, mode, pVariable, step)
	{
	}

	KeybindIncremental<T> m_inc;
	KeybindDecremental<T> m_dec;
};

template<typename T>
class KeybindDynamicIncrement
{
public:
	typedef IKeybind::Mode Mode;
	typedef T* (*GetterFn)(void);

	KeybindDynamicIncrement(void) : m_inc(), m_dec() {}

	constexpr KeybindDynamicIncrement(const wchar_t* szNameInc, const wchar_t* szNameDec, int keycodeInc, int keycodeDec, Mode mode, GetterFn pGetter, T step)
		: m_inc(szNameInc, keycodeInc, mode, pGetter, step), m_dec(szNameDec, keycodeDec, mode, pGetter, step)
	{
	}


	CKeybindDynamicIncremental<T> m_inc;
	CKeybindDynamicDecremental<T> m_dec;
};

template<typename Ret, typename... Args>
class CKeybindFunctional : public CKeybind
{
public:
	CKeybindFunctional(void) : CKeybind(), m_callback() {}

	CKeybindFunctional(const wchar_t* szName, int keycode, Mode mode, Ret(*callback)(Args...), Args... callback_params)
		: CKeybind(szName, keycode, mode), m_callback(std::bind(callback, callback_params...))
	{
	}

	virtual void OnPressed(void)
	{
		if (m_callback)
			m_callback();
	}

private:
	std::function<Ret()> m_callback;
};

template<typename Ret, typename Base, typename... Args>
class CKeybindVirtualFunctional : public CKeybind
{
public:
	CKeybindVirtualFunctional(void) : CKeybind(), m_callback() {}

	CKeybindVirtualFunctional(const wchar_t* szName, int keycode, Mode mode, size_t index, 
		const Base* pThis, Args&&... callback_params)
		: CKeybindVirtualFunctional(szName, keycode, mode, GetVirtual(pThis, index), pThis, callback_params...)
	{
	}

	CKeybindVirtualFunctional(const wchar_t* szName, int keycode, Mode mode, 
		Ret(Base::*callback)(Args...) const, const Base* pThis, Args&&... callback_params)
		: CKeybind(szName, keycode, mode), m_callback(std::bind(callback, pThis, callback_params...))
	{
	}


	CKeybindVirtualFunctional(const wchar_t* szName, int keycode, Mode mode,
		Ret(*callback)(const Base*, Args...), const Base* pThis, Args&&... callback_params)
		: CKeybind(szName, keycode, mode), m_callback(std::bind(callback, pThis, callback_params...))
	{
	}

	virtual void OnPressed(void)
	{
		if (m_callback)
			m_callback();
	}

	static Ret(*GetVirtual(const Base* pThis, size_t index))(const Base*, Args...)
	{
		assert(index > GetVTableFunctionCount(pThis));

		return (*((Ret(***)(const Base*, Args...))(pThis)))[index];
	}

	static inline size_t GetVTableFunctionCount(const Base* pThis)
	{
		size_t count = 0;

		if (pThis)
		{
			const void** pVftable = *(const void***)pThis;

			if (pVftable)
				for (; pVftable[count]; ++count)
					if (IsBadCodePtr(pVftable[count]))
						break;
		}

		return count;
	}

private:
	std::function<Ret()> m_callback;
};


class IConfigItemXml
{
public:
	virtual HRESULT Read(IXmlReader* pReader) = 0;
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

// TODO: make make like a setting class for templates with a desc and
template<typename T> // std::enable_if_t<std::is_fundamental_v<T>>
class CConfigItemXml : public IConfigItemXml
{
public:
	using value_type = std::remove_reference_t<T>;
	//CConfigItemXml(const wchar_t* szSector, const wchar_t* szName, T& Value)
	//	: m_szSector(szSector), m_szName(szName), m_Value(Value)
	//{
	//}

	CConfigItemXml(const wchar_t* szSector, const wchar_t* szName, T&& Value)
		: m_szSector(szSector), m_szName(szName), m_Value(std::forward<T>(Value))
	{
	}

	// Global Read Stub
	virtual HRESULT Read(IXmlReader* pReader) override
	{
		HRESULT hr = S_OK;

		XmlNodeType NodeType;

		LPCWSTR szPrefix;
		uint32_t cchPrefix;

		LPCWSTR szValue;
		uint32_t cchValue;

		LPCWSTR szName;
		uint32_t cchLocalName;

		//uint32_t nAttributes;

		hr = pReader->GetPrefix(&szPrefix, &cchPrefix);

		if (SUCCEEDED(hr = pReader->GetLocalName(&szName, &cchLocalName)))
		{
			if (!wcscmp(m_szName, szName))
			{
				// Read Next Node (should be text)
				if (SUCCEEDED(hr = pReader->Read(&NodeType)) && NodeType == XmlNodeType_Text)
				{
					if (SUCCEEDED(hr = pReader->GetValue(&szValue, &cchValue)))
					{
						CConfigItemXml<T>::SetValue(szValue, cchValue);
					}
				}
			}
		}

		return hr;
	}


	// Global Write Stub
	virtual HRESULT Write(IXmlWriter* pWriter) override
	{
		return CConfigItemXml<T>::Write(pWriter);
	}

	T& GetValue(void)
	{
		return m_Value;
	}

private:
	void SetValue(LPCWSTR szValue, uint32_t cchValue);

	//template<typename Ret, typename... Args>
	//HRESULT CConfigItemXml<CKeybindFunctional<Ret, Args...>>::Write(IXmlWriter* pWriter)
	//{
	//	return 0;
	//}

protected:
	// XML Namespace
	const wchar_t* m_szSector;
	// XML Local Name
	const wchar_t* m_szName;
	// XML Value Description
	//const wchar_t* m_szDesc;
	// XML Value
	T m_Value;
};

template<>
HRESULT CConfigItemXml<ImColor&>::Write(IXmlWriter* pWriter)
{
	HRESULT hr = S_OK;
	wchar_t szBuffer[33] = { 0 };

	//swprintf_s(szBuffer, L"%i", m_Value);
	_itow_s(m_Value, szBuffer, 16);

	hr = pWriter->WriteElementString(NULL, m_szName, NULL, szBuffer);
	// BUG: IDK where I was told to write this but this fucks with indentation
	//hr = pWriter->WriteWhitespace(L"\r\n");

	return hr;
}

class CConfigItemXmlNamespace
{
public:
	CConfigItemXmlNamespace(ComPtr<IXmlReader> pReader, const wchar_t* szName, uint32_t uChildrenCount)
		: m_pReader(pReader), m_pWriter(nullptr), m_szName(szName), m_uChildrenCount(uChildrenCount)
	{
		Read();
	}

	CConfigItemXmlNamespace(ComPtr<IXmlWriter> pWriter, const wchar_t* szName, uint32_t uChildrenCount)
		: m_pReader(nullptr), m_pWriter(pWriter), m_szName(szName), m_uChildrenCount(uChildrenCount)
	{
		BeginWrite();
	}

	~CConfigItemXmlNamespace(void)
	{
		if (m_pWriter)
			EndWrite();
	}

	HRESULT Read(void)
	{
		HRESULT hr = S_OK;
		XmlNodeType NodeType;
		// TODO: Forgot still need to move even if we don't want to read anything
		// Read whitespace
		hr = m_pReader->Read(&NodeType);

		return hr;
	}

	HRESULT BeginWrite(void)
	{
		HRESULT hr = S_OK;

		wchar_t szBuffer[33] = { 0 };

		_ultow_s(m_uChildrenCount, szBuffer, 10);

		if (SUCCEEDED(hr = m_pWriter->WriteStartElement(NULL, m_szName, NULL)))
		{
			hr = m_pWriter->WriteAttributeString(NULL, L"num_children", NULL, szBuffer);
		}

		return hr;
	}

	HRESULT EndWrite(void)
	{
		HRESULT hr = S_OK;

		if (SUCCEEDED(hr = m_pWriter->WriteFullEndElement()))
		{
			// BUG: IDK where I was told to write this but this fucks with indentation
			//hr = m_pWriter->WriteWhitespace(L"\r\n");
		}

		return hr;
	}

	bool operator!=(const wchar_t* szName) const
	{
		return (wcscmp(m_szName, szName) != 0);
	}

	bool operator==(const wchar_t* szName) const
	{
		return (wcscmp(m_szName, szName) == 0);
	}

	ComPtr<IXmlReader> m_pReader;
	ComPtr<IXmlWriter> m_pWriter;
	const wchar_t* m_szName;
	volatile uint32_t m_uChildrenCount;
};



class IConfigItem
{
public:

	virtual void Read(const char* szFilename) = 0;
	virtual void Write(const char* szFilename) = 0;

protected:
	const char* m_szCategory = nullptr;
	const char* m_szName = nullptr;
};

class ConfigItemBool : public IConfigItem
{
public:
	ConfigItemBool(const char* szCategory, const char* szName, bool& value)
		: m_value(value)
	{
		m_szCategory = szCategory;
		m_szName = szName;
	}

	virtual void Read(const char* szFilename)
	{
		char szBuffer[33];

		GetPrivateProfileString(m_szCategory, m_szName, "false", szBuffer, sizeof(szBuffer), szFilename);
		m_value = !_stricmp(szBuffer, "true");
	}

	virtual void Write(const char* szFilename)
	{
		WritePrivateProfileString(m_szCategory, m_szName, (m_value) ? "true" : "false", szFilename);
	}

	bool& m_value;
};

class ConfigItemInt : public IConfigItem
{
public:
	ConfigItemInt(const char* szCategory, const char* szName, int& value)
		: m_value(value)
	{
		m_szCategory = szCategory;
		m_szName = szName;
	}

	virtual void Read(const char* szFilename)
	{
		char szBuffer[33];

		GetPrivateProfileString(m_szCategory, m_szName, "0", szBuffer, sizeof(szBuffer), szFilename);
		m_value = strtol(szBuffer, NULL, 0);
	}

	virtual void Write(const char* szFilename)
	{
		char szBuffer[33];

		sprintf_s(szBuffer, "%i", m_value);
		WritePrivateProfileString(m_szCategory, m_szName, szBuffer, szFilename);
	}

	int& m_value;
};

class ConfigItemUInt : public IConfigItem
{
public:
	ConfigItemUInt(const char* szCategory, const char* szName, unsigned int& value)
		: m_value(value)
	{
		m_szCategory = szCategory;
		m_szName = szName;
	}

	virtual void Read(const char* szFilename)
	{
		char szBuffer[33];

		GetPrivateProfileString(m_szCategory, m_szName, "0", szBuffer, sizeof(szBuffer), szFilename);
		m_value = strtoul(szBuffer, NULL, 0);
	}

	virtual void Write(const char* szFilename)
	{
		char szBuffer[33];

		sprintf_s(szBuffer, "%u", m_value);
		WritePrivateProfileString(m_szCategory, m_szName, szBuffer, szFilename);
	}

	unsigned int& m_value;
};

class ConfigItemColor : public IConfigItem
{
public:
	ConfigItemColor(const char* szCategory, const char* szName, ImColor& value)
		: m_value(value)
	{
		m_szCategory = szCategory;
		m_szName = szName;
	}

	virtual void Read(const char* szFilename)
	{
		char szBuffer[33];

		GetPrivateProfileString(m_szCategory, m_szName, "0", szBuffer, sizeof(szBuffer), szFilename);
		m_value = strtoul(szBuffer, NULL, 16);
	}

	virtual void Write(const char* szFilename)
	{
		char szBuffer[33];

		sprintf_s(szBuffer, "%08x", (unsigned int)m_value);
		WritePrivateProfileString(m_szCategory, m_szName, szBuffer, szFilename);
	}

	ImColor& m_value;
};

class ConfigItemFloat : public IConfigItem
{
public:
	ConfigItemFloat(const char* szCategory, const char* szName, float& value)
		: m_value(value)
	{
		m_szCategory = szCategory;
		m_szName = szName;
	}

	virtual void Read(const char* szFilename)
	{
		char szBuffer[33];

		GetPrivateProfileString(m_szCategory, m_szName, "0.0f", szBuffer, sizeof(szBuffer), szFilename);
		m_value = (float)atof(szBuffer);
	}

	virtual void Write(const char* szFilename)
	{
		char szBuffer[33];

		sprintf_s(szBuffer, "%f", m_value);
		WritePrivateProfileString(m_szCategory, m_szName, szBuffer, szFilename);
	}

	float& m_value;
};

class ConfigItemString : public IConfigItem
{
public:
	ConfigItemString(const char* szCategory, const char* szName, std::string& value)
		: m_value(value)
	{
		m_szCategory = szCategory;
		m_szName = szName;
	}

	virtual void Read(const char* szFilename)
	{
		LPTSTR szRawValue = (LPTSTR)LocalAlloc(LPTR, m_value.size());

		if (szRawValue)
		{
			GetPrivateProfileString(m_szCategory, m_szName, "(null)", szRawValue, (DWORD)m_value.size(), szFilename);
			m_value.assign(szRawValue);
			LocalFree(szRawValue);
		}
	}

	virtual void Write(const char* szFilename)
	{
		WritePrivateProfileString(m_szCategory, m_szName, m_value.c_str(), szFilename);
	}

	std::string& m_value;
};

//class ConfigItemKeybind : public IConfigItem
//{
//public:
//	ConfigItemKeybind(const char* szCategory, CKeybind& Key)
//		: m_Key(Key)
//	{
//		m_szCategory = szCategory;
//		m_szName = Key.GetName();
//	}
//
//	virtual void Read(const char* szFilename)
//	{
//		char szDefaultKeybindKeycode[33];
//		char szKeybindKeycode[33];
//
//		_itoa_s(m_Key.GetKeycode(), szDefaultKeybindKeycode, 10);
//
//		GetPrivateProfileString(m_szCategory, m_szName, szDefaultKeybindKeycode, szKeybindKeycode, sizeof(szKeybindKeycode), szFilename);
//
//		m_Key.SetKeycode(strtol(szKeybindKeycode, NULL, 0));
//	}
//
//	virtual void Write(const char* szFilename)
//	{
//		char szKeybindKeycode[33];
//
//		_itoa_s(m_Key.GetKeycode(), szKeybindKeycode, 10);
//
//		WritePrivateProfileString(m_szCategory, m_szName, szKeybindKeycode, szFilename);
//	}
//
//private:
//	CKeybind& m_Key;
//};

//template<typename T>
//class ConfigItemVector : public IConfigItem
//{
//public:
//	enum Type
//	{
//		VOID_PTR,
//		BOOL,
//		INT,
//		UINT,
//		INT64,
//		UINT64,
//		CSTRING,
//		WCSTRING,
//		STRING,
//		WSTRING,
//	};
//
//	ConfigItemVector(const char* szCategory, const char* szName, Type type, std::vector<T>& vec)
//		: m_vec(vec), m_type(type)
//	{
//		m_szCategory = szCategory;
//		m_szName = szName;
//	}
//
//	virtual void Read(const char* szFilename)
//	{
//		switch (m_type)
//		{
//		case STRING:
//			WriteString(szFilename);
//			break;
//		}
//	}
//
//	virtual void Write(const char* szFilename)
//	{
//		switch (m_type)
//		{
//		case STRING:
//			WriteString(szFilename);
//			break;
//		}
//	}
//
//	void WriteString(const char* szFilename)
//	{
//		char szItem[32];
//		size_t i = 0;
//
//		for (auto& it : m_vec)
//		{
//			sprintf_s(szItem, "%s[%i]", m_szName, i++);
//
//			WritePrivateProfileString(m_szCategory, szItem, it, szFilename);
//		}
//	}
//
//	void WriteInt(const char* szFilename)
//	{
//		char szItem[32];
//		char szBuffer[32];
//		size_t i = 0;
//
//		for (auto& it : m_vec)
//		{
//			sprintf_s(szItem, "%s[%i]", m_szName, i++);
//			sprintf_s(szBuffer, "%i", i);
//
//			WritePrivateProfileString(m_szCategory, szItem, szBuffer, szFilename);
//		}
//
//		void WriteUInt(const char* szFilename)
//		{
//			char szItem[32];
//			char szBuffer[32];
//			size_t i = 0;
//
//			for (auto& it : m_vec)
//			{
//				sprintf_s(szItem, "%s[%u]", m_szName, i++);
//				sprintf_s(szBuffer, "%u", i);
//
//				WritePrivateProfileString(m_szCategory, szItem, szBuffer, szFilename);
//			}
//		}
//	}
//
//private:
//	std::vector<T>& m_vec;
//	Type m_type;
//};
#if 0
#include <sstream>

template<class X, class Y, class Op>
struct op_valid_impl
{
	template<class U, class L, class R>
	static auto test(int) -> decltype(std::declval<U>()(std::declval<L>(), std::declval<R>()),
		void(), std::true_type());

	template<class U, class L, class R>
	static auto test(...) -> std::false_type;

	using type = decltype(test<Op, X, Y>(0));

};
template<class X, class Y, class Op> using op_valid = typename op_valid_impl<X, Y, Op>::type;

struct left_shift {
	template <class L, class R>
	constexpr auto operator()(L&& l, R&& r) const
		noexcept(noexcept(std::forward<L>(l) << std::forward<R>(r)))
		-> decltype(std::forward<L>(l) << std::forward<R>(r))
	{
		return std::forward<L>(l) << std::forward<R>(r);
	}
};

struct right_shift {
	template <class L, class R>
	constexpr auto operator()(L&& l, R&& r) const
		noexcept(noexcept(std::forward<L>(l) >> std::forward<R>(r)))
		-> decltype(std::forward<L>(l) >> std::forward<R>(r))
	{
		return std::forward<L>(l) >> std::forward<R>(r);
	}
};

template<class X, class Y> using has_left_shift = op_valid<X, Y, left_shift>;
template<class X, class Y> using has_right_shift = op_valid<X, Y, right_shift>;

template<typename T>
class ConfigItemVector : public IConfigItem
{
public:
	ConfigItemVector(const char* szCategory, const char* szName, std::vector<T>& vec)
		: m_vec(vec)
	{
		assert((has_right_shift<std::stringstream, T>()()));
		assert((has_left_shift<std::stringstream, T>()()));

		m_szCategory = szCategory;
		m_szName = szName;
	}

	virtual void Read(const char* szFilename)
	{
		char szBuffer[32];
		std::stringstream ss_name, ss_val;
		T type;

		for (size_t i = 0; ; ++i)
		{
			ss_name << m_szName << "[" << i << "]";
			GetPrivateProfileString(m_szCategory, ss_name.str().c_str(), "(null)", szBuffer, sizeof(szBuffer), szFilename);

			if (!strcmp(szBuffer, "(null)"))
				break;

			ss_val << szBuffer;
			ss_val >> type;
			m_vec.emplace_back(type);
			ss_name.str(std::string());
			ss_val.str(std::string());
			ss_val.clear();
		}
	}

	virtual void Write(const char* szFilename)
	{
		std::stringstream ss_name, ss_val;
		size_t i = 0;

		for (auto& it : m_vec)
		{
			ss_name << m_szName << "[" << i++ << "]";
			ss_val << it;
			WritePrivateProfileString(m_szCategory, ss_name.str().c_str(), ss_val.str().c_str(), szFilename);
			ss_name.str(std::string());
			ss_val.str(std::string());
		}
	}

private:
	std::vector<T>& m_vec;
};
#endif


//TODO("implement shifted keys as a second code or bool")
struct KeyOrdinal { const char* m_szName; USHORT m_uKeyCode; };

static const KeyOrdinal s_Keycodes[] =
{
	{ "ESCAPE", DIK_ESCAPE },
	{ "1", DIK_1 },
	{ "2", DIK_2 },
	{ "3", DIK_3 },
	{ "4", DIK_4 },
	{ "5", DIK_5 },
	{ "6", DIK_6 },
	{ "7", DIK_7 },
	{ "8", DIK_8 },
	{ "9", DIK_9 },
	{ "0", DIK_0 },
	{ "-", DIK_MINUS },
	{ "=", DIK_EQUALS },
	{ "BACKSPACE", DIK_BACK },
	{ "TAB", DIK_TAB },
	{ "Q", DIK_Q },
	{ "W", DIK_W },
	{ "E", DIK_E },
	{ "R", DIK_R },
	{ "T", DIK_T },
	{ "Y", DIK_Y },
	{ "U", DIK_U },
	{ "I", DIK_I },
	{ "O", DIK_O },
	{ "P", DIK_P },
	{ "[", DIK_LBRACKET },
	{ "]", DIK_RBRACKET },
	{ "ENTER", DIK_RETURN },
	{ "LCTRL", DIK_LCONTROL },
	{ "A", DIK_A },
	{ "S", DIK_S },
	{ "D", DIK_D },
	{ "F", DIK_F },
	{ "G", DIK_G },
	{ "H", DIK_H },
	{ "J", DIK_J },
	{ "K", DIK_K },
	{ "L", DIK_L },
	{ ";", DIK_SEMICOLON },
	{ "`", DIK_GRAVE },
	{ "LSHIFT", DIK_LSHIFT },
	{ "\\", DIK_BACKSLASH },
	{ "Z", DIK_Z },
	{ "X", DIK_X },
	{ "C", DIK_C },
	{ "V", DIK_V },
	{ "B", DIK_B },
	{ "N", DIK_N },
	{ "M", DIK_M },
	{ ",", DIK_COMMA },
	{ ".", DIK_PERIOD },
	{ "/", DIK_SLASH },
	{ "RSHIFT", DIK_RSHIFT },
	{ "MULTIPLY", DIK_MULTIPLY },
	{ "LALT", DIK_LMENU },
	{ "SPACE", DIK_SPACE },
	{ "CAPSLOCK", DIK_CAPITAL },
	{ "F1", DIK_F1 },
	{ "F2", DIK_F2 },
	{ "F3", DIK_F3 },
	{ "F4", DIK_F4 },
	{ "F5", DIK_F5 },
	{ "F6", DIK_F6 },
	{ "F7", DIK_F7 },
	{ "F8", DIK_F8 },
	{ "F9", DIK_F9 },
	{ "F10", DIK_F10 },
	{ "NUMLOCK", DIK_NUMLOCK },
	{ "SCROLLLOCK", DIK_SCROLL },
	{ "NUM7", DIK_NUMPAD7 },
	{ "NUM8", DIK_NUMPAD8 },
	{ "NUM9", DIK_NUMPAD9 },
	{ "SUBTRACT", DIK_SUBTRACT },
	{ "NUM4", DIK_NUMPAD4 },
	{ "NUM5", DIK_NUMPAD5 },
	{ "NUM6", DIK_NUMPAD6 },
	{ "ADD", DIK_ADD },
	{ "NUM1", DIK_NUMPAD1 },
	{ "NUM2", DIK_NUMPAD2 },
	{ "NUM3", DIK_NUMPAD3 },
	{ "NUM0", DIK_NUMPAD0 },
	{ "DECMIAL", DIK_DECIMAL },
	{ "|", DIK_OEM_102 },
	{ "F11", DIK_F11 },
	{ "F12", DIK_F12 },
	{ "F13", DIK_F13 },
	{ "F14", DIK_F14 },
	{ "F15", DIK_F15 },
	{ "PREV TRACK", DIK_PREVTRACK },
	{ "NEXT TRACK", DIK_NEXTTRACK },
	{ "NUMENTER", DIK_NUMPADENTER },
	{ "RCTRL", DIK_RCONTROL },
	{ "MUTE", DIK_MUTE },
	{ "CALCULATOR", DIK_CALCULATOR },
	{ "PLAYPAUSE", DIK_PLAYPAUSE },
	{ "STOP", DIK_MEDIASTOP },
	{ "VOL-", DIK_VOLUMEDOWN },
	{ "VOL+", DIK_VOLUMEUP },
	{ "WEBHOME", DIK_WEBHOME },
	{ "PRINT SCREEN", DIK_SYSRQ },
	{ "RALT", DIK_RMENU },
	{ "PAUSE BREAK", DIK_PAUSE },
	{ "HOME", DIK_HOME },
	{ "UP ARROW", DIK_UP },
	{ "PAGE UP", DIK_PRIOR },
	{ "LEFT ARROW", DIK_LEFT },
	{ "RIGHT ARROW", DIK_RIGHT },
	{ "END", DIK_END },
	{ "DOWN ARROW", DIK_DOWN },
	{ "PAGE DOWN", DIK_NEXT },
	{ "INSERT", DIK_INSERT },
	{ "DELETE", DIK_DELETE }
};

const KeyOrdinal* FindKeyOrdinal(USHORT uKeycode);

typedef struct _WIN32_FIND_DATA_LISTA
{
	struct _WIN32_FIND_DATA_LISTA* m_pNext;
	struct _WIN32_FIND_DATA_LISTA* m_pPrevious;
	WIN32_FIND_DATAA m_Data;
} WIN32_FIND_DATA_LISTA, * PWIN32_FIND_DATA_LISTA;
typedef CONST PWIN32_FIND_DATA_LISTA PCWIN32_FIND_DATA_LISTA;

typedef struct _WIN32_FIND_DATA_LISTW
{
	struct _WIN32_FIND_DATA_LISTW* m_pNext;
	struct _WIN32_FIND_DATA_LISTW* m_pPrevious;
	WIN32_FIND_DATAW m_Data;
} WIN32_FIND_DATA_LISTW, * PWIN32_FIND_DATA_LISTW;
typedef CONST PWIN32_FIND_DATA_LISTW PCWIN32_FIND_DATA_LISTW;

#ifdef _UNICODE
typedef WIN32_FIND_DATA_LISTW WIN32_FIND_DATA_LIST;
typedef PWIN32_FIND_DATA_LISTW PWIN32_FIND_DATA_LIST;
#else
typedef WIN32_FIND_DATA_LISTA WIN32_FIND_DATA_LIST;
typedef PWIN32_FIND_DATA_LISTA PWIN32_FIND_DATA_LIST;
#endif
typedef CONST PWIN32_FIND_DATA_LIST PCWIN32_FIND_DATA_LIST;

VOID FindDataListFree(PCWIN32_FIND_DATA_LIST pList);
SIZE_T FindDataListCount(PCWIN32_FIND_DATA_LIST pList);
VOID FindDataListSort(PWIN32_FIND_DATA_LIST pList);
PWIN32_FIND_DATA_LIST FindDataListNav(PCWIN32_FIND_DATA_LIST pList, INT iIndex);

class IConfig
{
public:

	virtual bool CreateConfig(LPCWSTR szFilename) = 0;
	virtual void ResetConfig(void) = 0;
	virtual HRESULT Load(LPCWSTR szFilename) = 0;
	virtual HRESULT Save(LPCWSTR szFilename) = 0;

	static BOOL FileExists(LPCTSTR szFilename);

	static BOOL SanitizePath(IN LPCTSTR szDelimiter, IN LPTSTR szOriginalPath, IN SIZE_T cchOriginalPath,
		OUT LPTSTR szSanitizedPath, IN SIZE_T cchSanitizedPath);

	static BOOL SanitizePath(IN LPCTSTR szDelimiter, IN LPTSTR szOriginalPath, IN SIZE_T cchOriginalPath,
		OUT LPTSTR* pszSanitizedPath, OUT SIZE_T* pcchSanitizedPath);

	static BOOL SanitizePath(IN LPCWSTR szDelimiter, IN LPWSTR szOriginalPath, IN SIZE_T cchOriginalPath,
		OUT LPWSTR szSanitizedPath, IN SIZE_T cchSanitizedPath);

	static BOOL SanitizePath(IN LPCWSTR szDelimiter, IN LPWSTR szOriginalPath, IN SIZE_T cchOriginalPath,
		OUT LPWSTR* pszSanitizedPath, OUT SIZE_T* pcchSanitizedPath);

protected:
	bool SetFilename(LPCTSTR szFilename);

};

class CConfigXml : public IConfig
{
public:
	CConfigXml(void);
	~CConfigXml(void);

	virtual bool CreateConfig(LPCWSTR szFilename) override;
	virtual void ResetConfig(void) override;
	virtual HRESULT Load(LPCWSTR szFilename) override;
	virtual HRESULT Save(LPCWSTR szFilename) override;

	HRESULT ReadNode(XmlNodeType Type);
	HRESULT WriteNode(XmlNodeType Type);

	HRESULT BeginReadElement(void);
	HRESULT EndReadElement(void);

	std::pair<
		std::multimap<std::wstring, IConfigItemXml*>::const_iterator,
		std::multimap<std::wstring, IConfigItemXml*>::const_iterator
	> GetKeybinds(void) const;

	template<class T>
	void AddKeybind(T&& Keybind);

	friend class CMenu;
protected:
	bool SetFilename(LPCWSTR szFilename);
	BOOL EnumerateConfigs(OPTIONAL IN LPCTSTR szDirectory, OUT PWIN32_FIND_DATA_LIST* ppData) const;
private:

	void InitializeConfig(void);
	void PurgeConfig(void);

	static BOOL CConfigXml::FileExists(LPCWSTR szFilename);

	WCHAR m_szFilename[MAX_PATH];
	ComPtr<IXmlReader> m_pReader;
	ComPtr<IXmlWriter> m_pWriter;
	XmlNodeType m_CurrentNodeType;

	std::multimap<const std::wstring, IConfigItemXml*> m_Items;
	//std::multimap<const wchar_t*, IConfigItemXml*> m_Items;
	//std::vector<CKeybind> m_Keybinds;
	// std::vector<CKeybind*> m_Keybinds; // works
};

class CConfig : public IConfig
{
public:
	CConfig(void);
	~CConfig(void);

	virtual bool CreateConfig(LPCWSTR szFilename) override;
	virtual void ResetConfig(void) override;
	virtual HRESULT Load(LPCWSTR szFilename) override;
	virtual HRESULT Save(LPCWSTR szFilename) override;

	// HACK! for compat
	void Load(LPCTSTR szFilename);
	void Save(LPCTSTR szFilename);

	BOOL EnumerateConfigs(OPTIONAL IN LPCTSTR szDirectory, OUT PWIN32_FIND_DATA_LIST* ppData) const;

	std::vector<CKeybind*>& GetKeybinds(void) { return m_Keybinds; }
	LPCTSTR GetConfigPath(void) const { return m_szFilename; }

	friend class CMenu;

private:
	void InitializeConfig(void);

	void PurgeConfig(void);
	void LoadDefault(void);

	void AddKeybind(CKeybind* pKeybind);

	template<size_t N>
	void AddKeybinds(CKeybind* (&Keybind)[N])
	{
		for (auto it : Keybind)
		{
			AddKeybind(it);
		}
	}

	bool SetFilename(LPCTSTR szFilename);

	TCHAR m_szFilename[MAX_PATH];
	std::vector<IConfigItem*> m_Items;
	std::vector<CKeybind*> m_Keybinds;
};


//extern CConfig* g_pConfig;
#include "Menu.h"
#include "Variables.h"
#include "Features.h"