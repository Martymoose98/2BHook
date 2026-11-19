#pragma once

// 2BHook's version, set here and nowhere else. 2BHook.rc's VERSIONINFO (the DLL's file and
// product version), the menu title and the startup log all read it, and release.yml refuses to
// release a tag that doesn't match HOOK_VERSION_STRING.
//
// Scheme: Major.Minor.Patch.Hotfix, as since 1.1.0.0 (the Revelations Update, Jan 2024). Release
// tags carry the first three parts: v1.2.0.
//
// Keep the strings in step with the numbers. RC can't build strings from the numbers, and
// release.yml checks that they agree. If Visual Studio's resource editor rewrites 2BHook.rc,
// check that the VERSIONINFO block still uses these macros.

#define HOOK_VERSION_MAJOR			1
#define HOOK_VERSION_MINOR			2
#define HOOK_VERSION_PATCH			0
#define HOOK_VERSION_HOTFIX			0

#define HOOK_VERSION_STRING			"1.2.0"		// Major.Minor.Patch, as tagged
#define HOOK_VERSION_FILE_STRING	"1.2.0.0"	// all four parts, for VERSIONINFO
