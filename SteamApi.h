#pragma once
#include <stdio.h>

#define k_SteamMusicNameMaxLength 255
#define k_SteamMusicPNGMaxLength 65535

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int int32;
typedef unsigned int uint32;
typedef unsigned long long uint64;

typedef int32 HSteamPipe;
typedef int32 HSteamUser;
typedef int32 HSteamCall;
typedef uint32 HAuthTicket;
typedef uint32 AccountID_t;
typedef uint64 SteamAPICall_t;
typedef uint64 PublishedFileId_t;
typedef uint32 SNetSocket_t;		// CreateP2PConnectionSocket()
typedef uint32 SNetListenSocket_t;	// CreateListenSocket()
typedef uint32 CellID_t;
typedef uint32 AppId_t;
typedef uint32 DepotId_t;
typedef int32 HNewItemRequest;
typedef uint64 ItemID;
typedef uint32 HTTPRequestHandle;
typedef uint32 ScreenshotHandle;
typedef uint64 ClientUnifiedMessageHandle;
typedef uint32 HHTMLBrowser;

const DepotId_t k_uDepotIdInvalid = 0x0;

const AppId_t k_uAppIdInvalid = 0x0;
const AppId_t k_nGameIDNotepad = 65535;
const AppId_t k_nGameIDCSSTestApp = 65534;
const AppId_t k_nGameIDDRMTestApp_Static = 6710;
const AppId_t k_nGameIDDRMTestApp_Blob = 6711;
const AppId_t k_nGameIDDRMTestApp_Dynamic = 6712;
const AppId_t k_nGameIDDRMTestApp_SDK = 6713;
const AppId_t k_nGameIDWinUI = 7;
const AppId_t k_nGameIDWinUI2 = 8;
const AppId_t k_nGameIDCS = 10;
const AppId_t k_nGameIDTFC = 20;
const AppId_t k_nGameIDDOD = 30;
const AppId_t k_nGameIDDMC = 40;
const AppId_t k_nGameIDOpFor = 50;
const AppId_t k_nGameIDRicochet = 60;
const AppId_t k_nGameIDHL1 = 70;
const AppId_t k_nGameIDCZero = 80;
const AppId_t k_nGameIDCSBeta = 150;
const AppId_t k_nGameIDMacVAC = 160;
const AppId_t k_nGameIDWinVAC = 202;
const AppId_t k_nGameIDScreenshots = 760;
const AppId_t k_nGameDRMTest = 199;
const AppId_t k_nGameIDBaseSourceSDK = 215;
const AppId_t k_nGameIDHL2 = 220;
const AppId_t k_nDepotHL2Buka = 235;
const AppId_t k_nGameIDCSS = 240;
const AppId_t k_nGameIDCSSBeta = 260;
const AppId_t k_nGameHL1SRC = 280;
const AppId_t k_nGameIDDODSRC = 300;
const AppId_t k_nGameIDHL2DM = 320;
const AppId_t k_nGameIDPortal = 400;
const AppId_t k_nGameIDHL2EP2 = 420;
const AppId_t k_nGameIDTF2 = 440;
const AppId_t k_nGameIDL4D = 500;
const AppId_t k_nGameIDL4DDemo = 530;
const AppId_t k_nGameIDL4D2 = 550;
const AppId_t k_nGameIDASW = 630;
const AppId_t k_nGameIDTF2Staging = 810;
const AppId_t k_nGameIDPortal2Main = 852;
const AppId_t k_nGameIDPortal2 = 620;
const AppId_t k_nGameIDASWMain = 877;
const AppId_t k_nGameIDDOTA = 882;
const AppId_t k_nGameIDASWStaging = 886;
const AppId_t k_nGameIDRedOrchestra = 1200;
const AppId_t k_nGameIDRedOrchestraBeta = 1210;
const AppId_t k_nGameIDKillingFloor = 1250;
const AppId_t k_nGameIDSin1 = 1309;
const AppId_t k_nGameIDEarth2160 = 1900;
const AppId_t k_nGameIDTheShip = 2400;
const AppId_t k_nGameIDTheShipBeta = 2410;
const AppId_t k_nGameIDDarkMessiahSP = 2100;
const AppId_t k_nGameIDDarkMessiahMPBeta = 2110;
const AppId_t k_nGameIDDarkMessiahMP = 2115;
const AppId_t k_nGameIDDarkMessiahSPDemo = 2120;
const AppId_t k_nGameIDDarkMessiahFix = 2130;
const AppId_t k_nGameRaceWTCC = 4230;
const AppId_t k_nGameIDLostPlanetOld = 6500;
const AppId_t k_nGameIDLostPlanet = 6510;
const AppId_t k_nGameIDNBA2K9 = 7740;
const AppId_t k_nGameIDCallofDuty4 = 7940;
const AppId_t k_nMLBFrontOfficeManager = 7780;
const AppId_t k_nGameIDMW2SP = 10180;
const AppId_t k_nGameIDMW2MP = 10190;
const AppId_t k_nGameIDIW5SP = 42680;
const AppId_t k_nGameIDIW5MP = 42690;
const AppId_t k_nGameIDCODBLOPSSP = 42700;
const AppId_t k_nGameIDCODBLOPSMP = 42710;
const AppId_t k_nGameIDEmpireTotalWar = 10500;
const AppId_t k_nGameCSSOnline = 11600;
const AppId_t k_nGameIDFirstSource = 200;
const AppId_t k_nGameIDLastSource = 999;
const AppId_t k_nGameIDFirstGoldSource = 10;
const AppId_t k_nGameIDLastGoldSource = 199;
const AppId_t k_nGameIDFirstNonSource = 1000;
const AppId_t k_nGameIDMax = 2147483647;
const AppId_t k_nGameIDStress = 30020;
const AppId_t k_nGameIDGCTest = 30100;
const AppId_t k_nAppATIDriver_Vista7_32 = 61800;
const AppId_t k_nAppATIDriver_Vista7_64 = 61810;
const AppId_t k_nAppATIDriver_XP_32 = 61820;
const AppId_t k_nAppATIDriver_XP_64 = 61830;

// RTime32
// We use this 32 bit time representing real world time.
// It offers 1 second resolution beginning on January 1, 1970 (Unix time)
typedef uint32 RTime32;
const RTime32 k_RTime32Nil = 0;
const RTime32 k_RTime32MinValid = 10;
const RTime32 k_RTime32Infinite = 2147483647;

const uint32 INVALID_HTTMLBROWSER = 0;

const CellID_t k_uCellIDInvalid = 0xFFFFFFFF;
const SteamAPICall_t k_uAPICallInvalid = 0;

const HAuthTicket k_HAuthTicketInvalid = 0;

const unsigned int k_unSteamAccountIDMask = 0xFFFFFFFF;
const unsigned int k_unSteamAccountInstanceMask = 0x000FFFFF;
// we allow 3 simultaneous user account instances right now, 1= desktop, 2 = console, 4 = web, 0 = all
const unsigned int k_unSteamUserDesktopInstance = 1;
const unsigned int k_unSteamUserConsoleInstance = 2;
const unsigned int k_unSteamUserWebInstance = 4;

// General result codes
typedef enum EResult
{
	k_EResultOK = 1,										// success
	k_EResultFail = 2,										// generic failure 
	k_EResultNoConnection = 3,								// no/failed network connection
	//	k_EResultNoConnectionRetry = 4,						// OBSOLETE - removed
	k_EResultInvalidPassword = 5,							// password/ticket is invalid
	k_EResultLoggedInElsewhere = 6,							// same user logged in elsewhere
	k_EResultInvalidProtocolVer = 7,						// protocol version is incorrect
	k_EResultInvalidParam = 8,								// a parameter is incorrect
	k_EResultFileNotFound = 9,								// file was not found
	k_EResultBusy = 10,										// called method busy - action not taken
	k_EResultInvalidState = 11,								// called object was in an invalid state
	k_EResultInvalidName = 12,								// name is invalid
	k_EResultInvalidEmail = 13,								// email is invalid
	k_EResultDuplicateName = 14,							// name is not unique
	k_EResultAccessDenied = 15,								// access is denied
	k_EResultTimeout = 16,									// operation timed out
	k_EResultBanned = 17,									// VAC2 banned
	k_EResultAccountNotFound = 18,							// account not found
	k_EResultInvalidSteamID = 19,							// steamID is invalid
	k_EResultServiceUnavailable = 20,						// The requested service is currently unavailable
	k_EResultNotLoggedOn = 21,								// The user is not logged on
	k_EResultPending = 22,									// Request is pending (may be in process, or waiting on third party)
	k_EResultEncryptionFailure = 23,						// Encryption or Decryption failed
	k_EResultInsufficientPrivilege = 24,					// Insufficient privilege
	k_EResultLimitExceeded = 25,							// Too much of a good thing
	k_EResultRevoked = 26,									// Access has been revoked (used for revoked guest passes)
	k_EResultExpired = 27,									// License/Guest pass the user is trying to access is expired
	k_EResultAlreadyRedeemed = 28,							// Guest pass has already been redeemed by account, cannot be acked again
	k_EResultDuplicateRequest = 29,							// The request is a duplicate and the action has already occurred in the past, ignored this time
	k_EResultAlreadyOwned = 30,								// All the games in this guest pass redemption request are already owned by the user
	k_EResultIPNotFound = 31,								// IP address not found
	k_EResultPersistFailed = 32,							// failed to write change to the data store
	k_EResultLockingFailed = 33,							// failed to acquire access lock for this operation
	k_EResultLogonSessionReplaced = 34,
	k_EResultConnectFailed = 35,
	k_EResultHandshakeFailed = 36,
	k_EResultIOFailure = 37,
	k_EResultRemoteDisconnect = 38,
	k_EResultShoppingCartNotFound = 39,						// failed to find the shopping cart requested
	k_EResultBlocked = 40,									// a user didn't allow it
	k_EResultIgnored = 41,									// target is ignoring sender
	k_EResultNoMatch = 42,									// nothing matching the request found
	k_EResultAccountDisabled = 43,
	k_EResultServiceReadOnly = 44,							// this service is not accepting content changes right now
	k_EResultAccountNotFeatured = 45,						// account doesn't have value, so this feature isn't available
	k_EResultAdministratorOK = 46,							// allowed to take this action, but only because requester is admin
	k_EResultContentVersion = 47,							// A Version mismatch in content transmitted within the Steam protocol.
	k_EResultTryAnotherCM = 48,								// The current CM can't service the user making a request, user should try another.
	k_EResultPasswordRequiredToKickSession = 49,			// You are already logged in elsewhere, this cached credential login has failed.
	k_EResultAlreadyLoggedInElsewhere = 50,					// You are already logged in elsewhere, you must wait
	k_EResultSuspended = 51,								// Long running operation (content download) suspended/paused
	k_EResultCancelled = 52,								// Operation canceled (typically by user: content download)
	k_EResultDataCorruption = 53,							// Operation canceled because data is ill formed or unrecoverable
	k_EResultDiskFull = 54,									// Operation canceled - not enough disk space.
	k_EResultRemoteCallFailed = 55,							// an remote call or IPC call failed
	k_EResultPasswordUnset = 56,							// Password could not be verified as it's unset server side
	k_EResultPSNAccountUnlinked = 57,						// Attempt to logon from a PS3 failed because the PSN online id is not linked to a Steam account
	k_EResultPSNTicketInvalid = 58,							// PSN ticket was invalid
	k_EResultPSNAccountAlreadyLinked = 59,					// PSN account is already linked to some other account, must explicitly request to replace/delete the link first
	k_EResultRemoteFileConflict = 60,						// The sync cannot resume due to a conflict between the local and remote files
	k_EResultIllegalPassword = 61,							// The requested new password is not legal
	k_EResultSameAsPreviousValue = 62,						// new value is the same as the old one ( secret question and answer )
	k_EResultAccountLogonDenied = 63,						// account login denied due to 2nd factor authentication failure
	k_EResultCannotUseOldPassword = 64,						// The requested new password is not legal
	k_EResultInvalidLoginAuthCode = 65,						// account login denied due to auth code invalid
	k_EResultAccountLogonDeniedNoMail = 66,					// account login denied due to 2nd factor auth failure - and no mail has been sent
	k_EResultHardwareNotCapableOfIPT = 67,					// 
	k_EResultIPTInitError = 68,								// 
	k_EResultParentalControlRestrictions = 69,				// Operation failed due to parental control restrictions for current user
	k_EResultFacebookQueryError = 70,						// Facebook query returned an error
	k_EResultExpiredLoginAuthCode = 71,						// Expired Login Auth Code
	k_EResultIPLoginRestrictionFailed = 72,					// IP Login Restriction Failed
	k_EResultAccountLockedDown = 73,						// Account Locked Down
	k_EResultAccountLogonDeniedVerifiedEmailRequired = 74,	// Account Logon Denied Verified Email Required
	k_EResultNoMatchingURL = 75,							// No matching URL
	k_EResultBadResponse = 76,								// parse failure, missing field, etc.
	k_EResultRequirePasswordReEntry = 77,					// The user cannot complete the action until they re-enter their password
	k_EResultValueOutOfRange = 78,							// the value entered is outside the acceptable range
	k_EResultUnexpectedError = 79,							// 
	k_EResultFeatureDisabled = 80,							//
	k_EResultInvalidCEGSubmission = 81,						//
	k_EResultRestrictedDevice = 82,							//
	k_EResultRegionLocked = 83,								//
	k_EResultRateLimitExceeded = 84,						//
} EResult;

// Steam API call failure results
typedef enum ESteamAPICallFailure
{
	k_ESteamAPICallFailureNone = -1,			// no failure
	k_ESteamAPICallFailureSteamGone = 0,		// the local Steam process has gone away
	k_ESteamAPICallFailureNetworkFailure = 1,	// the network connection to Steam has been broken, or was already broken
	// SteamServersDisconnected_t callback will be sent around the same time
	// SteamServersConnected_t will be sent when the client is able to talk to the Steam servers again
	k_ESteamAPICallFailureInvalidHandle = 2,	// the SteamAPICall_t handle passed in no longer exists
	k_ESteamAPICallFailureMismatchedCallback = 3,// GetAPICallResult() was called with the wrong callback type for this API call
} ESteamAPICallFailure;

typedef enum EAccountType
{
	k_EAccountTypeInvalid = 0,
	k_EAccountTypeIndividual = 1,		// single user account
	k_EAccountTypeMultiseat = 2,		// multiseat (e.g. cybercafe) account
	k_EAccountTypeGameServer = 3,		// game server account
	k_EAccountTypeAnonGameServer = 4,	// anonymous game server account
	k_EAccountTypePending = 5,			// pending
	k_EAccountTypeContentServer = 6,	// content server
	k_EAccountTypeClan = 7,
	k_EAccountTypeChat = 8,
	k_EAccountTypeConsoleUser = 9,		// Fake SteamID for local PSN account on PS3 or Live account on 360, etc.
	k_EAccountTypeAnonUser = 10,

	// Max of 16 items in this field
	k_EAccountTypeMax
} EAccountType;

// Steam universes.  Each universe is a self-contained Steam instance.
typedef enum EUniverse
{
	k_EUniverseInvalid = 0,
	k_EUniversePublic = 1,
	k_EUniverseBeta = 2,
	k_EUniverseInternal = 3,
	k_EUniverseDev = 4,
	//	k_EUniverseRC = 5, // Removed

	k_EUniverseMax
} EUniverse;


// Error codes for use with the voice functions
typedef enum EVoiceResult
{
	k_EVoiceResultOK = 0,
	k_EVoiceResultNotInitialized = 1,
	k_EVoiceResultNotRecording = 2,
	k_EVoiceResultNoData = 3,
	k_EVoiceResultBufferTooSmall = 4,
	k_EVoiceResultDataCorrupted = 5,
	k_EVoiceResultRestricted = 6,
	k_EVoiceResultUnsupportedCodec = 7,
} EVoiceResult;

//-----------------------------------------------------------------------------
// Purpose: Base values for callback identifiers, each callback must
//			have a unique ID.
//-----------------------------------------------------------------------------
enum ECallbackType
{
	k_iSteamUserCallbacks = 100,
	k_iSteamGameServerCallbacks = 200,
	k_iSteamFriendsCallbacks = 300,
	k_iSteamBillingCallbacks = 400,
	k_iSteamMatchmakingCallbacks = 500,
	k_iSteamContentServerCallbacks = 600,
	k_iSteamUtilsCallbacks = 700,
	k_iClientFriendsCallbacks = 800,
	k_iClientUserCallbacks = 900,
	k_iSteamAppsCallbacks = 1000,
	k_iSteamUserStatsCallbacks = 1100,
	k_iSteamNetworkingCallbacks = 1200,
	k_iClientRemoteStorageCallbacks = 1300,
	k_iSteamUserItemsCallbacks = 1400,
	k_iSteamGameServerItemsCallbacks = 1500,
	k_iClientUtilsCallbacks = 1600,
	k_iSteamGameCoordinatorCallbacks = 1700,
	k_iSteamGameServerStatsCallbacks = 1800,
	k_iSteam2AsyncCallbacks = 1900,
	k_iSteamGameStatsCallbacks = 2000,
	k_iClientHTTPCallbacks = 2100,
	k_iClientScreenshotsCallbacks = 2200,
	k_iSteamScreenshotsCallbacks = 2300,
	k_iClientAudioCallbacks = 2400,
	k_iSteamUnifiedMessagesCallbacks = 2500,
	k_iClientUnifiedMessagesCallbacks = 2600,
	k_iClientControllerCallbacks = 2700,
	k_iSteamControllerCallbacks = 2800,
	k_iClientParentalSettingsCallbacks = 2900,
	k_iClientDeviceAuthCallbacks = 3000,
	k_iClientNetworkDeviceManagerCallbacks = 3100,
	k_iClientMusicCallbacks = 3200,
	k_iClientRemoteClientManagerCallbacks = 3300,
	k_iClientUGCCallbacks = 3400,
	k_iSteamStreamClientCallbacks = 3500,
	k_IClientProductBuilderCallbacks = 3600,
	k_iClientShortcutsCallbacks = 3700,
	k_iClientRemoteControlManagerCallbacks = 3800,
	k_iSteamAppListCallbacks = 3900,
	k_iSteamMusicCallbacks = 4000,
	k_iSteamMusicRemoteCallbacks = 4100,
	k_iClientVRCallbacks = 4200,
	k_iClientReservedCallbacks = 4300,
	k_iSteamReservedCallbacks = 4400,
	k_iSteamHTMLSurfaceCallbacks = 4500,
	k_iClientVideoCallbacks = 4600,
	k_iClientInventoryCallbacks = 4700,
};

