#pragma once

#include "WinInclude.h"
#include <xmllite.h>

#include <functional>

// Replaces one section (a child element of the root) of an XML config on disk, and copies
// every other node across as it is. Save Appearance uses it to write the <Appearance> section
// alone, so settings that were changed but not saved don't reach the file.
//
// WriteSection writes the whole section element: its start, children and end. It is called
// where the old section was, or just before the root's end if the file has none.
//
// The new file is written next to szPath as "<szPath>.tmp" and then moved over szPath. On
// any failure, including a file that doesn't parse, szPath is left as it was and the temp
// file is deleted. Returns HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) if szPath doesn't exist.
HRESULT RewriteXmlSection(LPCWSTR szPath, LPCWSTR szSection, const std::function<HRESULT(IXmlWriter*)>& WriteSection);
