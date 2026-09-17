#pragma once

#include <Windows.h>
#include <deque>
#include <mutex>
#include <string>
#include <unordered_map>
#include <stdio.h>
#include "ImGui/imgui.h"

#include "Utils.h"

class CConsole
{
	typedef enum KConsoleFlags
	{
		CONFLAGS_ENABLE_AUTOSCROLL = (1 << 0),
		CONFLAGS_SHOULD_AUTOSCROLL = (1 << 1),
		CONFLAGS_ENABLE_TIMESTAMPS = (1 << 2),
	} ConsoleFlags;

	struct ConCmd
	{
		const char* m_szName;
	};

	struct Entry
	{
		std::string m_Text;
		ImVec4      m_Color;
		// Captured when the entry is logged, not when it is drawn, so toggling timestamps
		// on shows the real times of lines that are already in the buffer.
		SYSTEMTIME  m_Time;
	};

public:

	// Oldest entries are dropped past this. Bounds both memory and the per-frame cost of
	// walking the list, which is what made the console crawl once it filled up.
	static constexpr size_t s_uMaxEntries = 4096;

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

	// Floor for the command box so it stays usable in a narrow window.
	static constexpr float s_flMinInputWidth = 64.0f;

	void FilterBar(void);
	void InputBar(float flWidth);
	void EnumConsoleData(void);

	void WritePrefixed(const ImVec4& color, const char* szPrefix, const char* szFormat, va_list args);
	void Append(const ImVec4& color, std::string&& Text);

private:
	char						m_szInput[256];
	ImGuiTextFilter				m_TextFilter;
	uint32_t					m_uFlags;

	// Log entries are produced from whatever thread happens to be logging - the game's CRI
	// callback threads and the Setup thread both do - while Draw() walks the list on the
	// render thread. Everything touching m_Items or m_uFlags takes this.
	mutable std::mutex			m_Mutex;

	// deque, not vector: dropping the oldest entry is O(1) and never reallocates the rest.
	std::deque<Entry>			m_Items;
	std::unordered_map<std::string, ConCmd> m_Commands;
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