enum EAppState
{
	k_EAppStateInvalid = 0,
	k_EAppStateUninstalled = 1,
	k_EAppStateUpdateRequired = 2,
	k_EAppStateFullyInstalled = 4,
	k_EAppStateDataEncrypted = 8,
	k_EAppStateSharedOnly = 64,
	k_EAppStateDataLocked = 16,
	k_EAppStateFilesMissing = 32,
	k_EAppStateFilesCorrupt = 128,
	k_EAppStateAppRunning = 8192,
	k_EAppStateBackupRunning = 4096,
	k_EAppStateUpdateRunning = 256,
	k_EAppStateUpdateStopping = 8388608,
	k_EAppStateUpdatePaused = 512,
	k_EAppStateUpdateStarted = 1024,
	k_EAppStateReconfiguring = 65536,
	k_EAppStateAddingFiles = 262144,
	k_EAppStateDownloading = 1048576,
	k_EAppStateStaging = 2097152,
	k_EAppStateCommitting = 4194304,
	k_EAppStateUninstalling = 2048,
	k_EAppStatePreallocating = 524288,
	k_EAppStateValidating = 131072,
};

typedef enum EAppUsageEvent
{
	k_EAppUsageEventGameLaunch = 1,
	k_EAppUsageEventGameLaunchTrial = 2,
	k_EAppUsageEventMedia = 3,
	k_EAppUsageEventPreloadStart = 4,
	k_EAppUsageEventPreloadFinish = 5,
	k_EAppUsageEventMarketingMessageView = 6,	// deprecated, do not use
	k_EAppUsageEventInGameAdViewed = 7,
	k_EAppUsageEventGameLaunchFreeWeekend = 8,
} EAppUsageEvent;

enum EAppEvent
{
	k_EAppEventDownloadComplete = 2,
};

enum EAppInfoSection
{
	k_EAppInfoSectionUnknown = 0,
	k_EAppInfoSectionAll,
	k_EAppInfoSectionCommon,
	k_EAppInfoSectionExtended,
	k_EAppInfoSectionConfig,
	k_EAppInfoSectionStats,
	k_EAppInfoSectionInstall,
	k_EAppInfoSectionDepots,
	k_EAppInfoSectionVac,
	k_EAppInfoSectionDrm,
	k_EAppInfoSectionUfs,
	k_EAppInfoSectionOgg,
	k_EAppInfoSectionItems,
	k_EAppInfoSectionPolicies,
	k_EAppInfoSectionSysreqs,
	k_EAppInfoSectionCommunity,
	k_EAppInfoSectionStore // store
};

#pragma pack( push, 1 )
struct AppUpdateInfo_s
{
	RTime32 m_timeUpdateStart;
	uint64 m_unBytesToDownload;
	uint64 m_unBytesDownloaded;
	uint64 m_unBytesToProcess;
	uint64 m_unBytesProcessed;
	uint32 m_uUnk;
};
#pragma pack( pop )

struct DownloadStats_s
{
	uint32 m_uIsDownloadEnabled;
	uint32 m_unCurrentConnections;
	uint32 m_unCurrentBytesPerSec;
	uint64 m_unTotalBytesDownload;
	CellID_t m_unCurrentCell;
};

enum EAppDownloadPriority
{
	k_EAppDownloadPriorityNone = 0,
	k_EAppDownloadPriorityFirst = 1,
	k_EAppDownloadPriorityUp = 2,
	k_EAppDownloadPriorityDown = 3,
	k_EAppDownloadPriorityLast = 4,
	k_EAppDownloadPriorityPaused = 5,
};

enum EAppUpdateError
{
	k_EAppErrorNone = 0,
	k_EAppErrorUnspecified = 1,
	k_EAppErrorPaused = 2,
	k_EAppErrorCanceled = 3,
	k_EAppErrorSuspended = 4,
	k_EAppErrorNoSubscription = 5,
	k_EAppErrorNoConnection = 6,
	k_EAppErrorTimeout = 7,
	k_EAppErrorMissingKey = 8,
	k_EAppErrorMissingConfig = 9,
	k_EAppErrorDiskReadFailure = 10,
	k_EAppErrorDiskWriteFailure = 11,
	k_EAppErrorCorruptContent = 13,
	k_EAppErrorWaitingForDisk = 14,
	k_EAppErrorInvalidInstallPath = 15,
	k_EAppErrorApplicationRunning = 16,
	k_EAppErrorDependencyFailure = 17,
	k_EAppErrorNotInstalled = 18,
	k_EAppErrorUpdateRequired = 19,
	k_EAppErrorStillBusy = 20,
	k_EAppErrorNoConnectionToContentServers = 21,
	k_EAppErrorInvalidApplicationConfiguration = 22,
	k_EAppErrorInvalidContentConfiguration = 23,
	k_EAppErrorMissingManifest = 24,
	k_EAppErrorNotReleased = 25,
	k_EAppErrorRegionRestricted = 26,
	k_EAppErrorCorruptDepotCache = 27,
	k_EAppErrorMissingExecutable = 28,
	k_EAppErrorInvalidPlatform = 29,
	k_EAppErrorInvalidFileSystem = 30,
	k_EAppErrorCorruptUpdateFiles = 31,
	k_EAppUpdateErrorDownloadCorrupt = 32,
	k_EAppUpdateErrorDownloadDisabled = 33,
	k_EAppUpdateErrorSharedLibraryLocked = 34,
	k_EAppUpdateErrorPurchasePending = 35,
	k_EAppUpdateErrorOtherSessionPlaying = 36,
};

//-----------------------------------------------------------------------------
// Purpose: possible results when registering an activation code
//-----------------------------------------------------------------------------
enum ERegisterActivactionCodeResult
{
	k_ERegisterActivactionCodeResultOK = 0,
	k_ERegisterActivactionCodeResultFail = 1,
	k_ERegisterActivactionCodeResultAlreadyRegistered = 2,
	k_ERegisterActivactionCodeResultTimeout = 3,
	k_ERegisterActivactionCodeAlreadyOwned = 4
};

enum EAppOwnershipFlags
{
	k_EAppOwnershipFlagsNone = 0,
	k_EAppOwnershipFlagsOwnsLicense = 1 << 0,
	k_EAppOwnershipFlagsFreeLicense = 1 << 1,
	k_EAppOwnershipFlagsRegionRestricted = 1 << 2,
	k_EAppOwnershipFlagsLowViolence = 1 << 3,
	k_EAppOwnershipFlagsInvalidPlatform = 1 << 4,
	k_EAppOwnershipFlagsSharedLicense = 1 << 5,
	k_EAppOwnershipFlagsFreeWeekend = 1 << 6,
	k_EAppOwnershipFlagsLockedLicense = 1 << 7,
	k_EAppOwnershipFlagsPending = 1 << 8,
	k_EAppOwnershipFlagsExpired = 1 << 9,
	k_EAppOwnershipFlagsPermanent = 1 << 10,
	k_EAppOwnershipFlagsRecurring = 1 << 11,
	k_EAppOwnershipFlagsCanceled = 8192,    // Canceled
	k_EAppOwnershipFlagsAutoGrant = 16384,  // Auto Grant
};

enum EAppReleaseState
{
	k_EAppReleaseStateUnknown = 0,
	k_EAppReleaseStateUnavailable,
	k_EAppReleaseStatePrerelease,
	k_EAppReleaseStatePreloadonly,
	k_EAppReleaseStateReleased,
};

enum EAppAutoUpdateBehavior
{
	// TODO: Reverse this enum
};

enum EAppAllowDownloadsWhileRunningBehavior
{
	// TODO: Reverse this enum
};

enum EAppDownloadQueuePlacement
{
	k_EAppDownloadQueuePlacementPriorityNone = 0,   // Priority None
	k_EAppDownloadQueuePlacementPriorityFirst,      // Priority First
	k_EAppDownloadQueuePlacementPriorityUp, // Priority Up
	k_EAppDownloadQueuePlacementPriorityDown,       // Priority Down
	k_EAppDownloadQueuePlacementPriorityLast,       // Priority Last
	k_EAppDownloadQueuePlacementPriorityPaused,     // Priority Paused
};

struct SHADigestWrapper_t
{
	uint32 A;
	uint32 B;
	uint32 C;
	uint32 D;
	uint32 E;
};

const int k_cubAppProofOfPurchaseKeyMax = 64;			// max bytes of a legacy cd key we support

// Special flags for Chat accounts - they go in the top 8 bits
// of the steam ID's "instance", leaving 12 for the actual instances
enum EChatSteamIDInstanceFlags
{
	k_EChatAccountInstanceMask = 0x00000FFF, // top 8 bits are flags

	k_EChatInstanceFlagClan = (k_unSteamAccountInstanceMask + 1) >> 1,	// top bit
	k_EChatInstanceFlagLobby = (k_unSteamAccountInstanceMask + 1) >> 2,	// next one down, etc
	k_EChatInstanceFlagMMSLobby = (k_unSteamAccountInstanceMask + 1) >> 3,	// next one down, etc

	// Max of 8 flags
};

enum EBeginAuthSessionResult;
enum EUserHasLicenseForAppResult;

//-----------------------------------------------------------------------------
// Purpose: Chat Entry Types (previously was only friend-to-friend message types)
//-----------------------------------------------------------------------------
enum EChatEntryType
{
	k_EChatEntryTypeInvalid = 0,
	k_EChatEntryTypeChatMsg = 1,		// Normal text message from another user
	k_EChatEntryTypeTyping = 2,			// Another user is typing (not used in multi-user chat)
	k_EChatEntryTypeInviteGame = 3,		// Invite from other user into that users current game
	k_EChatEntryTypeEmote = 4,			// text emote message
	k_EChatEntryTypeLobbyGameStart = 5,	// lobby game is starting
	k_EChatEntryTypeLeftConversation = 6, // user has left the conversation ( closed chat window )
	// Above are previous FriendMsgType entries, now merged into more generic chat entry types
};

// Every individual instance of an item has a globally-unique ItemInstanceID.
// This ID is unique to the combination of (player, specific item instance)
// and will not be transferred to another player or re-used for another item.
typedef uint64 SteamItemInstanceID_t;

static const SteamItemInstanceID_t k_SteamItemInstanceIDInvalid = ~(SteamItemInstanceID_t)0;

// Types of items in your game are identified by a 32-bit "item definition number".
// Valid definition numbers are between 1 and 999999999; numbers less than or equal to
// zero are invalid, and numbers greater than or equal to one billion (1x10^9) are
// reserved for internal Steam use.
typedef int32 SteamItemDef_t;

enum ESteamItemFlags
{
	// Item status flags - these flags are permenantly attached to specific item instances
	k_ESteamItemNoTrade = 1 << 0, // This item is account-locked and cannot be traded or given away.

	// Action confirmation flags - these flags are set one time only, as part of a result set
	k_ESteamItemRemoved = 1 << 8,	// The item has been destroyed, traded away, expired, or otherwise invalidated
	k_ESteamItemConsumed = 1 << 9,	// The item quantity has been decreased by 1 via ConsumeItem API.

	// All other flag bits are currently reserved for internal Steam use at this time.
	// Do not assume anything about the state of other flags which are not defined here.
};

struct SteamItemDetails_t
{
	SteamItemInstanceID_t m_itemId;
	SteamItemDef_t m_iDefinition;
	uint16 m_unQuantity;
	uint16 m_unFlags; // see ESteamItemFlags
};

typedef int32 SteamInventoryResult_t;

static const SteamInventoryResult_t k_SteamInventoryResultInvalid = -1;

// SteamInventoryResultReady_t callbacks are fired whenever asynchronous
// results transition from "Pending" to "OK" or an error state. There will
// always be exactly one callback per handle.
struct SteamInventoryResultReady_t
{
	enum { k_iCallback = k_iClientInventoryCallbacks + 0 };
	SteamInventoryResult_t m_handle;
	EResult m_result;
};


// SteamInventoryFullUpdate_t callbacks are triggered when GetAllItems
// successfully returns a result which is newer / fresher than the last
// known result. (It will not trigger if the inventory hasn't changed,
// or if results from two overlapping calls are reversed in flight and
// the earlier result is already known to be stale/out-of-date.)
// The normal ResultReady callback will still be triggered immediately
// afterwards; this is an additional notification for your convenience.
struct SteamInventoryFullUpdate_t
{
	enum { k_iCallback = k_iClientInventoryCallbacks + 1 };
	SteamInventoryResult_t m_handle;
};


// A SteamInventoryDefinitionUpdate_t callback is triggered whenever
// item definitions have been updated, which could be in response to 
// LoadItemDefinitions() or any other async request which required
// a definition update in order to process results from the server.
struct SteamInventoryDefinitionUpdate_t
{
	enum { k_iCallback = k_iClientInventoryCallbacks + 2 };
};

enum AudioPlayback_Status
{
	AudioPlayback_Undefined = 0,
	AudioPlayback_Playing = 1,
	AudioPlayback_Paused = 2,
	AudioPlayback_Idle = 3
};

enum EVRScreenshotType { };

enum ELobbyComparison
{
	k_ELobbyComparisonEqualToOrLessThan = -2,
	k_ELobbyComparisonLessThan = -1,
	k_ELobbyComparisonEqual = 0,
	k_ELobbyComparisonGreaterThan = 1,
	k_ELobbyComparisonEqualToOrGreaterThan = 2,
	k_ELobbyComparisonNotEqual = 3,
};

// lobby search distance
enum ELobbyDistanceFilter
{
	k_ELobbyDistanceFilterClose,		// only lobbies in the same immediate region will be returned
	k_ELobbyDistanceFilterDefault,		// only lobbies in the same region or close, but looking further if the current region has infrequent lobby activity (the default)
	k_ELobbyDistanceFilterFar,			// for games that don't have many latency requirements, will return lobbies about half-way around the globe
	k_ELobbyDistanceFilterWorldwide,	// no filtering, will match lobbies as far as India to NY (not recommended, expect multiple seconds of latency between the clients)
};

// lobby type description
enum ELobbyType
{
	k_ELobbyTypePrivate = 0,		// only way to join the lobby is to invite to someone else
	k_ELobbyTypeFriendsOnly = 1,	// shows for friends or invitees, but not in lobby list
	k_ELobbyTypePublic = 2,			// visible for friends and in lobby list
	k_ELobbyTypeInvisible = 3,		// returned by search, but not visible to other friends 
	//    useful if you want a user in two lobbies, for example matching groups together
	//	  a user can be in only one regular lobby, and up to two invisible lobbies
};

enum EGamepadTextInputMode
{
	k_EGamepadTextInputModeNormal,
	k_EGamepadTextInputModePassword
};

enum EGamepadTextInputLineMode
{
	k_EGamepadTextInputLineModeSingleLine,
	k_EGamepadTextInputLineModeMultipleLines
};

enum ENotificationPosition
{
	k_EPositionTopLeft,
	k_EPositionTopRight,
	k_EPositionBottomLeft,
	k_EPositionBottomRight
};

enum EHTMLMouseButton
{
	eHTMLMouseButton_Left = 0,
	eHTMLMouseButton_Right = 1,
	eHTMLMouseButton_Middle = 2,
};

enum EHTMLKeyModifiers
{
	eHTMLKeyModifier_None = 0,
	eHTMLKeyModifier_AltDown = 1 << 0,
	eHTMLKeyModifier_CrtlDown = 1 << 1,
	eHTMLKeyModifier_ShiftDown = 1 << 2,
};

enum EP2PSend
{
	// Basic UDP send. Packets can't be bigger than 1200 bytes (your typical MTU size). Can be lost, or arrive out of order (rare).
	// The sending API does have some knowledge of the underlying connection, so if there is no NAT-traversal accomplished or
	// there is a recognized adjustment happening on the connection, the packet will be batched until the connection is open again.
	k_EP2PSendUnreliable = 0,

	// As above, but if the underlying p2p connection isn't yet established the packet will just be thrown away. Using this on the first
	// packet sent to a remote host almost guarantees the packet will be dropped.
	// This is only really useful for kinds of data that should never buffer up, i.e. voice payload packets
	k_EP2PSendUnreliableNoDelay = 1,

	// Reliable message send. Can send up to 1MB of data in a single message. 
	// Does fragmentation/re-assembly of messages under the hood, as well as a sliding window for efficient sends of large chunks of data. 
	k_EP2PSendReliable = 2,

	// As above, but applies the Nagle algorithm to the send - sends will accumulate 
	// until the current MTU size (typically ~1200 bytes, but can change) or ~200ms has passed (Nagle algorithm). 
	// Useful if you want to send a set of smaller messages but have the coalesced into a single packet
	// Since the reliable stream is all ordered, you can do several small message sends with k_EP2PSendReliableWithBuffering and then
	// do a normal k_EP2PSendReliable to force all the buffered data to be sent.
	k_EP2PSendReliableWithBuffering = 3,

};

// describes how the socket is currently connected
enum ESNetSocketConnectionType
{
	k_ESNetSocketConnectionTypeNotConnected = 0,
	k_ESNetSocketConnectionTypeUDP = 1,
	k_ESNetSocketConnectionTypeUDPRelay = 2,
};

struct P2PSessionState_t
{
	uint8 m_bConnectionActive;		// true if we've got an active open connection
	uint8 m_bConnecting;			// true if we're currently trying to establish a connection
	uint8 m_eP2PSessionError;		// last error recorded (see enum above)
	uint8 m_bUsingRelay;			// true if it's going through a relay server (TURN)
	int32 m_nBytesQueuedForSend;
	int32 m_nPacketsQueuedForSend;
	uint32 m_nRemoteIP;				// potential IP:Port of remote host. Could be TURN server. 
	uint16 m_nRemotePort;			// Only exists for compatibility with older authentication api's
};

#pragma pack( push, 1 )		

//-----------------------------------------------------------------------------
// Purpose: encapsulates an appID/modID pair
//-----------------------------------------------------------------------------
class CGameID
{
public:

	CGameID()
	{
		m_gameID.m_nType = k_EGameIDTypeApp;
		m_gameID.m_nAppID = k_uAppIdInvalid;
		m_gameID.m_nModID = 0;
	}

	//explicit CGameID(uint64 ulGameID)
	//{
	//	m_ulGameID = ulGameID;
	//}

	//explicit CGameID(int32 nAppID)
	//{
	//	m_ulGameID = 0;
	//	m_gameID.m_nAppID = nAppID;
	//}

