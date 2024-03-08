#pragma once

#include <Windows.h>
#include <vector>
#include <stdio.h>
#include "ImGui/imgui.h"

#include "Utils.h"

class CConsole
{
	struct Entry
	{
		char* szText;
		ImVec4 color;
	};

	char					 m_szInput[256];
	char					 m_szFilter[256];
	std::vector<Entry>		 m_Items;
	bool					 m_bFilter;
	bool					 m_bScrollToBottom;
	bool					 m_bShouldScrollToBottom;
	std::vector<Entry>		 m_History;
	int						 m_iHistoryPos;    // -1: new line, 0..History.Size-1 browsing history.
	std::vector<const char*> m_Commands;

public:

	CConsole(void);

	~CConsole(void);

	void Clear(void);

	void Log(const ImVec4& color, const char* szFormat, ...);

	void Warn(const char* szFormat, ...);

	void Error(const char* szFormat, ...);

	void Write(const ImVec4& color, const char* szFormat, ...);

	void Write(const ImVec4& color, const char* szFormat, va_list args);

	void Draw(const char* szTitle);
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