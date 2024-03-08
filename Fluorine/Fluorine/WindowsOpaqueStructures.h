#ifndef __WINDOWSOPAQUESTRUCTURES_H__
#define __WINDOWSOPAQUESTRUCTURES_H__

#define UMDF_USING_NTSTATUS
#include <Windows.h>

// Needed cause windows headers with winternl.h and ntdef.h
// have a stroke
typedef struct _UNICODE_STRING {
	USHORT Length;
	USHORT MaximumLength;
	PWSTR  Buffer;
} UNICODE_STRING;
typedef UNICODE_STRING* PUNICODE_STRING;
typedef const UNICODE_STRING* PCUNICODE_STRING;

typedef struct _RTL_DRIVE_LETTER_CURDIR
{
	USHORT                  Flags;
	USHORT                  Length;
	ULONG                   TimeStamp;
	UNICODE_STRING          DosPath;
} RTL_DRIVE_LETTER_CURDIR, * PRTL_DRIVE_LETTER_CURDIR;

typedef struct _LDR_MODULE
{
	LIST_ENTRY              InLoadOrderModuleList;
	LIST_ENTRY              InMemoryOrderModuleList;
	LIST_ENTRY              InInitializationOrderModuleList;
	PVOID                   BaseAddress;
	PVOID                   EntryPoint;
	ULONG                   SizeOfImage;
	UNICODE_STRING          FullDllName;
	UNICODE_STRING          BaseDllName;
	ULONG                   Flags;
	SHORT                   LoadCount;
	SHORT                   TlsIndex;
	LIST_ENTRY              HashTableEntry;
	ULONG                   TimeDateStamp;
} LDR_MODULE, * PLDR_MODULE;

typedef struct _RTL_USER_PROCESS_PARAMETERS
{
	ULONG                   MaximumLength;
	ULONG                   Length;
	ULONG                   Flags;
	ULONG                   DebugFlags;
	PVOID                   ConsoleHandle;
	ULONG                   ConsoleFlags;
	HANDLE                  StdInputHandle;
	HANDLE                  StdOutputHandle;
	HANDLE                  StdErrorHandle;
	UNICODE_STRING          CurrentDirectoryPath;
	HANDLE                  CurrentDirectoryHandle;
	UNICODE_STRING          DllPath;
	UNICODE_STRING          ImagePathName;
	UNICODE_STRING          CommandLine;
	PVOID                   Environment;
	ULONG                   StartingPositionLeft;
	ULONG                   StartingPositionTop;
	ULONG                   Width;
	ULONG                   Height;
	ULONG                   CharWidth;
	ULONG                   CharHeight;
	ULONG                   ConsoleTextAttributes;
	ULONG                   WindowFlags;
	ULONG                   ShowWindowFlags;
	UNICODE_STRING          WindowTitle;
	UNICODE_STRING          DesktopName;
	UNICODE_STRING          ShellInfo;
	UNICODE_STRING          RuntimeData;
	RTL_DRIVE_LETTER_CURDIR DLCurrentDirectory[32];
} RTL_USER_PROCESS_PARAMETERS, * PRTL_USER_PROCESS_PARAMETERS;

typedef struct _PEB_LDR_DATA
{
	ULONG Length;								//0x00
	UCHAR Initialized;							//0x04 | 3 byte padding
	PVOID SsHandle;								//0x08
	LIST_ENTRY InLoadOrderModuleList;			//0x1C
	LIST_ENTRY InMemoryOrderModuleList;			//0x2C
	LIST_ENTRY InInitializationOrderModuleList;	//0x3C
	PVOID EntryInProgress;						//0x40
} PEB_LDR_DATA, * PPEB_LDR_DATA;

