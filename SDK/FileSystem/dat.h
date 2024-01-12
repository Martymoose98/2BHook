#pragma once

#include "../Common.h"

struct DATHeader
{
	union {
		char Signature[4];	//0x00
		DWORD dwMagic;		//0x00
	};
	DWORD dwFileCount;		//0x04
	DWORD dwFileTableOffset;//0x08
	DWORD dwExtensionOffset;//0x0C
	DWORD dwNameTableOffset;//0x10
	DWORD dwSizeTableOffset;
	DWORD dwUnknownOffset;
	DWORD dwNull;
};


struct DatafileDesc
{
	void* pDatafiles[2];	//0x00
	char pad[8];
	int m_ObjectId2;
	int m_ObjectId;
	char m_szObjectName2[16];
	char m_szObjectName[16]; //0x30
};