#pragma once

// 2BHook's version, set here and nowhere else. 2BHook.rc's VERSIONINFO (the DLL's file and
// product version), the menu title and the startup log all read it, and release.yml refuses to
// release a tag that doesn't match HOOK_VERSION_STRING.
//
// Scheme: Major.Minor.Patch.Hotfix, as since 1.1.0.0 (the Revelations Update, Jan 2024). Release
// tags carry the first three parts, and the fourth when it isn't 0: v1.2.0, v1.2.3.1 (a hotfix,
// which replaces the release it fixes on Nexus).
//
// Only the numbers are set by hand. The strings are built from them, each as one stringized run
// of tokens, so each is still a single string literal in 2BHook.rc. If Visual Studio's resource
// editor rewrites 2BHook.rc, check that the VERSIONINFO block still uses these macros.

#define HOOK_VERSION_MAJOR			1
#define HOOK_VERSION_MINOR			2
#define HOOK_VERSION_PATCH			4
#define HOOK_VERSION_HOTFIX			0

#define HOOK_VERSION_STRINGIZE_(x)	#x
#define HOOK_VERSION_STRINGIZE(x)	HOOK_VERSION_STRINGIZE_(x)

// As tagged: "1.2.1", or "1.2.3.1" for a hotfix
#if HOOK_VERSION_HOTFIX
#define HOOK_VERSION_STRING			HOOK_VERSION_STRINGIZE(HOOK_VERSION_MAJOR.HOOK_VERSION_MINOR.HOOK_VERSION_PATCH.HOOK_VERSION_HOTFIX)
#else
#define HOOK_VERSION_STRING			HOOK_VERSION_STRINGIZE(HOOK_VERSION_MAJOR.HOOK_VERSION_MINOR.HOOK_VERSION_PATCH)
#endif

// All four parts, for VERSIONINFO: "1.2.1.0"
#define HOOK_VERSION_FILE_STRING	HOOK_VERSION_STRINGIZE(HOOK_VERSION_MAJOR.HOOK_VERSION_MINOR.HOOK_VERSION_PATCH.HOOK_VERSION_HOTFIX)
