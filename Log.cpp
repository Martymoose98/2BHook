#pragma once
#include "Log.h"

CConsole* g_pConsole = new CConsole;

FILE* Log::pStdout;
FILE* Log::pStderr;

void Log::LogOffset(const char* szName, void* p)
{
	if (szName)
	{
		g_pConsole->Log(ImVec4(0.0f, 0.5f, 0.8f, 1.0f), "%s -> %llx", szName, (QWORD)p);

		if (!p)
			g_pConsole->Warn("%s is null! This may have fatal consequences!", szName);
	}
}