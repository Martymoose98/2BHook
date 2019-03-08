#pragma once
#include <vector>
#include <stdio.h>
#include "ImGui/imgui.h"
#include "Configuration.h"

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

	CConsole()
		: m_Items(), m_bShouldScrollToBottom(true), m_History(), m_iHistoryPos(0), m_Commands()
	{
		ZeroMemory(m_szInput, sizeof(m_szInput));
		ZeroMemory(m_szFilter, sizeof(m_szFilter));
	}

	~CConsole()
	{
		Clear();
	}

	void Clear(void) 
	{
		for (auto& it : m_Items)
			free(it.szText);

		m_Items.clear();
	}

	void Log(const ImVec4& color, const char* szFormat, ...)
	{
		va_list args;

		va_start(args, szFormat);
		Write(color, szFormat, args);
		va_end(args);
	}

	void Warn(const char* szFormat, ...)
	{
		va_list args;

		size_t cbSize = strlen(szFormat) + 9;

		char* szFmt = (char*)malloc(cbSize);
		strcpy_s(szFmt, cbSize, "[Warn]: ");
		strcat_s(szFmt, cbSize, szFormat);

		va_start(args, szFormat);
		Write(ImVec4(0.95f, 0.6f, 0.14f, 1.0f), szFmt, args);
		va_end(args);
		free(szFmt);
	}
	
	void Error(const char* szFormat, ...)
	{
		va_list args;

		size_t cbSize = strlen(szFormat) + 10;

		char* szFmt = (char*)malloc(cbSize);
		strcpy_s(szFmt, cbSize, "[Error]: ");
		strcat_s(szFmt, cbSize, szFormat);

		va_start(args, szFormat);
		Write(ImVec4(0.95f, 0.2f, 0.14f, 1.0f), szFmt, args);
		va_end(args);
		free(szFmt);
	}

	void Write(const ImVec4& color, const char* szFormat, va_list args)
	{
		Entry next;

		int cchBuffer = _vscprintf(szFormat, args);

		if (cchBuffer != -1)
		{
			char* szBuffer = (char*)malloc(cchBuffer + 1);

			vsnprintf(szBuffer, cchBuffer + 1, szFormat, args);

			next.szText = szBuffer;
			next.color = color;

			m_Items.emplace_back(next);
		}
		m_bShouldScrollToBottom = true;
	}

	void Write(const ImVec4& color, const char* szFormat, ...)
	{
		Entry next;
		va_list args;

		va_start(args, szFormat);

		int cchBuffer = _vscprintf(szFormat, args);

		if (cchBuffer != -1)
		{
			char* szBuffer = (char*)malloc(cchBuffer + 1);

			vsnprintf(szBuffer, cchBuffer + 1, szFormat, args);

			next.szText = szBuffer;
			next.color = color;

			m_Items.emplace_back(next);
		}
		va_end(args);

		m_bShouldScrollToBottom = true;
	}

	void Draw(const char* szTitle)
	{
		bool bReclaimFocus = false;
		ImVec2 size = ImVec2(820, 400);
		std::vector<Entry> m_Display = m_Items;

		if (!ImGui::BeginChild(szTitle, size, true, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
		{
			ImGui::EndChild();
			return;
		}

		if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows))
			ImGui::InputText("Filter", m_szFilter, ARRAYSIZE(m_szFilter));

		if (strlen(m_szFilter) > 0)
		{
			for (auto& it = m_Display.begin(); it != m_Display.end(); )
				if (!strstr(it->szText, m_szFilter))
					it = m_Display.erase(it);
				else 
					++it;
		}

		ImGui::PushTextWrapPos(ImGui::GetWindowPos().x + size.x);

		for (auto& it : m_Display)
		{
			ImGui::PushStyleColor(ImGuiCol_Text, it.color);
			ImGui::TextUnformatted(it.szText);
			ImGui::PopStyleColor();
		}

		if (m_bScrollToBottom)
		{
			if (m_bShouldScrollToBottom)
			{
				ImGui::SetScrollHereY(1.0f);
				m_bShouldScrollToBottom = false;
			}
		}

		ImGui::PopTextWrapPos();
		ImGui::EndChild();

		if (ImGui::InputText("Command", m_szInput, ARRAYSIZE(m_szInput), ImGuiInputTextFlags_EnterReturnsTrue))
		{
			bReclaimFocus = true;
		}

		// Auto-focus on window apparition
		ImGui::SetItemDefaultFocus();

		if (bReclaimFocus)
			ImGui::SetKeyboardFocusHere(-1); // Auto focus previous widget

		ImGui::SameLine();
		
		ImGui::Checkbox("Auto Scroll", &m_bScrollToBottom);
		
		ImGui::SameLine();

		if (ImGui::Button("Clear"))
			Clear();

		ImGui::Separator();
		ImGui::Checkbox("Show Game Errors", &Vars.Misc.bConsoleShowGameErrors);
	}
};
extern CConsole* g_pConsole;