	//explicit CGameID(uint32 nAppID)
	//{
	//	m_ulGameID = 0;
	//	m_gameID.m_nAppID = nAppID;
	//}

	//CGameID(uint32 nAppID, uint32 nModID)
	//{
	//	m_ulGameID = 0;
	//	m_gameID.m_nAppID = nAppID;
	//	m_gameID.m_nModID = nModID;
	//	m_gameID.m_nType = k_EGameIDTypeGameMod;
	//}

	// Hidden functions used only by Steam
	explicit CGameID(const char *pchGameID);
	const char *Render() const;					// render this Game ID to string
	static const char *Render(uint64 ulGameID);		// static method to render a uint64 representation of a Game ID to a string

	// must include checksum_crc.h first to get this functionality
#if defined( CHECKSUM_CRC_H )
	CGameID(uint32 nAppID, const char *pchModPath)
	{
		m_ulGameID = 0;
		m_gameID.m_nAppID = nAppID;
		m_gameID.m_nType = k_EGameIDTypeGameMod;

		char rgchModDir[MAX_PATH];
		Q_FileBase(pchModPath, rgchModDir, sizeof(rgchModDir));
		CRC32_t crc32;
		CRC32_Init(&crc32);
		CRC32_ProcessBuffer(&crc32, rgchModDir, Q_strlen(rgchModDir));
		CRC32_Final(&crc32);

		// set the high-bit on the mod-id 
		// reduces crc32 to 31bits, but lets us use the modID as a guaranteed unique
		// replacement for appID's
		m_gameID.m_nModID = crc32 | (0x80000000);
	}

	CGameID(const char *pchExePath, const char *pchAppName)
	{
		m_ulGameID = 0;
		m_gameID.m_nAppID = k_uAppIdInvalid;
		m_gameID.m_nType = k_EGameIDTypeShortcut;

		CRC32_t crc32;
		CRC32_Init(&crc32);
		CRC32_ProcessBuffer(&crc32, pchExePath, Q_strlen(pchExePath));
		CRC32_ProcessBuffer(&crc32, pchAppName, Q_strlen(pchAppName));
		CRC32_Final(&crc32);

		// set the high-bit on the mod-id 
		// reduces crc32 to 31bits, but lets us use the modID as a guaranteed unique
		// replacement for appID's
		m_gameID.m_nModID = crc32 | (0x80000000);
	}

#if defined( VSTFILEID_H )

	CGameID(VstFileID vstFileID)
	{
		m_ulGameID = 0;
		m_gameID.m_nAppID = k_uAppIdInvalid;
		m_gameID.m_nType = k_EGameIDTypeP2P;

		CRC32_t crc32;
		CRC32_Init(&crc32);
		const char *pchFileId = vstFileID.Render();
		CRC32_ProcessBuffer(&crc32, pchFileId, Q_strlen(pchFileId));
		CRC32_Final(&crc32);

		// set the high-bit on the mod-id 
		// reduces crc32 to 31bits, but lets us use the modID as a guaranteed unique
		// replacement for appID's
		m_gameID.m_nModID = crc32 | (0x80000000);
	}

#endif /* VSTFILEID_H */

#endif /* CHECKSUM_CRC_H */


	uint64 ToUint64() const
	{
		return m_ulGameID;
	}

	uint64 *GetUint64Ptr()
	{
		return &m_ulGameID;
	}

	bool IsMod() const
	{
		return (m_gameID.m_nType == k_EGameIDTypeGameMod);
	}

	bool IsShortcut() const
	{
		return (m_gameID.m_nType == k_EGameIDTypeShortcut);
	}

	bool IsP2PFile() const
	{
		return (m_gameID.m_nType == k_EGameIDTypeP2P);
	}

	bool IsSteamApp() const
	{
		return (m_gameID.m_nType == k_EGameIDTypeApp);
	}

	uint32 ModID() const
	{
		return m_gameID.m_nModID;
	}

	uint32 AppID() const
	{
		return m_gameID.m_nAppID;
	}

	bool operator == (const CGameID &rhs) const
	{
		return m_ulGameID == rhs.m_ulGameID;
	}

	bool operator != (const CGameID &rhs) const
	{
		return !(*this == rhs);
	}

	bool operator < (const CGameID &rhs) const
	{
		return (m_ulGameID < rhs.m_ulGameID);
	}

	bool IsValid() const
	{
		// each type has it's own invalid fixed point:
		switch (m_gameID.m_nType)
		{
		case k_EGameIDTypeApp:
			return m_gameID.m_nAppID != k_uAppIdInvalid;
			break;
		case k_EGameIDTypeGameMod:
			return m_gameID.m_nAppID != k_uAppIdInvalid && m_gameID.m_nModID & 0x80000000;
			break;
		case k_EGameIDTypeShortcut:
			return (m_gameID.m_nModID & 0x80000000) != 0;
			break;
		case k_EGameIDTypeP2P:
			return m_gameID.m_nAppID == k_uAppIdInvalid && m_gameID.m_nModID & 0x80000000;
			break;
		default:
#if defined(Assert)
			Assert(false);
#endif
			return false;
		}

	}

	void Reset()
	{
		m_ulGameID = 0;
	}



private:

	enum EGameIDType
	{
		k_EGameIDTypeApp = 0,
		k_EGameIDTypeGameMod = 1,
		k_EGameIDTypeShortcut = 2,
		k_EGameIDTypeP2P = 3,
	};

	struct GameID_t
	{
#ifdef VALVE_BIG_ENDIAN
		unsigned int m_nModID : 32;
		unsigned int m_nType : 8;
		unsigned int m_nAppID : 24;
#else
		unsigned int m_nAppID : 24;
		unsigned int m_nType : 8;
		unsigned int m_nModID : 32;
#endif
	};

	union
	{
		uint64 m_ulGameID;
		GameID_t m_gameID;
	};
};

#pragma pack( pop )


//-----------------------------------------------------------------------------
// Purpose: called when new information about an app has arrived
//-----------------------------------------------------------------------------
struct AppDataChanged_t
{
	enum { k_iCallback = k_iSteamAppsCallbacks + 1 };

	AppId_t m_nAppID;

	bool m_bBySteamUI;
	bool m_bCDDBUpdate;
};

struct RequestAppCallbacksComplete_t
{
	enum { k_iCallback = k_iSteamAppsCallbacks + 2 };
};

struct AppInfoUpdateComplete_t
{
	enum { k_iCallback = k_iSteamAppsCallbacks + 3 };

	EResult m_EResult;
	uint32 m_cAppsUpdated;
	bool m_bSteam2CDDBChanged;
};

struct AppEventTriggered_t
{
	enum { k_iCallback = k_iSteamAppsCallbacks + 4 };

	AppId_t m_nAppID;
	EAppEvent m_eAppEvent;
};

//-----------------------------------------------------------------------------
// Purpose: posted after the user gains ownership of DLC & that DLC is installed
//-----------------------------------------------------------------------------
struct DlcInstalled_t
{
	enum { k_iCallback = k_iSteamAppsCallbacks + 5 };

	AppId_t m_nAppID;		// AppID of the DLC
};

struct AppEventStateChange_t
{
	enum { k_iCallback = k_iSteamAppsCallbacks + 6 };

	AppId_t m_nAppID;
	uint32 m_eOldState;
	uint32 m_eNewState;
	EAppUpdateError m_eAppError;
};

struct AppValidationComplete_t
{
	enum { k_iCallback = k_iSteamAppsCallbacks + 7 };

	AppId_t m_nAppID;
	bool m_bFinished;

	uint64 m_TotalBytesValidated;
	uint64 m_TotalBytesFailed;
	uint32 m_TotalFilesValidated;
	uint32 m_TotalFilesFailed;
	uint32 m_TotalFilesFailedCEGFiles;
};

//-----------------------------------------------------------------------------
// Purpose: response to RegisterActivationCode()
//-----------------------------------------------------------------------------
struct RegisterActivationCodeResponse_t
{
	enum { k_iCallback = k_iSteamAppsCallbacks + 8 };

	ERegisterActivactionCodeResult m_eResult;
	uint32 m_unPackageRegistered;						// package that was registered. Only set on success
};

struct DownloadScheduleChanged_t
{
	enum { k_iCallback = k_iSteamAppsCallbacks + 9 };

	bool m_bDownloadEnabled;
	uint32 m_nTotalAppsScheduled;
	unsigned int m_rgunAppSchedule[32];
};

struct DlcInstallRequest_t
{
	enum { k_iCallback = k_iSteamAppsCallbacks + 10 };

	AppId_t m_nAppID;
	bool m_bInstall;
};

struct AppLaunchTenFootOverlay_t
{
	enum { k_iCallback = k_iSteamAppsCallbacks + 11 };

	CGameID m_GameID;
	uint64 m_nPid;
	bool m_bCanShareSurfaces;
};

struct AppBackupStatus_t
{
	enum { k_iCallback = k_iSteamAppsCallbacks + 12 };

	AppId_t m_nAppID;
	EResult m_eResult;

	uint64 m_unBytesToProcess;
	uint64 m_unBytesProcessed;
	uint64 m_unTotalBytesWritten;
	uint64 m_unBytesFailed;
};

struct RequestAppProofOfPurchaseKeyResponse_t
{
	enum { k_iCallback = k_iSteamAppsCallbacks + 13 };

	EResult m_eResult;
	AppId_t m_nAppID;
	char m_rgchKey[k_cubAppProofOfPurchaseKeyMax];
};

#pragma pack( push, 1 )

// Steam ID structure (64 bits total)
class CSteamID
{
public:

	//-----------------------------------------------------------------------------
	// Purpose: Constructor
	//-----------------------------------------------------------------------------
	CSteamID()
	{
		m_steamid.m_comp.m_unAccountID = 0;
		m_steamid.m_comp.m_EAccountType = k_EAccountTypeInvalid;
		m_steamid.m_comp.m_EUniverse = k_EUniverseInvalid;
		m_steamid.m_comp.m_unAccountInstance = 0;
	}


	//-----------------------------------------------------------------------------
	// Purpose: Constructor
	// Input  : unAccountID -	32-bit account ID
	//			eUniverse -		Universe this account belongs to
	//			eAccountType -	Type of account
	//-----------------------------------------------------------------------------
	CSteamID(uint32 unAccountID, EUniverse eUniverse, EAccountType eAccountType)
	{
		Set(unAccountID, eUniverse, eAccountType);
	}


	//-----------------------------------------------------------------------------
	// Purpose: Constructor
	// Input  : unAccountID -	32-bit account ID
	//			unAccountInstance - instance
	//			eUniverse -		Universe this account belongs to
	//			eAccountType -	Type of account
	//-----------------------------------------------------------------------------
	CSteamID(uint32 unAccountID, unsigned int unAccountInstance, EUniverse eUniverse, EAccountType eAccountType)
	{
#if defined(_SERVER) && defined(Assert)
		Assert(!((k_EAccountTypeIndividual == eAccountType) && (unAccountInstance > k_unSteamUserWebInstance)));	// enforce that for individual accounts, instance is always 1
#endif // _SERVER
		InstancedSet(unAccountID, unAccountInstance, eUniverse, eAccountType);
	}


	//-----------------------------------------------------------------------------
	// Purpose: Constructor
	// Input  : ulSteamID -		64-bit representation of a Steam ID
	// Note:	Will not accept a uint32 or int32 as input, as that is a probable mistake.
	//			See the stubbed out overloads in the private: section for more info.
	//-----------------------------------------------------------------------------
	CSteamID(uint64 ulSteamID)
	{
		SetFromUint64(ulSteamID);
	}


	//-----------------------------------------------------------------------------
	// Purpose: Sets parameters for steam ID
	// Input  : unAccountID -	32-bit account ID
	//			eUniverse -		Universe this account belongs to
	//			eAccountType -	Type of account
	//-----------------------------------------------------------------------------
	void Set(uint32 unAccountID, EUniverse eUniverse, EAccountType eAccountType)
	{
		m_steamid.m_comp.m_unAccountID = unAccountID;
		m_steamid.m_comp.m_EUniverse = eUniverse;
		m_steamid.m_comp.m_EAccountType = eAccountType;

		if (eAccountType == k_EAccountTypeClan)
		{
			m_steamid.m_comp.m_unAccountInstance = 0;
		}
		else
		{
			// by default we pick the desktop instance
			m_steamid.m_comp.m_unAccountInstance = k_unSteamUserDesktopInstance;
		}
	}


	//-----------------------------------------------------------------------------
	// Purpose: Sets parameters for steam ID
	// Input  : unAccountID -	32-bit account ID
	//			eUniverse -		Universe this account belongs to
	//			eAccountType -	Type of account
	//-----------------------------------------------------------------------------
	void InstancedSet(uint32 unAccountID, uint32 unInstance, EUniverse eUniverse, EAccountType eAccountType)
	{
		m_steamid.m_comp.m_unAccountID = unAccountID;
		m_steamid.m_comp.m_EUniverse = eUniverse;
		m_steamid.m_comp.m_EAccountType = eAccountType;
		m_steamid.m_comp.m_unAccountInstance = unInstance;
	}


	//-----------------------------------------------------------------------------
	// Purpose: Initializes a steam ID from its 52 bit parts and universe/type
	// Input  : ulIdentifier - 52 bits of goodness
	//-----------------------------------------------------------------------------
	void FullSet(uint64 ulIdentifier, EUniverse eUniverse, EAccountType eAccountType)
	{
		m_steamid.m_comp.m_unAccountID = (ulIdentifier & k_unSteamAccountIDMask);						// account ID is low 32 bits
		m_steamid.m_comp.m_unAccountInstance = ((ulIdentifier >> 32) & k_unSteamAccountInstanceMask);			// account instance is next 20 bits
		m_steamid.m_comp.m_EUniverse = eUniverse;
		m_steamid.m_comp.m_EAccountType = eAccountType;
	}


	//-----------------------------------------------------------------------------
	// Purpose: Initializes a steam ID from its 64-bit representation
	// Input  : ulSteamID -		64-bit representation of a Steam ID
	//-----------------------------------------------------------------------------
	void SetFromUint64(uint64 ulSteamID)
	{
		m_steamid.m_unAll64Bits = ulSteamID;
	}

#ifdef TSTEAMGLOBALUSERID_H // NO_STEAM would not include TSteamGlobalUserID.h, thus we lose this facet of CSteamID
	//-----------------------------------------------------------------------------
	// Purpose: Initializes a steam ID from a Steam2 ID structure
	// Input:	pTSteamGlobalUserID -	Steam2 ID to convert
	//			eUniverse -				universe this ID belongs to
	//-----------------------------------------------------------------------------
	void SetFromSteam2(TSteamGlobalUserID *pTSteamGlobalUserID, EUniverse eUniverse)
	{
		m_steamid.m_comp.m_unAccountID = pTSteamGlobalUserID->m_SteamLocalUserID.Split.Low32bits * 2 +
			pTSteamGlobalUserID->m_SteamLocalUserID.Split.High32bits;
		m_steamid.m_comp.m_EUniverse = eUniverse;		// set the universe
		m_steamid.m_comp.m_EAccountType = k_EAccountTypeIndividual; // Steam 2 accounts always map to account type of individual
		m_steamid.m_comp.m_unAccountInstance = k_unSteamUserDesktopInstance; // Steam2 only knew desktop instances
	}

	//-----------------------------------------------------------------------------
	// Purpose: Fills out a Steam2 ID structure
	// Input:	pTSteamGlobalUserID -	Steam2 ID to write to
	//-----------------------------------------------------------------------------
	void ConvertToSteam2(TSteamGlobalUserID *pTSteamGlobalUserID) const
	{
		// only individual accounts have any meaning in Steam 2, only they can be mapped
		// Assert( m_steamid.m_comp.m_EAccountType == k_EAccountTypeIndividual );

		pTSteamGlobalUserID->m_SteamInstanceID = 0;
		pTSteamGlobalUserID->m_SteamLocalUserID.Split.High32bits = m_steamid.m_comp.m_unAccountID % 2;
		pTSteamGlobalUserID->m_SteamLocalUserID.Split.Low32bits = m_steamid.m_comp.m_unAccountID / 2;
	}
#endif // TSTEAMGLOBALUSERID_H

	//-----------------------------------------------------------------------------
	// Purpose: Converts steam ID to its 64-bit representation
	// Output : 64-bit representation of a Steam ID
	//-----------------------------------------------------------------------------
	uint64 ConvertToUint64() const
	{
		return m_steamid.m_unAll64Bits;
	}


	//-----------------------------------------------------------------------------
	// Purpose: Converts the static parts of a steam ID to a 64-bit representation.
	//			For multiseat accounts, all instances of that account will have the
	//			same static account key, so they can be grouped together by the static
	//			account key.
	// Output : 64-bit static account key
	//-----------------------------------------------------------------------------
	uint64 GetStaticAccountKey() const
	{
		// note we do NOT include the account instance (which is a dynamic property) in the static account key
		return (uint64)((((uint64)m_steamid.m_comp.m_EUniverse) << 56) + ((uint64)m_steamid.m_comp.m_EAccountType << 52) + m_steamid.m_comp.m_unAccountID);
	}


	//-----------------------------------------------------------------------------
	// Purpose: create an anonymous game server login to be filled in by the AM
	//-----------------------------------------------------------------------------
	void CreateBlankAnonLogon(EUniverse eUniverse)
	{
		m_steamid.m_comp.m_unAccountID = 0;
		m_steamid.m_comp.m_EAccountType = k_EAccountTypeAnonGameServer;
		m_steamid.m_comp.m_EUniverse = eUniverse;
		m_steamid.m_comp.m_unAccountInstance = 0;
	}


	//-----------------------------------------------------------------------------
	// Purpose: create an anonymous game server login to be filled in by the AM
	//-----------------------------------------------------------------------------
	void CreateBlankAnonUserLogon(EUniverse eUniverse)
	{
		m_steamid.m_comp.m_unAccountID = 0;
		m_steamid.m_comp.m_EAccountType = k_EAccountTypeAnonUser;
		m_steamid.m_comp.m_EUniverse = eUniverse;
		m_steamid.m_comp.m_unAccountInstance = 0;
	}

	//-----------------------------------------------------------------------------
	// Purpose: Is this an anonymous game server login that will be filled in?
	//-----------------------------------------------------------------------------
	bool BBlankAnonAccount() const
	{
		return m_steamid.m_comp.m_unAccountID == 0 && BAnonAccount() && m_steamid.m_comp.m_unAccountInstance == 0;
	}

