#pragma once
#include <Windows.h>
#include <vector>
#include <functional>
#include <tchar.h>
#include "Globals.h"
#include "ConfigGetters.h"

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

#define CATEGORY_VISUALS "Visuals"
#define CATEGORY_GAMEPLAY "Gameplay"
#define CATEGORY_KEYBINDS "Keybinds"
#define CATEGORY_MISC "Misc"
#define CATEGORY_MENU "Menu"

#define CONFIG_DEFAULT TEXT("default")
#define CONFIG_EXTENSION TEXT(".ini")
#define CONFIG_SEARCH_WILDCARD _CRT_CONCATENATE(TEXT("*"), CONFIG_EXTENSION)
#define CONFIG_DEFAULT_INI _CRT_CONCATENATE(CONFIG_DEFAULT, CONFIG_EXTENSION)

#ifdef UNICODE
#define _tcsrchr wcsrchr
#else
#define _tcsrchr strrchr
#endif // UNICODE


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

	IKeybind() : m_szName(NULL), m_keycode(-1), m_mode(KEYBIND_INVALID) {}

	IKeybind(const char* szName, int keycode, Mode mode)
		: m_szName(szName), m_keycode(keycode), m_mode(mode)
	{
	}

	virtual void OnPressed() = 0;

	const char* GetName() const { return m_szName; }
	int GetKeycode() const { return m_keycode; }
	void SetKeycode(int keycode) { m_keycode = keycode; }
	int GetMode() const { return m_mode; }

private:
	const char* m_szName;
	int m_keycode;
	int m_mode;
};

class KeybindToggleable : public IKeybind
{
public:
	KeybindToggleable() : IKeybind(), m_pbToggle(NULL) {}

	KeybindToggleable(const char* szName, int keycode, bool* pToggle)
		: IKeybind(szName, keycode, IKeybind::KEYBIND_ON_KEYPRESSED), m_pbToggle(pToggle)
	{
	}

	virtual void OnPressed() { if (m_pbToggle) *m_pbToggle = !(*m_pbToggle); }

private:
	bool* m_pbToggle;
};

class KeybindDynamicToggleable : public IKeybind
{
public:
	typedef bool*(*GetterFn)();

	KeybindDynamicToggleable() : IKeybind(), m_pGetter(NULL)
	{}

	KeybindDynamicToggleable(const char* szName, int keycode, GetterFn pGetter)
		: IKeybind(szName, keycode, IKeybind::KEYBIND_ON_KEYPRESSED), m_pGetter(pGetter)
	{
	}

	virtual void OnPressed()
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
class KeybindIncremental : public IKeybind
{
public:
	KeybindIncremental() : IKeybind(), m_pIncremental(NULL) {}

	KeybindIncremental(const char* szName, int keycode, Mode mode, T* pIncremental, T step)
		: IKeybind(szName, keycode, mode), m_pIncremental(pIncremental), m_step(step)
	{
	}

	virtual void OnPressed() { if (m_pIncremental) *m_pIncremental += m_step; }

private:
	T m_step;
	T* m_pIncremental;
};

template<typename T>
class KeybindDynamicIncremental : public IKeybind
{
public:
	typedef T*(*GetterFn)();

	KeybindDynamicIncremental() : IKeybind(), m_pGetter(NULL) {}

	KeybindDynamicIncremental(const char* szName, int keycode, Mode mode, GetterFn pGetter, T step)
		: IKeybind(szName, keycode, mode), m_pGetter(pGetter), m_step(step)
	{
	}

	virtual void OnPressed()
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
class KeybindDecremental : public IKeybind
{
public:
	KeybindDecremental() : IKeybind(), m_pDecremental(NULL) {}

	KeybindDecremental(const char* szName, int keycode, Mode mode, T* pDecremental, T step)
		: IKeybind(szName, keycode, mode), m_pDecremental(pDecremental), m_step(step)
	{
	}

	virtual void OnPressed() { if (m_pDecremental) *m_pDecremental -= m_step; }

private:
	T m_step;
	T* m_pDecremental;
};

template<typename T>
class KeybindDynamicDecremental : public IKeybind
{
public:
	typedef T*(*GetterFn)();

	KeybindDynamicDecremental() : IKeybind(), m_pGetter(NULL) {}

	KeybindDynamicDecremental(const char* szName, int keycode, Mode mode, GetterFn pGetter, T step)
		: IKeybind(szName, keycode, mode), m_pGetter(pGetter), m_step(step)
	{
	}

	virtual void OnPressed()
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

	KeybindIncrement() : m_inc(), m_dec() {}

	KeybindIncrement(const char* szNameInc, const char* szNameDec, int keycodeInc, int keycodeDec, Mode mode, T* pVariable, T step)
		: m_inc(szNameInc, keycodeInc, mode, pVariable, step), m_dec(szNameDec, keycodeDec, mode, pVariable, step)
	{}

	KeybindIncremental<T> m_inc;
	KeybindDecremental<T> m_dec;
};

template<typename T>
class KeybindDynamicIncrement
{
public:
	typedef IKeybind::Mode Mode;
	typedef T*(*GetterFn)();

	KeybindDynamicIncrement() : m_inc(), m_dec() {}

	KeybindDynamicIncrement(const char* szNameInc, const char* szNameDec, int keycodeInc, int keycodeDec, Mode mode, GetterFn pGetter, T step)
		: m_inc(szNameInc, keycodeInc, mode, pGetter, step), m_dec(szNameDec, keycodeDec, mode, pGetter, step)
	{}

