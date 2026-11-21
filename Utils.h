#pragma once
#include "WinInclude.h"
#include <d3dcompiler.h>
#include <dbghelp.h>
#include <Shlobj.h>
#include <wincodec.h>

#include "Globals.h"
#include "Matrix4x4.h"
#include "Log.h"

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
	NIERVER_STEAM_26534214,			// Steam build 26534214 (Nov 2026 update, no SteamStub/Denuvo)
	NIERVER_STEAM_26534214_COMPAT,	// NieRAutomataCompat.exe from the same build
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
	case NIERVER_STEAM_26534214:
		os << "NieR:Automata (Steam build 26534214)";
		break;
	case NIERVER_STEAM_26534214_COMPAT:
		os << "NieR:Automata (Steam build 26534214 Compat)";
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
		: m_pHash((BYTE*)Hash), m_uHashSize(32), m_Version(Version) // SHA-256 digests are binary and can contain zero bytes, so strlen can't size them
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
		case NIERVER_STEAM_26534214:
			m_szVersion = TEXT("NieR:Automata (Steam build 26534214)");
			break;
		case NIERVER_STEAM_26534214_COMPAT:
			m_szVersion = TEXT("NieR:Automata (Steam build 26534214 Compat)");
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

// Hashes a buffer with SHA-256. Opens a real algorithm provider instead of using the
// BCRYPT_SHA256_ALG_HANDLE pseudo-handle, which Wine/Proton doesn't support (issue #13).
static NTSTATUS HashBufferSHA256(const BYTE* pData, DWORD cbData, BYTE*& pHash, ULONG& uHashSize)
{
	BCRYPT_ALG_HANDLE hAlgorithm = NULL;
	NTSTATUS Status = BCryptOpenAlgorithmProvider(&hAlgorithm, BCRYPT_SHA256_ALGORITHM, NULL, 0);

	if (!BCRYPT_SUCCESS(Status))
		return Status;

	ULONG uHashLengthSize;

	// Query the hash length
	Status = BCryptGetProperty(hAlgorithm, BCRYPT_HASH_LENGTH, (PUCHAR)&uHashSize, sizeof(ULONG), &uHashLengthSize, 0);

	if (BCRYPT_SUCCESS(Status))
	{
		// Allocate the memory for hash
		pHash = new BYTE[uHashSize];

		// Create the hash
		Status = BCryptHash(hAlgorithm, NULL, 0, (PUCHAR)pData, cbData, pHash, uHashSize);

		if (!BCRYPT_SUCCESS(Status))
		{
			delete[] pHash;
			pHash = NULL;
		}
	}

	BCryptCloseAlgorithmProvider(hAlgorithm, 0);

	return Status;
}