	//-----------------------------------------------------------------------------
	// Purpose: Is this a game server account id?
	//-----------------------------------------------------------------------------
	bool BGameServerAccount() const
	{
		return m_steamid.m_comp.m_EAccountType == k_EAccountTypeGameServer || m_steamid.m_comp.m_EAccountType == k_EAccountTypeAnonGameServer;
	}

	//-----------------------------------------------------------------------------
	// Purpose: Is this a content server account id?
	//-----------------------------------------------------------------------------
	bool BContentServerAccount() const
	{
		return m_steamid.m_comp.m_EAccountType == k_EAccountTypeContentServer;
	}


	//-----------------------------------------------------------------------------
	// Purpose: Is this a clan account id?
	//-----------------------------------------------------------------------------
	bool BClanAccount() const
	{
		return m_steamid.m_comp.m_EAccountType == k_EAccountTypeClan;
	}


	//-----------------------------------------------------------------------------
	// Purpose: Is this a chat account id?
	//-----------------------------------------------------------------------------
	bool BChatAccount() const
	{
		return m_steamid.m_comp.m_EAccountType == k_EAccountTypeChat;
	}

	//-----------------------------------------------------------------------------
	// Purpose: Is this a chat account id?
	//-----------------------------------------------------------------------------
	bool IsLobby() const
	{
		return (m_steamid.m_comp.m_EAccountType == k_EAccountTypeChat)
			&& (m_steamid.m_comp.m_unAccountInstance & k_EChatInstanceFlagLobby);
	}


	//-----------------------------------------------------------------------------
	// Purpose: Is this an individual user account id?
	//-----------------------------------------------------------------------------
	bool BIndividualAccount() const
	{
		return m_steamid.m_comp.m_EAccountType == k_EAccountTypeIndividual || m_steamid.m_comp.m_EAccountType == k_EAccountTypeConsoleUser;
	}


	//-----------------------------------------------------------------------------
	// Purpose: Is this an anonymous account?
	//-----------------------------------------------------------------------------
	bool BAnonAccount() const
	{
		return m_steamid.m_comp.m_EAccountType == k_EAccountTypeAnonUser || m_steamid.m_comp.m_EAccountType == k_EAccountTypeAnonGameServer;
	}

	//-----------------------------------------------------------------------------
	// Purpose: Is this an anonymous user account? ( used to create an account or reset a password )
	//-----------------------------------------------------------------------------
	bool BAnonUserAccount() const
	{
		return m_steamid.m_comp.m_EAccountType == k_EAccountTypeAnonUser;
	}

	//-----------------------------------------------------------------------------
	// Purpose: Is this a faked up Steam ID for a PSN friend account?
	//-----------------------------------------------------------------------------
	bool BConsoleUserAccount() const
	{
		return m_steamid.m_comp.m_EAccountType == k_EAccountTypeConsoleUser;
	}


	// simple accessors
	void SetAccountID(uint32 unAccountID) { m_steamid.m_comp.m_unAccountID = unAccountID; }
	void SetAccountInstance(uint32 unInstance) { m_steamid.m_comp.m_unAccountInstance = unInstance; }
	void ClearIndividualInstance() { if (BIndividualAccount()) m_steamid.m_comp.m_unAccountInstance = 0; }
	bool HasNoIndividualInstance() const { return BIndividualAccount() && (m_steamid.m_comp.m_unAccountInstance == 0); }
	AccountID_t GetAccountID() const { return m_steamid.m_comp.m_unAccountID; }
	uint32 GetUnAccountInstance() const { return m_steamid.m_comp.m_unAccountInstance; }
	EAccountType GetEAccountType() const { return (EAccountType)m_steamid.m_comp.m_EAccountType; }
	EUniverse GetEUniverse() const { return m_steamid.m_comp.m_EUniverse; }
	void SetEUniverse(EUniverse eUniverse) { m_steamid.m_comp.m_EUniverse = eUniverse; }
	bool IsValid() const;

	// this set of functions is hidden, will be moved out of class
	explicit CSteamID(const char *pchSteamID, EUniverse eDefaultUniverse = k_EUniverseInvalid);

	const char * Render() const				// renders this steam ID to string
	{
		const int k_cBufLen = 30;
		const int k_cBufs = 4;
		char* pchBuf;

		static char rgchBuf[k_cBufs][k_cBufLen];
		static int nBuf = 0;

		pchBuf = rgchBuf[nBuf++];
		nBuf %= k_cBufs;

		switch (m_steamid.m_comp.m_EAccountType)
		{
		case k_EAccountTypeInvalid:
		case k_EAccountTypeIndividual:
			sprintf_s(pchBuf, k_cBufLen, "STEAM_0:%u:%u", (m_steamid.m_comp.m_unAccountID % 2) ? 1 : 0, (uint32)m_steamid.m_comp.m_unAccountID / 2);
			break;
		default:
			sprintf_s(pchBuf, k_cBufLen, "%llu", ConvertToUint64());
		}
		return pchBuf;
	}
	static const char * Render(uint64 ulSteamID)	// static method to render a uint64 representation of a steam ID to a string
	{
		return CSteamID(ulSteamID).Render();
	}

	const char *SteamRender() const // renders this steam ID to string using the new rendering style
	{
		const int k_cBufLen = 37;
		const int k_cBufs = 4;
		char* pchBuf;

		static char rgchBuf[k_cBufs][k_cBufLen];
		static int nBuf = 0;

		pchBuf = rgchBuf[nBuf++];
		nBuf %= k_cBufs;

		switch (m_steamid.m_comp.m_EAccountType)
		{
		case k_EAccountTypeAnonGameServer:
			sprintf_s(pchBuf, k_cBufLen, "[A:%u:%u:%u]", m_steamid.m_comp.m_EUniverse, m_steamid.m_comp.m_unAccountID, m_steamid.m_comp.m_unAccountInstance);
			break;
		case k_EAccountTypeGameServer:
			sprintf_s(pchBuf, k_cBufLen, "[G:%u:%u]", m_steamid.m_comp.m_EUniverse, m_steamid.m_comp.m_unAccountID);
			break;
		case k_EAccountTypeMultiseat:
			sprintf_s(pchBuf, k_cBufLen, "[M:%u:%u:%u]", m_steamid.m_comp.m_EUniverse, m_steamid.m_comp.m_unAccountID, m_steamid.m_comp.m_unAccountInstance);
			break;
		case k_EAccountTypePending:
			sprintf_s(pchBuf, k_cBufLen, "[P:%u:%u]", m_steamid.m_comp.m_EUniverse, m_steamid.m_comp.m_unAccountID);
			break;
		case k_EAccountTypeContentServer:
			sprintf_s(pchBuf, k_cBufLen, "[C:%u:%u]", m_steamid.m_comp.m_EUniverse, m_steamid.m_comp.m_unAccountID);
			break;
		case k_EAccountTypeClan:
			sprintf_s(pchBuf, k_cBufLen, "[g:%u:%u]", m_steamid.m_comp.m_EUniverse, m_steamid.m_comp.m_unAccountID);
			break;
		case k_EAccountTypeChat:
			switch (m_steamid.m_comp.m_unAccountInstance & ~k_EChatAccountInstanceMask)
			{
			case k_EChatInstanceFlagClan:
				sprintf_s(pchBuf, k_cBufLen, "[c:%u:%u]", m_steamid.m_comp.m_EUniverse, m_steamid.m_comp.m_unAccountID);
				break;
			case k_EChatInstanceFlagLobby:
				sprintf_s(pchBuf, k_cBufLen, "[L:%u:%u]", m_steamid.m_comp.m_EUniverse, m_steamid.m_comp.m_unAccountID);
				break;
			default:
				sprintf_s(pchBuf, k_cBufLen, "[T:%u:%u]", m_steamid.m_comp.m_EUniverse, m_steamid.m_comp.m_unAccountID);
				break;
			}
			break;
		case k_EAccountTypeInvalid:
			sprintf_s(pchBuf, k_cBufLen, "[I:%u:%u]", m_steamid.m_comp.m_EUniverse, m_steamid.m_comp.m_unAccountID);
			break;
		case k_EAccountTypeIndividual:
			sprintf_s(pchBuf, k_cBufLen, "[U:%u:%u]", m_steamid.m_comp.m_EUniverse, m_steamid.m_comp.m_unAccountID);
			break;
		default:
			sprintf_s(pchBuf, k_cBufLen, "[i:%u:%u]", m_steamid.m_comp.m_EUniverse, m_steamid.m_comp.m_unAccountID);
			break;
		}

		return pchBuf;
	}

	static const char *SteamRender(uint64 ulSteamID)	// static method to render a uint64 representation of a steam ID to a string
	{
		return CSteamID(ulSteamID).SteamRender();
	}


	void SetFromString(const char *pchSteamID, EUniverse eDefaultUniverse);
	bool SetFromSteam2String(const char *pchSteam2ID, EUniverse eUniverse);

	inline bool operator==(const CSteamID &val) const { return m_steamid.m_unAll64Bits == val.m_steamid.m_unAll64Bits; }
	inline bool operator!=(const CSteamID &val) const { return !operator==(val); }
	inline bool operator<(const CSteamID &val) const { return m_steamid.m_unAll64Bits < val.m_steamid.m_unAll64Bits; }
	inline bool operator>(const CSteamID &val) const { return m_steamid.m_unAll64Bits > val.m_steamid.m_unAll64Bits; }

	// DEBUG function
	bool BValidExternalSteamID() const;

private:
	// These are defined here to prevent accidental implicit conversion of a u32AccountID to a CSteamID.
	// If you get a compiler error about an ambiguous constructor/function then it may be because you're
	// passing a 32-bit int to a function that takes a CSteamID. You should explicitly create the SteamID
	// using the correct Universe and account Type/Instance values.
	//CSteamID(uint32);
	//CSteamID(int32);

	// 64 bits total
	union SteamID_t
	{
		struct SteamIDComponent_t
		{
#ifdef VALVE_BIG_ENDIAN
			EUniverse			m_EUniverse : 8;	// universe this account belongs to
			unsigned int		m_EAccountType : 4;			// type of account - can't show as EAccountType, due to signed / unsigned difference
			unsigned int		m_unAccountInstance : 20;	// dynamic instance ID
			uint32				m_unAccountID : 32;			// unique account identifier
#else
			uint32				m_unAccountID : 32;			// unique account identifier
			unsigned int		m_unAccountInstance : 20;	// dynamic instance ID
			unsigned int		m_EAccountType : 4;			// type of account - can't show as EAccountType, due to signed / unsigned difference
			EUniverse			m_EUniverse : 8;	// universe this account belongs to
#endif
		} m_comp;

		uint64 m_unAll64Bits;
	} m_steamid;
};

#pragma pack(pop)

inline bool CSteamID::IsValid() const
{
	if (m_steamid.m_comp.m_EAccountType <= k_EAccountTypeInvalid || m_steamid.m_comp.m_EAccountType >= k_EAccountTypeMax)
		return false;

	if (m_steamid.m_comp.m_EUniverse <= k_EUniverseInvalid || m_steamid.m_comp.m_EUniverse >= k_EUniverseMax)
		return false;

	if (m_steamid.m_comp.m_EAccountType == k_EAccountTypeIndividual)
	{
		if (m_steamid.m_comp.m_unAccountID == 0 || m_steamid.m_comp.m_unAccountInstance > k_unSteamUserWebInstance)
			return false;
	}

	if (m_steamid.m_comp.m_EAccountType == k_EAccountTypeClan)
	{
		if (m_steamid.m_comp.m_unAccountID == 0 || m_steamid.m_comp.m_unAccountInstance != 0)
			return false;
	}

	if (m_steamid.m_comp.m_EAccountType == k_EAccountTypeGameServer)
	{
		if (m_steamid.m_comp.m_unAccountID == 0)
			return false;
		// Any limit on instances?  We use them for local users and bots
	}
	return true;
}

// generic invalid CSteamID
#define k_steamIDNil CSteamID()

// This steamID comes from a user game connection to an out of date GS that hasnt implemented the protocol
// to provide its steamID
#define k_steamIDOutofDateGS CSteamID( 0, 0, k_EUniverseInvalid, k_EAccountTypeInvalid )
// This steamID comes from a user game connection to an sv_lan GS
#define k_steamIDLanModeGS CSteamID( 0, 0, k_EUniversePublic, k_EAccountTypeInvalid )
// This steamID can come from a user game connection to a GS that has just booted but hasnt yet even initialized
// its steam3 component and started logging on.
#define k_steamIDNotInitYetGS CSteamID( 1, 0, k_EUniverseInvalid, k_EAccountTypeInvalid )
// This steamID can come from a user game connection to a GS that isn't using the steam authentication system but still
// wants to support the "Join Game" option in the friends list
#define k_steamIDNonSteamGS CSteamID( 2, 0, k_EUniverseInvalid, k_EAccountTypeInvalid )

// Returns the matching chat steamID, with the default instance of 0
// If the steamID passed in is already of type k_EAccountTypeChat it will be returned with the same instance
inline CSteamID ChatIDFromSteamID(const CSteamID &steamID)
{
	if (steamID.GetEAccountType() == k_EAccountTypeClan)
	{
		return CSteamID(steamID.GetAccountID(), k_EChatInstanceFlagClan, steamID.GetEUniverse(), k_EAccountTypeChat);
	}
	else if (steamID.GetEAccountType() == k_EAccountTypeChat)
	{
		return steamID;
	}
	else
	{
		return k_steamIDNil;
	}
}

// Returns the matching clan steamID, with the default instance of 0
// If the steamID passed in is already of type k_EAccountTypeClan it will be returned with the same instance
inline CSteamID ClanIDFromSteamID(const CSteamID &steamID)
{
	if (steamID.GetEAccountType() == k_EAccountTypeChat && steamID.GetUnAccountInstance() & k_EChatInstanceFlagClan)
	{
		return CSteamID(steamID.GetAccountID(), 0, steamID.GetEUniverse(), k_EAccountTypeClan);
	}
	else if (steamID.GetEAccountType() == k_EAccountTypeClan)
	{
		return steamID;
	}
	else
	{
		return k_steamIDNil;
	}
}

// Asserts steamID type before conversion
CSteamID ChatIDFromClanID(const CSteamID &steamIDClan);
// Asserts steamID type before conversion
CSteamID ClanIDFromChatID(const CSteamID &steamIDChat);

class CCallbackBase
{
public:

	CCallbackBase() { m_nCallbackFlags = 0; m_iCallback = 0; }

	// don't add a virtual destructor because we export this binary interface across dll's

	virtual void Run(void *pvParam) = 0;

	virtual void Run(void *pvParam, bool bIOFailure, SteamAPICall_t hSteamAPICall) = 0;

	int GetICallback() { return m_iCallback; }

	virtual int GetCallbackSizeBytes() = 0;

protected:

	enum { k_ECallbackFlagsRegistered = 0x01, k_ECallbackFlagsGameServer = 0x02 };

	unsigned __int8 m_nCallbackFlags;
	int m_iCallback;
	friend class CCallbackMgr;
};

template< class T, class P, bool bGameServer >
class CCallback : private CCallbackBase
{
public:
	typedef void (T::*func_t)(P*);

	// If you can't support constructing a callback with the correct parameters
	// then uncomment the empty constructor below and manually call ::Register() for your object
	// Or, just call the regular constructor with (NULL, NULL)

	// CCallback() {}

	// constructor for initializing this object in owner's constructor

	CCallback(T *pObj, func_t func) : m_pObj(pObj), m_Func(func)
	{
		if (pObj && func)
			Register(pObj, func);
	}

	~CCallback()
	{
		if (m_nCallbackFlags & k_ECallbackFlagsRegistered)
			Unregister();
	}

	// manual registration of the callback

	void Register(T *pObj, func_t func)
	{
		if (!pObj || !func)
			return;

		if (m_nCallbackFlags & k_ECallbackFlagsRegistered)
			Unregister();

		if (bGameServer)
		{
			m_nCallbackFlags |= k_ECallbackFlagsGameServer;
		}

		m_pObj = pObj;
		m_Func = func;

		// SteamAPI_RegisterCallback sets k_ECallbackFlagsRegistered

		SteamAPI_RegisterCallback(this, P::k_iCallback);
	}

	void Unregister()
	{
		// SteamAPI_UnregisterCallback removes k_ECallbackFlagsRegistered
		SteamAPI_UnregisterCallback(this);
	}

	void SetGameserverFlag() { m_nCallbackFlags |= k_ECallbackFlagsGameServer; }

private:

	virtual void Run(void *pvParam)
	{
		(m_pObj->*m_Func)((P *)pvParam);
	}

	virtual void Run(void *pvParam, bool, SteamAPICall_t)
	{
		(m_pObj->*m_Func)((P *)pvParam);
	}

	int GetCallbackSizeBytes()
	{
		return sizeof(P);
	}

	T *m_pObj;
	func_t m_Func;
};

void SteamAPI_RegisterCallback(CCallbackBase* pCallback, int iCallback);
void SteamAPI_UnregisterCallback(CCallbackBase* pCallback);

struct GameOverlayActivated_t { bool m_bActive; };

// interface predec
class ISteamClient;
class ISteamUser;
class ISteamGameServer;
class ISteamFriends;
class ISteamUtils;
class ISteamMatchmaking;
class ISteamContentServer;
class ISteamMasterServerUpdater;
class ISteamMatchmakingServers;
class ISteam2Bridge;
class ISteamUserStats;
class ISteamApps;
class ISteamBilling;
class IVAC;
class ISteamNetworking;
class ISteamRemoteStorage;
class ISteamGameServerItems;
class ISteamGameServerStats;
class ISteamHTTP;
class ISteamScreenshots;
class ISteamUnifiedMessages;
class ISteamController;
class ISteamUGC;
class ISteamAppList;
class ISteamMusic;
class ISteamMusicRemote;
class ISteamHTMLSurface;
class ISteamInventory;
class ISteamVideo;

typedef void(*SteamAPIWarningMessageHook_t)(int hpipe, const char *message);

class ISteamClient017
{
public:
	// Creates a communication pipe to the Steam client
	virtual HSteamPipe CreateSteamPipe() = 0;

	// Releases a previously created communications pipe
	virtual bool BReleaseSteamPipe(HSteamPipe hSteamPipe) = 0;

	// connects to an existing global user, failing if none exists
	// used by the game to coordinate with the steamUI
	virtual HSteamUser ConnectToGlobalUser(HSteamPipe hSteamPipe) = 0;

