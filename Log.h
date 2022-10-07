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
#else 
#define LOG(fmt, ...) ((void)0)
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
	explicit Log(const char* szFunction, const char* fmt, ...)
	{
		va_list args;

		printf("[%s]: ", szFunction);
		va_start(args, fmt);
		vprintf(fmt, args);
		va_end(args);
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

	static void SetConsoleColors(enum ConsoleColors foreground, enum ConsoleColors background)
	{
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), (foreground + (background * 16)));
	}

	static void GotoXY(SHORT x, SHORT y)
	{
		COORD CursorPosition = { x, y };
		SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), CursorPosition);
	}

	static void AnimateWait(const char* szIn, HANDLE hWait, DWORD dwTickrate)
	{
		const static char s_animations[4] = { '|', '/', '-', '\\' };
		INT iSequence = 0;
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		COORD now;

		printf(szIn);

		GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
		now = csbi.dwCursorPosition;

		do {
			GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
			now = csbi.dwCursorPosition;
			printf("%c", s_animations[iSequence++]);
			iSequence %= ARRAYSIZE(s_animations);
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