typedef struct _LDR_DATA_TABLE_ENTRY
{
	LIST_ENTRY InLoadOrderLinks;			//0x00
	LIST_ENTRY InMemoryOrderLinks;			//0x10
	LIST_ENTRY InInitializationOrderLinks;  //0x20
	PVOID DllBase;							//0x30
	PVOID EntryPoint;						//0x38
	ULONG SizeOfImage;						//0x40
	UNICODE_STRING FullDllName;				//0x48
	UNICODE_STRING BaseDllName;				//0x58
	ULONG Flags;							//0x68
	WORD LoadCount;							//0x6C
	WORD TlsIndex;							//0x6E
	union
	{
		LIST_ENTRY HashLinks;				//0x70
		struct
		{
			PVOID SectionPointer;			//0x80
			ULONG CheckSum;					//0x80
		};
	};
	union
	{
		ULONG TimeDateStamp;				//0x84
		PVOID LoadedImports;				//0x84
	};
	PVOID EntryPointActivationContext;	    //0x0  _ACTIVATION_CONTEXT* I could not find this structure
	PVOID PatchInformation;
	LIST_ENTRY ForwarderLinks;
	LIST_ENTRY ServiceTagLinks;
	LIST_ENTRY StaticLinks;
} LDR_DATA_TABLE_ENTRY, * PLDR_DATA_TABLE_ENTRY;


typedef struct _PEB_FREE_BLOCK
{
	struct _PEB_FREE_BLOCK* Next;
	ULONG                   Size;
} PEB_FREE_BLOCK, * PPEB_FREE_BLOCK;

typedef void(*PPEBLOCKROUTINE)(PVOID PebLock);

