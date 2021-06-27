#pragma once
#include <Windows.h>
#include <stdio.h>

//#define VERBOSE		// helps with debugging for retards
#define TODO_CLICKABLE

#ifndef _MAKE_STRING
#define _MAKE_STRING(x)  #x
#endif // !_MAKE_STRING

#ifndef MAKE_STRING
#define MAKE_STRING(x) _MAKE_STRING(x)
#endif // !MAKE_STRING

// Move todo out of Log.h!!

#ifndef TODO
#ifndef TODO_CLICKABLE
#define TODO(x) __pragma(message("TODO: " MAKE_STRING(x) " -> " __FILE__ "@" MAKE_STRING(__LINE__))) 
#else
#define TODO(x) __pragma(message(__FILE__ "(" MAKE_STRING(__LINE__) "): TODO: " MAKE_STRING(x))) 
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

	static void LogOffset(const char* szName, void* p);

private:
	static FILE* pStdout;
	static FILE* pStderr;
};