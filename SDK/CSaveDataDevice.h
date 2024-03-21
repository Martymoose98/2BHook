
#ifndef __CSAVE_DATA_DEVICE_H__
#define __CSAVE_DATA_DEVICE_H__

#include "SDK\Common.h"

struct CSaveSlot
{
	uint32_t m_uAccountId;
	BOOL m_bInUse;
};

/*
Size of the struct is 0xD0 (208) bytes
*/
struct CSaveDataDevice
{
	enum Slot : int32_t
	{
		SLOT_SAVE_SYSTEMDATA = -2,
		SLOT_SAVE_GAMEDATA = -1,
		SLOT_SAVE_SLOT0DATA = 0,
		SLOT_SAVE_SLOT1DATA = 1,
		SLOT_SAVE_SLOT2DATA = 2,
	};

	enum Status : uint32_t
	{
		STATUS_SAVE_DATA_INVALID,
		STATUS_SAVE_DATA_READ_SLOTS,
		STATUS_SAVE_DATA_READ,
		STATUS_SAVE_DATA_WRITE,
		STATUS_SAVE_DATA_DELETE
	};

	void* m_pVtable;				//0x0000
	DWORD dwUnk0x08;				//0x0008
	char _0x000C[4];				//0x000C
	CSaveSlot* m_pSaveSlots;		//0x0010
	int32_t m_nMaxSlot;				//0x0018
	DWORD m_dwError;				//0x001C
	QWORD qw0x0020;					//0x0020
	int32_t i0x0028;				//0x0028
	char _0x002C[4];				//0x002C
	union
	{
		uint64_t m_uFlags64;		//0x0030
		struct
		{
			uint32_t m_fFlags;		//0x0030
			Status m_Status;		//0x0034
		};
	};
	int32_t m_nSlot;				//0x0038
	char unk0x003C[4];				//0x003C | Aligment maybe
	HANDLE m_hFile;					//0x0040
	OVERLAPPED m_Overlapped;		//0x0048
	uint32_t m_uBytesToIO;			//0x0068 | maybe nBytesToIO and nBytesIO are a struct
	uint32_t m_uBytesIO;			//0x006C
	void* m_pSlotDataBuffer;		//0x0070
	QWORD qwSlotDataBufferSize;		//0x0078
	void* m_pGameDataBuffer;		//0x0080
	QWORD qwGameDataBufferSize;		//0x0088
	void* m_pSystemDataBuffer;		//0x0090 | includes screen dimensions
	QWORD qwSystemDataBufferSize;	//0x0098
	void* m_pBuffer;				//0x00A0
	QWORD m_nSystemDataBytesIO;		//0x00A8
	QWORD m_nGameDataBytesIO;		//0x00B0
	QWORD m_nSaveDataBytesIO;		//0x00B8
	DWORD dwUnk0xC0;				//0x00C0
	DWORD dwUnk0xC4;				//0x00C4
	QWORD qwUnk0xC8;				//0x00C8 | this is an inline struct
	DWORD dwUnk0xD0;				//0x00D0 | this is a member of unk0xc8
	DWORD dwUnk0xD4;				//0x00D4 | this is a member of unk0xc8
};

#endif // !__CSAVE_DATA_DEVICE_H__