typedef struct _PEB
{
	UCHAR InheritedAddressSpace;                                            //0x00
	UCHAR ReadImageFileExecOptions;                                         //0x01
	UCHAR BeingDebugged;                                                    //0x02
	union
	{
		UCHAR BitField;                                                     //0x03
		struct
		{
			UCHAR ImageUsesLargePages : 1;                                  //0x03
			UCHAR IsProtectedProcess : 1;                                   //0x03
			UCHAR IsImageDynamicallyRelocated : 1;                          //0x03
			UCHAR SkipPatchingUser32Forwarders : 1;                         //0x03
			UCHAR IsPackagedProcess : 1;                                    //0x03
			UCHAR IsAppContainer : 1;                                       //0x03
			UCHAR IsProtectedProcessLight : 1;                              //0x03
			UCHAR IsLongPathAwareProcess : 1;                               //0x03
		};
	};
	UCHAR Padding0[4];                                                      //0x4
	VOID* Mutant;                                                           //0x8
	VOID* ImageBaseAddress;                                                 //0x10
	struct _PEB_LDR_DATA* Ldr;                                              //0x18
	struct _RTL_USER_PROCESS_PARAMETERS* ProcessParameters;                 //0x20
	VOID* SubSystemData;                                                    //0x28
	VOID* ProcessHeap;                                                      //0x30
	struct _RTL_CRITICAL_SECTION* FastPebLock;                              //0x38
	union _SLIST_HEADER* volatile AtlThunkSListPtr;                         //0x40
	VOID* IFEOKey;                                                          //0x48
	union
	{
		ULONG CrossProcessFlags;                                            //0x50
		struct
		{
			ULONG ProcessInJob : 1;                                           //0x50
			ULONG ProcessInitializing : 1;                                    //0x50
			ULONG ProcessUsingVEH : 1;                                        //0x50
			ULONG ProcessUsingVCH : 1;                                        //0x50
			ULONG ProcessUsingFTH : 1;                                        //0x50
			ULONG ProcessPreviouslyThrottled : 1;                             //0x50
			ULONG ProcessCurrentlyThrottled : 1;                              //0x50
			ULONG ProcessImagesHotPatched : 1;                                //0x50
			ULONG ReservedBits0 : 24;                                         //0x50
		};
	};
	UCHAR Padding1[4];                                                      //0x54
	union
	{
		VOID* KernelCallbackTable;                                          //0x58
		VOID* UserSharedInfoPtr;                                            //0x58
	};
	ULONG SystemReserved;                                                   //0x60
	ULONG AtlThunkSListPtr32;                                               //0x64
	VOID* ApiSetMap;                                                        //0x68
	ULONG TlsExpansionCounter;                                              //0x70
	UCHAR Padding2[4];                                                      //0x74
	struct _RTL_BITMAP* TlsBitmap;                                          //0x78
	ULONG TlsBitmapBits[2];                                                 //0x80
	VOID* ReadOnlySharedMemoryBase;                                         //0x88
	VOID* SharedData;                                                       //0x90
	VOID** ReadOnlyStaticServerData;                                        //0x98
	VOID* AnsiCodePageData;                                                 //0xa0
	VOID* OemCodePageData;                                                  //0xa8
	VOID* UnicodeCaseTableData;                                             //0xb0
	ULONG NumberOfProcessors;                                               //0xb8
	ULONG NtGlobalFlag;                                                     //0xbc
	union _LARGE_INTEGER CriticalSectionTimeout;                            //0xc0
	ULONGLONG HeapSegmentReserve;                                           //0xc8
	ULONGLONG HeapSegmentCommit;                                            //0xd0
	ULONGLONG HeapDeCommitTotalFreeThreshold;                               //0xd8
	ULONGLONG HeapDeCommitFreeBlockThreshold;                               //0xe0
	ULONG NumberOfHeaps;                                                    //0xe8
	ULONG MaximumNumberOfHeaps;                                             //0xec
	VOID** ProcessHeaps;                                                    //0xf0
	VOID* GdiSharedHandleTable;                                             //0xf8
	VOID* ProcessStarterHelper;                                             //0x100
	ULONG GdiDCAttributeList;                                               //0x108
	UCHAR Padding3[4];                                                      //0x10c
	struct _RTL_CRITICAL_SECTION* LoaderLock;                               //0x110
	ULONG OSMajorVersion;                                                   //0x118
	ULONG OSMinorVersion;                                                   //0x11c
	USHORT OSBuildNumber;                                                   //0x120
	USHORT OSCSDVersion;                                                    //0x122
	ULONG OSPlatformId;                                                     //0x124
	ULONG ImageSubsystem;                                                   //0x128
	ULONG ImageSubsystemMajorVersion;                                       //0x12c
	ULONG ImageSubsystemMinorVersion;                                       //0x130
	UCHAR Padding4[4];                                                      //0x134
	ULONGLONG ActiveProcessAffinityMask;                                    //0x138
	ULONG GdiHandleBuffer[60];                                              //0x140
	VOID(*PostProcessInitRoutine)();										//0x230
	struct _RTL_BITMAP* TlsExpansionBitmap;                                 //0x238
	ULONG TlsExpansionBitmapBits[32];                                       //0x240
	ULONG SessionId;                                                        //0x2c0
	UCHAR Padding5[4];                                                      //0x2c4
	union _ULARGE_INTEGER AppCompatFlags;                                   //0x2c8
	union _ULARGE_INTEGER AppCompatFlagsUser;                               //0x2d0
	VOID* pShimData;                                                        //0x2d8
	VOID* AppCompatInfo;                                                    //0x2e0
	struct _UNICODE_STRING CSDVersion;                                      //0x2e8
	struct _ACTIVATION_CONTEXT_DATA* ActivationContextData;                 //0x2f8
	struct _ASSEMBLY_STORAGE_MAP* ProcessAssemblyStorageMap;                //0x300
	struct _ACTIVATION_CONTEXT_DATA* SystemDefaultActivationContextData;    //0x308
	struct _ASSEMBLY_STORAGE_MAP* SystemAssemblyStorageMap;                 //0x310
	ULONGLONG MinimumStackCommit;                                           //0x318
	VOID* SparePointers[2];                                                 //0x320
	VOID* PatchLoaderData;                                                  //0x330
	struct _CHPEV2_PROCESS_INFO* ChpeV2ProcessInfo;                         //0x338
	ULONG AppModelFeatureState;                                             //0x340
	ULONG SpareUlongs[2];                                                   //0x344
	USHORT ActiveCodePage;                                                  //0x34c
	USHORT OemCodePage;                                                     //0x34e
	USHORT UseCaseMapping;                                                  //0x350
	USHORT UnusedNlsField;                                                  //0x352
	VOID* WerRegistrationData;                                              //0x358
	VOID* WerShipAssertPtr;                                                 //0x360
	VOID* EcCodeBitMap;                                                     //0x368
	VOID* pImageHeaderHash;                                                 //0x370
	union
	{
		ULONG TracingFlags;                                                 //0x378
		struct
		{
			ULONG HeapTracingEnabled : 1;                                   //0x378
			ULONG CritSecTracingEnabled : 1;                                //0x378
			ULONG LibLoaderTracingEnabled : 1;                              //0x378
			ULONG SpareTracingBits : 29;                                    //0x378
		};
	};
	UCHAR Padding6[4];                                                      //0x37c
	ULONGLONG CsrServerReadOnlySharedMemoryBase;                            //0x380
	ULONGLONG TppWorkerpListLock;                                           //0x388
	struct _LIST_ENTRY TppWorkerpList;                                      //0x390
	VOID* WaitOnAddressHashTable[128];                                      //0x3a0
	VOID* TelemetryCoverageHeader;                                          //0x7a0
	ULONG CloudFileFlags;                                                   //0x7a8
	ULONG CloudFileDiagFlags;                                               //0x7ac
	CHAR PlaceholderCompatibilityMode;                                      //0x7b0
	CHAR PlaceholderCompatibilityModeReserved[7];                           //0x7b1
	struct _LEAP_SECOND_DATA* LeapSecondData;                               //0x7b8
	union
	{
		ULONG LeapSecondFlags;                                              //0x7c0
		struct
		{
			ULONG SixtySecondEnabled : 1;                                   //0x7c0
			ULONG Reserved : 31;                                            //0x7c0
		};
	};
	ULONG NtGlobalFlag2;                                                    //0x7c4
	ULONGLONG ExtendedFeatureDisableMask;                                   //0x7c8
} PEB, * PPEB;


