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

#define CONFIG_DEFAULT TEXT("default")
#define CONFIG_DEFAULT_INI TEXT("default.ini")

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

struct KeybindOrdinal
{
	const char* m_szName;
	IKeybind::Type m_type;
};

static KeybindOrdinal s_keyOrdinals[] =
{
	{ "kb_airstuck", IKeybind::Type::KEYBIND_TOGGLE },
	{ "kb_change_player", IKeybind::Type::KEYBIND_FUNCTION },
	{ "kb_duplicate_buddy", IKeybind::Type::KEYBIND_FUNCTION },
	{ "kb_teleport_forward", IKeybind::Type::KEYBIND_FUNCTION },
	{ "kb_play_animation", IKeybind::Type::KEYBIND_VIRTUAL_FUNCTION },
	{ "kb_model_gravity", IKeybind::Type::KEYBIND_TOGGLE },
	{ "kb_model_ycontrol", IKeybind::Type::KEYBIND_INCREMENT }
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

template<typename Ret, class Base, typename... Args>
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
		char szBuffer[32];

		GetPrivateProfileString(m_szCategory, m_szName, "false", szBuffer, sizeof(szBuffer), szFilename);
		m_value = !_stricmp(szBuffer, "true");
	}

	virtual void Write(const char* szFilename)
	{
		char szBuffer[32];

		strcpy_s(szBuffer, (m_value) ? "true" : "false");
		WritePrivateProfileString(m_szCategory, m_szName, szBuffer, szFilename);
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
		char szBuffer[32];

		GetPrivateProfileString(m_szCategory, m_szName, "0", szBuffer, sizeof(szBuffer), szFilename);
		m_value = strtol(szBuffer, NULL, 0);
	}

	virtual void Write(const char* szFilename)
	{
		char szBuffer[32];

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
		char szBuffer[32];

		GetPrivateProfileString(m_szCategory, m_szName, "0", szBuffer, sizeof(szBuffer), szFilename);
		m_value = strtoul(szBuffer, NULL, 0);
	}

	virtual void Write(const char* szFilename)
	{
		char szBuffer[32];

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
		char szBuffer[32];

		GetPrivateProfileString(m_szCategory, m_szName, "0.0f", szBuffer, sizeof(szBuffer), szFilename);
		m_value = (float)atof(szBuffer);
	}

	virtual void Write(const char* szFilename)
	{
		char szBuffer[32];

		sprintf_s(szBuffer, "%f", m_value);
		WritePrivateProfileString(m_szCategory, m_szName, szBuffer, szFilename);
	}

	float& m_value;
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
		char szKeybindKeycode[32];

		GetPrivateProfileString(m_szCategory, m_szName, "0", szKeybindKeycode, sizeof(szKeybindKeycode), szFilename);

		m_key.SetKeycode(strtol(szKeybindKeycode, NULL, 0));
	}

	virtual void Write(const char* szFilename)
	{
		char szKeybindKeycode[32];

		sprintf_s(szKeybindKeycode, "%i", m_key.GetKeycode());

		WritePrivateProfileString(m_szCategory, m_szName, szKeybindKeycode, szFilename);
	}

private:
	IKeybind& m_key;
};

class CConfig
{
public:
	CConfig();
	~CConfig();

	bool CreateConfig(LPTSTR szFilename);
	void ResetConfig();
	void Load(LPTSTR szFilename);
	void Save(LPTSTR szFilename);

	std::vector<IKeybind*>& GetKeybinds() {	return m_keybinds; }
	LPCTSTR GetConfigPath() const { return m_szFilename; }

private:
	void InitializeConfig();

	void PurgeConfig();
	void LoadDefault();
	bool SetFilename(LPTSTR szFilename);
	BOOL FileExists(LPTSTR szFilename);
	BOOL SanitizePath(IN LPCTSTR szDelimiter, IN LPTSTR szOriginalPath, IN SIZE_T cchOriginalPath, OUT LPTSTR szSanitizedPath, IN SIZE_T cchSanitizedPath);

	TCHAR m_szFilename[MAX_PATH];
	std::vector<IConfigItem*> m_items;
	std::vector<IKeybind*> m_keybinds;
};
extern CConfig* g_pConfig;
#include "Variables.h"
#include "Features.h"