#pragma once
#include <dbghelp.h>
#include <d3dcompiler.h>
#include <Shlobj.h>
#include "Globals.h"
#include "Matrix4x4.h"
#include "Log.h"

#define STACK_TIMER(name) StackTimer name(__FUNCTION__)
#define CS_LOCK() ReadWriteLock _CRT_CONCATENATE(__FUNCTION__, Lock)()
#define CS_LOCK_SPINCOUNT(spincount) ReadWriteLock _CRT_CONCATENATE(__FUNCTION__, Lock)((DWORD)(spincount))

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

struct ReadWriteLock
{
	ReadWriteLock() 
	{
		if (!m_bCriticalSectionInitalized)
		{
			InitializeCriticalSection(&m_CriticalSection);
			m_bCriticalSectionInitalized = TRUE;
			EnterCriticalSection(&m_CriticalSection);
		}
	}

	ReadWriteLock(DWORD dwSpinCount)
	{
		if (!m_bCriticalSectionInitalized)
		{
			InitializeCriticalSectionAndSpinCount(&m_CriticalSection, dwSpinCount);
			m_bCriticalSectionInitalized = TRUE;
			EnterCriticalSection(&m_CriticalSection);
		}
	}

	~ReadWriteLock() 
	{ 
		if (m_bCriticalSectionInitalized)
		{
			LeaveCriticalSection(&m_CriticalSection);
			DeleteCriticalSection(&m_CriticalSection);
			m_bCriticalSectionInitalized = FALSE;
		}
	}

private:
	CRITICAL_SECTION m_CriticalSection;
	BOOL m_bCriticalSectionInitalized;
};

static bool WorldToScreen(const Vector3& vIn, Vector2& vOut)
{
	VMatrix vMatrix = *g_pViewMatrix;//*(VMatrix*)0x1419C73C0;
	vMatrix.Transpose();

	float w = vMatrix[3][0] * vIn[0] + vMatrix[3][1] * vIn[1] + vMatrix[3][2] * vIn[2] + vMatrix[3][3];

	if (w < 0.01f)
		return false;

	vOut.x = vMatrix[0][0] * vIn[0] + vMatrix[0][1] * vIn[1] + vMatrix[0][2] * vIn[2] + vMatrix[0][3];
	vOut.y = vMatrix[1][0] * vIn[0] + vMatrix[1][1] * vIn[1] + vMatrix[1][2] * vIn[2] + vMatrix[1][3];

	float invw = 1.0f / w;

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

// Reversed from binary with added functionality
static BOOL ObjectIdToObjectName(char* szObjectName, size_t size, int objectId, ObjectIdConvert** ppConvert)
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
			szObjectName[2] = HexChars[(objectId >> 12) & 0xF];
			szObjectName[3] = HexChars[(objectId >> 8) & 0xF];
			szObjectName[4] = HexChars[(objectId >> 4) & 0xF];
			szObjectName[5] = HexChars[objectId & 0xF];
			szObjectName[6] = 0; //null terminator
			
			if (*ppConvert)
				*ppConvert = &Converts[i];

			return TRUE;
		}
		++i;
	} while (i < ARRAYSIZE(Converts));

	memset(szObjectName, 0, size);

	if (objectId == -1)
		strcpy_s(szObjectName, size, "eObjInvalid");
	else
		strcpy_s(szObjectName, size, (objectId != 0x700000) ? "Unknow" : "Null");

	if (*ppConvert)
		*ppConvert = NULL;

	return FALSE;
}

static BOOL ObjectNameToObjectId(int* pObjectId, const char* szObjectName)
{
	static ObjectIdConvert Converts[] = { { 0x10000, "pl" }, { 0x20000, "em" }, { 0x30000, "wp" }, { 0x40000, "et" }, { 0x50000, "ef" }, { 0x60000, "es" },
										{ 0x70000, "it" }, { 0x90000, "sc" }, { 0xA0000, "um" }, { 0xC0000, "bg" }, { 0xE0000, "bh" }, { 0xF0000, "ba" } };

	size_t i = 0;
	BOOLEAN bFound = FALSE;

	if (!szObjectName)
		return FALSE;

	for (; i < ARRAYSIZE(Converts); ++i)
	{
		if (Converts[i].m_szPrefix[0] == szObjectName[0] && Converts[i].m_szPrefix[1] == szObjectName[1])
		{
			bFound = TRUE;
			break;
		}
	}

	*pObjectId = (bFound) ? Converts[i].m_ObjectIdBase | ((szObjectName[2] - 0x30) << 12) | ((szObjectName[3] - 0x30) << 8) | ((szObjectName[4] - 0x30) << 4) | (szObjectName[5] - 0x30) : -1;

	return TRUE;
}