typedef struct _PEB64
{
	BOOLEAN                 InheritedAddressSpace;
	BOOLEAN                 ReadImageFileExecOptions;
	BOOLEAN                 BeingDebugged;
	BOOLEAN                 Spare;
	ULONG64                 Mutant;
	ULONG64                 ImageBaseAddress;
	PPEB_LDR_DATA           LoaderData;
	PRTL_USER_PROCESS_PARAMETERS ProcessParameters;
	PVOID                   SubSystemData;
	PVOID                   ProcessHeap;
	PRTL_CRITICAL_SECTION   FastPebLock;
	PPEBLOCKROUTINE         FastPebLockRoutine; //AtlThunkSListPtr
	PPEBLOCKROUTINE         FastPebUnlockRoutine; //IFEOKey;
	ULONG                   EnvironmentUpdateCount;
	PVOID* KernelCallbackTable;
	PVOID                   EventLogSection;
	PVOID                   EventLog;
	PPEB_FREE_BLOCK         FreeList;
	ULONG                   TlsExpansionCounter;
	PVOID                   TlsBitmap;
	ULONG                   TlsBitmapBits[2];
	PVOID                   ReadOnlySharedMemoryBase;
	PVOID                   ReadOnlySharedMemoryHeap;
	PVOID* ReadOnlyStaticServerData;
	PVOID                   AnsiCodePageData;
	PVOID                   OemCodePageData;
	PVOID                   UnicodeCaseTableData;
	ULONG                   NumberOfProcessors;
	ULONG                   NtGlobalFlag;
	BYTE                    Spare2[0x4];
	LARGE_INTEGER           CriticalSectionTimeout;
	ULONG                   HeapSegmentReserve;
	ULONG                   HeapSegmentCommit;
	ULONG                   HeapDeCommitTotalFreeThreshold;
	ULONG                   HeapDeCommitFreeBlockThreshold;
	ULONG                   NumberOfHeaps;
	ULONG                   MaximumNumberOfHeaps;
	PVOID** ProcessHeaps;
	PVOID                   GdiSharedHandleTable;
	PVOID                   ProcessStarterHelper;
	PVOID                   GdiDCAttributeList;
	PRTL_CRITICAL_SECTION   LoaderLock;
	ULONG                   OSMajorVersion;
	ULONG                   OSMinorVersion;
	ULONG                   OSBuildNumber;
	ULONG                   OSPlatformId;
	ULONG                   ImageSubSystem;
	ULONG                   ImageSubSystemMajorVersion;
	ULONG                   ImageSubSystemMinorVersion;
	ULONG                   GdiHandleBuffer[34];
	ULONG                   PostProcessInitRoutine;
	ULONG                   TlsExpansionBitmap;
	BYTE                    TlsExpansionBitmapBits[128];
	ULONG                   SessionId;
} PEB64, * PPEB64;



