#pragma once
#include <dbghelp.h>
#include <d3dcompiler.h>
#include <Shlobj.h>
#include "Globals.h"
#include "Matrix4x4.h"
#include "Log.h"

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

static bool WorldToScreen(const Vector3& vIn, Vector2& vOut)
{
	Matrix4x4 vMatrix = *(Matrix4x4*)0x1419C73C0;
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
	return (unsigned short)((handle & 0x00FFFF00) >> 8);
}

static inline EntityHandle GenerateEntityHandle(const EntityInfoList* pList, const unsigned short index)
{
	return (index | (pList->m_dwShift << 16)) << 8;
}

// Reversed from binary
static BOOL ObjectIdToObjectName(char*szObjectName, size_t size, int objectId)
{
	static char HexChars[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
	static ObjectIdConvert Converts[] = { { 0x10000, "pl" }, { 0x20000, "em" }, { 0x30000, "wp" }, { 0x40000, "et" }, { 0x50000, "ef" }, { 0x60000, "es" },
										{ 0x70000, "it" }, { 0x90000, "sc" }, { 0xA0000, "um" }, { 0xC0000, "bg" }, { 0xE0000, "bh" }, { 0xF0000, "ba" } };

	unsigned int i = 0;

	do
	{
		if (Converts[i].m_ObjectIdBase == (objectId & 0xFFFF0000)) // high word of the int
		{
			szObjectName[0] = Converts[i].m_szPrefix[0];
			szObjectName[1] = Converts[i].m_szPrefix[1];
			szObjectName[2] = HexChars[((signed __int64)objectId >> 12) & 0xF];
			szObjectName[3] = HexChars[((signed __int64)objectId >> 8) & 0xF];
			szObjectName[4] = HexChars[((signed __int64)objectId >> 4) & 0xF];
			szObjectName[5] = HexChars[objectId & 0xF];
			szObjectName[6] = 0; //null terminator
			return TRUE;
		}
		++i;
	} while (i < ARRAYSIZE(Converts));

	memset(szObjectName, 0, size);

	if (objectId == -1)
	{
		strcpy_s(szObjectName, size, "eObjInvalid");
		return FALSE;
	}
	else
	{
		strcpy_s(szObjectName, size, (objectId != 0x700000) ? "Unknow" : "Null");
		return FALSE;
	}
}

static BOOL ObjectNameToObjectId(int* pObjectId, const char* szObjectName)
{
	static ObjectIdConvert Converts[] = { { 0x10000, "pl" }, { 0x20000, "em" }, { 0x30000, "wp" }, { 0x40000, "et" }, { 0x50000, "ef" }, { 0x60000, "es" },
										{ 0x70000, "it" }, { 0x90000, "sc" }, { 0xA0000, "um" }, { 0xC0000, "bg" }, { 0xE0000, "bh" }, { 0xF0000, "ba" } };

	size_t i = 0;

	if (!szObjectName)
		return FALSE;

	for (; i < ARRAYSIZE(Converts); ++i)
		if (!strncmp(Converts[i].m_szPrefix, szObjectName, 2))
			break;

	*pObjectId = Converts[i].m_ObjectIdBase | ((szObjectName[2] - 0x30) << 12) | ((szObjectName[3] - 0x30) << 8) | ((szObjectName[4] - 0x30) << 4) | (szObjectName[5] - 0x30);

	return TRUE;
}

static HRESULT GeneratePixelShader(ID3D11Device* pDevice, ID3D11PixelShader** ppPixelShader, float r, float g, float b)
{
	char szCast[] = "struct VS_OUT"
		"{"
		" float4 Position : SV_Position;"
		" float4 Color : COLOR0;"
		"};"
		"float4 main( VS_OUT input ) : SV_Target"
		"{"
		" float4 fake;"
		" fake.a = 1.0f;"
		" fake.r = %f;"
		" fake.g = %f;"
		" fake.b = %f;"
		" return fake;"
		"}";

	ID3D10Blob* pBlob;
	ID3DBlob* pErrorMsgBlob;
	char szPixelShader[1000];

	sprintf_s(szPixelShader, szCast, r, g, b);

	HRESULT hr = D3DCompile(szPixelShader, sizeof(szPixelShader), "shader", NULL, NULL, "main", "ps_4_0", NULL, NULL, &pBlob, &pErrorMsgBlob);

	if (FAILED(hr))
		return hr;

	hr = pDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, ppPixelShader);

	if (FAILED(hr))
		return hr;

	return S_OK;
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

	UNREFERENCED_PARAMETER(Entry);

	//HeapWalk(p)

	return ERROR_SUCCESS;
}

static LONG UnhandledExceptionHandlerChild(EXCEPTION_POINTERS* pException)
{
	if (!WriteMiniDump(pException))
		LOG("Failed to write dump file!\n");

	return EXCEPTION_CONTINUE_SEARCH;
}

static LONG UnhandledExceptionHandler(EXCEPTION_POINTERS* pException)
{
	for (auto& handler : g_pExceptionHandlers)
		handler(pException);

	return EXCEPTION_CONTINUE_SEARCH;
}