	// used by game servers, create a steam user that won't be shared with anyone else
	virtual HSteamUser CreateLocalUser(HSteamPipe *phSteamPipe, EAccountType eAccountType) = 0;

	// removes an allocated user
	virtual void ReleaseUser(HSteamPipe hSteamPipe, HSteamUser hUser) = 0;

	// retrieves the ISteamUser interface associated with the handle
	virtual ISteamUser *GetISteamUser(HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion) = 0;

	// retrieves the ISteamGameServer interface associated with the handle
	virtual ISteamGameServer *GetISteamGameServer(HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion) = 0;

	// set the local IP and Port to bind to
	// this must be set before CreateLocalUser()
	virtual void SetLocalIPBinding(uint32 unIP, uint16 usPort) = 0;

	// returns the ISteamFriends interface
	virtual ISteamFriends *GetISteamFriends(HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion) = 0;

	// returns the ISteamUtils interface
	virtual ISteamUtils *GetISteamUtils(HSteamPipe hSteamPipe, const char *pchVersion) = 0;

	// returns the ISteamMatchmaking interface
	virtual ISteamMatchmaking *GetISteamMatchmaking(HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion) = 0;

	// returns the ISteamMatchmakingServers interface
	virtual ISteamMatchmakingServers *GetISteamMatchmakingServers(HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion) = 0;

	// returns the a generic interface
	virtual void *GetISteamGenericInterface(HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion) = 0;

	// returns the ISteamUserStats interface
	virtual ISteamUserStats *GetISteamUserStats(HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion) = 0;

	// returns the ISteamGameServerStats interface
	virtual ISteamGameServerStats* GetISteamGameServerStats(HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char *pchVersion) = 0;

	// returns apps interface
	virtual ISteamApps* GetISteamApps(HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion) = 0;

	// networking
	virtual ISteamNetworking* GetISteamNetworking(HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion) = 0;

	// remote storage
	virtual ISteamRemoteStorage *GetISteamRemoteStorage(HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char *pchVersion) = 0;

	// user screenshots
	virtual ISteamScreenshots *GetISteamScreenshots(HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char *pchVersion) = 0;

	// this needs to be called every frame to process matchmaking results
	// redundant if you're already calling SteamAPI_RunCallbacks()
	virtual void RunFrame() = 0;

	// returns the number of IPC calls made since the last time this function was called
	// Used for perf debugging so you can understand how many IPC calls your game makes per frame
	// Every IPC call is at minimum a thread context switch if not a process one so you want to rate
	// control how often you do them.
	virtual uint32 GetIPCCallCount() = 0;

	// API warning handling
	// 'int' is the severity; 0 for msg, 1 for warning
	// 'const char *' is the text of the message
	// callbacks will occur directly after the API function is called that generated the warning or message
	virtual void SetWarningMessageHook(SteamAPIWarningMessageHook_t pFunction) = 0;

	// Trigger global shutdown for the DLL
	virtual bool BShutdownIfAllPipesClosed() = 0;

	// Expose HTTP interface
	virtual ISteamHTTP *GetISteamHTTP(HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char *pchVersion) = 0;

	// Exposes the ISteamUnifiedMessages interface
	virtual ISteamUnifiedMessages *GetISteamUnifiedMessages(HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char *pchVersion) = 0;

	// Exposes the ISteamController interface
	virtual ISteamController *GetISteamController(HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion) = 0;

	// Exposes the ISteamUGC interface
	virtual ISteamUGC *GetISteamUGC(HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion) = 0;

	// returns app list interface, only available on specially registered apps
	virtual ISteamAppList *GetISteamAppList(HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion) = 0;

	// Music Player
	virtual ISteamMusic *GetISteamMusic(HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char *pchVersion) = 0;

	// Music Player Remote
	virtual ISteamMusicRemote *GetISteamMusicRemote(HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char *pchVersion) = 0;

	// html page display
	virtual ISteamHTMLSurface *GetISteamHTMLSurface(HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char *pchVersion) = 0;

	// Helper functions for internal Steam usage
	virtual void Set_SteamAPI_CPostAPIResultInProcess(void(*)(SteamAPICall_t callHandle, void *, uint32 unCallbackSize, int iCallbackNum)) = 0;
	virtual void Remove_SteamAPI_CPostAPIResultInProcess(void(*)(SteamAPICall_t callHandle, void *, uint32 unCallbackSize, int iCallbackNum)) = 0;
	virtual void Set_SteamAPI_CCheckCallbackRegisteredInProcess(unsigned int(*)(int iCallbackNum)) = 0;

	// inventory
	virtual ISteamInventory *GetISteamInventory(HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char *pchVersion) = 0;

	// Video
	virtual ISteamVideo *GetISteamVideo(HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char *pchVersion) = 0;
};

class ISteamUser019
{
public:
	// returns the HSteamUser this interface represents
	// this is only used internally by the API, and by a few select interfaces that support multi-user
	virtual HSteamUser GetHSteamUser() = 0;

	// returns true if the Steam client current has a live connection to the Steam servers. 
	// If false, it means there is no active connection due to either a networking issue on the local machine, or the Steam server is down/busy.
	// The Steam client will automatically be trying to recreate the connection as often as possible.
	virtual bool BLoggedOn() = 0;

	// returns the CSteamID of the account currently logged into the Steam client
	// a CSteamID is a unique identifier for an account, and used to differentiate users in all parts of the Steamworks API
	virtual CSteamID GetSteamID() = 0;

	// Multiplayer Authentication functions

	// InitiateGameConnection() starts the state machine for authenticating the game client with the game server
	// It is the client portion of a three-way handshake between the client, the game server, and the steam servers
	//
	// Parameters:
	// void *pAuthBlob - a pointer to empty memory that will be filled in with the authentication token.
	// int cbMaxAuthBlob - the number of bytes of allocated memory in pBlob. Should be at least 2048 bytes.
	// CSteamID steamIDGameServer - the steamID of the game server, received from the game server by the client
	// CGameID gameID - the ID of the current game. For games without mods, this is just CGameID( <appID> )
	// uint32 unIPServer, uint16 usPortServer - the IP address of the game server
	// bool bSecure - whether or not the client thinks that the game server is reporting itself as secure (i.e. VAC is running)
	//
	// return value - returns the number of bytes written to pBlob. If the return is 0, then the buffer passed in was too small, and the call has failed
	// The contents of pBlob should then be sent to the game server, for it to use to complete the authentication process.
	virtual int InitiateGameConnection(void *pAuthBlob, int cbMaxAuthBlob, CSteamID steamIDGameServer, uint32 unIPServer, uint16 usPortServer, bool bSecure) = 0;

	// notify of disconnect
	// needs to occur when the game client leaves the specified game server, needs to match with the InitiateGameConnection() call
	virtual void TerminateGameConnection(uint32 unIPServer, uint16 usPortServer) = 0;

	// Legacy functions

	// used by only a few games to track usage events
	virtual void TrackAppUsageEvent(CGameID gameID, EAppUsageEvent eAppUsageEvent, const char *pchExtraInfo = "") = 0;

	// get the local storage folder for current Steam account to write application data, e.g. save games, configs etc.
	// this will usually be something like "C:\Progam Files\Steam\userdata\<SteamID>\<AppID>\local"
	virtual bool GetUserDataFolder(char *pchBuffer, int cubBuffer) = 0;

	// Starts voice recording. Once started, use GetVoice() to get the data
	virtual void StartVoiceRecording() = 0;

	// Stops voice recording. Because people often release push-to-talk keys early, the system will keep recording for
	// a little bit after this function is called. GetVoice() should continue to be called until it returns
	// k_eVoiceResultNotRecording
	virtual void StopVoiceRecording() = 0;

	// Determine the amount of captured audio data that is available in bytes.
	// This provides both the compressed and uncompressed data. Please note that the uncompressed
	// data is not the raw feed from the microphone: data may only be available if audible 
	// levels of speech are detected.
	// nUncompressedVoiceDesiredSampleRate is necessary to know the number of bytes to return in pcbUncompressed - can be set to 0 if you don't need uncompressed (the usual case)
	virtual EVoiceResult GetAvailableVoice(uint32 *pcbCompressed, uint32 *pcbUncompressed, uint32 nUncompressedVoiceDesiredSampleRate) = 0;

	// Gets the latest voice data from the microphone. Compressed data is an arbitrary format, and is meant to be handed back to 
	// DecompressVoice() for playback later as a binary blob. Uncompressed data is 16-bit, signed integer, 11025Hz PCM format.
	// Please note that the uncompressed data is not the raw feed from the microphone: data may only be available if audible 
	// levels of speech are detected, and may have passed through denoising filters, etc.
	// This function should be called as often as possible once recording has started; once per frame at least.
	// nBytesWritten is set to the number of bytes written to pDestBuffer. 
	// nUncompressedBytesWritten is set to the number of bytes written to pUncompressedDestBuffer. 
	// You must grab both compressed and uncompressed here at the same time, if you want both.
	// Matching data that is not read during this call will be thrown away.
	// GetAvailableVoice() can be used to determine how much data is actually available.
	virtual EVoiceResult GetVoice(bool bWantCompressed, void *pDestBuffer, uint32 cbDestBufferSize, uint32 *nBytesWritten, bool bWantUncompressed, void *pUncompressedDestBuffer, uint32 cbUncompressedDestBufferSize, uint32 *nUncompressBytesWritten, uint32 nUncompressedVoiceDesiredSampleRate) = 0;

	// Decompresses a chunk of compressed data produced by GetVoice().
	// nBytesWritten is set to the number of bytes written to pDestBuffer unless the return value is k_EVoiceResultBufferTooSmall.
	// In that case, nBytesWritten is set to the size of the buffer required to decompress the given
	// data. The suggested buffer size for the destination buffer is 22 kilobytes.
	// The output format of the data is 16-bit signed at the requested samples per second.
	virtual EVoiceResult DecompressVoice(const void *pCompressed, uint32 cbCompressed, void *pDestBuffer, uint32 cbDestBufferSize, uint32 *nBytesWritten, uint32 nDesiredSampleRate) = 0;

	// This returns the frequency of the voice data as it's stored internally; calling DecompressVoice() with this size will yield the best results
	virtual uint32 GetVoiceOptimalSampleRate() = 0;

	// Retrieve ticket to be sent to the entity who wishes to authenticate you. 
	// pcbTicket retrieves the length of the actual ticket.
	virtual HAuthTicket GetAuthSessionTicket(void *pTicket, int cbMaxTicket, uint32 *pcbTicket) = 0;

	// Authenticate ticket from entity steamID to be sure it is valid and isnt reused
	// Registers for callbacks if the entity goes offline or cancels the ticket ( see ValidateAuthTicketResponse_t callback and EAuthSessionResponse )
	virtual EBeginAuthSessionResult BeginAuthSession(const void *pAuthTicket, int cbAuthTicket, CSteamID steamID) = 0;

	// Stop tracking started by BeginAuthSession - called when no longer playing game with this entity
	virtual void EndAuthSession(CSteamID steamID) = 0;

	// Cancel auth ticket from GetAuthSessionTicket, called when no longer playing game with the entity you gave the ticket to
	virtual void CancelAuthTicket(HAuthTicket hAuthTicket) = 0;

	// After receiving a user's authentication data, and passing it to BeginAuthSession, use this function
	// to determine if the user owns downloadable content specified by the provided AppID.
	virtual EUserHasLicenseForAppResult UserHasLicenseForApp(CSteamID steamID, AppId_t appID) = 0;

	// returns true if this users looks like they are behind a NAT device. Only valid once the user has connected to steam 
	// (i.e a SteamServersConnected_t has been issued) and may not catch all forms of NAT.
	virtual bool BIsBehindNAT() = 0;

	// set data to be replicated to friends so that they can join your game
	// CSteamID steamIDGameServer - the steamID of the game server, received from the game server by the client
	// uint32 unIPServer, uint16 usPortServer - the IP address of the game server
	virtual void AdvertiseGame(CSteamID steamIDGameServer, uint32 unIPServer, uint16 usPortServer) = 0;

	// Requests a ticket encrypted with an app specific shared key
	// pDataToInclude, cbDataToInclude will be encrypted into the ticket
	// ( This is asynchronous, you must wait for the ticket to be completed by the server )
	virtual SteamAPICall_t RequestEncryptedAppTicket(void *pDataToInclude, int cbDataToInclude) = 0;

	// retrieve a finished ticket
	virtual bool GetEncryptedAppTicket(void *pTicket, int cbMaxTicket, uint32 *pcbTicket) = 0;

	// Trading Card badges data access
	// if you only have one set of cards, the series will be 1
	// the user has can have two different badges for a series; the regular (max level 5) and the foil (max level 1)
	virtual int GetGameBadgeLevel(int nSeries, bool bFoil) = 0;

	// gets the Steam Level of the user, as shown on their profile
	virtual int GetPlayerSteamLevel() = 0;

	//Requests a URL which authenticates an in-game browser for store check-out,
	//and then redirects to the specified URL. As long as the in-game browser
	//accepts and handles session cookies, Steam microtransaction checkout pages
	//will automatically recognize the user instead of presenting a login page.
	//The result of this API call will be a StoreAuthURLResponse_t callback.
	//NOTE: The URL has a very short lifetime to prevent history-snooping attacks,
	//so you should only call this API when you are about to launch the browser,
	//or else immediately navigate to the result URL using a hidden browser window.
	//NOTE 2: The resulting authorization cookie has an expiration time of one day,
	//so it would be a good idea to request and visit a new auth URL every 12 hours.
	virtual SteamAPICall_t RequestStoreAuthURL(const char *pchRedirectURL) = 0;

	virtual bool BIsPhoneVerified() = 0;
	virtual bool BIsTwoFactorEnabled() = 0;
};

class ISteamFriends015;

class ISteamUtils008
{
public:
	// return the number of seconds since the user 
	virtual uint32 GetSecondsSinceAppActive() = 0;
	virtual uint32 GetSecondsSinceComputerActive() = 0;

	// the universe this client is connecting to
	virtual EUniverse GetConnectedUniverse() = 0;

	// Steam server time - in PST, number of seconds since January 1, 1970 (i.e unix time)
	virtual uint32 GetServerRealTime() = 0;

	// returns the 2 digit ISO 3166-1-alpha-2 format country code this client is running in (as looked up via an IP-to-location database)
	// e.g "US" or "UK".
	virtual const char *GetIPCountry() = 0;

	// returns true if the image exists, and valid sizes were filled out
	virtual bool GetImageSize(int iImage, uint32 *pnWidth, uint32 *pnHeight) = 0;

	// returns true if the image exists, and the buffer was successfully filled out
	// results are returned in RGBA format
	// the destination buffer size should be 4 * height * width * sizeof(char)
	virtual bool GetImageRGBA(int iImage, uint8 *pubDest, int nDestBufferSize) = 0;

	// returns the IP of the reporting server for valve - currently only used in Source engine games
	virtual bool GetCSERIPPort(uint32 *unIP, uint16 *usPort) = 0;

	// return the amount of battery power left in the current system in % [0..100], 255 for being on AC power
	virtual uint8 GetCurrentBatteryPower() = 0;

	// returns the appID of the current process
	virtual uint32 GetAppID() = 0;

	// Sets the position where the overlay instance for the currently calling game should show notifications.
	// This position is per-game and if this function is called from outside of a game context it will do nothing.
	virtual void SetOverlayNotificationPosition(ENotificationPosition eNotificationPosition) = 0;

	// API asynchronous call results
	// can be used directly, but more commonly used via the callback dispatch API (see steam_api.h)
	virtual bool IsAPICallCompleted(SteamAPICall_t hSteamAPICall, bool *pbFailed) = 0;
	virtual ESteamAPICallFailure GetAPICallFailureReason(SteamAPICall_t hSteamAPICall) = 0;
	virtual bool GetAPICallResult(SteamAPICall_t hSteamAPICall, void *pCallback, int cubCallback, int iCallbackExpected, bool *pbFailed) = 0;

	// this needs to be called every frame to process matchmaking results
	// redundant if you're already calling SteamAPI_RunCallbacks()
	virtual void RunFrame() = 0;

	// returns the number of IPC calls made since the last time this function was called
	// Used for perf debugging so you can understand how many IPC calls your game makes per frame
	// Every IPC call is at minimum a thread context switch if not a process one so you want to rate
	// control how often you do them.
	virtual uint32 GetIPCCallCount() = 0;

	// API warning handling
	// 'int' is the severity; 0 for msg, 1 for warning
	// 'const char *' is the text of the message
	// callbacks will occur directly after the API function is called that generated the warning or message
	virtual void SetWarningMessageHook(SteamAPIWarningMessageHook_t pFunction) = 0;

	// Returns true if the overlay is running & the user can access it. The overlay process could take a few seconds to
	// start & hook the game process, so this function will initially return false while the overlay is loading.
	virtual bool IsOverlayEnabled() = 0;

	// Normally this call is unneeded if your game has a constantly running frame loop that calls the 
	// D3D Present API, or OGL SwapBuffers API every frame.
	//
	// However, if you have a game that only refreshes the screen on an event driven basis then that can break 
	// the overlay, as it uses your Present/SwapBuffers calls to drive it's internal frame loop and it may also
	// need to Present() to the screen any time an even needing a notification happens or when the overlay is
	// brought up over the game by a user.  You can use this API to ask the overlay if it currently need a present
	// in that case, and then you can check for this periodically (roughly 33hz is desirable) and make sure you
	// refresh the screen with Present or SwapBuffers to allow the overlay to do it's work.
	virtual bool BOverlayNeedsPresent() = 0;

#ifndef _PS3
	// Asynchronous call to check if an executable file has been signed using the public key set on the signing tab
	// of the partner site, for example to refuse to load modified executable files.  
	// The result is returned in CheckFileSignature_t.
	//   k_ECheckFileSignatureNoSignaturesFoundForThisApp - This app has not been configured on the signing tab of the partner site to enable this function.
	//   k_ECheckFileSignatureNoSignaturesFoundForThisFile - This file is not listed on the signing tab for the partner site.
	//   k_ECheckFileSignatureFileNotFound - The file does not exist on disk.
	//   k_ECheckFileSignatureInvalidSignature - The file exists, and the signing tab has been set for this file, but the file is either not signed or the signature does not match.
	//   k_ECheckFileSignatureValidSignature - The file is signed and the signature is valid.
	virtual SteamAPICall_t CheckFileSignature(const char *szFileName) = 0;
#endif

#ifdef _PS3
	virtual void PostPS3SysutilCallback(uint64_t status, uint64_t param, void* userdata) = 0;
	virtual bool BIsReadyToShutdown() = 0;
	virtual bool BIsPSNOnline() = 0;