typedef enum _SYSTEM_INFORMATION_CLASS
{
	SystemBasicInformation = 0,
	SystemProcessorInformation = 1,             // obsolete...delete
	SystemPerformanceInformation = 2,
	SystemTimeOfDayInformation = 3,
	SystemPathInformation = 4,
	SystemProcessInformation = 5,
	SystemCallCountInformation = 6,
	SystemDeviceInformation = 7,
	SystemProcessorPerformanceInformation = 8,
	SystemFlagsInformation = 9,
	SystemCallTimeInformation = 10,
	SystemModuleInformation = 11,
	SystemLocksInformation = 12,
	SystemStackTraceInformation = 13,
	SystemPagedPoolInformation = 14,
	SystemNonPagedPoolInformation = 15,
	SystemHandleInformation = 16,
	SystemObjectInformation = 17,
	SystemPageFileInformation = 18,
	SystemVdmInstemulInformation = 19,
	SystemVdmBopInformation = 20,
	SystemFileCacheInformation = 21,
	SystemPoolTagInformation = 22,
	SystemInterruptInformation = 23,
	SystemDpcBehaviorInformation = 24,
	SystemFullMemoryInformation = 25,
	SystemLoadGdiDriverInformation = 26,
	SystemUnloadGdiDriverInformation = 27,
	SystemTimeAdjustmentInformation = 28,
	SystemSummaryMemoryInformation = 29,
	SystemMirrorMemoryInformation = 30,
	SystemPerformanceTraceInformation = 31,
	SystemObsolete0 = 32,
	SystemExceptionInformation = 33,
	SystemCrashDumpStateInformation = 34,
	SystemKernelDebuggerInformation = 35,
	SystemContextSwitchInformation = 36,
	SystemRegistryQuotaInformation = 37,
	SystemExtendServiceTableInformation = 38,
	SystemPrioritySeperation = 39,
	SystemVerifierAddDriverInformation = 40,
	SystemVerifierRemoveDriverInformation = 41,
	SystemProcessorIdleInformation = 42,
	SystemLegacyDriverInformation = 43,
	SystemCurrentTimeZoneInformation = 44,
	SystemLookasideInformation = 45,
	SystemTimeSlipNotification = 46,
	SystemSessionCreate = 47,
	SystemSessionDetach = 48,
	SystemSessionInformation = 49,
	SystemRangeStartInformation = 50,
	SystemVerifierInformation = 51,
	SystemVerifierThunkExtend = 52,
	SystemSessionProcessInformation = 53,
	SystemLoadGdiDriverInSystemSpace = 54,
	SystemNumaProcessorMap = 55,
	SystemPrefetcherInformation = 56,
	SystemExtendedProcessInformation = 57,
	SystemRecommendedSharedDataAlignment = 58,
	SystemComPlusPackage = 59,
	SystemNumaAvailableMemory = 60,
	SystemProcessorPowerInformation = 61,
	SystemEmulationBasicInformation = 62,
	SystemEmulationProcessorInformation = 63,
	SystemExtendedHandleInformation = 64,
	SystemLostDelayedWriteInformation = 65,
	SystemBigPoolInformation = 66,
	SystemSessionPoolTagInformation = 67,
	SystemSessionMappedViewInformation = 68,
	SystemHotpatchInformation = 69,
	SystemObjectSecurityMode = 70,
	SystemWatchdogTimerHandler = 71,
	SystemWatchdogTimerInformation = 72,
	SystemLogicalProcessorInformation = 73,
	SystemWow64SharedInformation = 74,
	SystemRegisterFirmwareTableInformationHandler = 75,
	SystemFirmwareTableInformation = 76,
	SystemModuleInformationEx = 77,
	SystemVerifierTriageInformation = 78,
	SystemSuperfetchInformation = 79,
	SystemMemoryListInformation = 80,
	SystemFileCacheInformationEx = 81,
	MaxSystemInfoClass = 82  // MaxSystemInfoClass should always be the last enum
} SYSTEM_INFORMATION_CLASS;