static NTSTATUS QueryNierBinaryHash(NierVersionInfo& Version)
{
	//	TCHAR szFileName[MAX_PATH];

	// Set the inital status to success
	NTSTATUS Status = STATUS_SUCCESS;

	// Retrieve the size of the nier binary file path
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
	DWORD dwBytesRead = 0;

	// Allocate the memory for the nier binary
	BYTE* pBinary = new BYTE[dwFileSize];

	// Read the binary file
	BOOL bRead = ReadFile(hFile, pBinary, dwFileSize, &dwBytesRead, NULL);

	// Close the file handle to the nier binary
	CloseHandle(hFile);

	// Bail out if the read failed or was partial
	if (!bRead || dwFileSize != dwBytesRead)
	{
		delete[] pBinary;
		return STATUS_PARTIAL_COPY;
	}

	ULONG uHashSize = 0;

	// Files bigger than 0xFFFFFFFF are unsupported and invoke UB
	Status = HashBufferSHA256(pBinary, dwFileSize, Version.m_pHash, uHashSize);
	Version.m_uHashSize = uHashSize;

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
	DWORD dwBytesRead = 0;

	// Allocate the memory for the nier binary
	BYTE* pBinary = new BYTE[dwFileSize];

	// Read the binary file
	BOOL bRead = ReadFile(hFile, pBinary, dwFileSize, &dwBytesRead, NULL);

	// Close the file handle to the nier binary
	CloseHandle(hFile);

	// Bail out if the read failed or was partial
	if (!bRead || dwFileSize != dwBytesRead)
	{
		delete[] pBinary;
		return STATUS_PARTIAL_COPY;
	}

	Status = HashBufferSHA256(pBinary, dwFileSize, pHash, uHashSize);

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
		{ NIERVER_STEAM_26534214, "\x09\x85\x4C\x2D\x45\xD9\xA7\x2E\x66\x35\x7E\x72\xF5\x0C\xEE\x37\x9B\xB2\xEA\xFE\x35\xB3\xD6\x4E\x5E\x88\xF4\xED\xDC\x86\x13\xFA" },
		{ NIERVER_STEAM_26534214_COMPAT, "\x8A\x51\x0D\x8D\x6F\x56\x91\xB6\x9D\xCE\xDD\xC3\x35\x50\x8B\x2D\xC6\xCD\xFF\x92\x74\xC2\x36\xBD\x6C\xC8\x0B\x14\xE5\x48\x50\x0C" },
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

	// For an unknown build, keep the hash we computed so it can still be logged
	// and reported (issue #13), instead of leaving the entry's hash null.
	if (pVersion->m_Version == NIERVER_UNKNOWN && uHashSize == 32)
	{
		static BYTE s_UnknownHash[32];

		memcpy(s_UnknownHash, pHash, sizeof(s_UnknownHash));
		pVersion->m_pHash = s_UnknownHash;
		pVersion->m_uHashSize = sizeof(s_UnknownHash);
	}

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

// TODO: finish dinput8.dll & gameoverlayrenderer64.dll stuff
// Thanks valve and your gameoverlayrenderer64.dll!HookedDirectInput8Create
#pragma region gameoverlayrenderer64

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

#pragma endregion gameoverlayrenderer64

// The DirectInput helpers below are inline rather than static so their one-time state (the
// resolved module, what was logged) is shared by every translation unit instead of copied
// into each. They call Log:: directly rather than through LOG/WARN,
// which expand differently per translation unit (and to nothing in Release).

// Reads a pointer out of memory laid out by someone else's hook, where it may not be valid
inline bool TryReadPointer(const void* pAddress, void** ppValue)
{
	__try
	{
		*ppValue = *(void* const*)pAddress;
		return true;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		return false;
	}
}

// The real dinput8.dll from the system directory. A dinput8.dll proxy in the game folder (a
// mod loader) has the same name, so plain GetModuleHandle("dinput8.dll") could return that.
inline HMODULE GetSystemDirectInput8(void)
{
	static HMODULE s_hDirectInput8 = NULL;
	static bool s_bResolved = false;

	if (s_bResolved)
		return s_hDirectInput8;

	s_bResolved = true;
	s_hDirectInput8 = GetModuleHandleW(L"System32\\dinput8.dll");

	// GetModuleHandle isn't documented to take a relative path, so if that form doesn't
	// resolve, build the full path from the real system directory.
	if (!s_hDirectInput8)
	{
		WCHAR szPath[MAX_PATH];
		const UINT cchPath = GetSystemDirectoryW(szPath, MAX_PATH);

		if (cchPath && cchPath < MAX_PATH && !wcscat_s(szPath, L"\\dinput8.dll"))
			s_hDirectInput8 = GetModuleHandleW(szPath);
	}

	if (s_hDirectInput8)
	{
		CHAR szModule[MAX_PATH] = "";

		GetModuleFileNameA(s_hDirectInput8, szModule, MAX_PATH);
		Log::Log(__FUNCSIG__, "DirectInput: system dinput8.dll is %p (%s)\n", s_hDirectInput8, szModule);
	}
	else
	{
		Log::Warn(__FUNCSIG__, "DirectInput: the system dinput8.dll isn't loaded, so the keyboard and mouse are hooked without their device lock\n");
	}

	return s_hDirectInput8;
}

// Logs each module that owns a device we can't lock, once
inline void LogUnlockableDirectInputDevice(HMODULE hOwner, const void* pVtable)
{
	static HMODULE s_Logged[8];
	static int s_nLogged = 0;

	for (int i = 0; i < s_nLogged; ++i)
	{
		if (s_Logged[i] == hOwner)
			return;
	}

	if (s_nLogged < ARRAYSIZE(s_Logged))
		s_Logged[s_nLogged++] = hOwner;

	CHAR szOwner[MAX_PATH] = "(no module)";

	if (hOwner)
		GetModuleFileNameA(hOwner, szOwner, MAX_PATH);

	Log::Warn(__FUNCSIG__, "DirectInput: device vtable %p belongs to %s, not dinput8.dll, so it is hooked without the device lock\n",
		pVtable, szOwner);
}

// Finds the dinput8.dll CDIDev behind pDevice, whose critical section is held while the
// device's vtable is swapped. Returns NULL when that can't be done safely:
//	- dinput8.dll's own device: the CDIDev it belongs to, as before.
//	- the Steam overlay's wrapper (gameoverlayrenderer64.dll): the device it wraps, but only
//	  if that one really is dinput8.dll's.
//	- anything else - ReShade, Special K, Wine/Proton's dinput, unknown wrappers - has its own
//	  layout, so nothing is locked. That's acceptable: the vtable swap is one atomic pointer
//	  write. This used to treat every non-dinput8 device as the Steam overlay's wrapper and
//	  enter a "critical section" at an offset into whatever object it really was.
inline CDirectInputDevice8A* ResolveDirectInputDevice(IDirectInputDevice8A* pDevice)
{
	void* pVtable = NULL;

	if (!pDevice || !TryReadPointer(pDevice, &pVtable))
		return NULL;

	const HMODULE hDirectInput8 = GetSystemDirectInput8();
	HMODULE hOwner = NULL;

	RtlPcToFileHeader(pVtable, (PVOID*)&hOwner);

	if (hDirectInput8 && hOwner == hDirectInput8)
		return CONTAINING_RECORD(pDevice, CDirectInputDevice8A, m_pDerived);

	if (hDirectInput8 && hOwner && hOwner == GetModuleHandleW(L"gameoverlayrenderer64.dll"))
	{
		void* pInner = NULL;
		void* pInnerVtable = NULL;
		HMODULE hInnerOwner = NULL;

		if (TryReadPointer(&((CWrapDirectInputDevice8*)pDevice)->m_pDeviceA, &pInner) && pInner &&
			TryReadPointer(pInner, &pInnerVtable) &&
			RtlPcToFileHeader(pInnerVtable, (PVOID*)&hInnerOwner) && hInnerOwner == hDirectInput8)
		{
			return CONTAINING_RECORD((IDirectInputDevice8A*)pInner, CDirectInputDevice8A, m_pDerived);
		}

		// The overlay wraps something other than dinput8.dll (another hook underneath it)
		if (hInnerOwner)
		{
			LogUnlockableDirectInputDevice(hInnerOwner, pInnerVtable);
			return NULL;
		}
	}

	LogUnlockableDirectInputDevice(hOwner, pVtable);

	return NULL;
}

// Holds a DirectInput device's CDIDev critical section for the guard's lifetime, so its vtable
// can be swapped without racing DirectInput's own calls on it. Locks nothing when
// ResolveDirectInputDevice can't find dinput8.dll's device behind it (see there). The release
// uses the device resolved here, so it can't disagree with the lock even if the vtable changes.
class CDirectInputDeviceLock
{
public:
	explicit CDirectInputDeviceLock(IDirectInputDevice8A* pDevice)
		: m_pImpl(ResolveDirectInputDevice(pDevice))
	{
		if (m_pImpl)
			EnterCriticalSection(&m_pImpl->m_CriticalSection);
	}

	~CDirectInputDeviceLock(void)
	{
		if (m_pImpl)
			LeaveCriticalSection(&m_pImpl->m_CriticalSection);
	}

	CDirectInputDeviceLock(const CDirectInputDeviceLock&) = delete;
	CDirectInputDeviceLock& operator=(const CDirectInputDeviceLock&) = delete;

private:
	CDirectInputDevice8A* m_pImpl;
};

// https://stackoverflow.com/questions/46182845/field-of-view-aspect-ratio-view-matrix-from-projection-matrix-hmd-ost-calib
static float GetFovFromProjectionMatrix(void)
{
	return 2.0f * atan(1.0f / g_pCamera->m_pInstance->m_ViewProjection[1][1]);
}

/*
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

	// transform by view matrix
	vMatrix.Transform(vIn, vTransform);

	// multiply by inverse w (vTransform /= w)
	vTransform *= 1.0f / w;

	float width = (float)g_pGraphicDevice->m_iScreenWidth;
	float height = (float)g_pGraphicDevice->m_iScreenHeight;

	float flFocalLengthY = 1.0f / tanf(g_pCamera->m_flFov * 0.5f);
	[[maybe_unused]] float flFocalLengthX = flFocalLengthY / (width / height);

	float x = (1.f + (vTransform.x/* * flFocalLengthX * (1.0f / vTransform.z))*/));
	float y = (1.f - (vTransform.y /** flFocalLengthY*/ /* * (1.0f / vTransform.z)*/));
	vOut.x = x * width * 0.5f;
	vOut.y = y * height * 0.5f;

	//vOut.x = x + vTransform.x * flFocalLengthX * width / vTransform.z;
	//vOut.y = y - vTransform.y * flFocalLengthY * height / vTransform.z;

	//vOut.x = x + 0.5f * vTransform.x * width + 0.5f;
	//vOut.y = y - 0.5f * vTransform.y * height + 0.5f;

	return true;
}