	// Call this with localized strings for the language the game is running in, otherwise default english
	// strings will be used by Steam.
	virtual void SetPSNGameBootInviteStrings(const char *pchSubject, const char *pchBody) = 0;
#endif

	// Activates the Big Picture text input dialog which only supports gamepad input
	virtual bool ShowGamepadTextInput(EGamepadTextInputMode eInputMode, EGamepadTextInputLineMode eLineInputMode, const char *pchDescription, uint32 unCharMax, const char *pchExistingText) = 0;

	// Returns previously entered text & length
	virtual uint32 GetEnteredGamepadTextLength() = 0;
	virtual bool GetEnteredGamepadTextInput(char *pchText, uint32 cchText) = 0;

	// returns the language the steam client is running in, you probably want ISteamApps::GetCurrentGameLanguage instead, this is for very special usage cases
	virtual const char *GetSteamUILanguage() = 0;

	// returns true if Steam itself is running in VR mode
	virtual bool IsSteamRunningInVR() = 0;

	virtual void SetOverlayNotificationInset(int nHorizontalInset, int nVerticalInset) = 0;

	virtual bool IsSteamInBigPictureMode() = 0;

	virtual void StartVRDashboard() = 0;
};

class ISteamMatchmaking009
{
public:
	// game server favorites storage
	// saves basic details about a multiplayer game server locally

	// returns the number of favorites servers the user has stored
	virtual int GetFavoriteGameCount() = 0;

	// returns the details of the game server
	// iGame is of range [0,GetFavoriteGameCount())
	// *pnIP, *pnConnPort are filled in the with IP:port of the game server
	// *punFlags specify whether the game server was stored as an explicit favorite or in the history of connections
	// *pRTime32LastPlayedOnServer is filled in the with the Unix time the favorite was added
	virtual bool GetFavoriteGame(int iGame, AppId_t *pnAppID, uint32 *pnIP, uint16 *pnConnPort, uint16 *pnQueryPort, uint32 *punFlags, uint32 *pRTime32LastPlayedOnServer) = 0;

	// adds the game server to the local list; updates the time played of the server if it already exists in the list
	virtual int AddFavoriteGame(AppId_t nAppID, uint32 nIP, uint16 nConnPort, uint16 nQueryPort, uint32 unFlags, uint32 rTime32LastPlayedOnServer) = 0;

	// removes the game server from the local storage; returns true if one was removed
	virtual bool RemoveFavoriteGame(AppId_t nAppID, uint32 nIP, uint16 nConnPort, uint16 nQueryPort, uint32 unFlags) = 0;

	///////
	// Game lobby functions

	// Get a list of relevant lobbies
	// this is an asynchronous request
	// results will be returned by LobbyMatchList_t callback & call result, with the number of lobbies found
	// this will never return lobbies that are full
	// to add more filter, the filter calls below need to be call before each and every RequestLobbyList() call
	// use the CCallResult<> object in steam_api.h to match the SteamAPICall_t call result to a function in an object, e.g.
	/*
		class CMyLobbyListManager
		{
			CCallResult<CMyLobbyListManager, LobbyMatchList_t> m_CallResultLobbyMatchList;
			void FindLobbies()
			{
				// SteamMatchmaking()->AddRequestLobbyListFilter() functions would be called here, before RequestLobbyList()
				SteamAPICall_t hSteamAPICall = SteamMatchmaking()->RequestLobbyList();
				m_CallResultLobbyMatchList.Set( hSteamAPICall, this, &CMyLobbyListManager::OnLobbyMatchList );
			}
			void OnLobbyMatchList( LobbyMatchList_t *pLobbyMatchList, bool bIOFailure )
			{
				// lobby list has be retrieved from Steam back-end, use results
			}
		}
	*/
	// 
	virtual SteamAPICall_t RequestLobbyList() = 0;
	// filters for lobbies
	// this needs to be called before RequestLobbyList() to take effect
	// these are cleared on each call to RequestLobbyList()
	virtual void AddRequestLobbyListStringFilter(const char *pchKeyToMatch, const char *pchValueToMatch, ELobbyComparison eComparisonType) = 0;
	// numerical comparison
	virtual void AddRequestLobbyListNumericalFilter(const char *pchKeyToMatch, int nValueToMatch, ELobbyComparison eComparisonType) = 0;
	// returns results closest to the specified value. Multiple near filters can be added, with early filters taking precedence
	virtual void AddRequestLobbyListNearValueFilter(const char *pchKeyToMatch, int nValueToBeCloseTo) = 0;
	// returns only lobbies with the specified number of slots available
	virtual void AddRequestLobbyListFilterSlotsAvailable(int nSlotsAvailable) = 0;
	// sets the distance for which we should search for lobbies (based on users IP address to location map on the Steam backed)
	virtual void AddRequestLobbyListDistanceFilter(ELobbyDistanceFilter eLobbyDistanceFilter) = 0;
	// sets how many results to return, the lower the count the faster it is to download the lobby results & details to the client
	virtual void AddRequestLobbyListResultCountFilter(int cMaxResults) = 0;

	virtual void AddRequestLobbyListCompatibleMembersFilter(CSteamID steamIDLobby) = 0;

	// returns the CSteamID of a lobby, as retrieved by a RequestLobbyList call
	// should only be called after a LobbyMatchList_t callback is received
	// iLobby is of the range [0, LobbyMatchList_t::m_nLobbiesMatching)
	// the returned CSteamID::IsValid() will be false if iLobby is out of range
	virtual CSteamID GetLobbyByIndex(int iLobby) = 0;

	// Create a lobby on the Steam servers.
	// If private, then the lobby will not be returned by any RequestLobbyList() call; the CSteamID
	// of the lobby will need to be communicated via game channels or via InviteUserToLobby()
	// this is an asynchronous request
	// results will be returned by LobbyCreated_t callback and call result; lobby is joined & ready to use at this point
	// a LobbyEnter_t callback will also be received (since the local user is joining their own lobby)
	virtual SteamAPICall_t CreateLobby(ELobbyType eLobbyType, int cMaxMembers) = 0;

	// Joins an existing lobby
	// this is an asynchronous request
	// results will be returned by LobbyEnter_t callback & call result, check m_EChatRoomEnterResponse to see if was successful
	// lobby metadata is available to use immediately on this call completing
	virtual SteamAPICall_t JoinLobby(CSteamID steamIDLobby) = 0;

	// Leave a lobby; this will take effect immediately on the client side
	// other users in the lobby will be notified by a LobbyChatUpdate_t callback
	virtual void LeaveLobby(CSteamID steamIDLobby) = 0;

	// Invite another user to the lobby
	// the target user will receive a LobbyInvite_t callback
	// will return true if the invite is successfully sent, whether or not the target responds
	// returns false if the local user is not connected to the Steam servers
	// if the other user clicks the join link, a GameLobbyJoinRequested_t will be posted if the user is in-game,
	// or if the game isn't running yet the game will be launched with the parameter +connect_lobby <64-bit lobby id>
	virtual bool InviteUserToLobby(CSteamID steamIDLobby, CSteamID steamIDInvitee) = 0;

	// Lobby iteration, for viewing details of users in a lobby
	// only accessible if the lobby user is a member of the specified lobby
	// persona information for other lobby members (name, avatar, etc.) will be asynchronously received
	// and accessible via ISteamFriends interface

	// returns the number of users in the specified lobby
	virtual int GetNumLobbyMembers(CSteamID steamIDLobby) = 0;
	// returns the CSteamID of a user in the lobby
	// iMember is of range [0,GetNumLobbyMembers())
	// note that the current user must be in a lobby to retrieve CSteamIDs of other users in that lobby
	virtual CSteamID GetLobbyMemberByIndex(CSteamID steamIDLobby, int iMember) = 0;

	// Get data associated with this lobby
	// takes a simple key, and returns the string associated with it
	// "" will be returned if no value is set, or if steamIDLobby is invalid
	virtual const char *GetLobbyData(CSteamID steamIDLobby, const char *pchKey) = 0;
	// Sets a key/value pair in the lobby metadata
	// each user in the lobby will be broadcast this new value, and any new users joining will receive any existing data
	// this can be used to set lobby names, map, etc.
	// to reset a key, just set it to ""
	// other users in the lobby will receive notification of the lobby data change via a LobbyDataUpdate_t callback
	virtual bool SetLobbyData(CSteamID steamIDLobby, const char *pchKey, const char *pchValue) = 0;

	// returns the number of metadata keys set on the specified lobby
	virtual int GetLobbyDataCount(CSteamID steamIDLobby) = 0;

	// returns a lobby metadata key/values pair by index, of range [0, GetLobbyDataCount()]
	virtual bool GetLobbyDataByIndex(CSteamID steamIDLobby, int iLobbyData, char *pchKey, int cchKeyBufferSize, char *pchValue, int cchValueBufferSize) = 0;

	// removes a metadata key from the lobby
	virtual bool DeleteLobbyData(CSteamID steamIDLobby, const char *pchKey) = 0;

	// Gets per-user metadata for someone in this lobby
	virtual const char *GetLobbyMemberData(CSteamID steamIDLobby, CSteamID steamIDUser, const char *pchKey) = 0;
	// Sets per-user metadata (for the local user implicitly)
	virtual void SetLobbyMemberData(CSteamID steamIDLobby, const char *pchKey, const char *pchValue) = 0;

	// Broadcasts a chat message to the all the users in the lobby
	// users in the lobby (including the local user) will receive a LobbyChatMsg_t callback
	// returns true if the message is successfully sent
	// pvMsgBody can be binary or text data, up to 4k
	// if pvMsgBody is text, cubMsgBody should be strlen( text ) + 1, to include the null terminator
	virtual bool SendLobbyChatMsg(CSteamID steamIDLobby, const void *pvMsgBody, int cubMsgBody) = 0;
	// Get a chat message as specified in a LobbyChatMsg_t callback
	// iChatID is the LobbyChatMsg_t::m_iChatID value in the callback
	// *pSteamIDUser is filled in with the CSteamID of the member
	// *pvData is filled in with the message itself
	// return value is the number of bytes written into the buffer
	virtual int GetLobbyChatEntry(CSteamID steamIDLobby, int iChatID, CSteamID *pSteamIDUser, void *pvData, int cubData, EChatEntryType *peChatEntryType) = 0;

	// Refreshes metadata for a lobby you're not necessarily in right now
	// you never do this for lobbies you're a member of, only if your
	// this will send down all the metadata associated with a lobby
	// this is an asynchronous call
	// returns false if the local user is not connected to the Steam servers
	// results will be returned by a LobbyDataUpdate_t callback
	// if the specified lobby doesn't exist, LobbyDataUpdate_t::m_bSuccess will be set to false
	virtual bool RequestLobbyData(CSteamID steamIDLobby) = 0;

	// sets the game server associated with the lobby
	// usually at this point, the users will join the specified game server
	// either the IP/Port or the steamID of the game server has to be valid, depending on how you want the clients to be able to connect
	virtual void SetLobbyGameServer(CSteamID steamIDLobby, uint32 unGameServerIP, uint16 unGameServerPort, CSteamID steamIDGameServer) = 0;
	// returns the details of a game server set in a lobby - returns false if there is no game server set, or that lobby doesn't exist
	virtual bool GetLobbyGameServer(CSteamID steamIDLobby, uint32 *punGameServerIP, uint16 *punGameServerPort, CSteamID *psteamIDGameServer) = 0;

	// set the limit on the # of users who can join the lobby
	virtual bool SetLobbyMemberLimit(CSteamID steamIDLobby, int cMaxMembers) = 0;
	// returns the current limit on the # of users who can join the lobby; returns 0 if no limit is defined
	virtual int GetLobbyMemberLimit(CSteamID steamIDLobby) = 0;

	// updates which type of lobby it is
	// only lobbies that are k_ELobbyTypePublic or k_ELobbyTypeInvisible, and are set to joinable, will be returned by RequestLobbyList() calls
	virtual bool SetLobbyType(CSteamID steamIDLobby, ELobbyType eLobbyType) = 0;

	// sets whether or not a lobby is joinable - defaults to true for a new lobby
	// if set to false, no user can join, even if they are a friend or have been invited
	virtual bool SetLobbyJoinable(CSteamID steamIDLobby, bool bLobbyJoinable) = 0;

	// returns the current lobby owner
	// you must be a member of the lobby to access this
	// there always one lobby owner - if the current owner leaves, another user will become the owner
	// it is possible (bur rare) to join a lobby just as the owner is leaving, thus entering a lobby with self as the owner
	virtual CSteamID GetLobbyOwner(CSteamID steamIDLobby) = 0;

	// changes who the lobby owner is
	// you must be the lobby owner for this to succeed, and steamIDNewOwner must be in the lobby
	// after completion, the local user will no longer be the owner
	virtual bool SetLobbyOwner(CSteamID steamIDLobby, CSteamID steamIDNewOwner) = 0;

	// link two lobbies for the purposes of checking player compatibility
	// you must be the lobby owner of both lobbies
	virtual bool SetLinkedLobby(CSteamID steamIDLobby, CSteamID steamIDLobbyDependent) = 0;

#ifdef _PS3
	// changes who the lobby owner is
	// you must be the lobby owner for this to succeed, and steamIDNewOwner must be in the lobby
	// after completion, the local user will no longer be the owner
	virtual void CheckForPSNGameBootInvite(unsigned int iGameBootAttributes) = 0;
#endif
};

class ISteamMatchmakingServers002
{
public:
	virtual int GetFavoriteGameCount() = 0;

	// returns the details of the game server
	// iGame is of range [0,iGame]
	virtual bool GetFavoriteGame(int iGame, uint32 *pnAppID, uint32 *pnIP, uint16 *pnConnPort, uint16 *pnQueryPort, uint32 *punFlags, RTime32 *pRTime32LastPlayedOnServer) = 0;

	// returns the new index of the game
	virtual int AddFavoriteGame(uint32 nAppID, uint32 nIP, uint16 nConnPort, uint16 nQueryPort, uint32 unFlags, RTime32 rTime32LastPlayedOnServer) = 0;

	// removes the game; returns true if one was removed
	virtual bool RemoveFavoriteGame(uint32 nAppID, uint32 nIP, uint16 nConnPort, uint16 nQueryPort, uint32 unFlags) = 0;

	///////
	// Game lobby functions

	// Get a list of relevant lobbies
	virtual void RequestLobbyList();
	virtual CSteamID GetLobbyByIndex(int iLobby) = 0;
	// Create a lobby - you'll get the SteamID of it on success
	virtual void CreateLobby(bool bPrivate) = 0;

	// Join a lobby
	virtual void JoinLobby(CSteamID steamIDLobby) = 0;
	// Leave a lobby
	virtual void LeaveLobby(CSteamID steamIDLobby) = 0;
	// Invite someone to the lobby
	virtual bool InviteUserToLobby(CSteamID steamIDLobby, CSteamID steamIDInvitee) = 0;
	// List users in this lobby
	virtual int GetNumLobbyMembers(CSteamID steamIDLobby) = 0;
	virtual CSteamID GetLobbyMemberByIndex(CSteamID steamIDLobby, int iMember) = 0;
	// Get data associated with this lobby
	virtual const char *GetLobbyData(CSteamID SteamIDLobby, const char *pchKey) = 0;
	// Update lobby data (Admin only)
	virtual void SetLobbyData(CSteamID steamIDLobby, const char *pchKey, const char *pchValue) = 0;
	// Get per-user data for someone in this lobby
	virtual const char *GetLobbyMemberData(CSteamID steamIDLobby, CSteamID steamIDUser, const char *pchKey) = 0;
	// Update user data (for you only)
	virtual void SetLobbyMemberData(CSteamID steamIDLobby, const char *pchKey, const char *pchValue) = 0;
	// Send a chat message to the lobby
	virtual bool SendLobbyChatMsg(CSteamID steamIDLobby, const void *pvMsgBody, int cubMsgBody) = 0;
	// Get a chat message entry
	virtual int GetLobbyChatEntry(CSteamID steamIDLobby, int iChatID, CSteamID *pSteamIDUser, void *pvData, int cubData, EChatEntryType *peChatEntryType) = 0;

	// Refreshes metadata for a lobby you're not necessarily in right now
	// you never do this for lobbies you're a member of, only if your
	// this will send down all the metadata associated with a lobby
	// this is an asynchronous call
	// returns false if the local user is not connected to the Steam servers
	// restart are returned by a LobbyDataUpdate_t callback
	virtual bool RequestLobbyData(CSteamID steamIDLobby) = 0;

	// sets the game server associated with the lobby
	// usually at this point, the users will join the specified game server
	// either the IP/Port or the steamID of the game server has to be valid, depending on how you want the clients to be able to connect
	virtual void SetLobbyGameServer(CSteamID steamIDLobby, uint32 unGameServerIP, uint16 unGameServerPort, CSteamID steamIDGameServer) = 0;
};

class ISteamUserStats011
{

};

class ISteamApps008
{
public:

	virtual bool BIsSubscribed() = 0;

	virtual bool BIsLowViolence() = 0;

	virtual bool BIsCybercafe() = 0;

	virtual bool BIsVACBanned() = 0;

	virtual const char* GetCurrentGameLanguage() = 0;

	virtual const char* GetAvailableGameLanguages() = 0;

	// only use this member if you need to check ownership of another game related to yours, a demo for example
	virtual bool BIsSubscribedApp(AppId_t nAppID) = 0;

	// Takes AppID of DLC and checks if the user owns the DLC & if the DLC is installed
	virtual bool BIsDlcInstalled(AppId_t nAppID) = 0;

	// returns the Unix time of the purchase of the app
	virtual uint32 GetEarliestPurchaseUnixTime(AppId_t nAppID) = 0;

	// Checks if the user is subscribed to the current app through a free weekend
	// This function will return false for users who have a retail or other type of license
	// Before using, please ask your Valve technical contact how to package and secure your free weekened

	virtual bool BIsSubscribedFromFreeWeekend() = 0;

	// Returns the number of DLC pieces for the running app
	virtual int GetDLCCount() = 0;

	// Returns metadata for DLC by index, of range [0, GetDLCCount()]
	virtual bool BGetDLCDataByIndex(int iDLC, AppId_t *pAppID, bool *pbAvailable, char *pchName, int cchNameBufferSize) = 0;

