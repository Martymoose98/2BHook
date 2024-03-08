// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

// add headers that you want to pre-compile here
// TODO: vvvvvv
// https://www.unknowncheats.me/forum/c-and-c-/160171-wow32reserved-hook.html
// https://github.com/rwfpl/rewolf-wow64ext
// https://github.com/JustasMasiulis/wow64pp
// https://github.com/stevemk14ebr/PolyHook_2_0

// include NT
//#define NT_INCLUDED
#define UMDF_USING_NTSTATUS

#include <Windows.h>
//#include <ntdef.h>

#include "WindowsOpaqueStructures.h"

#endif //PCH_H