static DWORD DataFile_QueryFileIndex(DATHeader** ppBuffer, const char *szFileName)
{
	const char *szName; // r12
	LPBYTE *pHeaders; // r15
	unsigned int v4; // esi
	DATHeader* pHdr; // r8
	DWORD dwOffset; // r9
	DWORD dwFileCount; // ebp
	DWORD dwNextNameOffset; // r14
	const char *szCurrentFileName; // rdi
	unsigned int i; // ebx
	DWORD result; // eax

	szName = szFileName;
	pHeaders = (LPBYTE*)ppBuffer;
	v4 = 0;

	while (v4 < 2)
	{
		pHdr = (DATHeader*)*pHeaders;

		if (*pHeaders)
		{
			dwOffset = pHdr->dwNameTableOffset;
			if (dwOffset)
			{
				dwFileCount = pHdr->dwFileCount;
				dwNextNameOffset = *(DWORD*)((LPBYTE)pHdr + dwOffset);
szCurrentFileName = (const char*)(pHdr + dwOffset + 4);
i = 0;
if (dwFileCount)
{
	while (_stricmp(szName, szCurrentFileName))
	{
		++i;
		szCurrentFileName += dwNextNameOffset;
		if (i >= dwFileCount)
			goto next_file;
	}
	result = i + (v4 << 28);
	if (result != -1)
		return result;
}
			}
		}
	next_file:
		++v4;
		++pHeaders;
	}
	return 0xFFFFFFFF; //-1
}

static void DataFile_EnumContents(void* pBuffer, const char*** pppszFiles, DWORD* pdwFileCount)
{
	if (!pBuffer)
		return;

	if (!pppszFiles)
		return;

	if (!pdwFileCount)
		return;

	DATHeader* pHdr = (DATHeader*)pBuffer;

	*pdwFileCount = pHdr->dwFileCount;
	DWORD dwNextNameOffset = *(DWORD*)((LPBYTE)pBuffer + pHdr->dwNameTableOffset);

	*pppszFiles = (const char**)malloc(sizeof(const char*) * pHdr->dwFileCount);

	for (DWORD i = 0; i < pHdr->dwFileCount; ++i)
		(*pppszFiles)[i] = (const char*)((LPBYTE)pBuffer + pHdr->dwNameTableOffset + 4 + i * dwNextNameOffset);
}

static void DataFile_FindFile(void* pBuffer, const char* szName, void** ppFile)
{
	if (!ppFile)
		return;

	DWORD flags = DataFile_QueryFileIndex((DATHeader**)&pBuffer, szName);

	if (flags == -1)
	{
		*ppFile = NULL;
	}
	else
	{
		DWORD dwFileIndex = flags & 0xFFFFFFF; // pretty sure this and flags >> 28 and equivalent
		DATHeader* pHdr = (DATHeader*)((&pBuffer)[flags >> 28]); // get the highest 4 bytes (32 - 28) and use as the index

		if (pHdr->dwFileCount > dwFileIndex)
		{
			PDWORD pFileTable = (PDWORD)((LPBYTE)pHdr + pHdr->dwFileTableOffset);
			DWORD rva = pFileTable[dwFileIndex];
			*ppFile = rva ? MakePtr(void*, pHdr, rva) : NULL;
		}
		else
		{
			*ppFile = NULL;
		}
	}
}

