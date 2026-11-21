#pragma once

#include "../Common.h"

#include <string.h>

// PlatinumGames' DAT / DTT container. An object's files come as a pair, its .dat and the .dtt that
// holds its textures (CObj::m_pDataFiles, CTextureBatch::m_pHeaders), and the game reads files out
// of the pair through DataFile::QueryFileIndex (0x14026B400) and DATFile::GetFilePointer. The
// layout and the lookups are ExcitedDelirium's (DataFile.h, UiSystem.cpp).
//
// A DAT holds dwFileCount files. Five tables, each at an offset from the start of the header, are
// indexed by file number:
//   dwFileTableOffset  DWORD[count]           each file's data, as an offset from the header
//   dwExtensionOffset  char[count][4]         each file's extension, without the dot ("wmb", "wta")
//   dwNameTableOffset  DWORD stride, then char[count][stride], each name zero-terminated
//   dwSizeTableOffset  DWORD[count]           each file's size in bytes
//   dwFileCrcOffset    DATCrcTable            a hash table of the names' CRC32s, for QueryFileIndex

#define DAT_MAGIC			0x00544144	// "DAT\0"
#define DAT_FILE_NOT_FOUND	0xFFFFFFFF

// The two halves of an object's pair, and the bit QueryFileIndex packs that into its answer:
// (file of the pair << 28) | file number
#define DATFILE_DATA		0
#define DATFILE_TEXTURES	1
#define DATFILE_HANDLE(iDatFile, uIndex)	(((DWORD)(iDatFile) << 28) | (DWORD)(uIndex))
#define DATFILE_HANDLE_FILE(dwHandle)		((dwHandle) >> 28)
#define DATFILE_HANDLE_INDEX(dwHandle)		((dwHandle) & 0x0FFFFFFF)

// Size of struct 0x20 (32) bytes
struct DATHeader
{
	union {
		char Signature[4];	//0x00 | "DAT\0"
		DWORD dwMagic;		//0x00 | DAT_MAGIC
	};
	DWORD dwFileCount;		//0x04
	DWORD dwFileTableOffset;//0x08
	DWORD dwExtensionOffset;//0x0C
	DWORD dwNameTableOffset;//0x10
	DWORD dwSizeTableOffset;//0x14
	DWORD dwFileCrcOffset;	//0x18 | the CRC table (DATCrcTable), 0 when there is none; was dwUnknownOffset
	DWORD dwNull;			//0x1C
};
VALIDATE_SIZE(DATHeader, 0x20);
VALIDATE_OFFSET(DATHeader, dwFileCount, 0x04);
VALIDATE_OFFSET(DATHeader, dwFileCrcOffset, 0x18);

// At dwFileCrcOffset. Its own offsets are from the start of this table. A name's CRC32 picks a
// bucket by its top bits (crc >> m_iShift); the bucket holds the position of the first of the
// sorted CRCs in that bucket, and the index array maps each position back to a file number.
struct DATCrcTable
{
	int32_t m_iShift;			//0x00
	DWORD m_dwBucketOffset;		//0x04 | WORD per bucket, 0xFFFF for an empty one
	DWORD m_dwCrcOffset;		//0x08 | DWORD[count], sorted
	DWORD m_dwIndexOffset;		//0x0C | WORD[count]
};
VALIDATE_SIZE(DATCrcTable, 0x10);

struct DatafileDesc
{
	void* pDatafiles[2];	//0x00
	char pad[8];
	int m_ObjectId2;
	int m_ObjectId;
	char m_szObjectName2[16];
	char m_szObjectName[16]; //0x30
};

// Reading a DAT. Each takes the header (the start of the file in memory) and checks nothing
// beyond what it says, so check DatFile_IsValid on a pointer you didn't get from a loaded object:
// a file the game is still reading starts with something else (a CRILAYLA-compressed one does).

inline BOOL DatFile_IsValid(const DATHeader* pHeader)
{
	return pHeader && pHeader->dwMagic == DAT_MAGIC;
}

inline DWORD DatFile_GetFileCount(const DATHeader* pHeader)
{
	return pHeader->dwFileCount;
}

inline const BYTE* DatFile_GetBase(const DATHeader* pHeader)
{
	return (const BYTE*)pHeader;
}

