#pragma once
#include <Windows.h>
#include <stdio.h>

//#define VERBOSE		// helps with debugging for retards
#define TODO_CLICKABLE
#define FIXME_CLICKABLE

#ifndef _MAKE_STRING
#define _MAKE_STRING(x)  #x
#endif // !_MAKE_STRING

#ifndef MAKE_STRING
#define MAKE_STRING(x) _MAKE_STRING(x)
#endif // !MAKE_STRING

// Move todo & fixme macros out of Log.h!! (you lazy fuck)

#ifndef TODO
#ifndef TODO_CLICKABLE
#define TODO(x) __pragma(message("TODO: " MAKE_STRING(x) " -> " __FILE__ "@" MAKE_STRING(__LINE__))) 
#else
#define TODO(x) __pragma(message(__FILE__ "(" MAKE_STRING(__LINE__) "): TODO: " MAKE_STRING(x))) 
#endif // !TODO_CLICKABLE
#endif // !TODO

#ifndef FIXME
#ifndef FIXME_CLICKABLE
#define FIXME(x) __pragma(message("FIXME: " MAKE_STRING(x) " -> " __FILE__ "@" MAKE_STRING(__LINE__))) 
#else
#define FIXME(x) __pragma(message(__FILE__ "(" MAKE_STRING(__LINE__) "): FIXME: " MAKE_STRING(x))) 
#endif // !TODO_CLICKABLE
#endif // !TODO

#if defined(_DEBUG) || defined(VERBOSE)
#define LOG_OFFSETS
#define LOG(fmt, ...) Log::Log(__FUNCSIG__, fmt, __VA_ARGS__)
#define WARN(fmt, ...) Log::Warn(__FUNCSIG__, fmt, __VA_ARGS__)
#define ERROR(fmt, ...) Log::Error(__FUNCSIG__, fmt, __VA_ARGS__)
#else 
#define LOG(fmt, ...) ((void)0)
#define WARNfmt, ...) ((void)0)
#define ERROR(fmt, ...) ((void)0)
#endif

#ifdef LOG_OFFSETS
#define LOG_OFFSET(name, offset) Log::LogOffset(name, offset)
#else
#define LOG_OFFSET(name, offset)
#endif


typedef struct _USER_NOTIFY
{
	HANDLE hCallerEvent;
	CONST CHAR* szMsg;
	CONST CHAR* szCompletedMsg;
} USER_NOTIFY, * PUSER_NOTIFY;

enum ConsoleColors
{
	BLACK = 0x0,
	BLUE = 0x1,
	GREEN = 0x2,
	AQUA = 0x3,
	RED = 0x4,
	PURPLE = 0x5,
	YELLOW = 0x6,
	WHITE = 0x7,
	GRAY = 0x8,
	LIGHT_BLUE = 0x9,
	LIGHT_GREEN = 0xA,
	LIGHT_AQUA = 0xB,
	LIGHT_RED = 0xC,
	LIGHT_PURPLE = 0xD,
	LIGHT_YELLOW = 0xE,
	PURE_WHITE = 0xF
};

void WINAPI UserNotify(PUSER_NOTIFY pParams);

class Log
{
public:
	explicit Log(const char* szFunction, const char* Fmt, ...)
	{
		va_list Args;

		va_start(Args, Fmt);
		Write(stdout, LIGHT_GREEN, BLACK, szFunction, Fmt, Args);
		va_end(Args);
	}

	static void Warn(const char* szFunction, const char* Fmt, ...)
	{
		va_list Args;

		va_start(Args, Fmt);
		Write(stdout, LIGHT_YELLOW, BLACK, szFunction, Fmt, Args);
		va_end(Args);
	}

	static void Error(const char* szFunction, const char* Fmt, ...)
	{
		va_list Args;

		va_start(Args, Fmt);
		Write(stdout, LIGHT_RED, BLACK, szFunction, Fmt, Args);
		va_end(Args);
	}

	static void Write(FILE* pStream, enum ConsoleColors Foreground, enum ConsoleColors Background, const char* szFunction, const char* Fmt, va_list Args)
	{
		enum ConsoleColors PreviousForeground;
		enum ConsoleColors PreviousBackground;

		GetConsoleColors(PreviousForeground, PreviousBackground);
		fprintf(pStream, "[%s]: ", szFunction);
		SetConsoleColors(Foreground, Background);
		vfprintf(pStream, Fmt, Args);
		SetConsoleColors(PreviousForeground, PreviousBackground);
	}

	static void AttachConsole(const wchar_t* szConsoleName)
	{
		if (!AllocConsole())
			return;

		SetConsoleTitleW(szConsoleName);
		freopen_s(&pStdout, "CON", "w", stdout);
		freopen_s(&pStderr, "CON", "w", stderr);
	}

	static inline void DetachConsole(void)
	{
		fclose(pStdout);
		fclose(pStderr);
		FreeConsole();
	}

	static void SetConsoleColors(enum ConsoleColors Foreground, enum ConsoleColors Background)
	{
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), Foreground | (Background << 4));
	}

	static void GetConsoleColors(enum ConsoleColors& Foreground, enum ConsoleColors& Background)
	{
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
		Foreground = (ConsoleColors)(csbi.wAttributes & 0x0F);
		Background = (ConsoleColors)((csbi.wAttributes & 0xF0) >> 4);
	}

	static void GotoXY(SHORT x, SHORT y)
	{
		COORD CursorPosition = { x, y };
		SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), CursorPosition);
	}

	static void AnimateWait(const char* szIn, HANDLE hWait, DWORD dwTickrate)
	{
		const static char animations[4] = { '|', '/', '-', '\\' };
		INT iSequence = 0;
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		COORD now;

		printf(szIn);

		GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
		now = csbi.dwCursorPosition;

		do {
			GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
			now = csbi.dwCursorPosition;
			printf("%c", animations[iSequence++]);
			iSequence %= ARRAYSIZE(animations);
			GotoXY(now.X, now.Y);
			Sleep(dwTickrate);
		} while (WaitForSingleObject(hWait, 0) != WAIT_OBJECT_0);
		printf("\n");
	}

	static void AnimatePrint(const char* szIn, DWORD dwTickrate)
	{
		const static char s_animations[4] = { '|', '/', '-', '\\' };
		INT iSequence = 0;
		INT iLength = strlen(szIn);
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		COORD now;

		GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
		now = csbi.dwCursorPosition;

		for (INT i = 0; i < iLength; )
		{
			GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
			now = csbi.dwCursorPosition;
			printf("%c", s_animations[iSequence++]);
			iSequence %= ARRAYSIZE(s_animations);
			GotoXY(now.X, now.Y);

			if (!iSequence)
				printf("%c", szIn[i++]);

			Sleep(dwTickrate);
		}
	}

	static void LogOffset(const char* szName, void* p);

private:
	static FILE* pStdout;
	static FILE* pStderr;
};