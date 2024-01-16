#pragma once
#include <dbghelp.h>
#include <d3dcompiler.h>
#include <wincodec.h>
#include <Shlobj.h>
#include <ntstatus.h>

#include "Globals.h"
#include "Matrix4x4.h"

#define STACK_TIMER(Name) StackTimer Name(__FUNCTION__)
#define CS_LOCK(pCriticalSection) StackReadWriteLock _CRT_CONCATENATE(__FUNCTION__, Lock)(pCriticalSection)
#define CS_LOCK_SPINCOUNT(pCriticalSection, Spincount) StackReadWriteLock _CRT_CONCATENATE(__FUNCTION__, Lock)(pCriticalSection, (DWORD)(Spincount))

typedef struct StackTimer
{
	StackTimer(const char* szName)
		: m_szName(szName), m_start(clock()), m_end(0), m_duration(0.0)
	{

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

struct StackReadWriteLock
{
	StackReadWriteLock(LPCRITICAL_SECTION pCriticalSection)
	{
		m_pCriticalSection = pCriticalSection;
		InitializeCriticalSection(m_pCriticalSection);
		m_bCriticalSectionInitialized = TRUE;
		EnterCriticalSection(m_pCriticalSection);
	}

	StackReadWriteLock(LPCRITICAL_SECTION pCriticalSection, DWORD dwSpinCount)
		: m_bCriticalSectionInitialized(FALSE)
	{
		m_pCriticalSection = pCriticalSection;
		/*
		* Windows Server 2003 and Windows XP:  If the function succeeds, the return value is nonzero.
		* If the function fails, the return value is zero (0). To get extended error information, call GetLastError.
		* Starting with Windows Vista, the InitializeCriticalSectionAndSpinCount function always succeeds, even in low memory situations.
		*/
		if (!InitializeCriticalSectionAndSpinCount(m_pCriticalSection, dwSpinCount))
			return;

		m_bCriticalSectionInitialized = TRUE;
		EnterCriticalSection(m_pCriticalSection);
	}

	~StackReadWriteLock()
	{
		if (m_bCriticalSectionInitialized)
		{
			LeaveCriticalSection(m_pCriticalSection);
			DeleteCriticalSection(m_pCriticalSection);
			m_bCriticalSectionInitialized = FALSE;
		}
	}

	// Delete Copy Constructor
	StackReadWriteLock(const StackReadWriteLock&) = delete;

	StackReadWriteLock& operator=(const StackReadWriteLock&) = delete;

private:
	LPCRITICAL_SECTION m_pCriticalSection;
	BOOL m_bCriticalSectionInitialized;
};

enum NierVersion
{
	NIERVER_UNKNOWN,
	NIERVER_100,
	NIERVER_101,
	NIERVER_102,
	NIERVER_102_UNPACKED,
	NIERVER_102_WOLF_EXTENDED_HEAPS,
	NIERVER_WINSTORE,
	NIERVER_DEBUG,
	NIERVER_MAX
};

static std::ostream& operator<<(std::ostream& os, const NierVersion& v)
{
	switch (v)
	{
	case NIERVER_100:
		os << "NieR:Automata (v1.00)";
		break;
	case NIERVER_101:
		os << "NieR:Automata (v1.01)";
		break;
	case NIERVER_102:
		os << "NieR:Automata (v1.02)";
		break;
	case NIERVER_102_UNPACKED:
		os << "NieR:Automata (v1.02 Unpacked)";
		break;
	case NIERVER_102_WOLF_EXTENDED_HEAPS:
		os << "NieR:Automata (v1.02 Extended Heaps)";
		break;
	case NIERVER_WINSTORE:
		os << "NieR:Automata (Winstore)";
		break;
	case NIERVER_DEBUG:
		os << "NieR:Automata (Debug)";
		break;
	case NIERVER_UNKNOWN:
	default:
		os << "NieR:Automata (Unknown Version)";
		break;
	}
	return os;
}

struct NierVersionInfo
{
	NierVersionInfo(NierVersion Version)
		: m_pHash(NULL), m_uHashSize(0), m_Version(Version)
	{
		Init();
	}

	NierVersionInfo(NierVersion Version, const char* Hash)
		: m_pHash((BYTE*)Hash), m_uHashSize(strlen(Hash)), m_Version(Version)
	{
		Init();
	}

	void Init(void)
	{
		switch (m_Version)
		{
		case NIERVER_100:
			m_szVersion = TEXT("NieR:Automata (v1.00)");
			break;
		case NIERVER_101:
			m_szVersion = TEXT("NieR:Automata (v1.01)");
			break;
		case NIERVER_102:
			m_szVersion = TEXT("NieR:Automata (v1.02)");
			break;
		case NIERVER_102_UNPACKED:
			m_szVersion = TEXT("NieR:Automata (v1.02 Unpacked)");
			break;
		case NIERVER_102_WOLF_EXTENDED_HEAPS:
			m_szVersion = TEXT("NieR:Automata (v1.02 Extended Heaps)");
			break;
		case NIERVER_WINSTORE:
			m_szVersion = TEXT("NieR:Automata (Winstore)");
			break;
		case NIERVER_DEBUG:
			m_szVersion = TEXT("NieR:Automata (Debug)");
			break;
		case NIERVER_UNKNOWN:
		default:
			m_szVersion = TEXT("NieR:Automata (Unknown Version)");
			break;
		}
	}

	bool operator==(NierVersionInfo& other) const
	{
		return !memcmp(this->m_pHash, other.m_pHash, m_uHashSize);
	}

	bool operator==(const BYTE* pHash) const
	{
		return !memcmp(this->m_pHash, pHash, m_uHashSize);
	}

	BYTE* m_pHash;
	ULONG_PTR m_uHashSize;
	NierVersion m_Version;
	TCHAR* m_szVersion;
};

static NTSTATUS QueryNierBinaryHash(NierVersionInfo& Version)
{
	//	TCHAR szFileName[MAX_PATH];

	// Set the inital status to success
	NTSTATUS Status = STATUS_SUCCESS;

	// Retrieve the size of the nier bin ary file path
	uint32_t uFileNameSize = MAX_PATH;//GetModuleFileName(NULL, NULL, 0);

	// Allocate the correct amount of memory
	TCHAR* szFileName = new TCHAR[(uFileNameSize + 1) * sizeof(TCHAR)];

	// Query the nier binary file path
	GetModuleFileName(NULL, szFileName, uFileNameSize + 1);

	// Query a file handle to the nier binary 
	HANDLE hFile = CreateFile(szFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	// Free the nier binary file path
	delete[] szFileName;

	// If the handle is invalid, bail
	if (hFile == INVALID_HANDLE_VALUE)
		return STATUS_INVALID_HANDLE;

	// Query the file size of the nier binary
	DWORD dwFileSize = GetFileSize(hFile, NULL);
	DWORD dwBytesRead;

	// Allocate the memory for the nier binary
	BYTE* pBinary = new BYTE[dwFileSize];

	// Read the binary file
	ReadFile(hFile, pBinary, dwFileSize, &dwBytesRead, NULL);

	// Close the file handle to the nier binary
	CloseHandle(hFile);

	// Bail out if there was a partial read
	if (dwFileSize != dwBytesRead)
	{
		delete[] pBinary;
		return ERROR_CLUSTER_PARTIAL_READ;
	}

	ULONG uHashLengthSize;

	// Query the hash length
	Status = BCryptGetProperty(BCRYPT_SHA256_ALG_HANDLE, BCRYPT_HASH_LENGTH, (PUCHAR)&Version.m_uHashSize, sizeof(ULONG), &uHashLengthSize, 0);

	if (SUCCEEDED(Status))
	{
		// Allocate the memory for hash
		Version.m_pHash = new BYTE[Version.m_uHashSize];

		// Create the hash
		Status = BCryptHash(BCRYPT_SHA256_ALG_HANDLE, NULL, 0, pBinary, dwFileSize, Version.m_pHash, Version.m_uHashSize);
	}

	// Free the binary from memory
	delete[] pBinary;

	return Status;
}

static NTSTATUS QueryNierBinaryHash(BYTE*& pHash, ULONG& uHashSize)
{
	//	TCHAR szFileName[MAX_PATH];

	// Set the inital status to success
	NTSTATUS Status = STATUS_SUCCESS;

	// Retrive the size of the nier binary file path
	uint32_t uFileNameSize = MAX_PATH;//GetModuleFileName(NULL, NULL, 0);

	// Allocate the correct amount of memory
	TCHAR* szFileName = new TCHAR[(uFileNameSize + 1) * sizeof(TCHAR)];

	// Query the nier binary file path
	GetModuleFileName(NULL, szFileName, uFileNameSize + 1);

	// Query a file handle to the nier binary 
	HANDLE hFile = CreateFile(szFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	// Free the nier binary file path
	delete[] szFileName;

	// If the handle is invalid, bail
	if (hFile == INVALID_HANDLE_VALUE)
		return STATUS_INVALID_HANDLE;

	// Query the file size of the nier binary
	DWORD dwFileSize = GetFileSize(hFile, NULL);
	DWORD dwBytesRead;

	// Allocate the memory for the nier binary
	BYTE* pBinary = new BYTE[dwFileSize];

	// Read the binary file
	ReadFile(hFile, pBinary, dwFileSize, &dwBytesRead, NULL);

	// Close the file handle to the nier binary
	CloseHandle(hFile);

	// Bail out if there was a partial read
	if (dwFileSize != dwBytesRead)
	{
		delete[] pBinary;
		return ERROR_CLUSTER_PARTIAL_READ;
	}

	ULONG uHashLengthSize;

	// Query the hash length
	Status = BCryptGetProperty(BCRYPT_SHA256_ALG_HANDLE, BCRYPT_HASH_LENGTH, (PUCHAR)&uHashSize, sizeof(ULONG), &uHashLengthSize, 0);

	if (SUCCEEDED(Status))
	{
		// Allocate the memory for hash
		pHash = new BYTE[uHashSize];

		// Create the hash
		Status = BCryptHash(BCRYPT_SHA256_ALG_HANDLE, NULL, 0, pBinary, dwFileSize, pHash, uHashSize);
	}

	// Free the binary from memory
	delete[] pBinary;

	return Status;
}

static NierVersionInfo* QueryNierBinaryVersion(void)
{
	static NierVersionInfo Versions[] =
	{
		{ NIERVER_101, "\xA0\x1A\xC5\x13\x2E\x10\x92\x52\xD6\xD9\xA4\xCB\xF9\x74\x61\x4D\xEC\xFB\xE3\x23\x71\x3C\x1F\xBF\x5B\xC2\x48\xF0\x12\x61\x77\x3F" },
		{ NIERVER_102, "\x51\x71\xBE\xD0\x9E\x6F\xEC\x7B\x21\xBF\x0E\xA4\x79\xDB\xD2\xE1\xB2\x28\x69\x5C\x67\xD1\xF0\xB4\x78\x54\x9A\x9B\xE2\xF5\x72\x6A" },
		{ NIERVER_102_UNPACKED, "\x5F\x97\x20\xD8\xC7\x7C\xD5\x97\x8E\xFE\x49\x88\x89\x3A\xF8\xFD\x99\x9F\x90\xA4\x76\xA8\xDE\xEB\xB3\x91\x26\x94\xF6\x18\xDC\x43" },
		{ NIERVER_102_WOLF_EXTENDED_HEAPS, "\xC9\x90\x4A\x39\xE4\x48\xD6\xCB\x3C\x98\xC2\x8A\x90\x15\x9C\xF9\x31\x47\x53\xB0\xCF\xEA\x5C\xEB\x4E\x76\xA1\x2D\x33\x08\xA3\x55" },
		{ NIERVER_WINSTORE, "\x3D\xDE\x56\x6C\xEA\x3E\x3B\xC1\x5E\x45\x92\x66\x02\xFB\x4F\x24\xD4\x8F\x77\xDF\x8A\x7B\xC5\x50\xA5\xB2\xDC\xAE\xCC\xCF\x09\x48" },
		{ NIERVER_DEBUG, "\xE9\xEF\x66\x01\xEB\x40\xEB\x0A\x6D\x3F\x30\xA6\x63\x95\x43\xEC\x2F\x81\x71\xC2\x6A\x3D\xE8\xB2\xB1\x30\x39\xEE\xBE\x3B\xC8\x1C" },
		{ NIERVER_UNKNOWN }
	};
	BYTE* pHash;
	ULONG uHashSize;

	// Set the version the first entry
	NierVersionInfo* pVersion = Versions;

	// Query the nier binary hash, if it fails return NIERVER_UNKNOWN
	if (FAILED(QueryNierBinaryHash(pHash, uHashSize)))
		return &Versions[ARRAYSIZE(Versions) - 1];

	// Traverse the array comparing the loaded binary hash to the known hashes
	for (; pVersion->m_Version != NIERVER_UNKNOWN; ++pVersion)
		if (*pVersion == pHash)
			break;

	// Free the hash memory
	delete[] pHash;

	return pVersion;
}

struct CCommon : public IUnknown
{
	//IUnknownVtbl* m_pVtbl;	//0x0000
	CCommon* m_pSelf;			//0x0008
	IID* pGuid;					//0x0010
};

// Size of struct 0x1A0 from _Common_NewRiid_
struct CDirectInputDevice8A : public IDirectInputDevice8A
{
	//IDirectInputDeviceAVtbl* m_pVtbl;	// 0x0000
	CDirectInputDevice8A* m_pDerived;	// 0x0008
	IID* pGuid;							// 0x0010
	char pad0018[0x12C];				// 0x0018
	volatile INT m_iReferenceCount;		// 0x0148
	DWORD m_dwThreadId;					// 0x014C
	CRITICAL_SECTION m_CriticalSection;	// 0x0150
	char pad0178[40];					// 0x0178
};
VALIDATE_OFFSET(CDirectInputDevice8A, m_pDerived, 0x0008);
VALIDATE_OFFSET(CDirectInputDevice8A, m_CriticalSection, 0x0150);
VALIDATE_SIZE(CDirectInputDevice8A, 0x01A0);

// TODO: finish dinput8.dll & gamerendereroverlay64.dll stuff
// Thanks valve and your gamerendereroverlay64.dll!HookedDirectInput8Create
#pragma region gamerendereroverlay64

// Size of struct 0x18 (24) bytes
struct CWrapDirectInput8
{
	void* m_pVtbl;						 //0x0000
	union {
		IDirectInput8A* m_pDirectInputA; //0x0008
		IDirectInput8W* m_pDirectInputW; //0x0008
	};
	bool m_bWideVersion;				 //0x0010
	
};
VALIDATE_SIZE(CWrapDirectInput8, 0x18);

// imaginary base class 
struct CWrapDirectInputDevice8
{
	void* m_pVtbl;							//0x0000
	union {
		IDirectInputDevice8A* m_pDeviceA;	//0x0008
		IDirectInputDevice8W* m_pDeviceW;	//0x0008
	};
};

// Size of struct 0x138 (312) bytes
struct CWrapKeyboardDevice8 //: public IWrapMouseDevice8
{
	void* m_pVtbl;							//0x0000
	union {
		IDirectInputDevice8A* m_pDeviceA;	//0x0008
		IDirectInputDevice8W* m_pDeviceW;	//0x0008
	};
	char pad0010[0x128];					//0x0010
};
VALIDATE_SIZE(CWrapKeyboardDevice8, 0x138);

// Size of struct 0x48 (72) bytes
struct CWrapMouseDevice8 //: public IWrapMouseDevice8
{
	void* m_pVtbl;							//0x0000
	union {
		IDirectInputDevice8A* m_pDeviceA;	//0x0008
		IDirectInputDevice8W* m_pDeviceW;	//0x0008
	};
	char pad0010[56];						//0x0010
};
VALIDATE_SIZE(CWrapMouseDevice8, 0x48);

#pragma endregion gamerendereroverlay64

static void IDirectInputDevice_Lock(IDirectInputDevice8A* pDevice)
{
	static HMODULE s_hDirectInput8 = GetModuleHandle(TEXT("dinput8.dll"));
	HMODULE hOwnerModule = NULL;

	if (s_hDirectInput8 != RtlPcToFileHeader(*(void**)pDevice, (PVOID*)&hOwnerModule))
	{
		//pDevice = CONTAINING_RECORD(pDevice, CWrapDirectInputDevice8, m_pDeviceA);
		pDevice = ((CWrapDirectInputDevice8*)pDevice)->m_pDeviceA;
	}

	CDirectInputDevice8A* pImpl = CONTAINING_RECORD(pDevice, CDirectInputDevice8A, m_pDerived);

	EnterCriticalSection(&pImpl->m_CriticalSection);
}

static void IDirectInputDevice_Unlock(IDirectInputDevice8A* pDevice)
{
	static HMODULE s_hDirectInput8 = GetModuleHandle(TEXT("dinput8.dll"));
	HMODULE hOwnerModule = NULL;

	if (s_hDirectInput8 != RtlPcToFileHeader(*(void**)pDevice, (PVOID*)&hOwnerModule))
	{
		//pDevice = CONTAINING_RECORD(pDevice, CWrapDirectInputDevice8, m_pDeviceA);
		pDevice = ((CWrapDirectInputDevice8*)pDevice)->m_pDeviceA;
	}

	CDirectInputDevice8A* pImpl = CONTAINING_RECORD(pDevice, CDirectInputDevice8A, m_pDerived);

	LeaveCriticalSection(&pImpl->m_CriticalSection);
}

// https://stackoverflow.com/questions/46182845/field-of-view-aspect-ratio-view-matrix-from-projection-matrix-hmd-ost-calib
static float GetFovFromProjectionMatrix(void)
{
	return 2.0f * atan(1.0f / g_pCamera->m_pInstance->m_ViewProjection[1][1]);
}

/*
	Doesn't seem to be 100% accurate.
	I tired adding the fov math into it but no success
	all the examples use a different method than me.
	Either that or CGameCamera::m_flFov isn't the fov value and I fucked up
	https://guidedhacking.com/threads/world2screen-direct3d-and-opengl-worldtoscreen-functions.8044/
	https://www.cs.toronto.edu/~jepson/csc420/notes/imageProjection.pdf
	https://www.scratchapixel.com/lessons/3d-basic-rendering/computing-pixel-coordinates-of-3d-point/mathematics-computing-2d-coordinates-of-3d-points
	https://www.cse.unr.edu/~bebis/CS791E/Notes/CameraParameters.pdf
	https://github.com/VSES/SourceEngine2007/blob/master/se2007/engine/gl_rmain.cpp

	Source Eng:
	pScreen->x = 0.5f * ( pScreen->x + 1.0f ) * CurrentView().width + CurrentView().x;
	pScreen->y = 0.5f * ( pScreen->y + 1.0f ) * CurrentView().height + CurrentView().y;
	aka
	pScreen->x = 0.5f * pScreen->x * CurrentView().width + 0.5f * CurrenView().width + CurrentView.x;
	pScreen->y = 0.5f * pScreen->y * CurrentView().height + 0.5f * CurrenView().height + CurrentView.y;
*/
static bool WorldToScreen(const Vector3& vIn, Vector2& vOut)
{
	Vector3 vTransform;

	VMatrix& vMatrix = g_pCamera->m_pInstance->m_ViewProjection;

	float w = vMatrix[0][3] * vIn[0] + vMatrix[1][3] * vIn[1] + vMatrix[2][3] * vIn[2] + vMatrix[3][3];

	if (w < 0.001f)
		return false;

	// transform by viewmatrix
	vMatrix.Transform(vIn, vTransform);

	// multiply by inverse w (vTransform /= w)
	vTransform *= 1.0f / w;

	int width = g_pGraphicDevice->m_iScreenWidth;
	int height = g_pGraphicDevice->m_iScreenHeight;

	float x = width / 2.0f;
	float y = height / 2.0f;
	//float flFocalLengthY = 1.0f / tanf(g_pCamera->m_flFov * 0.5f);
	//float flFocalLengthX = flFocalLengthX / (width / height);

	//vOut.x = x * (1.f + (vTransform.x / tanf(g_pCamera->m_flFov * 0.5f) / vTransform.z));
	//vOut.y = y * (1.f - (vTransform.y / tanf(g_pCamera->m_flFov * 0.5f) / vTransform.z));

	//vOut.x = x + vTransform.x * flFocalLengthX * width / vTransform.z;
	//vOut.y = y - vTransform.y * flFocalLengthY * height / vTransform.z;

	vOut.x = x + 0.5f * vTransform.x * (float)width + 0.5f;
	vOut.y = y - 0.5f * vTransform.y * (float)height + 0.5f;

	return true;
}

static inline unsigned short EntityHandleToListIndex(const EntityHandle hEntity)
{
	return (unsigned short)((hEntity & 0x00FFFF00) >> 8); // (unsigned short)(handle >> 8)
}

static inline EntityHandle GenerateEntityHandle(const CEntityList* pList, const unsigned short index)
{
	return (index | (pList->m_dwShift << 16)) << 8;
}

// NOTE: You should use these functions two get entity's by their handle from now on.
// It is the proper way to do it, since it doesn't do any filtering.
static CBehaviorAppBase* GetEntityByHandle(const CEntityList* pList, const EntityHandle hEntity)
{
	int idx = EntityHandleToListIndex(hEntity);

	// Check if the index is valid and the top 24-bits of the handles match
	if (idx > pList->m_dwItems || idx < 0 || (hEntity ^ pList->m_pItems[idx].first) & 0xFFFFFF00)
		return NULL;

	CEntityInfo* pEntityInfo = pList->m_pItems[idx].second;

	if (!pEntityInfo || pEntityInfo->m_Flags & 3)
		return NULL;

	return pEntityInfo->m_pEntity;
}

// global func to get all the entity's from handle
// actual standalone function doesn't exist in the binary no more
// NOTE: You should use these functions two get entity's by their handle from now on.
// It is the proper way to do it, since it doesn't do any filtering.
static CBehaviorAppBase* GetEntityFromHandleGlobal(EntityHandle* phEntity)
{
	CEntityInfo* pEntityInfo = GetEntityInfoFromHandle(phEntity);
	return pEntityInfo ? pEntityInfo->m_pParent : NULL;
}

// Rebuilt from binary with added functionality
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

// FIXME: This is disgusting 
static DWORD DataFile_QueryFileIndex(DATHeader** ppBuffer, const char* szFileName)
{
	const char* szName; // r12
	LPBYTE* pHeaders; // r15
	unsigned int v4; // esi
	DATHeader* pHdr; // r8
	DWORD dwOffset; // r9
	DWORD dwFileCount; // ebp
	DWORD dwNextNameOffset; // r14
	const char* szCurrentFileName; // rdi
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

	if (*pppszFiles)
	{
		for (DWORD i = 0; i < pHdr->dwFileCount; ++i)
			(*pppszFiles)[i] = (const char*)((LPBYTE)pBuffer + pHdr->dwNameTableOffset + 4 + i * dwNextNameOffset);
	}
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

// Rebuilt from the binary for the reason that it was inlined which
// will complicate things in the hook otherwise
static void CSaveDataDevice_DeleteSaveData(CSaveDataDevice* pSavedata)
{
	char szFilename[MAX_PATH];

	switch (pSavedata->dwStatus)
	{
	case 0:
		if (((BOOL(*)(CSaveDataDevice*, char*))((LPBYTE)GetModuleHandle(NULL) + 0x282BD0))(pSavedata, szFilename)) // sub_140282BD0
		{
			if (DeleteFileA(szFilename))
			{
				pSavedata->pSaveSlots[pSavedata->nSlot].dwAccountId = 0;
				pSavedata->pSaveSlots[pSavedata->nSlot].bInUse = FALSE;

				if (pSavedata->hFile != INVALID_HANDLE_VALUE)
				{
					CloseHandle(pSavedata->hFile);
					pSavedata->hFile = INVALID_HANDLE_VALUE;
				}

				*(&pSavedata->nMaxSlot + 1) = 0;
				pSavedata->qwFlags = 0i64;
			}
			else
			{
				pSavedata->dwStatus = 1;
			}
		}
		break;
	case 1:
		if (pSavedata->hFile != INVALID_HANDLE_VALUE)
		{
			CloseHandle(pSavedata->hFile);
			pSavedata->hFile = INVALID_HANDLE_VALUE;
		}

		*(&pSavedata->nMaxSlot + 1) = 3;
		pSavedata->qwFlags = 0i64;
		break;
	case 2:

		if (pSavedata->qwUnk0xC8)
		{
			if (!*(DWORD*)(pSavedata->qwUnk0xC8 + 8))
				break;
			*(DWORD*)(pSavedata->qwUnk0xC8 + 12) = 1;
		}
		pSavedata->qwUnk0xC8 = 0i64;
		break;
	default:
		pSavedata->dwStatus = 1;
		break;
	}
}

static CMeshPart* GetModelMesh(CModelWork* pWork, const char* szMesh)
{
	for (int i = 0; i < pWork->m_nMeshes; ++i)
		if (!strcmp(pWork->m_pMeshes[i].m_szMeshName, szMesh))
			return &pWork->m_pMeshes[i];

	return NULL;
}

static int GetModelMeshIndex(const CModelWork* pWork, const char* szMesh)
{
	for (int i = 0; i < pWork->m_nMeshes; ++i)
		if (!strcmp(pWork->m_pMeshes[i].m_szMeshName, szMesh))
			return i;

	return -1;
}

// Rebuilt from binary
static short GetBoneIndex(const CModelData* pModelData, short id)
{
	short* pTable;
	short iBoneIndex, t1, t2;

	pTable = pModelData->m_pBoneIndexTranslationTable2;

	if (pTable)
	{
		t1 = pTable[(id >> 8) & 0xF];	// id / 256  clamped to 0-15

		if (t1 != -1)
		{
			t2 = pTable[t1 + ((id >> 4) & 0xF)]; // id

			if (t2 != -1)
			{
				iBoneIndex = pTable[t2 + (id & 0xF)];

				if (iBoneIndex != 0xFFF) // 4095
					return iBoneIndex;
			}
		}
	}
	return -1;
}

static short GetBoneId(short index)
{
}

static void SwapTexture(unsigned int srcid, CTexture* pReplace)
{
	CTextureResource* pRes = TextureResourceManager_FindResource(srcid);

	if (pRes)
		pRes->m_pTexture = pReplace;
}

static HRESULT CreateTextureEx(const wchar_t* szFile, CTextureDescription& Desc)
{
	IWICImagingFactory* pFactory;
	IWICFormatConverter* pConverter;
	IWICBitmapDecoder* pDecoder;
	IWICBitmapFrameDecode* pFrame;
	WICPixelFormatGUID Format;

	unsigned int w, h, uFrames, * rgba;

	HRESULT hr = CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_IWICImagingFactory, (void**)&pFactory);

	if (FAILED(hr))
		return hr;

	hr = pFactory->CreateDecoderFromFilename(szFile, NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pDecoder);

	if (FAILED(hr))
		return hr;

	hr = pDecoder->GetFrameCount(&uFrames);

	if (FAILED(hr))
		return hr;

	hr = pDecoder->GetFrame(0, &pFrame);

	if (FAILED(hr))
		return hr;

	hr = pFrame->GetPixelFormat(&Format);

	if (FAILED(hr))
		return hr;

	hr = pFrame->GetSize(&w, &h);


	return hr;
}

// FIXME: This is for sure broken on new version. Adapt to use the game's allocation routines to avoid crashes on freeing resources
static CTargetTexture* CreateTextureR(const char* szFile, CTextureDescription& Desc)
{
	CTargetTexture* pTexture = NULL;

	ZeroMemory(&Desc, sizeof(CTextureDescription));

	HANDLE hFile = CreateFileA(szFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile != INVALID_HANDLE_VALUE)
	{
		DWORD dwFileSize = GetFileSize(hFile, NULL);
		HANDLE hMapping = CreateFileMappingA(hFile, NULL, PAGE_READONLY, 0, dwFileSize, NULL);

		if (hMapping)
		{
			TextureFile* pFile = (TextureFile*)MapViewOfFile(hMapping, FILE_MAP_READ, 0, 0, dwFileSize);

			Desc.m_pDDS = pFile;
			Desc.m_uTextureSize = dwFileSize;
			Desc.dword18 = 2;

			pTexture = (CTargetTexture*)malloc(sizeof(CTargetTexture)); //AllocHeapMemoryFn or ReserveMemory

			if (pTexture)
			{
				ZeroMemory(pTexture, sizeof(CTargetTexture));
				// old sig: 33 D2 48 8D 05 ? ? ? ? 48 89 51 58
				// old: 0x140E9FA38 | base + 0xE9FA38
				// new sig: 48 89 58 50 48 8D 05 ? ? ? ? 
				// new: 0x140DB1DE8 | base + 0xDB1DE8
				pTexture->m_vtbl = (void*)FindPatternPtr(NULL, "48 89 58 50 48 8D 05 ? ? ? ?", 7);

				// old: 0x140934FE0
				// old sig: 48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8B 02 48 8B CA 49 8B F0 48 8B FA FF 50 08 48 83 3D 95 ? ? ? ?
				// new sig: 48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8B 44 24 ? 33 F6

				if (!CreateTexture(g_pGraphics, pTexture, &Desc))
					return NULL;
			}
		}
		CloseHandle(hFile);
	}

	return pTexture;
}

//Shitty delete doesn't check for references.
static void DeleteTexture(CTargetTexture* pTexture, CTextureDescription& desc)
{
	free(pTexture);
	UnmapViewOfFile(desc.m_pDDS);
}

//FIXME("this is for sure broken on new version. Probably should also be using nier's memory allocation routines")
static void CreateMaterial(
	const char* szName,
	const char* szShader,
	const char* szTechnique,
	const char** szTextureNames,
	CTextureDescription* pDescriptions,
	int nTextures,
	CSamplerParameterGroup* pParams, // float* pParams, int nParams,
	CMaterial** ppMaterial
)
{
	CTargetTexture Textures[16];
	int TextureIds[16];

	ZeroMemory(Textures, sizeof(CTargetTexture[16]));

	if (!ppMaterial)
		return;

	if (nTextures > 15)
		nTextures = 15;

	for (int i = 0; i < nTextures; ++i)
	{
		TextureIds[i] = HashStringCRC32(szTextureNames[i], strlen(szTextureNames[i]));
		// old sig: 33 D2 48 8D 05 ? ? ? ? 48 89 51 58
		// old: 0x140E9FA38 | base + 0xE9FA38
		// new sig: 48 89 58 50 48 8D 05 ? ? ? ? 
		// new: 0x140DB1DE8 | base + 0xDB1DE8
		Textures[i].m_vtbl = (void*)FindPatternPtr(NULL, "48 89 58 50 48 8D 05 ? ? ? ?", 7);

		// old sig: 48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8B 02 48 8B CA 49 8B F0 48 8B FA FF 50 08 48 83 3D 95 ? ? ? ?
		// new sig: 48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8B 44 24 ? 33 F6
		if (!CreateTexture(0, &Textures[i], &pDescriptions[i]))
			return;
	}

	*ppMaterial = (CMaterial*)malloc(sizeof(CMaterial)); // use AllocHeapMem from game HeapInfo is in rdx MODELTHING*

	if (!*ppMaterial)
		return;

	ZeroMemory(*ppMaterial, sizeof(CMaterial));

	(*ppMaterial)->m_szName = szName;
	(*ppMaterial)->m_szShaderName = szShader;
	(*ppMaterial)->m_szTechniqueName = szTechnique;
	(*ppMaterial)->m_nShaderParameters = 0;

	for (int i = 0; i < nTextures; ++i)
	{
		int nTexture = g_pModelAnalyzer->FindTextureIndexByName(szTextureNames[i]);

		if (nTexture > 15)
			break;

		(*ppMaterial)->m_TextureIds[nTexture] = TextureIds[i];
	}

	// for (int i = 0; i < nVariables; ++i)
	// {
	//	
	// }

	(*ppMaterial)->m_pParameterGroups = pParams;
}

static CMaterial* LoadMaterial(const char* szFile)
{
	CMaterial* pMetal = NULL;
	CTextureDescription tex_desc;
	const char* szName = "g_AlbedoMap";

	ZeroMemory(&tex_desc, sizeof(CTextureDescription));

	HANDLE hFile = CreateFileA(szFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile != INVALID_HANDLE_VALUE)
	{
		DWORD dwFileSize = GetFileSize(hFile, NULL);
		HANDLE hMapping = CreateFileMappingA(hFile, NULL, PAGE_READONLY, 0, dwFileSize, NULL);

		if (hMapping)
		{
			TextureFile* pHdr = (TextureFile*)MapViewOfFile(hMapping, FILE_MAP_READ, 0, 0, dwFileSize);
			tex_desc.m_pDDS = pHdr;
			tex_desc.m_uTextureSize = dwFileSize;
			tex_desc.dword18 = 2;

			CreateMaterial("Metal", "CNS00_XXXXX", "Default", &szName, &tex_desc, 1, 0, &pMetal);
		}
		CloseHandle(hFile);
	}
	return pMetal;
}

struct ShaderParamter { const char* m_szName; float m_flValue; };

static ShaderParamter CNS00_XXXXX_PARAMS[] = {
	{ "Binormal0", 0.0f },
	{ "Color0", 0.0 },
	{ "Normal", 0.0f },
	{ "Position", 0.0f },
	{ "Tangent0", 0.0f },
	{ "TexCoord0", 0.0f },
	{ "TexCoord1", 0.0f },
	{ "g_1BitMask", 0.0f },
	{ "g_AlbedoColor_X", 0.5f },
	{ "g_AlbedoColor_Y", 0.5f },
	{ "g_AlbedoColor_Z", 0.5f },
	{ "g_Decal", 0.0f },
	{ "g_Intensity", 1.0f },
	{ "g_InvalidFog", 0.0f },
	{ "g_IsSwatchRender", 0.0f },
	{ "g_LighIntensity0", 1.0f },
	{ "g_LighIntensity1", 1.0f },
	{ "g_LighIntensity2", 1.0f },
	{ "g_LightColor0_X", 1.0f },
	{ "g_LightColor0_Y", 1.0f },
	{ "g_LightColor0_Z", 1.0f },
	{ "g_LightColor1_X", 1.0f },
	{ "g_LightColor1_Y", 1.0f },
	{ "g_LightColor1_Z", 1.0f },
	{ "g_LightColor2_X", 1.0f },
	{ "g_LightColor2_Y", 1.0f },
	{ "g_LightColor2_Z", 1.0f },
	{ "g_Tile_X", 1.0f },
	{ "g_Tile_Y", 1.0f },
	{ "g_UseMultiplicationBlend", 1.0f },
	{ "g_UseSubtractionBlend", 0.0f },
	{ "g_UvAnimation_X", 0.0f },
	{ "g_UvAnimation_Y", 0.0f },
	{ "g_bAlbedoOverWrite", 0.0f }
};

static HRESULT MyPreloadModel(int objectId)
{
	char szObjectName[16];
	char szFilename[64];
	ObjectIdConvert* pConvert;
	CObjReadSystem::Work* pWork;
	CObjReadSystem::Work::Desc* pDesc;
	HeapAlloc_t lmao;

	if (ObjectIdToObjectName(szObjectName, ARRAYSIZE(szObjectName), objectId, &pConvert))
	{
		QueryHeap(&lmao, objectId, 1);
		snprintf(szFilename, ARRAYSIZE(szFilename), "%s\\%s%04x%s", pConvert->m_szPrefix, pConvert->m_szPrefix, HIWORD(objectId), ".dtt");
		pWork = GetWork(objectId);
		pDesc = PreloadFile(0, PRELOAD_TYPE_MODELDATA, szFilename, lmao.Pointer, lmao.Succeeded, pWork);

		if (pDesc)
		{
			_InterlockedCompareExchange((volatile LONG*)&pDesc->m_0x20, 1, 0);
			_InterlockedIncrement((volatile LONG*)&pDesc->m_0x20);
			pWork->m_ObjectId = pDesc->m_objectId;
		}

		snprintf(szFilename, ARRAYSIZE(szFilename), "%s\\%s%04x%s", pConvert->m_szPrefix, pConvert->m_szPrefix, HIWORD(objectId), ".dat");
		pDesc = PreloadFile(0, PRELOAD_TYPE_MODELDATA, szFilename, lmao.Pointer, lmao.Succeeded, pWork);

		if (pDesc)
		{
			_InterlockedCompareExchange((volatile LONG*)&pDesc->m_0x20, 1, 0);
			_InterlockedIncrement((volatile LONG*)&pDesc->m_0x20);
			pWork->m_ObjectId = pDesc->m_objectId;
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

	ID3DBlob* pBlob;
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
	MINIDUMP_TYPE Type = 
		MINIDUMP_TYPE(MiniDumpWithUnloadedModules |
			MiniDumpWithIndirectlyReferencedMemory | 
			MiniDumpWithFullMemoryInfo);
	
	BOOL Status = MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(),
		hFile, Type, pException ? &ExceptionInfo : NULL, NULL, NULL);

	CloseHandle(hFile);

	return Status;
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
		ERROR("Another component created a heap between calls therefore, the result will be inaccurate.");
		return ERROR_INSUFFICIENT_BUFFER;
	}

	if (pdwHeaps)
		*pdwHeaps = dwNumberOfHeaps;

	return ERROR_SUCCESS;
}

static BOOL EnumProcessHeapInfo(IN HANDLE* phHeaps, IN DWORD dwHeaps)
{
	//PROCESS_HEAP_ENTRY Entry;

	UNREFERENCED_PARAMETER(phHeaps);
	UNREFERENCED_PARAMETER(dwHeaps);

	//HeapWalk(p)

	return ERROR_SUCCESS;
}

static LONG UnhandledExceptionHandlerChild(EXCEPTION_POINTERS* pException)
{
	if (!WriteMiniDump(pException))
		ERROR("Failed to write dump file!\n");

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
	BOOL IsStringVar[3] = { FALSE, FALSE, FALSE };

	const char* szFmt = szFormat;
	QWORD v4 = *(QWORD*)(arg_ptr_low | ((unsigned __int64)arg_ptr_high << 32));	// 1st arg
	QWORD v5 = *(QWORD*)((arg_ptr_low | ((unsigned __int64)arg_ptr_high << 32)) + 8); //2nd arg
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
			return (char*)szFmt;
	}

	if (IsStringVar[0])
	{
		if (IsStringVar[0] != TRUE || IsStringVar[1])
			snprintf(g_szCRILogBuffer, 512, szFmt, v4, v5);
		else
			snprintf(g_szCRILogBuffer, 512, szFmt, v4, (unsigned int)v5);
	}
	else if (IsStringVar[1])
		snprintf(g_szCRILogBuffer, 512, szFmt, (unsigned int)v4, v5);
	else
		snprintf(g_szCRILogBuffer, 512, szFmt, (unsigned int)v4, (unsigned int)v5);

	return g_szCRILogBuffer;
}

static void CRILogCallbackWinConsole(const char* szFormat, unsigned int callback_arg_ptr_high, 
	unsigned int callback_arg_ptr_low, void* a4)
{
	QWORD stack_ptr = (((QWORD)callback_arg_ptr_high << 32) | callback_arg_ptr_low) + 0x28;
	QWORD caller_return_address = *(QWORD*)stack_ptr;
	printf("[%llx]: %s\n", caller_return_address, CRIGetBuffer(szFormat, callback_arg_ptr_high, callback_arg_ptr_low));
}

void CRILogCallbackConsole(const char* szFormat, unsigned int callback_arg_ptr_high,
	unsigned int callback_arg_ptr_low, void* a4);

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

/*
found in pl000d.wmb materials, trimmed out materials that use the same shader (have different values but same variables)

CLT00_XXXXX:
	Binormal0: 0.0
	Color0: 0.0
	Normal: 0.0
	Position: 0.0
	Tangent0: 0.0
	TexCoord0: 0.0
	TexCoord1: 0.0
	g_1BitMask: 0.0
	g_AlbedoColor_X: 0.5
	g_AlbedoColor_Y: 0.5
	g_AlbedoColor_Z: 0.5
	g_AmbientLightIntensity: 1.0
	g_AnisoLightMode: 0.0
	g_AnisoLightMode2: 0.0
	g_Anisotropic: 0.0
	g_Anisotropic_X: 0.10000000149011612
	g_Anisotropic_Y: 0.10000000149011612
	g_Decal: 0.0
	g_DetailNormalTile_X: 1.0
	g_DetailNormalTile_Y: 1.0
	g_FuzzColorCorrection_X: 1.0
	g_FuzzColorCorrection_Y: 1.0
	g_FuzzColorCorrection_Z: 1.0
	g_FuzzExponent: 2.0
	g_FuzzMaskEffective: 1.0
	g_FuzzMul: 0.0
	g_FuzzReverse: 0.0
	g_FuzzShadowLowerLimit: 0.30000001192092896
	g_Glossiness: 0.20000000298023224
	g_HilIghtIntensity: 1.0
	g_IsSwatchRender: 0.0
	g_LighIntensity0: 1.0
	g_LighIntensity1: 1.0
	g_LighIntensity2: 1.0
	g_LightColor0_X: 1.0
	g_LightColor0_Y: 1.0
	g_LightColor0_Z: 1.0
	g_LightColor1_X: 1.0
	g_LightColor1_Y: 1.0
	g_LightColor1_Z: 1.0
	g_LightColor2_X: 1.0
	g_LightColor2_Y: 1.0
	g_LightColor2_Z: 1.0
	g_LightIntensity: 1.0
	g_Metallic: 0.0
	g_NormalReverse: 0.0
	g_ObjWetStrength: 0.0
	g_OffShadowCast: 0.0
	g_ReflectionIntensity: 1.0
	g_Tile_X: 1.0
	g_Tile_Y: 1.0
	g_UseDetailNormalMap: 0.0
	g_UseEnvWet: 0.0
	g_UseNormalMap: 1.0
	g_UseObjWet: 1.0
	g_WetConvergenceGlossiness: 0.4000000059604645
	g_WetConvergenceHLI: 0.0
	g_WetConvergenceMetalic: 0.0
	g_WetMagAlbedo: 0.4000000059604645
	g_bAlbedoOverWrite: 0.0
	g_bGlossinessOverWrite: 0.0
	g_bMetalicOverWrite: 0.0

Eye00_XXXXX:
	Binormal0: 0.0
	Color0: 0.0
	Normal: 0.0
	Position: 0.0
	Tangent0: 0.0
	TexCoord0: 0.0
	TexCoord1: 0.0
	g_AddEnvCubeIntensity: 1.0
	g_AlbedoColor_X: 0.5
	g_AlbedoColor_Y: 0.5
	g_AlbedoColor_Z: 0.5
	g_AmbientLightIntensity: 1.0
	g_Anisotropic: 0.0
	g_Glossiness: 0.800000011920929
	g_GlossinessIris: 0.4000000059604645
	g_IsSwatchRender: 0.0
	g_LighIntensity0: 1.0
	g_LighIntensity1: 1.0
	g_LighIntensity2: 1.0
	g_LightColor0_X: 1.0
	g_LightColor0_Y: 1.0
	g_LightColor0_Z: 1.0
	g_LightColor1_X: 1.0
	g_LightColor1_Y: 1.0
	g_LightColor1_Z: 1.0
	g_LightColor2_X: 1.0
	g_LightColor2_Y: 1.0
	g_LightColor2_Z: 1.0
	g_LightIntensity: 1.0
	g_LightIrisIntensity: 0.6000000238418579
	g_Metallic: 0.0
	g_MetallicIris: 1.0
	g_NormalReverse: 0.0
	g_ParallaxStrength: 0.019999999552965164
	g_ReflectionIntensity: 1.0
	g_UseNormalMap: 1.0
	g_bAlbedoOverWrite: 0.0
	g_bGlossinessOverWrite: 1.0
	g_bMetalicOverWrite: 1.0

PBS00_XXXXX:
	Binormal0: 0.0
	Color0: 0.0
	Normal: 0.0
	Position: 0.0
	Tangent0: 0.0
	TexCoord0: 0.0
	TexCoord1: 0.0
	g_1BitMask: 0.0
	g_AlbedoColor_X: 0.5
	g_AlbedoColor_Y: 0.5
	g_AlbedoColor_Z: 0.5
	g_AmbientLightIntensity: 1.0
	g_Anisotropic: 0.0
	g_Decal: 0.0
	g_DetailNormalTile_X: 1.0
	g_DetailNormalTile_Y: 1.0
	g_Glossiness: 0.20000000298023224
	g_IsSwatchRender: 0.0
	g_LighIntensity0: 1.0
	g_LighIntensity1: 1.0
	g_LighIntensity2: 1.0
	g_LightColor0_X: 1.0
	g_LightColor0_Y: 1.0
	g_LightColor0_Z: 1.0
	g_LightColor1_X: 1.0
	g_LightColor1_Y: 1.0
	g_LightColor1_Z: 1.0
	g_LightColor2_X: 1.0
	g_LightColor2_Y: 1.0
	g_LightColor2_Z: 1.0
	g_LightIntensity: 1.0
	g_Metallic: 0.0
	g_NormalReverse: 0.0
	g_ObjWetStrength: 0.0
	g_OffShadowCast: 0.0
	g_ReflectionIntensity: 1.0
	g_Tile_X: 1.0
	g_Tile_Y: 1.0
	g_UV2Use: 0.0
	g_UseDetailNormalMap: 0.0
	g_UseEnvWet: 0.0
	g_UseLightMap: 0.0
	g_UseNormalMap: 1.0
	g_UseObjWet: 1.0
	g_UseOcclusionMap: 0.0
	g_WetConvergenceGlossiness: 0.6000000238418579
	g_WetMagAlbedo: 0.5
	g_bAlbedoOverWrite: 0.0
	g_bGlossinessOverWrite: 0.0
	g_bMetalicOverWrite: 0.0

CNS00_XXXXX:
	Binormal0: 0.0
	Color0: 0.0
	Normal: 0.0
	Position: 0.0
	Tangent0: 0.0
	TexCoord0: 0.0
	TexCoord1: 0.0
	g_1BitMask: 0.0
	g_AlbedoColor_X: 0.5
	g_AlbedoColor_Y: 0.5
	g_AlbedoColor_Z: 0.5
	g_Decal: 0.0
	g_Intensity: 1.0
	g_InvalidFog: 0.0
	g_IsSwatchRender: 0.0
	g_LighIntensity0: 1.0
	g_LighIntensity1: 1.0
	g_LighIntensity2: 1.0
	g_LightColor0_X: 1.0
	g_LightColor0_Y: 1.0
	g_LightColor0_Z: 1.0
	g_LightColor1_X: 1.0
	g_LightColor1_Y: 1.0
	g_LightColor1_Z: 1.0
	g_LightColor2_X: 1.0
	g_LightColor2_Y: 1.0
	g_LightColor2_Z: 1.0
	g_Tile_X: 1.0
	g_Tile_Y: 1.0
	g_UseMultiplicationBlend: 1.0
	g_UseSubtractionBlend: 0.0
	g_UvAnimation_X: 0.0
	g_UvAnimation_Y: 0.0
	g_bAlbedoOverWrite: 0.0

SKN00_XXXXX:
	Binormal0: 0.0
	Color0: 0.0
	Normal: 0.0
	Position: 0.0
	Tangent0: 0.0
	TexCoord0: 0.0
	TexCoord1: 0.0
	g_1BitMask: 0.0
	g_AlbedoColor_X: 0.5
	g_AlbedoColor_Y: 0.5
	g_AlbedoColor_Z: 0.5
	g_AmbientLightIntensity: 1.0
	g_Anisotropic: 0.0
	g_DetailNormalTile_X: 50.0
	g_DetailNormalTile_Y: 50.0
	g_EnvRoughnessHosei: 5.0
	g_Glossiness: 0.20000000298023224
	g_IsSwatchRender: 0.0
	g_LighIntensity0: 1.0
	g_LighIntensity1: 1.0
	g_LighIntensity2: 1.0
	g_LightColor0_X: 1.0
	g_LightColor0_Y: 1.0
	g_LightColor0_Z: 1.0
	g_LightColor1_X: 1.0
	g_LightColor1_Y: 1.0
	g_LightColor1_Z: 1.0
	g_LightColor2_X: 1.0
	g_LightColor2_Y: 1.0
	g_LightColor2_Z: 1.0
	g_LightIntensity: 1.0
	g_Metallic: 0.0
	g_NormalReverse: 0.0
	g_ObjWetStrength: 0.0
	g_OcclusionColor_X: 0.29177701473236084
	g_OcclusionColor_Y: 0.03954600170254707
	g_OcclusionColor_Z: 0.03954600170254707
	g_OffShadowCast: 0.0
	g_ReflectionIntensity: 1.0
	g_TransMissionColor_X: 0.8069549798965454
	g_TransMissionColor_Y: 0.30055099725723267
	g_TransMissionColor_Z: 0.20155400037765503
	g_UseDetailNormalMap: 1.0
	g_UseNormalMap: 1.0
	g_UseObjWet: 1.0
	g_WetConvergenceGlossiness: 0.800000011920929
	g_WetMagAlbedo: 0.6299999952316284
	g_bAlbedoOverWrite: 0.0
	g_bDispCurvature: 0.0
	g_bDispSpecular: 0.0
	g_bGlossinessOverWrite: 0.0
	g_bMetalicOverWrite: 0.0
	g_bUseCurvatureMap: 1.0
	g_rho_s: 1.0
	g_tuneCurvature: 1.0

Hair01_XXXXX:
	Binormal0: 0.0
	Color0: 0.0
	Normal: 0.0
	Position: 0.0
	Tangent0: 0.0
	TexCoord0: 0.0
	TexCoord1: 0.0
	g_1BitMask: 1.0
	g_AlbedoColor_X: 0.5149080157279968
	g_AlbedoColor_Y: 0.47352299094200134
	g_AlbedoColor_Z: 0.4286850094795227
	g_AmbientLightIntensity: 1.0
	g_AnisoLightMode: 0.0
	g_AnisoLightMode2: 1.0
	g_Anisotropic: 0.0
	g_Anisotropic_X: 0.10000000149011612
	g_Anisotropic_Y: 70.0
	g_DetailNormalTile_X: 1.0
	g_DetailNormalTile_Y: 1.0
	g_EnvRoughnessHosei: 2.0
	g_FuzzExponent: 4.0
	g_FuzzMaskEffective: 1.0
	g_Glossiness: 0.10000000149011612
	g_HilIghtIntensity: 0.800000011920929
	g_IsSwatchRender: 0.0
	g_LighIntensity0: 1.0
	g_LighIntensity1: 1.0
	g_LighIntensity2: 1.0
	g_LightColor0_X: 1.0
	g_LightColor0_Y: 1.0
	g_LightColor0_Z: 1.0
	g_LightColor1_X: 1.0
	g_LightColor1_Y: 1.0
	g_LightColor1_Z: 1.0
	g_LightColor2_X: 1.0
	g_LightColor2_Y: 1.0
	g_LightColor2_Z: 1.0
	g_LightIntensity: 1.0
	g_Metallic: 0.20000000298023224
	g_NoiseTile_X: 10.0
	g_NoiseTile_Y: 1.0
	g_NormalReverse: 0.0
	g_ObjWetStrength: 0.0
	g_OffShadowCast: 0.0
	g_ReflectionIntensity: 1.0
	g_SecondaryGlossiness: 0.621999979019165
	g_SecondaryMetalic: 0.800000011920929
	g_SecondarySpecShift: -0.36000001430511475
	g_SpecShift: 0.019999999552965164
	g_UseDetailNormalMap: 0.0
	g_UseNormalMap: 1.0
	g_UseObjWet: 1.0
	g_WetConvergenceGlossiness: 0.5
	g_WetConvergenceHLI: 0.5
	g_WetConvergenceSecondaryGlossiness: 1.0
	g_WetMagAlbedo: 0.6299999952316284
	g_WetMagNoiseTile_X: 1.0
	g_WetMagNoiseTile_Y: 1.0
	g_bAlbedoOverWrite: 1.0
	g_bDispNoise: 0.0
	g_bGlossinessOverWrite: 1.0
	g_bMetalicOverWrite: 1.0
*/