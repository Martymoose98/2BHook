#include "Console.h"

CConsole* g_pConsole = new CConsole();

CConsole::CConsole(void)
	: m_uFlags(CONFLAGS_ENABLE_AUTOSCROLL | CONFLAGS_ENABLE_TIMESTAMPS), m_Items(), m_Commands()
{
	ZeroMemory(m_szInput, sizeof(m_szInput));
}

CConsole::~CConsole(void)
{
	Clear();
}

void CConsole::Clear(void)
{
	std::lock_guard<std::mutex> Lock(m_Mutex);

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

	va_start(args, szFormat);
	WritePrefixed(ImVec4(0.95f, 0.6f, 0.14f, 1.0f), "[Warn]: ", szFormat, args);
	va_end(args);
}

void CConsole::Error(const char* szFormat, ...)
{
	va_list args;

	va_start(args, szFormat);
	WritePrefixed(ImVec4(0.95f, 0.2f, 0.14f, 1.0f), "[Error]: ", szFormat, args);
	va_end(args);
}

void CConsole::Write(const ImVec4& color, const char* szFormat, ...)
{
	va_list args;

	va_start(args, szFormat);
	Write(color, szFormat, args);
	va_end(args);
}

static bool FormatToString(std::string& Out, const char* szFormat, va_list args)
{
	// _vscprintf consumes args, so the vsnprintf below needs its own copy. Reusing a
	// va_list after it has been walked is undefined behaviour - it only appeared to work
	// here because of how x64 happens to lay va_list out.
	va_list SizeArgs;

	va_copy(SizeArgs, args);

	const int cchText = _vscprintf(szFormat, SizeArgs);

	va_end(SizeArgs);

	if (cchText < 0)
		return false;

	// Writing through &Out[0] is well defined since C++11 (the null terminator slot is
	// writable), and it drops the malloc/free pairing that Clear() had to mirror by hand.
	Out.resize((size_t)cchText);

	if (cchText > 0)
		vsnprintf(&Out[0], (size_t)cchText + 1, szFormat, args);

	return true;
}

// "[HH:MM:SS.mmm] ". Sized for the fixed-width form above.
static void FormatTimestamp(char* szBuffer, size_t cchBuffer, const SYSTEMTIME& Time)
{
	_snprintf_s(szBuffer, cchBuffer, _TRUNCATE, "[%02hu:%02hu:%02hu.%03hu] ",
		Time.wHour, Time.wMinute, Time.wSecond, Time.wMilliseconds);
}

void CConsole::Append(const ImVec4& color, std::string&& Text)
{
	Entry Next;

	Next.m_Color = color;
	Next.m_Text = std::move(Text);

	// Most callers end their format string with \n, which is right for the stdout logger
	// but leaves a blank line per entry here - the console is already line-based. It shows
	// up badly once timestamps put each entry on its own labelled row.
	while (!Next.m_Text.empty() &&
		(Next.m_Text.back() == '\n' || Next.m_Text.back() == '\r'))
	{
		Next.m_Text.pop_back();
	}

	GetLocalTime(&Next.m_Time);

	std::lock_guard<std::mutex> Lock(m_Mutex);

	m_Items.emplace_back(std::move(Next));

	while (m_Items.size() > s_uMaxEntries)
		m_Items.pop_front();

	m_uFlags |= CONFLAGS_SHOULD_AUTOSCROLL;
}

// Prefixing used to build a temporary "[Warn]: %s"-style format string with malloc +
// strcat_s, which meant a heap allocation per line and silently dropped the message
// entirely when that allocation failed. The prefix is not a format string, so format the
// caller's text first and prepend the literal before the entry is ever published.
void CConsole::WritePrefixed(const ImVec4& color, const char* szPrefix, const char* szFormat, va_list args)
{
	std::string Text;

	if (!szFormat || !FormatToString(Text, szFormat, args))
		return;

	Text.insert(0, szPrefix);

	Append(color, std::move(Text));
}

void CConsole::Write(const ImVec4& color, const char* szFormat, va_list args)
{
	std::string Text;

	if (!szFormat || !FormatToString(Text, szFormat, args))
		return;

	Append(color, std::move(Text));
}

