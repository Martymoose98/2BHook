#pragma once

#define CPK_MAGIC FOURCC('C', 'P', 'K', ' ')

typedef struct CPK_HEADER
{
	union 
	{ 
		char Signature[4];
		DWORD Magic; 
	};
	DWORD dwDataOffset;
	DWORD dwFileCount;
	DWORD dwFileTableOffset;
	DWORD dwExtensionTableOffset;
	DWORD dwNameTableOffset;
	DWORD dwSizeTableOffset;
	DWORD dwUnknownOffset1C;
	DWORD dwUnknown20;
};