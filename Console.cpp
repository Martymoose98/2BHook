#include "Console.h"

CConsole* g_pConsole = new CConsole();

// FIXME/REPROFILE: Console has perf issues when scratch buffer is populated
// Circular buffer? or LRU cache?
// can use std::lock_guard<std::mutex> lock(m_Mutex);
// to try and make it thread safe with mutex in this ptr obj.
// REF: https://github.com/rmxbalanque/imgui-console/blob/master/src/imgui_console.cpp
CConsole::CConsole(void)
	: m_Items(), m_uFlags(0), m_bFilter(false), m_bScrollToBottom(false),
	m_bShouldScrollToBottom(true), m_Commands()
{
	ZeroMemory(m_szInput, sizeof(m_szInput));
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

	m_uFlags |= CONFLAGS_SHOULD_AUTOSCROLL;
}

void CConsole::Draw(const char* szTitle, const ImVec2 WindowSize)
{
	bool bReclaimFocus = false;

	// NOTE: sending a zero vector for window size enables auto sizing
	if (!ImGui::BeginChild(szTitle, WindowSize, true, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
	{
		ImGui::EndChild();
		return;
	}

	// Fix the filter so it floats at the top
	// RETURNS? bool didChange = ??
	FilterBar();

	if (m_TextFilter.IsActive())
		m_uFlags |= CONFLAGS_FILTER_ENTRIES;
	else
		m_uFlags &= ~CONFLAGS_FILTER_ENTRIES;

	EnumConsoleData();

	ImGui::EndChild();

	// Command input box
	if (ImGui::InputText("Command", m_szInput, ARRAYSIZE(m_szInput), ImGuiInputTextFlags_EnterReturnsTrue))
	{
		bReclaimFocus = true;
	}

	// Auto-focus on window apparition
	ImGui::SetItemDefaultFocus();

	if (bReclaimFocus)
		ImGui::SetKeyboardFocusHere(-1); // Auto focus previous widget

	ImGui::SameLine();

	ImGui::CheckboxFlagsT<uint32_t>("Auto Scroll", &m_uFlags, CONFLAGS_ENABLE_AUTOSCROLL);

	ImGui::SameLine();

	if (ImGui::Button("Clear"))
		Clear();

	ImGui::Separator();
	ImGui::Checkbox("Show Game Errors", &Vars.Misc.bConsoleShowGameErrors);
}

void CConsole::FilterBar(void)
{
	m_TextFilter.Draw("Filter", ImGui::GetWindowWidth() * 0.25f);
	ImGui::Separator();
}


void CConsole::EnumConsoleData(void)
{
	if (!ImGui::BeginChild("ConsoleTextArea##"))
	{
		ImGui::EndChild();
		return;
	}

	ImGui::PushTextWrapPos(ImGui::GetWindowPos().x + ImGui::GetWindowWidth());

	// Display items.
	for (const auto& Entry : m_Items)
	{
		// Filter out the item if the text fails
		if (!m_TextFilter.PassFilter(Entry.szText))
			continue;

		ImGui::PushStyleColor(ImGuiCol_Text, Entry.color);
		ImGui::TextUnformatted(Entry.szText);
		ImGui::PopStyleColor();
	}

	ImGui::PopTextWrapPos();

	// Autoscroll
	if ((m_uFlags & CONFLAGS_ENABLE_AUTOSCROLL) && (m_uFlags & CONFLAGS_SHOULD_AUTOSCROLL))
	{
		ImGui::SetScrollHereY(1.0f);
		m_uFlags &= ~CONFLAGS_SHOULD_AUTOSCROLL;
	}

	ImGui::EndChild();
}

void CRILogCallbackConsole(const char* szFormat, unsigned int callback_arg_ptr_high, unsigned int callback_arg_ptr_low, void* a4)
{
	if (Vars.Misc.bConsoleShowGameErrors)
		g_pConsole->Warn("%s\n", CRIGetBuffer(szFormat, callback_arg_ptr_high, callback_arg_ptr_low));
}