#pragma once
#include "Log.h"
#include "Console.h"

FILE* Log::pStdout;
FILE* Log::pStderr;

void WINAPI UserNotify(PUSER_NOTIFY pParams)
{
	if (pParams->hCallerEvent == INVALID_HANDLE_VALUE)
		return;

	Log::SetConsoleColors(LIGHT_PURPLE, BLACK);
	Log::AnimateWait(pParams->szMsg, pParams->hCallerEvent, 200);
	Log::SetConsoleColors(GREEN, BLACK);
	LOG("%s\n", pParams->szCompletedMsg);
}

void Log::LogOffset(const char* szName, void* p)
{
	if (szName)
	{
		g_pConsole->Log(ImVec4(0.0f, 0.5f, 0.8f, 1.0f), "%s -> %p", szName, p);

		if (!p)
			g_pConsole->Warn("%s is null! This may have fatal consequences!", szName);
	}
}