static void CreateMaterial(
	const char* szName,
	const char* szShader,
	const char* szTechnique,
	const char** szTextureNames,
	CTextureDescription* pDescriptions,
	int nTextures,
	CSamplerParameterGroup* pParams,
	CMaterial** ppMaterial
)
{
	typedef BOOL(*CreateTextureFn)(__int64 rcx, CTargetTexture *, CTextureDescription *);
	CTargetTexture m_Textures[16];
	int TextureIds[16];

	ZeroMemory(m_Textures, sizeof(CTargetTexture[16]));

	if (!ppMaterial)
		return;

	if (nTextures > 15)
		nTextures = 15;

	for (int i = 0; i < nTextures; ++i)
	{
		TextureIds[i] = HashStringCRC32(szTextureNames[i], strlen(szTextureNames[i]));

		m_Textures[i].m_vtbl = (void*)0x140E9FA38;

		if (!((CreateTextureFn)(0x140934FE0))(0, &m_Textures[i], &pDescriptions[i]))
			return;
	}

	*ppMaterial = (CMaterial*)malloc(sizeof(CMaterial)); // use AllocHeapMem from game HeapInfo is in rdx MODELTHING*

	if (!*ppMaterial)
		return;

	ZeroMemory(*ppMaterial, sizeof(CMaterial));

	(*ppMaterial)->m_szName = szName;
	(*ppMaterial)->m_szShaderName = szShader;
	(*ppMaterial)->m_szTechniqueName = szTechnique;
	(*ppMaterial)->m_nParameters = 0;

	for (int i = 0; i < nTextures; ++i)
	{
		int nTexture = ((int(*)(void*, const char*))(0x143F4D0A0))(0, szTextureNames[i]); // CModelAnalyzer::FindTextureIndexByName 

		if (nTexture > 15)
			break;

		(*ppMaterial)->m_TextureIds[nTexture] = TextureIds[i];
	}

	(*ppMaterial)->m_pParameterGroups = pParams;
}