typedef enum _PROCESSINFOCLASS
{
	ProcessBasicInformation = 0x00,
	ProcessQuotaLimits = 0x01,
	ProcessIoCounters = 0x02,
	ProcessVmCounters = 0x03,
	ProcessTimes = 0x04,
	ProcessBasePriority = 0x05,
	ProcessRaisePriority = 0x06,
	ProcessDebugPort = 0x07,
	ProcessExceptionPort = 0x08,
	ProcessAccessToken = 0x09,
	ProcessLdtInformation = 0x0A,
	ProcessLdtSize = 0x0B,
	ProcessDefaultHardErrorMode = 0x0C,
	ProcessIoPortHandlers = 0x0D,
	ProcessPooledUsageAndLimits = 0x0E,
	ProcessWorkingSetWatch = 0x0F,
	ProcessUserModeIOPL = 0x10,
	ProcessEnableAlignmentFaultFixup = 0x11,
	ProcessPriorityClass = 0x12,
	ProcessWx86Information = 0x13,
	ProcessHandleCount = 0x14,
	ProcessAffinityMask = 0x15,
	ProcessPriorityBoost = 0x16,
	ProcessDeviceMap = 0x17,
	ProcessSessionInformation = 0x18,
	ProcessForegroundInformation = 0x19,
	ProcessWow64Information = 0x1A,
	ProcessImageFileName = 0x1B,
	ProcessLUIDDeviceMapsEnabled = 0x1C,
	ProcessBreakOnTermination = 0x1D,
	ProcessDebugObjectHandle = 0x1E,
	ProcessDebugFlags = 0x1F,
	ProcessHandleTracing = 0x20,
	ProcessIoPriority = 0x21,
	ProcessExecuteFlags = 0x22,
	ProcessResourceManagement = 0x23,
	ProcessCookie = 0x24,
	ProcessImageInformation = 0x25,
	ProcessCycleTime = 0x26,
	ProcessPagePriority = 0x27,
	ProcessInstrumentationCallback = 0x28,
	ProcessThreadStackAllocation = 0x29,
	ProcessWorkingSetWatchEx = 0x2A,
	ProcessImageFileNameWin32 = 0x2B,
	ProcessImageFileMapping = 0x2C,
	ProcessAffinityUpdateMode = 0x2D,
	ProcessMemoryAllocationMode = 0x2E,
	ProcessGroupInformation = 0x2F,
	ProcessTokenVirtualizationEnabled = 0x30,
	ProcessConsoleHostProcess = 0x31,
	ProcessWindowInformation = 0x32,
	ProcessHandleInformation = 0x33,
	ProcessMitigationPolicy = 0x34,
	ProcessDynamicFunctionTableInformation = 0x35,
	ProcessHandleCheckingMode = 0x36,
	ProcessKeepAliveCount = 0x37,
	ProcessRevokeFileHandles = 0x38,
	ProcessWorkingSetControl = 0x39,
	ProcessHandleTable = 0x3A,
	ProcessCheckStackExtentsMode = 0x3B,
	ProcessCommandLineInformation = 0x3C,
	ProcessProtectionInformation = 0x3D,
	ProcessMemoryExhaustion = 0x3E,
	ProcessFaultInformation = 0x3F,
	ProcessTelemetryIdInformation = 0x40,
	ProcessCommitReleaseInformation = 0x41,
	ProcessDefaultCpuSetsInformation = 0x42,
	ProcessAllowedCpuSetsInformation = 0x43,
	ProcessSubsystemProcess = 0x44,
	ProcessJobMemoryInformation = 0x45,
	ProcessInPrivate = 0x46,
	ProcessRaiseUMExceptionOnInvalidHandleClose = 0x47,
	ProcessIumChallengeResponse = 0x48,
	ProcessChildProcessInformation = 0x49,
	ProcessHighGraphicsPriorityInformation = 0x4A,
	ProcessSubsystemInformation = 0x4B,
	ProcessEnergyValues = 0x4C,
	ProcessActivityThrottleState = 0x4D,
	ProcessActivityThrottlePolicy = 0x4E,
	ProcessWin32kSyscallFilterInformation = 0x4F,
	ProcessDisableSystemAllowedCpuSets = 0x50,
	ProcessWakeInformation = 0x51,
	ProcessEnergyTrackingState = 0x52,
	ProcessManageWritesToExecutableMemory = 0x53,
	ProcessCaptureTrustletLiveDump = 0x54,
	ProcessTelemetryCoverage = 0x55,
	ProcessEnclaveInformation = 0x56,
	ProcessEnableReadWriteVmLogging = 0x57,
	ProcessUptimeInformation = 0x58,
	ProcessImageSection = 0x59,
	ProcessDebugAuthInformation = 0x5A,
	ProcessSystemResourceManagement = 0x5B,
	ProcessSequenceNumber = 0x5C,
	ProcessLoaderDetour = 0x5D,
	ProcessSecurityDomainInformation = 0x5E,
	ProcessCombineSecurityDomainsInformation = 0x5F,
	ProcessEnableLogging = 0x60,
	ProcessLeapSecondInformation = 0x61,
	ProcessFiberShadowStackAllocation = 0x62,
	ProcessFreeFiberShadowStackAllocation = 0x63,
	MaxProcessInfoClass = 0x64
} PROCESSINFOCLASS;

