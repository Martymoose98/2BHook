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

	// Widget labels, shared by the layout measurement and the widgets themselves. Kept in
	// one place so renaming one cannot silently shrink the measurement and push the row
	// off the right edge again.
	static constexpr const char* s_szAutoScroll = "Auto Scroll";
	static constexpr const char* s_szTimestamps = "Timestamps";
	static constexpr const char* s_szClear = "Clear";
	static constexpr const char* s_szCommand = "Command";

	// Bits the user drives from the UI, as opposed to CONFLAGS_SHOULD_AUTOSCROLL which
	// writers set. Only these are copied back when a checkbox is toggled.
	static constexpr uint32_t s_uUserFlags = CONFLAGS_ENABLE_AUTOSCROLL | CONFLAGS_ENABLE_TIMESTAMPS;

	void FilterBar(void);
	void InputBar(float flWidth);
	void EnumConsoleData(void);

	uint32_t GetFlags(void) const;
	void SetUserFlags(uint32_t uFlags);
	float GetTrailingWidth(void);

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

	// Cached width of the controls trailing the command box. Only depends on the style and
	// font, so it is recomputed when the font size changes rather than every frame - Draw
	// runs from the Present hook.
	float						m_flTrailingWidth;
	float						m_flTrailingFontSize;
};
extern CConsole* g_pConsole;

#ifdef _DEBUG
#define CCONSOLE_DEBUG_LOG(color, szFormat, ...) g_pConsole->Log(color, szFormat, __VA_ARGS__)
// Warn and Error carry their own colours, so unlike Log they take no colour argument.
// These macros used to pass one anyway, which put an ImVec4 where the format string goes.
#define CCONSOLE_DEBUG_WARN(szFormat, ...) g_pConsole->Warn(szFormat, __VA_ARGS__)
#define CCONSOLE_DEBUG_ERROR(szFormat, ...) g_pConsole->Error(szFormat, __VA_ARGS__)
#else
#define CCONSOLE_DEBUG_LOG(color, szFormat, ...)
#define CCONSOLE_DEBUG_WARN(szFormat, ...)
#define CCONSOLE_DEBUG_ERROR(szFormat, ...)
#endif

#include "Configuration.h"