static CMaterial* LoadMaterial(const char* szFile)
{
	CMaterial* pMetal = NULL;
	CTextureDescription tex_desc;
	const char* szName = "g_AlbedoMap";

	ZeroMemory(&tex_desc, sizeof(CTextureDescription));

	HANDLE hFile = CreateFileA(szFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	DWORD dwFileSize = GetFileSize(hFile, NULL);
	HANDLE hMapping = CreateFileMappingA(hFile, NULL, PAGE_READONLY, 0, dwFileSize, NULL);
	DDS_HEADER_WITH_MAGIC* pHdr = (DDS_HEADER_WITH_MAGIC*)MapViewOfFile(hMapping, FILE_MAP_READ, 0, 0, dwFileSize);
	tex_desc.m_pDDS = pHdr;
	tex_desc.m_uTextureSize = dwFileSize;
	tex_desc.dword18 = 2;

	CreateMaterial("Metal", "Sign_PS_XXXXX"/*"PBS00_XXXXX"*/, "Default", &szName, &tex_desc, 1, 0, &pMetal);

	CloseHandle(hFile);
	return pMetal;
}

static HRESULT MyPreloadModel(int objectId)
{
	char szObjectName[16];
	char szFilename[64];
	ObjectIdConvert* pConvert;
	ObjReadSystem::Work* pWork;
	ObjReadSystem::Work::Desc* pDesc;
	HeapThing lmao;

	if (ObjectIdToObjectName(szObjectName, ARRAYSIZE(szObjectName), objectId, &pConvert))
	{
		QueryHeap(&lmao, objectId, 1);
		snprintf(szFilename, ARRAYSIZE(szFilename), "%s\\%s%04x%s", pConvert->m_szPrefix, pConvert->m_szPrefix, HIWORD(objectId), ".dtt");
		pWork = GetWork(objectId);
		pDesc = PreloadFile(0, PRELOAD_TYPE_MODELDATA, szFilename, lmao.m_pHeap, lmao.m_bFlag, pWork);

		if (pDesc)
		{
			_InterlockedCompareExchange((volatile LONG*)&pDesc->m_0x20, 1, 0);
			_InterlockedIncrement((volatile LONG*)&pDesc->m_0x20);
			pWork->m_objectid = pDesc->m_objectId;
		}

		snprintf(szFilename, ARRAYSIZE(szFilename), "%s\\%s%04x%s", pConvert->m_szPrefix, pConvert->m_szPrefix, HIWORD(objectId), ".dat");
		pDesc = PreloadFile(0, PRELOAD_TYPE_MODELDATA, szFilename, lmao.m_pHeap, lmao.m_bFlag, pWork);

		if (pDesc)
		{
			_InterlockedCompareExchange((volatile LONG*)&pDesc->m_0x20, 1, 0);
			_InterlockedIncrement((volatile LONG*)&pDesc->m_0x20);
			pWork->m_objectid = pDesc->m_objectId;
		}
		return S_OK;
	}
	return E_FAIL;
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
	SYSTEMTIME time;

	if (!IS_SAVE_SLOTDATA(nSlot))
		return ERROR_INVALID_PARAMETER;

	if (!SHGetSpecialFolderPathW(NULL, szPath, CSIDL_MYDOCUMENTS, FALSE))
		return ERROR_PATH_NOT_FOUND;

	int nCharsWritten = swprintf_s(szSavePath, MAX_PATH, L"%s\\My Games\\NieR_Automata\\SlotData_%d.dat", szPath, nSlot);

	if (nCharsWritten == -1)
		return ERROR_NOT_ENOUGH_MEMORY;
	
	GetSystemTime(&time);

	nCharsWritten = swprintf_s(szBackupPath, MAX_PATH, L"%s\\My Games\\NieR_Automata\\SlotData_%d_%4d%02d%02d_%02d%02d%02d.dat.bak", szPath, nSlot, 
		time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond);

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

	if (*pphHeaps)
		return ERROR_NOT_ENOUGH_MEMORY;

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

	UNREFERENCED_PARAMETER(phHeaps);
	UNREFERENCED_PARAMETER(dwHeaps);
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

/*
Rebuilt from the debug build
*/
static char* CRIGetBuffer(const char* szFormat, unsigned int arg_ptr_high, unsigned int arg_ptr_low)
{
	BOOL IsStringVar[3];

	const char* szFmt = szFormat;
	QWORD v4 = *(QWORD *)(arg_ptr_low | ((unsigned __int64)arg_ptr_high << 32));	// 1st arg
	QWORD v5 = *(QWORD *)((arg_ptr_low | ((unsigned __int64)arg_ptr_high << 32)) + 8); //2nd arg
	SIZE_T length = strlen(szFormat);
	SIZE_T j = 0;
	DWORD i = 0;

	if (length)
	{
		do
		{
			if (szFmt[j] == '%')
			{
				if (szFmt[j + 1] == 's')
					IsStringVar[i] = TRUE;
				
				if (++i == 3)
					break;
			}
		} while (++j < length);

		// there is no format params just return the format string
		if (i > 2)
			return (char *)szFmt;
	}

	if (IsStringVar[0])
	{
		if (IsStringVar[0] != TRUE || IsStringVar[1])
			snprintf((char*)0x14270F500, 512, szFmt, v4, v5);
		else
			snprintf((char*)0x14270F500, 512, szFmt, v4, (unsigned int)v5);
	}
	else if (IsStringVar[1])
		snprintf((char*)0x14270F500, 512, szFmt, (unsigned int)v4, v5);
	else
		snprintf((char*)0x14270F500, 512, szFmt, (unsigned int)v4, (unsigned int)v5);

	return (char*)0x14270F500;
}

static void CRILogCallbackWinConsole(const char* szFormat, unsigned int callback_arg_ptr_high, unsigned int callback_arg_ptr_low, void* a4)
{
	QWORD stack_ptr = (((QWORD)callback_arg_ptr_high << 32) | callback_arg_ptr_low) + 0x28;
	QWORD caller_return_address = *(QWORD*)stack_ptr;
	printf("[%llx]: %s\n", caller_return_address, CRIGetBuffer(szFormat, callback_arg_ptr_high, callback_arg_ptr_low));
}

static void CRILogCallbackConsole(const char* szFormat, unsigned int callback_arg_ptr_high, unsigned int callback_arg_ptr_low, void* a4)
{
	if (Vars.Misc.bConsoleShowGameErrors)
		g_pConsole->Warn("%s\n", CRIGetBuffer(szFormat, callback_arg_ptr_high, callback_arg_ptr_low));
}

static void CRILogCallbackWinConsoleVerbose(const char* szFormat, unsigned int callback_arg_ptr_high, unsigned int callback_arg_ptr_low, void* a4)
{
	QWORD stack[100];
	SYMBOL_INFO_PACKAGE symbol;

	HANDLE hProc = GetCurrentProcess();

	SymInitialize(hProc, NULL, TRUE);
	
	WORD nFrames = CaptureStackBackTrace(0, ARRAYSIZE(stack), (PVOID*)stack, NULL);
	symbol.si.MaxNameLen = MAX_SYM_NAME;
	symbol.si.SizeOfStruct = sizeof(SYMBOL_INFO);

	for (WORD i = 0; i < nFrames; i++)
	{
		if (!SymFromAddr(hProc, stack[i], 0, &symbol.si))
			printf("%i: RSP: %llx\n", nFrames - i - 1, stack[i]);
		else
			printf("%i: %s - %llx\n", nFrames - i - 1, symbol.si.Name, symbol.si.Address);
	}

	printf("%s\n", CRIGetBuffer(szFormat, callback_arg_ptr_high, callback_arg_ptr_low));
}