typedef struct _SYSTEM_PROCESS_INFORMATION
{
	ULONG NextEntryOffset;
	ULONG NumberOfThreads;
	LARGE_INTEGER WorkingSetPrivateSize;
	ULONG HardFaultCount;
	ULONG NumberOfThreadsHighWatermark;
	ULONGLONG CycleTime;
	LARGE_INTEGER CreateTime;
	LARGE_INTEGER UserTime;
	LARGE_INTEGER KernelTime;
	UNICODE_STRING ImageName;
	LONG BasePriority;
	PVOID UniqueProcessId;
	PVOID InheritedFromUniqueProcessId;
	ULONG HandleCount;
	ULONG SessionId;
	ULONG_PTR UniqueProcessKey;
	ULONG_PTR PeakVirtualSize;
	ULONG_PTR VirtualSize;
	ULONG PageFaultCount;
	ULONG_PTR PeakWorkingSetSize;
	ULONG_PTR WorkingSetSize;
	ULONG_PTR QuotaPeakPagedPoolUsage;
	ULONG_PTR QuotaPagedPoolUsage;
	ULONG_PTR QuotaPeakNonPagedPoolUsage;
	ULONG_PTR QuotaNonPagedPoolUsage;
	ULONG_PTR PagefileUsage;
	ULONG_PTR PeakPagefileUsage;
	ULONG_PTR PrivatePageCount;
	LARGE_INTEGER ReadOperationCount;
	LARGE_INTEGER WriteOperationCount;
	LARGE_INTEGER OtherOperationCount;
	LARGE_INTEGER ReadTransferCount;
	LARGE_INTEGER WriteTransferCount;
	LARGE_INTEGER OtherTransferCount;
} SYSTEM_PROCESS_INFORMATION, * PSYSTEM_PROCESS_INFORMATION;

