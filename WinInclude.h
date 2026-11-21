#pragma once

// Windows.h for 2BHook, with every STATUS_ code coming from ntstatus.h. Include it before any other
// Windows or DirectX header, in place of <Windows.h>.
//
// winnt.h defines a handful of STATUS_ codes itself unless WIN32_NO_STATUS is set, and ntstatus.h,
// which Fluorine's Memory.h includes, defines them all again as NTSTATUS. A file that took
// Windows.h the plain way first got a C4005 for each of them. This is the order the Windows SDK
// gives for using both.
// dinput.h assumes 0x0800 without it, and says so in every file that includes it
#ifndef DIRECTINPUT_VERSION
#define DIRECTINPUT_VERSION 0x0800
#endif

#define WIN32_NO_STATUS
#include <Windows.h>
#undef WIN32_NO_STATUS
#include <ntstatus.h>

// ntstatus.h's codes are cast to NTSTATUS, and winbase.h builds WAIT_OBJECT_0 and the EXCEPTION_
// codes from them. ntdef.h, which declares it, can't be included next to Windows.h; this is its
// declaration, the same one bcrypt.h, winternl.h and Fluorine's WindowsOpaqueStructures.h make.
typedef _Return_type_success_(return >= 0) LONG NTSTATUS;