	KeybindDynamicIncremental<T> m_inc;
	KeybindDynamicDecremental<T> m_dec;
};

template<typename Ret, typename... Args>
class KeybindFunctional : public IKeybind
{
public:
	KeybindFunctional() : IKeybind(), m_callback() {}

	KeybindFunctional(const char* szName, int keycode, Mode mode, Ret(*callback)(Args...), Args... callback_params)
		: IKeybind(szName, keycode, mode), m_callback(std::bind(callback, callback_params...))
	{
	}

	virtual void OnPressed()
	{
		if (m_callback)
			m_callback();
	}

private:
	std::function<Ret()> m_callback;
};

template<typename Ret, typename Base, typename... Args>
class KeybindVirtualFunctional : public IKeybind
{
public:
	KeybindVirtualFunctional() : IKeybind(), m_callback() {}

	KeybindVirtualFunctional(const char* szName, int keycode, Mode mode, size_t index, const Base* pThis, Args... callback_params)
		: KeybindVirtualFunctional(szName, keycode, mode, GetVirtual(pThis, index), pThis, callback_params...)
	{
	}
	
	KeybindVirtualFunctional(const char* szName, int keycode, Mode mode, Ret(*callback)(const Base*, Args...), const Base* pThis, Args... callback_params)
		: IKeybind(szName, keycode, mode), m_callback(std::bind(callback, pThis, callback_params...))
	{
	}

	virtual void OnPressed()
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
					if (CMemory::IsBadCodePtr(pVftable[count]))
						break;
		}

		return count;
	}

private:
	std::function<Ret()> m_callback;
};

class IConfigItem
{
public:

	virtual void Read(const char* szFilename) = 0;
	virtual void Write(const char* szFilename) = 0;	

	const char* m_szCategory;
	const char* m_szName;
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

class ConfigItemKeybind : public IConfigItem
{
public:
	ConfigItemKeybind(const char* szCategory, IKeybind& key)
		: m_key(key)
	{
		m_szCategory = szCategory;
		m_szName = key.GetName();
	}

	virtual void Read(const char* szFilename)
	{
		char szDefaultKeybindKeycode[33];
		char szKeybindKeycode[33];

		_itoa_s(m_key.GetKeycode(), szDefaultKeybindKeycode, 10);

		GetPrivateProfileString(m_szCategory, m_szName, szDefaultKeybindKeycode, szKeybindKeycode, sizeof(szKeybindKeycode), szFilename);

		m_key.SetKeycode(strtol(szKeybindKeycode, NULL, 0));
	}

	virtual void Write(const char* szFilename)
	{
		char szKeybindKeycode[33];

		_itoa_s(m_key.GetKeycode(), szKeybindKeycode, 10);

		WritePrivateProfileString(m_szCategory, m_szName, szKeybindKeycode, szFilename);
	}

private:
	IKeybind& m_key;
};

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
	static auto test(...)->std::false_type;

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

static KeyOrdinal s_Keycodes[] =
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
	{ "F11", DIK_F11 },
	{ "F12", DIK_F12 },
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

KeyOrdinal* FindKeyOrdinal(USHORT uKeycode);

typedef struct _WIN32_FIND_DATA_LISTA
{
	struct _WIN32_FIND_DATA_LISTA* m_pNext;
	struct _WIN32_FIND_DATA_LISTA* m_pPrevious;
	WIN32_FIND_DATAA m_Data;
} WIN32_FIND_DATA_LISTA, *PWIN32_FIND_DATA_LISTA;
typedef CONST PWIN32_FIND_DATA_LISTA PCWIN32_FIND_DATA_LISTA;

typedef struct _WIN32_FIND_DATA_LISTW
{
	struct _WIN32_FIND_DATA_LISTW* m_pNext;
	struct _WIN32_FIND_DATA_LISTW* m_pPrevious;
	WIN32_FIND_DATAW m_Data;
} WIN32_FIND_DATA_LISTW, *PWIN32_FIND_DATA_LISTW;
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

class CConfig
{
public:
	CConfig();
	~CConfig();

	bool CreateConfig(LPTSTR szFilename);
	void ResetConfig();
	void Load(LPCTSTR szFilename);
	void Save(LPCTSTR szFilename);

	BOOL EnumerateConfigs(OPTIONAL IN LPCTSTR szDirectory, OUT PWIN32_FIND_DATA_LIST* ppData) const;

	std::vector<IKeybind*>& GetKeybinds() {	return m_keybinds; }
	LPCTSTR GetConfigPath() const { return m_szFilename; }

private:
	void InitializeConfig();

	void PurgeConfig();
	void LoadDefault();

	void AddKeybind(IKeybind* pKeybind);

	bool SetFilename(LPCTSTR szFilename);
	BOOL FileExists(LPTSTR szFilename);
	BOOL SanitizePath(IN LPCTSTR szDelimiter, IN LPTSTR szOriginalPath, IN SIZE_T cchOriginalPath, OUT LPTSTR szSanitizedPath, IN SIZE_T cchSanitizedPath) const;

	TCHAR m_szFilename[MAX_PATH];
	std::vector<IConfigItem*> m_items;
	std::vector<IKeybind*> m_keybinds;
};
extern CConfig* g_pConfig;
#include "Variables.h"
#include "Features.h"