	// Install/Uninstall control for optional DLC
	virtual void InstallDLC(AppId_t nAppID) = 0;
	virtual void UninstallDLC(AppId_t nAppID) = 0;

	// Request cd-key for yourself or owned DLC. If you are interested in this
	// data then make sure you provide us with a list of valid keys to be distributed
	// to users when they purchase the game, before the game ships.
	// You'll receive an AppProofOfPurchaseKeyResponse_t callback when
	// the key is available (which may be immediately).
	virtual void RequestAppProofOfPurchaseKey(AppId_t nAppID) = 0;

	virtual bool GetCurrentBetaName(char *pchName, int cchNameBufferSize) = 0; // returns current beta branch name, 'public' is the default branch

	virtual bool MarkContentCorrupt(bool bMissingFilesOnly) = 0; // signal Steam that game files seems corrupt or missing

	virtual uint32 GetInstalledDepots(AppId_t appID, DepotId_t *pvecDepots, uint32 cMaxDepots) = 0;

	// returns current app install folder for AppID, returns folder name length
	virtual uint32 GetAppInstallDir(AppId_t appID, char* pchFolder, uint32 cchFolderBufferSize) = 0;

	virtual bool BIsAppInstalled(AppId_t appID) = 0;

	virtual CSteamID GetAppOwner() = 0;

	virtual const char* GetLaunchQueryParam(const char *pchKey) = 0;

	virtual bool GetDlcDownloadProgress(uint32, uint64*, uint64*) = 0;

	virtual int GetAppBuildId() = 0;
	//below have unknown return types
	virtual __int64 RequestAllProofOfPurchaseKeys() = 0;

	virtual __int64 GetFileDetails(const char *) = 0;

	virtual __int64 GetLaunchCommandLine(char*, int) = 0;
};

class ISteamNetworking005
{
public:
	////////////////////////////////////////////////////////////////////////////////////////////
	// Session-less connection functions
	//    automatically establishes NAT-traversing or Relay server connections

	// Sends a P2P packet to the specified user
	// UDP-like, unreliable and a max packet size of 1200 bytes
	// the first packet send may be delayed as the NAT-traversal code runs
	// if we can't get through to the user, an error will be posted via the callback P2PSessionConnectFail_t
	// see EP2PSend enum above for the descriptions of the different ways of sending packets
	//
	// nChannel is a routing number you can use to help route message to different systems 	- you'll have to call ReadP2PPacket() 
	// with the same channel number in order to retrieve the data on the other end
	// using different channels to talk to the same user will still use the same underlying p2p connection, saving on resources
	virtual bool SendP2PPacket(CSteamID steamIDRemote, const void *pubData, uint32 cubData, EP2PSend eP2PSendType, int nChannel = 0) = 0;

	// returns true if any data is available for read, and the amount of data that will need to be read
	virtual bool IsP2PPacketAvailable(uint32 *pcubMsgSize, int nChannel = 0) = 0;

	// reads in a packet that has been sent from another user via SendP2PPacket()
	// returns the size of the message and the steamID of the user who sent it in the last two parameters
	// if the buffer passed in is too small, the message will be truncated
	// this call is not blocking, and will return false if no data is available
	virtual bool ReadP2PPacket(void *pubDest, uint32 cubDest, uint32 *pcubMsgSize, CSteamID *psteamIDRemote, int nChannel = 0) = 0;

	// AcceptP2PSessionWithUser() should only be called in response to a P2PSessionRequest_t callback
	// P2PSessionRequest_t will be posted if another user tries to send you a packet that you haven't talked to yet
	// if you don't want to talk to the user, just ignore the request
	// if the user continues to send you packets, another P2PSessionRequest_t will be posted periodically
	// this may be called multiple times for a single user
	// (if you've called SendP2PPacket() on the other user, this implicitly accepts the session request)
	virtual bool AcceptP2PSessionWithUser(CSteamID steamIDRemote) = 0;

	// call CloseP2PSessionWithUser() when you're done talking to a user, will free up resources under-the-hood
	// if the remote user tries to send data to you again, another P2PSessionRequest_t callback will be posted
	virtual bool CloseP2PSessionWithUser(CSteamID steamIDRemote) = 0;

	// call CloseP2PChannelWithUser() when you're done talking to a user on a specific channel. Once all channels
	// open channels to a user have been closed, the open session to the user will be closed and new data from this
	// user will trigger a P2PSessionRequest_t callback
	virtual bool CloseP2PChannelWithUser(CSteamID steamIDRemote, int nChannel) = 0;

	// fills out P2PSessionState_t structure with details about the underlying connection to the user
	// should only needed for debugging purposes
	// returns false if no connection exists to the specified user
	virtual bool GetP2PSessionState(CSteamID steamIDRemote, P2PSessionState_t *pConnectionState) = 0;

	// Allow P2P connections to fall back to being relayed through the Steam servers if a direct connection
	// or NAT-traversal cannot be established. Only applies to connections created after setting this value,
	// or to existing connections that need to automatically reconnect after this value is set.
	//
	// P2P packet relay is allowed by default
	virtual bool AllowP2PPacketRelay(bool bAllow) = 0;


	////////////////////////////////////////////////////////////////////////////////////////////
	// LISTEN / CONNECT style interface functions
	//
	// This is an older set of functions designed around the Berkeley TCP sockets model
	// it's preferential that you use the above P2P functions, they're more robust
	// and these older functions will be removed eventually
	//
	////////////////////////////////////////////////////////////////////////////////////////////


	// creates a socket and listens others to connect
	// will trigger a SocketStatusCallback_t callback on another client connecting
	// nVirtualP2PPort is the unique ID that the client will connect to, in case you have multiple ports
	//		this can usually just be 0 unless you want multiple sets of connections
	// unIP is the local IP address to bind to
	//		pass in 0 if you just want the default local IP
	// unPort is the port to use
	//		pass in 0 if you don't want users to be able to connect via IP/Port, but expect to be always peer-to-peer connections only
	virtual SNetListenSocket_t CreateListenSocket(int nVirtualP2PPort, uint32 nIP, uint16 nPort, bool bAllowUseOfPacketRelay) = 0;

	// creates a socket and begin connection to a remote destination
	// can connect via a known steamID (client or game server), or directly to an IP
	// on success will trigger a SocketStatusCallback_t callback
	// on failure or timeout will trigger a SocketStatusCallback_t callback with a failure code in m_eSNetSocketState
	virtual SNetSocket_t CreateP2PConnectionSocket(CSteamID steamIDTarget, int nVirtualPort, int nTimeoutSec, bool bAllowUseOfPacketRelay) = 0;
	virtual SNetSocket_t CreateConnectionSocket(uint32 nIP, uint16 nPort, int nTimeoutSec) = 0;

	// disconnects the connection to the socket, if any, and invalidates the handle
	// any unread data on the socket will be thrown away
	// if bNotifyRemoteEnd is set, socket will not be completely destroyed until the remote end acknowledges the disconnect
	virtual bool DestroySocket(SNetSocket_t hSocket, bool bNotifyRemoteEnd) = 0;
	// destroying a listen socket will automatically kill all the regular sockets generated from it
	virtual bool DestroyListenSocket(SNetListenSocket_t hSocket, bool bNotifyRemoteEnd) = 0;

	// sending data
	// must be a handle to a connected socket
	// data is all sent via UDP, and thus send sizes are limited to 1200 bytes; after this, many routers will start dropping packets
	// use the reliable flag with caution; although the resend rate is pretty aggressive,
	// it can still cause stalls in receiving data (like TCP)
	virtual bool SendDataOnSocket(SNetSocket_t hSocket, void *pubData, uint32 cubData, bool bReliable) = 0;

	// receiving data
	// returns false if there is no data remaining
	// fills out *pcubMsgSize with the size of the next message, in bytes
	virtual bool IsDataAvailableOnSocket(SNetSocket_t hSocket, uint32 *pcubMsgSize) = 0;

	// fills in pubDest with the contents of the message
	// messages are always complete, of the same size as was sent (i.e. packetized, not streaming)
	// if *pcubMsgSize < cubDest, only partial data is written
	// returns false if no data is available
	virtual bool RetrieveDataFromSocket(SNetSocket_t hSocket, void *pubDest, uint32 cubDest, uint32 *pcubMsgSize) = 0;

	// checks for data from any socket that has been connected off this listen socket
	// returns false if there is no data remaining
	// fills out *pcubMsgSize with the size of the next message, in bytes
	// fills out *phSocket with the socket that data is available on
	virtual bool IsDataAvailable(SNetListenSocket_t hListenSocket, uint32 *pcubMsgSize, SNetSocket_t *phSocket) = 0;

	// retrieves data from any socket that has been connected off this listen socket
	// fills in pubDest with the contents of the message
	// messages are always complete, of the same size as was sent (i.e. packetized, not streaming)
	// if *pcubMsgSize < cubDest, only partial data is written
	// returns false if no data is available
	// fills out *phSocket with the socket that data is available on
	virtual bool RetrieveData(SNetListenSocket_t hListenSocket, void *pubDest, uint32 cubDest, uint32 *pcubMsgSize, SNetSocket_t *phSocket) = 0;

	// returns information about the specified socket, filling out the contents of the pointers
	virtual bool GetSocketInfo(SNetSocket_t hSocket, CSteamID *pSteamIDRemote, int *peSocketStatus, uint32 *punIPRemote, uint16 *punPortRemote) = 0;

	// returns which local port the listen socket is bound to
	// *pnIP and *pnPort will be 0 if the socket is set to listen for P2P connections only
	virtual bool GetListenSocketInfo(SNetListenSocket_t hListenSocket, uint32 *pnIP, uint16 *pnPort) = 0;

	// returns true to describe how the socket ended up connecting
	virtual ESNetSocketConnectionType GetSocketConnectionType(SNetSocket_t hSocket) = 0;

	// max packet size, in bytes
	virtual int GetMaxPacketSize(SNetSocket_t hSocket) = 0;
};

class ISteamRemoteStorage014;

class ISteamScreenshots003
{
public:
	// Writes a screenshot to the user's screenshot library given the raw image data, which must be in RGB format.
	// The return value is a handle that is valid for the duration of the game process and can be used to apply tags.
	virtual ScreenshotHandle WriteScreenshot(void *pubRGB, uint32 cubRGB, int nWidth, int nHeight) = 0;

	// Adds a screenshot to the user's screenshot library from disk.  If a thumbnail is provided, it must be 200 pixels wide and the same aspect ratio
	// as the screenshot, otherwise a thumbnail will be generated if the user uploads the screenshot.  The screenshots must be in either JPEG or TGA format.
	// The return value is a handle that is valid for the duration of the game process and can be used to apply tags.
	// JPEG, TGA, and PNG formats are supported.
	virtual ScreenshotHandle AddScreenshotToLibrary(const char *pchFilename, const char *pchThumbnailFilename, int nWidth, int nHeight) = 0;

	// Causes the Steam overlay to take a screenshot.  If screenshots are being hooked by the game then a ScreenshotRequested_t callback is sent back to the game instead. 
	virtual void TriggerScreenshot() = 0;

	// Toggles whether the overlay handles screenshots when the user presses the screenshot hotkey, or the game handles them.  If the game is hooking screenshots,
	// then the ScreenshotRequested_t callback will be sent if the user presses the hotkey, and the game is expected to call WriteScreenshot or AddScreenshotToLibrary
	// in response.
	virtual void HookScreenshots(bool bHook) = 0;

	// Sets metadata about a screenshot's location (for example, the name of the map)
	virtual bool SetLocation(ScreenshotHandle hScreenshot, const char *pchLocation) = 0;

	// Tags a user as being visible in the screenshot
	virtual bool TagUser(ScreenshotHandle hScreenshot, CSteamID steamID) = 0;

	// Tags a published file as being visible in the screenshot
	virtual bool TagPublishedFile(ScreenshotHandle hScreenshot, PublishedFileId_t unPublishedFileID) = 0;

	virtual bool IsScreenshotsHooked() = 0;
	virtual ScreenshotHandle AddVRScreenshotToLibrary(EVRScreenshotType eType, const char *pchFilename, const char *pchVRFilename) = 0;
};

class ISteamHTTP002;

class ISteamUnifiedMessages001
{
public:
	static const ClientUnifiedMessageHandle k_InvalidUnifiedMessageHandle = 0;

	// Sends a service method (in binary serialized form) using the Steam Client.
	// Returns a unified message handle (k_InvalidUnifiedMessageHandle if could not send the message).
	virtual ClientUnifiedMessageHandle SendMethod(const char *pchServiceMethod, const void *pRequestBuffer, uint32 unRequestBufferSize, uint64 unContext) = 0;

	// Gets the size of the response and the EResult. Returns false if the response is not ready yet.
	virtual bool GetMethodResponseInfo(ClientUnifiedMessageHandle hHandle, uint32 *punResponseSize, EResult *peResult) = 0;

	// Gets a response in binary serialized form (and optionally release the corresponding allocated memory).
	virtual bool GetMethodResponseData(ClientUnifiedMessageHandle hHandle, void *pResponseBuffer, uint32 unResponseBufferSize, bool bAutoRelease) = 0;

	// Releases the message and its corresponding allocated memory.
	virtual bool ReleaseMethod(ClientUnifiedMessageHandle hHandle) = 0;

	// Sends a service notification (in binary serialized form) using the Steam Client.
	// Returns true if the notification was sent successfully.
	virtual bool SendNotification(const char *pchServiceNotification, const void *pNotificationBuffer, uint32 unNotificationBufferSize) = 0;
};

class ISteamController005;
class ISteamUGC009;

class ISteamAppList001
{
	// returns 0 if the key does not exist
	// this may be true on first call, since the app data may not be cached locally yet
	// If you expect it to exists wait for the AppDataChanged_t after the first failure and ask again
	virtual int GetAppData(AppId_t nAppID, const char *pchKey, char *pchValue, int cchValueMax) = 0;
};

class ISteamMusic001
{
public:
	virtual bool BIsEnabled() = 0;
	virtual bool BIsPlaying() = 0;

	virtual AudioPlayback_Status GetPlaybackStatus() = 0;

	virtual void Play() = 0;
	virtual void Pause() = 0;
	virtual void PlayPrevious() = 0;
	virtual void PlayNext() = 0;

	// volume is between 0.0 and 1.0
	virtual void SetVolume(float flVolume) = 0;
	virtual float GetVolume() = 0;

};

class ISteamMusicRemote001
{
public:
	// Service Definition
	virtual bool RegisterSteamMusicRemote(const char *pchName) = 0;
	virtual bool DeregisterSteamMusicRemote() = 0;
	virtual bool BIsCurrentMusicRemote() = 0;
	virtual bool BActivationSuccess(bool bValue) = 0;

	virtual bool SetDisplayName(const char *pchDisplayName) = 0;
	virtual bool SetPNGIcon_64x64(void *pvBuffer, uint32 cbBufferLength) = 0;

	// Abilities for the user interface
	virtual bool EnablePlayPrevious(bool bValue) = 0;
	virtual bool EnablePlayNext(bool bValue) = 0;
	virtual bool EnableShuffled(bool bValue) = 0;
	virtual bool EnableLooped(bool bValue) = 0;
	virtual bool EnableQueue(bool bValue) = 0;
	virtual bool EnablePlaylists(bool bValue) = 0;

	// Status
	virtual bool UpdatePlaybackStatus(AudioPlayback_Status nStatus) = 0;
	virtual bool UpdateShuffled(bool bValue) = 0;
	virtual bool UpdateLooped(bool bValue) = 0;
	virtual bool UpdateVolume(float flValue) = 0; // volume is between 0.0 and 1.0

	// Current Entry
	virtual bool CurrentEntryWillChange() = 0;
	virtual bool CurrentEntryIsAvailable(bool bAvailable) = 0;
	virtual bool UpdateCurrentEntryText(const char *pchText) = 0;
	virtual bool UpdateCurrentEntryElapsedSeconds(int nValue) = 0;
	virtual bool UpdateCurrentEntryCoverArt(void *pvBuffer, uint32 cbBufferLength) = 0;
	virtual bool CurrentEntryDidChange() = 0;

	// Queue
	virtual bool QueueWillChange() = 0;
	virtual bool ResetQueueEntries() = 0;
	virtual bool SetQueueEntry(int nID, int nPosition, const char *pchEntryText) = 0;
	virtual bool SetCurrentQueueEntry(int nID) = 0;
	virtual bool QueueDidChange() = 0;

	// Playlist
	virtual bool PlaylistWillChange() = 0;
	virtual bool ResetPlaylistEntries() = 0;
	virtual bool SetPlaylistEntry(int nID, int nPosition, const char *pchEntryText) = 0;
	virtual bool SetCurrentPlaylistEntry(int nID) = 0;
	virtual bool PlaylistDidChange() = 0;
};

class ISteamHTMLSurface003
{
public:
	virtual ~ISteamHTMLSurface003() {}

	// Must call init and shutdown when starting/ending use of the interface
	virtual bool Init() = 0;
	virtual bool Shutdown() = 0;

	// Create a browser object for display of a html page, when creation is complete the call handle
	// will return a HTML_BrowserReady_t callback for the HHTMLBrowser of your new browser.
	//   The user agent string is a substring to be added to the general user agent string so you can
	// identify your client on web servers.
	//   The userCSS string lets you apply a CSS style sheet to every displayed page, leave null if
	// you do not require this functionality.
	virtual SteamAPICall_t CreateBrowser(const char *pchUserAgent, const char *pchUserCSS) = 0;

	// Call this when you are done with a html surface, this lets us free the resources being used by it
	virtual void RemoveBrowser(HHTMLBrowser unBrowserHandle) = 0;

	// Navigate to this URL, results in a HTML_StartRequest_t as the request commences 
	virtual void LoadURL(HHTMLBrowser unBrowserHandle, const char *pchURL, const char *pchPostData) = 0;

	// Tells the surface the size in pixels to display the surface
	virtual void SetSize(HHTMLBrowser unBrowserHandle, uint32 unWidth, uint32 unHeight) = 0;

	// Stop the load of the current html page
	virtual void StopLoad(HHTMLBrowser unBrowserHandle) = 0;
	// Reload (most likely from local cache) the current page
	virtual void Reload(HHTMLBrowser unBrowserHandle) = 0;
	// navigate back in the page history
	virtual void GoBack(HHTMLBrowser unBrowserHandle) = 0;
	// navigate forward in the page history
	virtual void GoForward(HHTMLBrowser unBrowserHandle) = 0;