static inline uint16_t EntityHandleToListIndex(const EntityHandle hEntity)
{
	return (uint16_t)((hEntity & 0x00FFFF00) >> 8); // (uint16_t)(handle >> 8)
}

static inline EntityHandle GenerateEntityHandle(const CEntityList* pList, const uint16_t index)
{
	return (index | (pList->m_uSector << 16)) << 8;
}

// NOTE: You should use these functions two get entity's by their handle from now on.
// It is the proper way to do it, since it doesn't do any filtering.
static CBehaviorAppBase* GetEntityByHandle(const CEntityList* pList, const EntityHandle hEntity)
{
	int32_t idx = EntityHandleToListIndex(hEntity);

	// Check if the index is valid and the top 24-bits of the handles match
	if ((uint32_t)idx > pList->m_uItems || idx < 0 || (hEntity ^ pList->m_pItems[idx].first) & 0xFFFFFF00)
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
static CBehaviorAppBase* GetEntityFromHandleGlobal(EntityHandle_t* phEntity)
{
	CEntityInfo* pEntityInfo = GetEntityInfoFromHandle(phEntity);
	return pEntityInfo ? pEntityInfo->m_pParent : NULL;
}

// pEntity as a Pl0000, or NULL when it isn't one: the camera entity and entities looked up by
// handle can be anything. GetEntityFromHandle2 (0x1403877A0) is the game's Pl0000-only lookup,
// GetEntityInfoFromHandle and then CEntityInfo::GetPl0000 (0x1403085C0), which tests the type info.
static Pl0000* GetPl0000(CBehaviorAppBase* pEntity)
{
	if (!pEntity || !pEntity->m_pInfo || !GetEntityFromHandle2)
		return NULL;

	return (GetEntityFromHandle2(&pEntity->m_pInfo->m_hEntity) == pEntity) ? static_cast<Pl0000*>(pEntity) : NULL;
}

// The pod pOwner has out, picked the way Pl0000_GetActivePod (0x14033C2D0) does:
// m_hPods[m_iActivePod]. Callers only touch the pod's model, which every entity has.
static CBehaviorAppBase* GetActivePod(Pl0000* pOwner)
{
	if (!pOwner || pOwner->m_iActivePod < 0 || pOwner->m_iActivePod >= (int32_t)ARRAYSIZE(pOwner->m_hPods))
		return NULL;

	return GetEntityFromHandleGlobal(&pOwner->m_hPods[pOwner->m_iActivePod]);
}

// Rebuilt from binary with added functionality
static BOOL ObjectIdToObjectName(char* szObjectName, size_t size, uint32_t ObjectId, ObjectIdConvert** ppConvert)
{
	static char HexChars[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
	static ObjectIdConvert Converts[] = { { 0x10000, "pl" }, { 0x20000, "em" }, { 0x30000, "wp" }, { 0x40000, "et" }, { 0x50000, "ef" }, { 0x60000, "es" },
										{ 0x70000, "it" }, { 0x90000, "sc" }, { 0xA0000, "um" }, { 0xC0000, "bg" }, { 0xE0000, "bh" }, { 0xF0000, "ba" } };

	uint32_t i = 0;

	do
	{
		if (Converts[i].m_ObjectIdBase == (ObjectId & 0xFFFF0000)) // high word of the int
		{
			szObjectName[0] = Converts[i].m_szPrefix[0];
			szObjectName[1] = Converts[i].m_szPrefix[1];
			szObjectName[2] = HexChars[(ObjectId >> 12) & 0xF];
			szObjectName[3] = HexChars[(ObjectId >> 8) & 0xF];
			szObjectName[4] = HexChars[(ObjectId >> 4) & 0xF];
			szObjectName[5] = HexChars[ObjectId & 0xF];
			szObjectName[6] = 0; //null terminator

			// ppConvert itself is what's optional; testing *ppConvert read the caller's
			// uninitialised pointer (MyPreloadModel) and never set a NULL one
			if (ppConvert)
				*ppConvert = &Converts[i];

			return TRUE;
		}
		++i;
	} while (i < ARRAYSIZE(Converts));

	memset(szObjectName, 0, size);

	if (ObjectId == -1)
		strcpy_s(szObjectName, size, "eObjInvalid");
	else
		strcpy_s(szObjectName, size, (ObjectId != 0x700000) ? "Unknow" : "Null");

	if (ppConvert)
		*ppConvert = NULL;

	return FALSE;
}

static BOOL ObjectNameToObjectId(uint32_t* pObjectId, const char* szObjectName)
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

/// <summary>
/// Rebuilt from IDA Database
/// </summary>
/// <typeparam name="N">Size of szPath</typeparam>
/// <param name="szPath">File path of the game</param>
/// <returns></returns>
template<size_t N>
static bool GetSaveFolderPath(char (&szPath)[N])
{
	if (SHGetSpecialFolderPathA(NULL, szPath, CSIDL_PERSONAL, FALSE))
	{
		sprintf_s(szPath, "%s\\%s\\%s\\", szPath, "My Games", "NieR_Automata");
		return true;
	}
	return false;
}

/// <summary>
/// Rebuilt from IDA Database
/// </summary>
/// <typeparam name="N"></typeparam>
/// <param name="pSavedata"></param>
/// <param name="szPath"></param>
/// <returns></returns>
template<size_t N = MAX_PATH>
static bool GetGameDataPath(CSaveDataDevice* pSavedata, char(&szPath)[N])
{
	if (GetSaveFolderPath(szPath))
	{
		// Appended, not printed over: sprintf_s with szPath as both the buffer and an argument was
		// undefined behaviour
		char szName[32];

		switch (pSavedata->m_nSlot)
		{
		case CSaveDataDevice::SLOT_SAVE_SYSTEMDATA:
			strcpy_s(szName, "SystemData.dat");
			break;
		case CSaveDataDevice::SLOT_SAVE_GAMEDATA:
			strcpy_s(szName, "GameData.dat");
			break;
		default:
			sprintf_s(szName, "SlotData_%d.dat", pSavedata->m_nSlot);
			break;
		}

		strcat_s(szPath, szName);
		return true;
	}
	return false;
}

// Rebuilt from the binary for the reason that it was inlined which
// will complicate things in the hook otherwise
static void CSaveDataDevice_DeleteSaveData(CSaveDataDevice* pSavedata)
{
	char szFilename[MAX_PATH] = {};

	switch (pSavedata->m_Status)
	{
	case 0:
		// sub_140282BD0
		if (GetGameDataPath(pSavedata, szFilename))
		{
			if (DeleteFileA(szFilename))
			{
				pSavedata->m_pSaveSlots[pSavedata->m_nSlot].m_uAccountId = 0;
				pSavedata->m_pSaveSlots[pSavedata->m_nSlot].m_bInUse = FALSE;

				if (pSavedata->m_hFile != INVALID_HANDLE_VALUE)
				{
					CloseHandle(pSavedata->m_hFile);
					pSavedata->m_hFile = INVALID_HANDLE_VALUE;
				}

				*(&pSavedata->m_nMaxSlot + 1) = 0;
				pSavedata->m_uFlags64 = 0i64;
			}
			else
			{
				pSavedata->m_Status = CSaveDataDevice::STATUS_SAVE_DATA_READ_SLOTS;
			}
		}
		break;
	case 1:
		if (pSavedata->m_hFile != INVALID_HANDLE_VALUE)
		{
			CloseHandle(pSavedata->m_hFile);
			pSavedata->m_hFile = INVALID_HANDLE_VALUE;
		}

		*(&pSavedata->m_nMaxSlot + 1) = 3;
		pSavedata->m_uFlags64 = 0i64;
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
		pSavedata->m_Status = CSaveDataDevice::STATUS_SAVE_DATA_READ_SLOTS;
		break;
	}
}

static CMeshPart* GetModelMesh(CModelWork* pWork, const char* szMesh)
{
	for (int32_t i = 0; i < pWork->m_nMeshes; ++i)
		if (!strcmp(pWork->m_pMeshes[i].m_szMeshName, szMesh))
			return &pWork->m_pMeshes[i];

	return NULL;
}

static int32_t GetModelMeshIndex(const CModelWork* pWork, const char* szMesh)
{
	for (int32_t i = 0; i < pWork->m_nMeshes; ++i)
		if (!strcmp(pWork->m_pMeshes[i].m_szMeshName, szMesh))
			return i;

	return -1;
}

// Rebuilt from binary
static int16_t GetBoneIndex(const CModelData* pModelData, int16_t Id)
{
	int16_t* pTable;
	int16_t iBoneIndex, t1, t2;

	pTable = pModelData->m_pBoneIndexTranslationTable2;

	if (pTable)
	{
		t1 = pTable[(Id >> 8) & 0xF];	// id / 256  clamped to 0-15

		if (t1 != -1)
		{
			t2 = pTable[t1 + ((Id >> 4) & 0xF)]; // id

			if (t2 != -1)
			{
				iBoneIndex = pTable[t2 + (Id & 0xF)];

				if (iBoneIndex != 0xFFF) // 4095
					return iBoneIndex;
			}
		}
	}
	return -1;
}

static int16_t GetBoneId(int16_t index)
{
	UNREFERENCED_PARAMETER(index);
}

static void SwapTexture(uint32_t srcid, CTexture* pReplace)
{
	CTextureResource* pRes = TextureResourceManager_FindResource(srcid);

	if (pRes)
		pRes->m_pTexture = pReplace;
}

static HRESULT CreateTextureEx(const wchar_t* szFile, CTextureDescription& Desc)
{
	UNREFERENCED_PARAMETER(Desc);

	IWICImagingFactory* pFactory;
	IWICFormatConverter* pConverter;
	IWICBitmapDecoder* pDecoder;
	IWICBitmapFrameDecode* pFrame;
	WICPixelFormatGUID Format;

	uint32_t w = 0, h = 0, uFrames, uStride, uSize;
	std::vector<uint32_t> Pixels;	// rgba was never allocated, so CopyPixels wrote through a garbage pointer

	HRESULT hr = CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_IWICImagingFactory, (void**)&pFactory);

	if (FAILED(hr))
		return hr;

	hr = pFactory->CreateDecoderFromFilename(szFile, NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pDecoder);

	if (FAILED(hr))
	{
		pFactory->Release();
		return hr;
	}

	hr = pDecoder->GetFrameCount(&uFrames);

	if (FAILED(hr))
	{
		pDecoder->Release();
		pFactory->Release();
		return hr;
	}

	hr = pDecoder->GetFrame(0, &pFrame);

	if (FAILED(hr))
	{
		pDecoder->Release();
		pFactory->Release();
		return hr;
	}

	hr = pFrame->GetPixelFormat(&Format);

	if (FAILED(hr))
	{
		pFrame->Release();
		pDecoder->Release();
		pFactory->Release();
		return hr;
	}

	hr = pFrame->GetSize(&w, &h);

	uStride = w * sizeof(uint32_t);
	uSize = uStride * h;
	Pixels.resize((size_t)w * h);

	uint32_t* rgba = Pixels.data();

	if (!IsEqualGUID(Format, GUID_WICPixelFormat32bppBGRA))
	{
		hr = pFactory->CreateFormatConverter(&pConverter);

		hr = pConverter->Initialize(pFrame, GUID_WICPixelFormat32bppBGRA,
			WICBitmapDitherTypeNone,         // Specified dither pattern
			NULL,                            // Specify a particular palette 
			0.f,                             // Alpha threshold
			WICBitmapPaletteTypeCustom);     // Palette translation type

		hr = pConverter->CopyPixels(NULL, uStride, uSize, (uint8_t*)rgba);

		pConverter->Release();
	}
	else
		hr = pFrame->CopyPixels(NULL, uStride, uSize, (uint8_t*)rgba);


	pFrame->Release();
	pDecoder->Release();
	pFactory->Release();

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
				pTexture->m_vtbl = (void*)FindPatternPtr(NULL, "48 89 58 50 48 8D 05", 7);

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
	int32_t nTextures,
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

	for (int32_t i = 0; i < nTextures; ++i)
	{
		TextureIds[i] = HashStringCRC32(szTextureNames[i], strlen(szTextureNames[i]));
		// old sig: 33 D2 48 8D 05 ? ? ? ? 48 89 51 58
		// old: 0x140E9FA38 | base + 0xE9FA38
		// new sig: 48 89 58 50 48 8D 05 ? ? ? ? 
		// new: 0x140DB1DE8 | base + 0xDB1DE8
		Textures[i].m_vtbl = (void*)FindPatternPtr(NULL, "48 89 58 50 48 8D 05", 7);

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

	for (int32_t i = 0; i < nTextures; ++i)
	{
		int32_t nTexture = g_pModelAnalyzer->FindTextureIndexByName(szTextureNames[i]);

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

static HRESULT MyPreloadModel(uint32_t uObjectId)
{
	char szObjectName[16];
	char szFilename[64];
	ObjectIdConvert* pConvert;
	CObjReadSystem::Work* pWork;
	CObjReadSystem::Work::Desc* pDesc;
	HeapAlloc_t lmao;

	if (ObjectIdToObjectName(szObjectName, ARRAYSIZE(szObjectName), uObjectId, &pConvert))
	{
		QueryHeap(&lmao, uObjectId, 1);
		snprintf(szFilename, ARRAYSIZE(szFilename), "%s\\%s%04x%s", pConvert->m_szPrefix, pConvert->m_szPrefix, HIWORD(uObjectId), ".dtt");
		pWork = FindObjectWork(uObjectId);
		pDesc = PreloadFile(0, PRELOAD_TYPE_MODELDATA, szFilename, lmao.Pointer, (byte)lmao.Succeeded, pWork);

		if (pDesc)
		{
			_InterlockedCompareExchange((volatile LONG*)&pDesc->m_0x20, 1, 0);
			_InterlockedIncrement((volatile LONG*)&pDesc->m_0x20);
			pWork->m_ObjectId = pDesc->m_objectId;
		}

		snprintf(szFilename, ARRAYSIZE(szFilename), "%s\\%s%04x%s", pConvert->m_szPrefix, pConvert->m_szPrefix, HIWORD(uObjectId), ".dat");
		pDesc = PreloadFile(0, PRELOAD_TYPE_MODELDATA, szFilename, lmao.Pointer, (byte)lmao.Succeeded, pWork);

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

// A pixel shader that outputs one colour, for Chams' "Use Shader". It takes no inputs: a pixel
// shader's inputs have to line up with what the game's vertex shader outputs, and an empty input
// list fits any of them. (It used to read a COLOR0 the game's vertex shaders may not write.)
static HRESULT GeneratePixelShader(ID3D11Device* pDevice, ID3D11PixelShader** ppPixelShader, float r, float g, float b)
{
	char szPixelShader[256];

	sprintf_s(szPixelShader, "float4 main() : SV_Target { return float4(%f, %f, %f, 1.0f); }", r, g, b);

	ID3DBlob* pBlob = NULL;
	ID3DBlob* pErrorMsgBlob = NULL;

	// The source's length, not the buffer's: the bytes after the terminator used to go in too
	HRESULT hr = D3DCompile(szPixelShader, strlen(szPixelShader), "shader", NULL, NULL, "main", "ps_4_0", 0, 0, &pBlob, &pErrorMsgBlob);

	if (pErrorMsgBlob)
		pErrorMsgBlob->Release();

	if (FAILED(hr))
		return hr;

	hr = pDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, ppPixelShader);

	pBlob->Release();

	return hr;
}

/*
Warning: this method is not synchronized with the game threads, therefore anomalies
may occur when copying files whilst being modified by another thread. Thread
safety will be added in the future. This method fails if the "My Documents" folder's
path has a greater length than MAX_PATH (260).
*/
static BOOL BackupSave(CSaveDataDevice::Slot Slot)
{
	WCHAR szPath[MAX_PATH];
	WCHAR szSavePath[MAX_PATH];
	WCHAR szBackupPath[MAX_PATH];
	SYSTEMTIME time;

	if (!IS_SAVE_SLOTDATA(Slot))
		return ERROR_INVALID_PARAMETER;

	if (!SHGetSpecialFolderPathW(NULL, szPath, CSIDL_MYDOCUMENTS, FALSE))
		return ERROR_PATH_NOT_FOUND;

	int32_t nCharsWritten = swprintf_s(szSavePath, MAX_PATH, L"%s\\My Games\\NieR_Automata\\SlotData_%d.dat", szPath, Slot);

	if (nCharsWritten == -1)
		return ERROR_NOT_ENOUGH_MEMORY;

	GetSystemTime(&time);

	nCharsWritten = swprintf_s(szBackupPath, MAX_PATH, L"%s\\My Games\\NieR_Automata\\SlotData_%d_%4d%02d%02d_%02d%02d%02d.dat.bak", szPath, Slot,
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
	MINIDUMP_TYPE Type = MINIDUMP_TYPE(MiniDumpWithUnloadedModules | MiniDumpWithIndirectlyReferencedMemory | 
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
		LERROR("Another component created a heap between calls therefore, the result will be inaccurate.");
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
		LERROR("Failed to write dump file!\n");

	return EXCEPTION_CONTINUE_SEARCH;
}

static LONG UnhandledExceptionHandler(EXCEPTION_POINTERS* pException)
{
	for (auto& handler : g_pExceptionHandlers)
		handler(pException);

	return EXCEPTION_CONTINUE_SEARCH;
}


// Formats a CRI message. CRI never formats its messages itself: CriLogInternal (0x140098454) hands
// the callback the format string and a pointer, split into its high and low halves, to two 64-bit
// argument slots, and leaves the substitution to it (ExcitedDelirium, Criware.cpp). Both slots are
// passed whatever the format asks for: on x64 a %d reads the low half of its slot, and a format
// that uses fewer ignores the rest.
// Into a buffer of this thread's own. CRI logs from its own threads, and the game's 0x100-byte
// g_szCRILogBuffer, which this used to write 512 bytes into, is where CriFsLogSystemError builds
// the very message it then passes as the format.
static const char* CRIGetBuffer(const char* szFormat, unsigned int arg_ptr_high, unsigned int arg_ptr_low)
{
	static thread_local char s_szBuffer[512];

	const QWORD* pArgs = (const QWORD*)(((QWORD)arg_ptr_high << 32) | arg_ptr_low);

	if (!szFormat)
		return "(null)";

#pragma warning(push)
#pragma warning(disable : 4774)	// the format isn't a literal: it is CRI's message
	if (pArgs)
		snprintf(s_szBuffer, sizeof(s_szBuffer), szFormat, pArgs[0], pArgs[1]);
	else
		snprintf(s_szBuffer, sizeof(s_szBuffer), "%s", szFormat);
#pragma warning(pop)

	return s_szBuffer;
}

static void CRILogCallbackWinConsole(const char* szFormat, unsigned int callback_arg_ptr_high, 
	unsigned int callback_arg_ptr_low, void* a4)
{
	UNREFERENCED_PARAMETER(a4);

	QWORD stack_ptr = (((QWORD)callback_arg_ptr_high << 32) | callback_arg_ptr_low) + 0x28;
	QWORD caller_return_address = *(QWORD*)stack_ptr;
	printf("[%llx]: %s\n", caller_return_address, CRIGetBuffer(szFormat, callback_arg_ptr_high, callback_arg_ptr_low));
}

const char* CRILogCallbackConsole(const char* szFormat, unsigned int callback_arg_ptr_high,
	unsigned int callback_arg_ptr_low, void* a4);

static void CRILogCallbackWinConsoleVerbose(const char* szFormat, unsigned int callback_arg_ptr_high, unsigned int callback_arg_ptr_low, void* a4)
{
	UNREFERENCED_PARAMETER(a4);

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