#include "Console.h"

CConsole* g_pConsole = new CConsole();

// FIXME: Console has perf issues when scratch buffer is populated
// Circular buffer? or LRU cache?
CConsole::CConsole(void)
	: m_Items(), m_bFilter(false), m_bScrollToBottom(false), 
	m_bShouldScrollToBottom(true), m_History(), m_iHistoryPos(0), m_Commands()
{
	ZeroMemory(m_szInput, sizeof(m_szInput));
	ZeroMemory(m_szFilter, sizeof(m_szFilter));
}

CConsole::~CConsole(void)
{
	Clear();
}

void CConsole::Clear(void)
{
	for (auto& it : m_Items)
		free(it.szText);

	m_Items.clear();
}

void CConsole::Log(const ImVec4& color, const char* szFormat, ...)
{
	va_list args;

	va_start(args, szFormat);
	Write(color, szFormat, args);
	va_end(args);
}

void CConsole::Warn(const char* szFormat, ...)
{
	va_list args;

	size_t cbSize = strlen(szFormat) + 9;

	char* szFmt = (char*)malloc(cbSize);

	if (szFmt)
	{
		strcpy_s(szFmt, cbSize, "[Warn]: ");
		strcat_s(szFmt, cbSize, szFormat);

		va_start(args, szFormat);
		Write(ImVec4(0.95f, 0.6f, 0.14f, 1.0f), szFmt, args);
		va_end(args);
		free(szFmt);
	}
}

void CConsole::Error(const char* szFormat, ...)
{
	va_list args;

	size_t cbSize = strlen(szFormat) + 10;

	char* szFmt = (char*)malloc(cbSize);

	if (szFmt)
	{
		strcpy_s(szFmt, cbSize, "[Error]: ");
		strcat_s(szFmt, cbSize, szFormat);

		va_start(args, szFormat);
		Write(ImVec4(0.95f, 0.2f, 0.14f, 1.0f), szFmt, args);
		va_end(args);
		free(szFmt);
	}
}

void CConsole::Write(const ImVec4& color, const char* szFormat, ...)
{
	va_list args;

	va_start(args, szFormat);
	Write(color, szFormat, args);
	va_end(args);
}

void CConsole::Write(const ImVec4& color, const char* szFormat, va_list args)
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

void CConsole::Draw(const char* szTitle)
{
	bool bReclaimFocus = false;
	ImVec2 size = ImVec2(820, 400);
	std::vector<Entry> m_Display = m_Items;

	if (!ImGui::BeginChild(szTitle, size, true, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
	{
		ImGui::EndChild();
		return;
	}

	// Fix the filter so it floats at the top
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

void CRILogCallbackConsole(const char* szFormat, unsigned int callback_arg_ptr_high, unsigned int callback_arg_ptr_low, void* a4)
{
	if (Vars.Misc.bConsoleShowGameErrors)
		g_pConsole->Warn("%s\n", CRIGetBuffer(szFormat, callback_arg_ptr_high, callback_arg_ptr_low));
}