#pragma once

#include <Windows.h>
#include <vector>
#include <stdio.h>
#include "ImGui/imgui.h"

#include "Utils.h"

template<typename T>
class CircularBuffer
{

};

class CConsole
{
	typedef enum KConsoleFlags
	{
		CONFLAGS_ENABLE_AUTOSCROLL = (1 << 0),
		CONFLAGS_SHOULD_AUTOSCROLL = (1 << 1),
		CONFLAGS_FILTER_ENTRIES = (1 << 2),
		CONFLAGS_ENABLE_TIMESTAMPS = (1 << 3),
	} ConsoleFlags;


	struct ConCmd
	{
		const char* m_szName;

	};

	struct Entry
	{
		typedef enum Type
		{

		};

		char* szText;
		ImVec4 color;
	};

public:

	CConsole(void);

	~CConsole(void);

	void Clear(void);

	void Log(const ImVec4& color, const char* szFormat, ...);

	void Warn(const char* szFormat, ...);

	void Error(const char* szFormat, ...);

	void Write(const ImVec4& color, const char* szFormat, ...);

	void Write(const ImVec4& color, const char* szFormat, va_list args);

	void Draw(const char* szTitle, const ImVec2 WindowSize = ImVec2(820, 400));
private:

	void FilterBar(void);
	void InputBar(void);
	void EnumConsoleData(void);

private:
	char						m_szInput[256];
	ImGuiTextFilter				m_TextFilter;
	uint32_t					m_uFlags;	// FIXME: maybe volatile?
	bool						m_bFilter;
	bool						m_bScrollToBottom;
	bool						m_bShouldScrollToBottom;
	std::vector<Entry>			m_Items;
	//std::deque<Entry>			m_Items;
	std::unordered_map<const char*, ConCmd> m_Commands;
};
extern CConsole* g_pConsole;

#ifdef _DEBUG
#define CCONSOLE_DEBUG_LOG(color, szFormat, ...) g_pConsole->Log(color, szFormat, __VA_ARGS__)
#define CCONSOLE_DEBUG_WARN(color, szFormat, ...) g_pConsole->Warn(color, szFormat, __VA_ARGS__)
#define CCONSOLE_DEBUG_ERROR(color, szFormat, ...) g_pConsole->Error(color, szFormat, __VA_ARGS__)
#else
#define CCONSOLE_DEBUG_LOG(color, szFormat, ...)
#define CCONSOLE_DEBUG_WARN(color, szFormat, ...)
#define CCONSOLE_DEBUG_ERROR(color, szFormat, ...)
#endif

#include "Configuration.h"