// The name of file uIndex, as stored ("pl0000.wmb")
inline const char* DatFile_GetFileName(const DATHeader* pHeader, DWORD uIndex)
{
	const BYTE* pNames = DatFile_GetBase(pHeader) + pHeader->dwNameTableOffset;
	DWORD dwStride = *(const DWORD*)pNames;

	return (const char*)(pNames + sizeof(DWORD) + (SIZE_T)uIndex * dwStride);
}

// Its extension, without the dot. Four bytes, zero-padded, so a three-letter one ends in a zero.
inline const char* DatFile_GetFileExtension(const DATHeader* pHeader, DWORD uIndex)
{
	return (const char*)(DatFile_GetBase(pHeader) + pHeader->dwExtensionOffset + (SIZE_T)uIndex * 4);
}

inline DWORD DatFile_GetFileSize(const DATHeader* pHeader, DWORD uIndex)
{
	return ((const DWORD*)(DatFile_GetBase(pHeader) + pHeader->dwSizeTableOffset))[uIndex];
}

inline void* DatFile_GetFileData(const DATHeader* pHeader, DWORD uIndex)
{
	DWORD dwOffset = ((const DWORD*)(DatFile_GetBase(pHeader) + pHeader->dwFileTableOffset))[uIndex];

	return (void*)(DatFile_GetBase(pHeader) + dwOffset);
}

// A file's number by its name, case-insensitively, or DAT_FILE_NOT_FOUND. A walk of the name
// table, as CObjReadWork_QueryFilePointer (0x14026ACD0) does it; the game's other lookup, through
// the CRC table, needs the game's CRC32 of the lowercased name (DatFile_FindFileByCrc).
inline DWORD DatFile_FindFile(const DATHeader* pHeader, const char* szName)
{
	for (DWORD i = 0; i < pHeader->dwFileCount; ++i)
	{
		if (!_stricmp(DatFile_GetFileName(pHeader, i), szName))
			return i;
	}

	return DAT_FILE_NOT_FOUND;
}

// The same through the CRC table, as DataFile::QueryFileIndex (0x14026B400) does it: uCrc32 is the
// CRC32 of the lowercased name (HashLowercaseStringNoLenCRC32). DAT_FILE_NOT_FOUND when the DAT
// has no CRC table or the name isn't in it.
inline DWORD DatFile_FindFileByCrc(const DATHeader* pHeader, DWORD uCrc32)
{
	if (!pHeader->dwFileCrcOffset || !pHeader->dwFileCount)
		return DAT_FILE_NOT_FOUND;

	const BYTE* pTableBase = DatFile_GetBase(pHeader) + pHeader->dwFileCrcOffset;
	const DATCrcTable* pTable = (const DATCrcTable*)pTableBase;
	const WORD* pBuckets = (const WORD*)(pTableBase + pTable->m_dwBucketOffset);
	const DWORD* pCrcs = (const DWORD*)(pTableBase + pTable->m_dwCrcOffset);
	const WORD* pIndices = (const WORD*)(pTableBase + pTable->m_dwIndexOffset);

	WORD uFirst = pBuckets[uCrc32 >> pTable->m_iShift];

	if (uFirst == 0xFFFF)
		return DAT_FILE_NOT_FOUND;

	// The bucket's CRCs are the run that shares its top bits
	for (DWORD uSlot = uFirst; uSlot < pHeader->dwFileCount && (pCrcs[uSlot] >> pTable->m_iShift) == (uCrc32 >> pTable->m_iShift); ++uSlot)
	{
		if (pCrcs[uSlot] == uCrc32)
			return pIndices[uSlot];
	}

	return DAT_FILE_NOT_FOUND;
}

// A file of an object's pair (CObj::m_pDataFiles) by name: the .dat first, then the .dtt, as
// DATFile::GetFilePointer (0x14026AD90) looks. Its data and size, or NULL.
inline void* DatFile_FindInPair(DATHeader* const pPair[2], const char* szName, DWORD* pdwSize = NULL)
{
	for (int iDatFile = DATFILE_DATA; iDatFile <= DATFILE_TEXTURES; ++iDatFile)
	{
		const DATHeader* pHeader = pPair[iDatFile];

		if (!DatFile_IsValid(pHeader))
			continue;

		DWORD uIndex = DatFile_FindFile(pHeader, szName);

		if (uIndex == DAT_FILE_NOT_FOUND)
			continue;

		if (pdwSize)
			*pdwSize = DatFile_GetFileSize(pHeader, uIndex);

		return DatFile_GetFileData(pHeader, uIndex);
	}

	return NULL;
}
