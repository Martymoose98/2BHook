#pragma once
#include <dbghelp.h>
#include <d3dx9.h>
#include <d3dx9math.h>
#include <Shlobj.h>
#include "Globals.h"
#include "Variables.h"

#pragma comment(lib, "d3dx9.lib")

#define STACK_TIMER(name) StackTimer name(__FUNCTION__)

typedef struct StackTimer
{
	StackTimer(const char* szName)
	{
		m_szName = szName;
		m_start = clock();
	}

	~StackTimer()
	{
		m_end = clock();
		m_duration = (double)(m_end - m_start) / CLOCKS_PER_SEC;

		printf("%s took %fs\n", m_szName, m_duration);
	}

private:
	clock_t m_start, m_end;
	double m_duration;
	const char* m_szName;
} DebugStackTimer;

static void God()
{
	if (Vars.Gameplay.bGodmode || Vars.Gameplay.bNoEnemyDamage)
		g_pMemory->NopMemory(&nop_Health[NOP_DAMAGE_ENEMY]);

	if (Vars.Gameplay.bGodmode || Vars.Gameplay.bNoWorldDamage)
		g_pMemory->NopMemory(&nop_Health[NOP_DAMAGE_WORLD]);
}

static void Ungod()
{
	if (!Vars.Gameplay.bGodmode && !Vars.Gameplay.bNoEnemyDamage)
		g_pMemory->RestoreMemory(&nop_Health[NOP_DAMAGE_ENEMY]);

	if (!Vars.Gameplay.bGodmode && !Vars.Gameplay.bNoWorldDamage)
		g_pMemory->RestoreMemory(&nop_Health[NOP_DAMAGE_WORLD]);
}

static bool WorldToScreen(const Vector3& vIn, Vector2& vOut)
{
	Matrix4x4& vMatrix = *(Matrix4x4*)0x1419C73C0;
	vMatrix.Transpose();

	vOut.x = vMatrix[0][0] * vIn[0] + vMatrix[0][1] * vIn[1] + vMatrix[0][2] * vIn[2] + vMatrix[0][3];
	vOut.y = vMatrix[1][0] * vIn[0] + vMatrix[1][1] * vIn[1] + vMatrix[1][2] * vIn[2] + vMatrix[1][3];
	float w = vMatrix[3][0] * vIn[0] + vMatrix[3][1] * vIn[1] + vMatrix[3][2] * vIn[2] + vMatrix[3][3];

	if (w < 0.01f)
	{
		return false;
	}

	FLOAT invw = 1.0f / w;

	vOut *= invw;

	int width = g_pGraphicDevice->iScreenWidth;
	int height = g_pGraphicDevice->iScreenHeight;

	float x = (float)(width / 2);
	float y = (float)(height / 2);

	x += 0.5f * vOut.x * (float)width + 0.5f;
	y -= 0.5f * vOut.y * (float)height + 0.5f;

	vOut.x = x;
	vOut.y = y;
	return true;
}

static inline unsigned short EntityHandleToListIndex(const EntityHandle handle)
{
	return (handle & 0x00FFFF00) >> 8;
}

static inline EntityHandle GenerateEntityHandle(const EntityInfoList* pList, const unsigned short index)
{
	return (index | (pList->m_dwShift << 16)) << 8;
}

/*
Warning: this method is not synchronized with the game threads, therefore anomalies
may occur when copying files whilst being modified by another thread. Thread
safety will be added in the future. This method fails if the "My Documents" folder's
path has a greater length than MAX_PATH (260).
*/
static BOOL BackupSave(int nSlot)
{
	WCHAR szPath[MAX_PATH];
	WCHAR szSavePath[MAX_PATH];
	WCHAR szBackupPath[MAX_PATH];
	char szDateTime[32];
	tm timestruct;

	if (!IS_SAVE_SLOTDATA(nSlot))
		return ERROR_INVALID_PARAMETER;

	if (!SHGetSpecialFolderPathW(NULL, szPath, CSIDL_MYDOCUMENTS, FALSE))
		return ERROR_PATH_NOT_FOUND;

	int nCharsWritten = swprintf_s(szSavePath, MAX_PATH, L"%s\\My Games\\NieR_Automata\\SlotData_%d.dat", szPath, nSlot);

	if (nCharsWritten == -1)
		return ERROR_NOT_ENOUGH_MEMORY;

	time_t tTime = time(NULL);

	localtime_s(&timestruct, &tTime);

	strftime(szDateTime, _ARRAYSIZE(szDateTime), "%F_%H-%M-%S", &timestruct);

	nCharsWritten = swprintf_s(szBackupPath, MAX_PATH, L"%s\\My Games\\NieR_Automata\\SlotData_%d_%S.dat.bak", szPath, nSlot, szDateTime);

	if (nCharsWritten == -1)
		return ERROR_NOT_ENOUGH_MEMORY;

	if (!CopyFileW(szSavePath, szBackupPath, TRUE))
		return GetLastError();

	return ERROR_SUCCESS;
}

static BOOL WriteMiniDump(EXCEPTION_POINTERS* pException)
{
	WCHAR szDumpFile[MAX_PATH];
	SYSTEMTIME time;

	DWORD nCharsWritten = GetModuleFileNameW(NULL, szDumpFile, MAX_PATH);

	GetSystemTime(&time);

	wsprintfW(&szDumpFile[nCharsWritten - 4], L"_%4d%02d%02d_%02d%02d%02d.dmp", time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond);

	HANDLE hFile = CreateFileW(szDumpFile, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile == INVALID_HANDLE_VALUE)
		return FALSE;

	MINIDUMP_EXCEPTION_INFORMATION ExceptionInfo = { GetCurrentThreadId(), pException, FALSE };

	BOOL status = MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MINIDUMP_TYPE(MiniDumpWithUnloadedModules | MiniDumpWithIndirectlyReferencedMemory | MiniDumpWithFullMemoryInfo), pException ? &ExceptionInfo : NULL, NULL, NULL);

	CloseHandle(hFile);

	return status;
}

static BOOL QueryProcessHeaps(OUT HANDLE** pphHeaps, OUT OPTIONAL DWORD* pdwHeaps)
{
	if (!pphHeaps)
		return ERROR_INVALID_PARAMETER;

	DWORD dwNumberOfHeaps = GetProcessHeaps(0, NULL);

	if (!dwNumberOfHeaps)
		return ERROR_NO_MORE_ITEMS;

	*pphHeaps = (HANDLE*)malloc(dwNumberOfHeaps * sizeof(HANDLE));

	DWORD dwHeapsLength = dwNumberOfHeaps;

	dwNumberOfHeaps = GetProcessHeaps(dwNumberOfHeaps, *pphHeaps);

	if (!dwNumberOfHeaps)
	{
		return ERROR_NO_MORE_ITEMS;
	}
	else if (dwNumberOfHeaps > dwHeapsLength)
	{
		LOG("Another component created a heap between calls therefore, the result will be inaccurate.");
		return ERROR_INSUFFICIENT_BUFFER;
	}

	if (pdwHeaps)
		*pdwHeaps = dwNumberOfHeaps;

	return ERROR_SUCCESS;
}

static BOOL EnumProcessHeapInfo(IN HANDLE* phHeaps, IN DWORD dwHeaps)
{
	PROCESS_HEAP_ENTRY Entry;

	//HeapWalk(p)

	return ERROR_SUCCESS;
}

static LONG UnhandledExceptionHandler(EXCEPTION_POINTERS* pException)
{
	if (!WriteMiniDump(pException))
		LOG("Failed to write dump file!\n");

	return EXCEPTION_CONTINUE_SEARCH;
}