void CConsole::Draw(const char* szTitle, const ImVec2 WindowSize)
{
	// NOTE: sending a zero vector for window size enables auto sizing
	if (!ImGui::BeginChild(szTitle, WindowSize, ImGuiChildFlags_Borders,
		ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
	{
		ImGui::EndChild();
		return;
	}

	FilterBar();

	EnumConsoleData();

	ImGui::EndChild();

	InputBar();

	ImGui::SameLine();

	ImGui::CheckboxFlags("Auto Scroll", &m_uFlags, (uint32_t)CONFLAGS_ENABLE_AUTOSCROLL);

	ImGui::SameLine();

	ImGui::CheckboxFlags("Timestamps", &m_uFlags, (uint32_t)CONFLAGS_ENABLE_TIMESTAMPS);

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

void CConsole::InputBar(void)
{
	// SetKeyboardFocusHere(-1) has to be issued before the widget it refocuses is
	// submitted, so the previous version - which set it after InputText had already run -
	// never actually returned focus to the box after a command.
	if (ImGui::IsWindowAppearing())
		ImGui::SetKeyboardFocusHere();

	if (ImGui::InputText("Command", m_szInput, ARRAYSIZE(m_szInput),
		ImGuiInputTextFlags_EnterReturnsTrue))
	{
		if (m_szInput[0])
		{
			// TODO: dispatch through m_Commands once commands are actually registered.
			Log(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "> %s", m_szInput);
		}

		// The buffer was never cleared, so the previous command stayed in the box.
		m_szInput[0] = '\0';

		ImGui::SetKeyboardFocusHere(-1);
	}
}

void CConsole::EnumConsoleData(void)
{
	if (!ImGui::BeginChild("ConsoleTextArea"))
	{
		ImGui::EndChild();
		return;
	}

	// PushTextWrapPos takes an X position in *window local* space. The old call passed
	// GetWindowPos().x + GetWindowWidth(), which mixes a screen coordinate with a width,
	// putting the wrap position hundreds of pixels off the right edge - so nothing ever
	// wrapped. 0.0f means "wrap at the right edge of the content region".
	ImGui::PushTextWrapPos(0.0f);

	{
		std::lock_guard<std::mutex> Lock(m_Mutex);

		const bool bTimestamps = (m_uFlags & CONFLAGS_ENABLE_TIMESTAMPS) != 0;

		// The UI font is proportional, so digit strings are not all the same width. Lay the
		// message out at a fixed column instead of relying on the stamp's own width, or the
		// text edges end up ragged between rows.
		const float flMessageColumn = (bTimestamps)
			? ImGui::CalcTextSize("[00:00:00.000] ").x : 0.0f;

		for (const Entry& Item : m_Items)
		{
			// Filter on the message only - matching against the timestamp would make an
			// innocent filter like "01" hit half the buffer.
			if (!m_TextFilter.PassFilter(Item.m_Text.c_str()))
				continue;

			if (bTimestamps)
			{
				char szTime[24];

				FormatTimestamp(szTime, sizeof(szTime), Item.m_Time);

				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.45f, 0.45f, 0.45f, 1.0f));
				ImGui::TextUnformatted(szTime);
				ImGui::PopStyleColor();

				ImGui::SameLine(flMessageColumn, 0.0f);
			}

			ImGui::PushStyleColor(ImGuiCol_Text, Item.m_Color);
			ImGui::TextUnformatted(Item.m_Text.c_str());
			ImGui::PopStyleColor();
		}

		// Autoscroll. Consuming the flag inside the lock keeps it in step with the writers
		// that set it.
		if ((m_uFlags & CONFLAGS_ENABLE_AUTOSCROLL) && (m_uFlags & CONFLAGS_SHOULD_AUTOSCROLL))
		{
			ImGui::SetScrollHereY(1.0f);
			m_uFlags &= ~CONFLAGS_SHOULD_AUTOSCROLL;
		}
	}

	ImGui::PopTextWrapPos();

	ImGui::EndChild();
}

void CRILogCallbackConsole(const char* szFormat, unsigned int callback_arg_ptr_high, unsigned int callback_arg_ptr_low, void* a4)
{
	if (Vars.Misc.bConsoleShowGameErrors)
		g_pConsole->Warn("%s\n", CRIGetBuffer(szFormat, callback_arg_ptr_high, callback_arg_ptr_low));
}