	// add this header to any url requests from this browser
	virtual void AddHeader(HHTMLBrowser unBrowserHandle, const char *pchKey, const char *pchValue) = 0;
	// run this javascript script in the currently loaded page
	virtual void ExecuteJavascript(HHTMLBrowser unBrowserHandle, const char *pchScript) = 0;

	// Mouse click and mouse movement commands
	virtual void MouseUp(HHTMLBrowser unBrowserHandle, EHTMLMouseButton eMouseButton) = 0;
	virtual void MouseDown(HHTMLBrowser unBrowserHandle, EHTMLMouseButton eMouseButton) = 0;
	virtual void MouseDoubleClick(HHTMLBrowser unBrowserHandle, EHTMLMouseButton eMouseButton) = 0;
	// x and y are relative to the HTML bounds
	virtual void MouseMove(HHTMLBrowser unBrowserHandle, int x, int y) = 0;
	// nDelta is pixels of scroll
	virtual void MouseWheel(HHTMLBrowser unBrowserHandle, int32 nDelta) = 0;


	// keyboard interactions, native keycode is the virtual key code value from your OS
	virtual void KeyDown(HHTMLBrowser unBrowserHandle, uint32 nNativeKeyCode, EHTMLKeyModifiers eHTMLKeyModifiers) = 0;
	virtual void KeyUp(HHTMLBrowser unBrowserHandle, uint32 nNativeKeyCode, EHTMLKeyModifiers eHTMLKeyModifiers) = 0;
	// cUnicodeChar is the unicode character point for this keypress (and potentially multiple chars per press)
	virtual void KeyChar(HHTMLBrowser unBrowserHandle, uint32 cUnicodeChar, EHTMLKeyModifiers eHTMLKeyModifiers) = 0;

	// programmatically scroll this many pixels on the page
	virtual void SetHorizontalScroll(HHTMLBrowser unBrowserHandle, uint32 nAbsolutePixelScroll) = 0;
	virtual void SetVerticalScroll(HHTMLBrowser unBrowserHandle, uint32 nAbsolutePixelScroll) = 0;

	// tell the html control if it has key focus currently, controls showing the I-beam cursor in text controls amongst other things
	virtual void SetKeyFocus(HHTMLBrowser unBrowserHandle, bool bHasKeyFocus) = 0;

	// open the current pages html code in the local editor of choice, used for debugging
	virtual void ViewSource(HHTMLBrowser unBrowserHandle) = 0;
	// copy the currently selected text on the html page to the local clipboard
	virtual void CopyToClipboard(HHTMLBrowser unBrowserHandle) = 0;
	// paste from the local clipboard to the current html page
	virtual void PasteFromClipboard(HHTMLBrowser unBrowserHandle) = 0;

	// find this string in the browser, if bCurrentlyInFind is true then instead cycle to the next matching element
	virtual void Find(HHTMLBrowser unBrowserHandle, const char *pchSearchStr, bool bCurrentlyInFind, bool bReverse) = 0;
	// cancel a currently running find
	virtual void StopFind(HHTMLBrowser unBrowserHandle) = 0;

	// return details about the link at position x,y on the current page
	virtual void GetLinkAtPosition(HHTMLBrowser unBrowserHandle, int x, int y) = 0;

	// set a webcookie for the hostname in question
	virtual void SetCookie(const char *pchHostname, const char *pchKey, const char *pchValue, const char *pchPath = "/", RTime32 nExpires = 0, bool bSecure = false, bool bHTTPOnly = false) = 0;

	// Zoom the current page by flZoom ( from 0.0 to 4.0, so to zoom to 120% use 1.2 ), zooming around point X,Y in the page (use 0,0 if you don't care)
	virtual void SetPageScaleFactor(HHTMLBrowser unBrowserHandle, float flZoom, int nPointX, int nPointY) = 0;

	virtual void SetBackgroundMode(HHTMLBrowser unBrowserHandle, bool bAllowed) = 0;


	// CALLBACKS
	//
	//  These set of functions are used as responses to callback requests
	//

	// You MUST call this in response to a HTML_StartRequest_t callback
	//  Set bAllowed to true to allow this navigation, false to cancel it and stay 
	// on the current page. You can use this feature to limit the valid pages
	// allowed in your HTML surface.
	virtual void AllowStartRequest(HHTMLBrowser unBrowserHandle, bool bAllowed) = 0;

	// You MUST call this in response to a HTML_JSAlert_t or HTML_JSConfirm_t callback
	//  Set bResult to true for the OK option of a confirm, use false otherwise
	virtual void JSDialogResponse(HHTMLBrowser unBrowserHandle, bool bResult) = 0;

	// You MUST call this in response to a HTML_FileOpenDialog_t callback
	virtual void FileLoadDialogResponse(HHTMLBrowser unBrowserHandle, const char **pchSelectedFiles) = 0;
};

class ISteamInventory001
{
public:
	// INVENTORY ASYNC RESULT MANAGEMENT
	//
	// Asynchronous inventory queries always output a result handle which can be used with
	// GetResultStatus, GetResultItems, etc. A SteamInventoryResultReady_t callback will
	// be triggered when the asynchronous result becomes ready (or fails).
	//

	// Find out the status of an asynchronous inventory result handle. Possible values:
	//  k_EResultPending - still in progress
	//  k_EResultOK - done, result ready
	//  k_EResultExpired - done, result ready, maybe out of date (see DeserializeResult)
	//  k_EResultInvalidParam - ERROR: invalid API call parameters
	//  k_EResultServiceUnavailable - ERROR: service temporarily down, you may retry later
	//  k_EResultLimitExceeded - ERROR: operation would exceed per-user inventory limits
	//  k_EResultFail - ERROR: unknown / generic error
	virtual EResult GetResultStatus(SteamInventoryResult_t resultHandle) = 0;

	// Copies the contents of a result set into a flat array. The specific
	// contents of the result set depend on which query which was used.
	virtual bool GetResultItems(SteamInventoryResult_t resultHandle,
		SteamItemDetails_t *pOutItemsArray,
		uint32 *punOutItemsArraySize) = 0;

	// Returns the server time at which the result was generated. Compare against
	// the value of IClientUtils::GetServerRealTime() to determine age.
	virtual uint32 GetResultTimestamp(SteamInventoryResult_t resultHandle) = 0;

	// Returns true if the result belongs to the target steam ID, false if the
	// result does not. This is important when using DeserializeResult, to verify
	// that a remote player is not pretending to have a different user's inventory.
	virtual bool CheckResultSteamID(SteamInventoryResult_t resultHandle, CSteamID steamIDExpected) = 0;

	// Destroys a result handle and frees all associated memory.
	virtual void DestroyResult(SteamInventoryResult_t resultHandle) = 0;


	// INVENTORY ASYNC QUERY
	//

	// Captures the entire state of the current user's Steam inventory.
	// You must call DestroyResult on this handle when you are done with it.
	// Returns false and sets *pResultHandle to zero if inventory is unavailable.
	// Note: calls to this function are subject to rate limits and may return
	// cached results if called too frequently. It is suggested that you call
	// this function only when you are about to display the user's full inventory,
	// or if you expect that the inventory may have changed.
	virtual bool GetAllItems(SteamInventoryResult_t *pResultHandle) = 0;


	// Captures the state of a subset of the current user's Steam inventory,
	// identified by an array of item instance IDs. The results from this call
	// can be serialized and passed to other players to "prove" that the current
	// user owns specific items, without exposing the user's entire inventory.
	// For example, you could call GetItemsByID with the IDs of the user's
	// currently equipped cosmetic items and serialize this to a buffer, and
	// then transmit this buffer to other players upon joining a game.
	virtual bool GetItemsByID(SteamInventoryResult_t *pResultHandle, const SteamItemInstanceID_t *pInstanceIDs, uint32 unCountInstanceIDs) = 0;


	// RESULT SERIALIZATION AND AUTHENTICATION
	//
	// Serialized result sets contain a short signature which can't be forged
	// or replayed across different game sessions. A result set can be serialized
	// on the local client, transmitted to other players via your game networking,
	// and deserialized by the remote players. This is a secure way of preventing
	// hackers from lying about posessing rare/high-value items.

	// Serializes a result set with signature bytes to an output buffer. Pass
	// NULL as an output buffer to get the required size via punOutBufferSize.
	// The size of a serialized result depends on the number items which are being
	// serialized. When securely transmitting items to other players, it is
	// recommended to use "GetItemsByID" first to create a minimal result set.
	// Results have a built-in timestamp which will be considered "expired" after
	// an hour has elapsed. See DeserializeResult for expiration handling.
	virtual bool SerializeResult(SteamInventoryResult_t resultHandle, void *pOutBuffer, uint32 *punOutBufferSize) = 0;

	// Deserializes a result set and verifies the signature bytes. Returns false
	// if bRequireFullOnlineVerify is set but Steam is running in Offline mode.
	// Otherwise returns true and then delivers error codes via GetResultStatus.
	//
	// The bRESERVED_MUST_BE_FALSE flag is reserved for future use and should not
	// be set to true by your game at this time.
	//
	// DeserializeResult has a potential soft-failure mode where the handle status
	// is set to k_EResultExpired. GetResultItems() still succeeds in this mode.
	// The "expired" result could indicate that the data may be out of date - not
	// just due to timed expiration (one hour), but also because one of the items
	// in the result set may have been traded or consumed since the result set was
	// generated. You could compare the timestamp from GetResultTimestamp() to
	// ISteamUtils::GetServerRealTime() to determine how old the data is. You could
	// simply ignore the "expired" result code and continue as normal, or you
	// could challenge the player with expired data to send an updated result set.
	virtual bool DeserializeResult(SteamInventoryResult_t *pOutResultHandle, const void *pBuffer, uint32 unBufferSize, bool bRESERVED_MUST_BE_FALSE = false) = 0;


	// INVENTORY ASYNC MODIFICATION
	//

	// GenerateItems() creates one or more items and then generates a SteamInventoryCallback_t
	// notification with a matching nCallbackContext parameter. This API is insecure, and could
	// be abused by hacked clients. It is, however, very useful as a development cheat or as
	// a means of prototyping item-related features for your game. The use of GenerateItems can
	// be restricted to certain item definitions or fully blocked via the Steamworks website.
	// If punArrayQuantity is not NULL, it should be the same length as pArrayItems and should
	// describe the quantity of each item to generate.
	virtual bool GenerateItems(SteamInventoryResult_t *pResultHandle, const SteamItemDef_t *pArrayItemDefs, const uint32 *punArrayQuantity, uint32 unArrayLength) = 0;

	// GrantPromoItems() checks the list of promotional items for which the user may be eligible
	// and grants the items (one time only).  On success, the result set will include items which
	// were granted, if any. If no items were granted because the user isn't eligible for any
	// promotions, this is still considered a success.
	virtual bool GrantPromoItems(SteamInventoryResult_t *pResultHandle) = 0;

	// AddPromoItem() / AddPromoItems() are restricted versions of GrantPromoItems(). Instead of
	// scanning for all eligible promotional items, the check is restricted to a single item
	// definition or set of item definitions. This can be useful if your game has custom UI for
	// showing a specific promo item to the user.
	virtual bool AddPromoItem(SteamInventoryResult_t *pResultHandle, SteamItemDef_t itemDef) = 0;
	virtual bool AddPromoItems(SteamInventoryResult_t *pResultHandle, const SteamItemDef_t *pArrayItemDefs, uint32 unArrayLength) = 0;

	// ConsumeItem() removes items from the inventory, permenantly. They cannot be recovered.
	// Not for the faint of heart - if your game implements item removal at all, a high-friction
	// UI confirmation process is highly recommended. Similar to GenerateItems, punArrayQuantity
	// can be NULL or else an array of the same length as pArrayItems which describe the quantity
	// of each item to destroy. ConsumeItem can be restricted to certain item definitions or
	// fully blocked via the Steamworks website to minimize support/abuse issues such as the
	// clasic "my brother borrowed my laptop and deleted all of my rare items".
	virtual bool ConsumeItem(SteamInventoryResult_t *pResultHandle, SteamItemInstanceID_t itemConsume, uint32 unQuantity) = 0;

	// ExchangeItems() is an atomic combination of GenerateItems and DestroyItems. It can be
	// used to implement crafting recipes or transmutations, or items which unpack themselves
	// into other items. Like GenerateItems, this is a flexible and dangerous API which is
	// meant for rapid prototyping. You can configure restrictions on ExchangeItems via the
	// Steamworks website, such as limiting it to a whitelist of input/output combinations
	// corresponding to recipes.
	// (Note: although GenerateItems may be hard or impossible to use securely in your game,
	// ExchangeItems is perfectly reasonable to use once the whitelists are set accordingly.)
	virtual bool ExchangeItems(SteamInventoryResult_t *pResultHandle,
		const SteamItemDef_t *pArrayGenerate, const uint32 *punArrayGenerateQuantity, uint32 unArrayGenerateLength,
		const SteamItemInstanceID_t *pArrayDestroy, const uint32 *punArrayDestroyQuantity, uint32 unArrayDestroyLength) = 0;


	// TransferItemQuantity() is intended for use with items which are "stackable" (can have
	// quantity greater than one). It can be used to split a stack into two, or to transfer
	// quantity from one stack into another stack of identical items. To split one stack into
	// two, pass k_SteamItemInstanceIDInvalid for itemIdDest and a new item will be generated.
	virtual bool TransferItemQuantity(SteamInventoryResult_t *pResultHandle, SteamItemInstanceID_t itemIdSource, uint32 unQuantity, SteamItemInstanceID_t itemIdDest) = 0;


	// TIMED DROPS AND PLAYTIME CREDIT
	//

	// Applications which use timed-drop mechanics should call SendItemDropHeartbeat() when
	// active gameplay begins, and at least once every two minutes afterwards. The backend
	// performs its own time calculations, so the precise timing of the heartbeat is not
	// critical as long as you send at least one heartbeat every two minutes. Calling the
	// function more often than that is not harmful, it will simply have no effect. Note:
	// players may be able to spoof this message by hacking their client, so you should not
	// attempt to use this as a mechanism to restrict playtime credits. It is simply meant
	// to distinguish between being in any kind of gameplay situation vs the main menu or
	// a pre-game launcher window. (If you are stingy with handing out playtime credit, it
	// will only encourage players to run bots or use mouse/kb event simulators.)
	//
	// Playtime credit accumulation can be capped on a daily or weekly basis through your
	// Steamworks configuration.
	//
	virtual void SendItemDropHeartbeat() = 0;

	// Playtime credit must be consumed and turned into item drops by your game. Only item
	// definitions which are marked as "playtime item generators" can be spawned. The call
	// will return an empty result set if there is not enough playtime credit for a drop.
	// Your game should call TriggerItemDrop at an appropriate time for the user to receive
	// new items, such as between rounds or while the player is dead. Note that players who
	// hack their clients could modify the value of "dropListDefinition", so do not use it
	// to directly control rarity. It is primarily useful during testing and development,
	// where you may wish to perform experiments with different types of drops.
	virtual bool TriggerItemDrop(SteamInventoryResult_t *pResultHandle, SteamItemDef_t dropListDefinition) = 0;


	// IN-GAME TRADING
	//
	// TradeItems() implements limited in-game trading of items, if you prefer not to use
	// the overlay or an in-game web browser to perform Steam Trading through the website.
	// You should implement a UI where both players can see and agree to a trade, and then
	// each client should call TradeItems simultaneously (+/- 5 seconds) with matching
	// (but reversed) parameters. The result is the same as if both players performed a
	// Steam Trading transaction through the web. Each player will get an inventory result
	// confirming the removal or quantity changes of the items given away, and the new
	// item instance id numbers and quantities of the received items.
	// (Note: new item instance IDs are generated whenever an item changes ownership.)
	virtual bool TradeItems(SteamInventoryResult_t *pResultHandle, CSteamID steamIDTradePartner,
		const SteamItemInstanceID_t *pArrayGive, const uint32 *pArrayGiveQuantity, uint32 nArrayGiveLength,
		const SteamItemInstanceID_t *pArrayGet, const uint32 *pArrayGetQuantity, uint32 nArrayGetLength) = 0;


	// ITEM DEFINITIONS
	//
	// Item definitions are a mapping of "definition IDs" (integers between 1 and 1000000)
	// to a set of string properties. Some of these properties are required to display items
	// on the Steam community web site. Other properties can be defined by applications.
	// Use of these functions is optional; there is no reason to call LoadItemDefinitions
	// if your game hardcodes the numeric definition IDs (eg, purple face mask = 20, blue
	// weapon mod = 55) and does not allow for adding new item types without a client patch.
	//

	// LoadItemDefinitions triggers the automatic load and refresh of item definitions.
	// Every time new item definitions are available (eg, from the dynamic addition of new
	// item types while players are still in-game), a SteamInventoryDefinitionUpdate_t
	// callback will be fired.
	virtual bool LoadItemDefinitions() = 0;

	// GetItemDefinitionIDs returns the set of all defined item definition IDs (which are
	// defined via Steamworks configuration, and not necessarily contiguous integers).
	// If pItemDefIDs is null, the call will return true and *punItemDefIDsArraySize will
	// contain the total size necessary for a subsequent call. Otherwise, the call will
	// return false if and only if there is not enough space in the output array.
	virtual bool GetItemDefinitionIDs(
		SteamItemDef_t *pItemDefIDs,
		uint32 *punItemDefIDsArraySize) = 0;

	// GetItemDefinitionProperty returns a string property from a given item definition.
	// Note that some properties (for example, "name") may be localized and will depend
	// on the current Steam language settings (see ISteamApps::GetCurrentGameLanguage).
	// Property names are always composed of ASCII letters, numbers, and/or underscores.
	// Pass a NULL pointer for pchPropertyName to get a comma - separated list of available
	// property names. 
	virtual bool GetItemDefinitionProperty(SteamItemDef_t iDefinition, const char *pchPropertyName,
		char *pchValueBuffer, uint32 *punValueBufferSize) = 0;

	virtual SteamAPICall_t RequestEligiblePromoItemDefinitionsIDs(CSteamID steamID) = 0;
	virtual bool GetEligiblePromoItemDefinitionIDs(CSteamID steamID, int32 *ItemDefIDs, uint32 *ItemDefIDsArraySize) = 0;

};

class ISteamVideo001
{
public:

	// Get a URL suitable for streaming the given Video app ID's video
	virtual void GetVideoURL(AppId_t unVideoAppID) = 0;

	// returns true if user is uploading a live broadcast
	virtual bool IsBroadcasting(int *pnNumViewers) = 0;
};