//
// Thread priority
//

typedef LONG KPRIORITY;

typedef struct _PROCESS_BASIC_INFORMATION
{
	NTSTATUS ExitStatus;
	PPEB PebBaseAddress;
	ULONG_PTR AffinityMask;
	KPRIORITY BasePriority;
	HANDLE UniqueProcessId;
	HANDLE InheritedFromUniqueProcessId;
} PROCESS_BASIC_INFORMATION, * PPROCESS_BASIC_INFORMATION;

typedef struct _PROCESS_BASIC_INFORMATION64
{
	NTSTATUS ExitStatus;
	ULONG64 PebBaseAddress;
	ULONG64 AffinityMask;
	KPRIORITY BasePriority;
	ULONG64 UniqueProcessId;
	ULONG64 InheritedFromUniqueProcessId;
} PROCESS_BASIC_INFORMATION64, * PPROCESS_BASIC_INFORMATION64;

typedef NTSTATUS(NTAPI* NtQueryInformationProcessFn)(
	IN HANDLE			ProcessHandle,
	IN PROCESSINFOCLASS ProcessInformationClass,
	OUT PVOID			ProcessInformation,
	IN ULONG			ProcessInformationLength,
	OUT PULONG			ReturnLength
	);

typedef NTSTATUS(NTAPI* NtQuerySystemInformationFn)(
	IN SYSTEM_INFORMATION_CLASS SystemInformationClass,
	IN PVOID                    SystemInformation,
	IN ULONG                    SystemInformationLength,
	IN PULONG                   ReturnLength
	);

typedef NTSTATUS(NTAPI* NtWriteVirtualMemoryFn)(
	IN HANDLE	 ProcessHandle,
	IN PVOID64	 BaseAddress,
	OUT PVOID	 Buffer,
	IN ULONG64	 Size,
	OUT PULONG64 NumberOfBytesWritten
	);

typedef NTSTATUS(NTAPI* NtWow64AllocateVirtualMemory64Fn)(
	IN  HANDLE   ProcessHandle,
	IN  PULONG64 BaseAddress,
	IN  ULONG64  ZeroBits,
	IN  PULONG64 Size,
	IN  ULONG    AllocationType,
	IN  ULONG    Protection
	);

typedef NTSTATUS(NTAPI* NtWow64ReadVirtualMemory64Fn)(
	IN HANDLE	 ProcessHandle,
	IN PVOID64	 BaseAddress,
	OUT PVOID	 Buffer,
	IN ULONG64	 Size,
	OUT PULONG64 NumberOfBytesRead
	);

typedef NTSTATUS(NTAPI* NtWow64WriteVirtualMemory64Fn)(
	IN HANDLE	 ProcessHandle,
	IN PVOID64	 BaseAddress,
	OUT PVOID	 Buffer,
	IN ULONG64	 Size,
	OUT PULONG64 NumberOfBytesWritten
	);

/*
* NtWow64QueryInformationProcess64
* NtWow64ReadVirtualMemory64
* NtWow64WriteVirtualMemory64
*/

#define ProcessEnvironmentBlock32 ((PPEB)__readfsdword(0x30))
#define ProcessEnvironmentBlock64 ((PPEB)__readgsqword(0x60))

#if _WIN64
#define GetProcessEnvironmentBlock() ProcessEnvironmentBlock64
#define ProcessEnvironmentBlock ProcessEnvironmentBlock64
#else
#define GetProcessEnvironmentBlock() ProcessEnvironmentBlock32
#define ProcessEnvironmentBlock ProcessEnvironmentBlock32
#endif

#endif // !__WINDOWSOPAQUESTRUCTURES_H__