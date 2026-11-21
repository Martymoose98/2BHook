#pragma once

// TODO: maybe temp include stdint.h
#include "../WinInclude.h"

#include <stdint.h>

#include "../Math.h"

//#include "tgmath.h" wtf???

//#define PURE = 0

#define FLAGS_FREE 1

#define FILTER_UI_NOISE_MAX 0x20000
#define FILTER_UI_NOISE_LV3 0x1000000
#define FILTER_UI_NOISE_LV2 0x2000000
#define FILTER_UI_NOISE_LV1 0x4000000	
#define FILTER_UI_NOISE_LV0 0x8000000

#define FILTER_2B_DIE_LV0 FILTER_UI_NOISE_LV0
#define FILTER_2B_DIE_LV1 FILTER_UI_NOISE_LV1
#define FILTER_2B_DIE_LV2 (FILTER_UI_NOISE_LV2 | 0x400)
#define FILTER_2B_DIE_LV3 (FILTER_UI_NOISE_LV3 | 0x400)

#define FILTER_2B_DIE_CLEAR 	// 0xF07DFFFF

#define DATAFILE_DATA	0
#define DATAFILE_MODEL	1

#define SAVE_FLAGS_READ_SLOTS	0x00000001
#define SAVE_FLAGS_READ			0x00000002
#define SAVE_FLAGS_WRITE		0x00000003
#define SAVE_FLAGS_DELETE		0x00000004

#define SAVE_SYSTEMDATA -2
#define SAVE_GAMEDATA -1
#define SAVE_SLOT0DATA 0
#define SAVE_SLOT1DATA 1
#define SAVE_SLOT2DATA 2

#define IS_SAVE_SLOTDATA(s) ((s) > -1 && (s) < 3)

#define DBGFLGS_UNKNOWN_0		0x0020
#define DBGFLGS_UNKNOWN_1		0x0080
#define DBGFLGS_SELFDESTRUCT	0x0400
#define DBGFLGS_UNKNOWN_2		0x1000
#define DBGFLGS_SLOWTIME		0x2000
#define DBGFLGS_UNKNOWN_3		0x4000
#define DBGFLGS_UNKNOWN_4		0x8000

#define GAMEFLGS_UNK_2 0x200
#define GAMEFLGS_DRAWMODELS_GLOBAL 0x800 // not sure
#define GAMEFLGS_DRAWMODELS_0 0x8000

#define DBGFLGS_MAX_VALUE (DBGFLGS_UNKNOWN_0 | DBGFLGS_UNKNOWN_1 | DBGFLGS_SELFDESTRUCT | DBGFLGS_UNKNOWN_2 | DBGFLGS_SLOWTIME | DBGFLGS_UNKNOWN_3 |  DBGFLGS_UNKNOWN_4)

#define BIG_ENDIAN

// visual studio says correct offsets are wrong before compalation on interfaces rifk
#ifndef VALIDATE_OFFSET
#define VALIDATE_OFFSET(s, m, offset) static_assert(offsetof(s, m) == (offset),#s"::"#m" is not at "#offset)
#endif

#ifndef VALIDATE_SIZE
#define VALIDATE_SIZE(s, size) static_assert(sizeof(s) == (size), #s" is not of size "#size)
#endif

// https://archive.org/download/CRI-SDKs

typedef unsigned __int64 QWORD;


// Filler for unknown type ptrs
struct Unknown_t;

/*
Nier Automata's lib::Array<T>. Its vtable has five slots (ExcitedDelirium, LibContainers.h, read out
of lib::StaticArray<int,64,4>::`vftable' at 0x140CEAD10), and their bodies are the game's, small
enough to copy. 2BHook never makes one, so a call on one of the game's arrays still goes through
the game's vtable. Slots 2 and 3 were declared as GetItem and Find, but they append and insert.
*/
template<typename T>
class Array
{
public:
	// Slot 0, the scalar deleting destructor: empties the array. The game then frees the object when
	// bit 0 of flags is set, with its own allocator; 2BHook never deletes one of the game's arrays.
	virtual Array<T>* Clear(BYTE flags)
	{
		UNREFERENCED_PARAMETER(flags);

		if (m_pItems && m_Count)
			m_Count = 0;

		m_pItems = NULL;
		m_Size = 0;
		return this;
	}

	// Slot 1: the capacity, m_Size, not the live count
	virtual size_t GetSize(void) { return m_Size; }

	// Slot 2: appends a copy of *pItem. False when there is no buffer or it is full.
	virtual bool PushBack(IN const T* pItem)
	{
		if (!m_pItems || m_Count >= m_Size)
			return false;

		m_pItems[m_Count++] = *pItem;
		return true;
	}

	// Slot 3: inserts a copy of *pItem at pWhere, moving the rest up one. Answers pWhere, or the end
	// of the live items when it refuses (no buffer, full, or pWhere outside them), which is how the
	// game's callers tell.
	virtual T* Insert(IN T* pWhere, IN const T* pItem)
	{
		T* pEnd = m_pItems + m_Count;

		if (!m_pItems || m_Count >= m_Size || (size_t)(pWhere - m_pItems) > m_Count)
			return pEnd;

		for (T* pWalk = pEnd; pWalk != pWhere; --pWalk)
			*pWalk = *(pWalk - 1);

		*pWhere = *pItem;
		++m_Count;
		return pWhere;
	}

	// Slot 4: reserve. Empty here; lib::DynamicArray's grows the buffer before PushBack and Insert.
	virtual void Reserve(size_t uCapacity) { UNREFERENCED_PARAMETER(uCapacity); }

	T& operator[](int32_t i) { return m_pItems[i]; }

	//T* operator[](int32_t i) { return m_pItems[i]; }

	T* m_pItems;	//0x0008 
	size_t m_Count;	//0x0010
	size_t m_Size;	//0x0018
};

/*
Nier Automata's lib::StaticArray<T, size, element_size>
*/
template<typename T, size_t size, size_t element_size = sizeof(T)>
class StaticArray : public Array<T>
{
public:
	T m_Items[size];	//0x0020
};

/*
Nier Automata's lib::AllocatedArray<T>
*/
template<typename T>
class AllocatedArray
{
public:
	T* m_pItems;	//0x0008 
	size_t m_Count;	//0x0010
	size_t m_Size;	//0x0018
};

/*
Nier Automata's lib::DynamicArray<T, A> : lib::Array<T>
A = allocator
*/
template<typename T, typename A = void>
class DynamicArray : public Array<T>
{

};

template<typename T>
class CRedBlackTreeNode
{
	int32_t m_iColour;					// 0x00  RB colour / balance factor
	int32_t pad04;
	CRedBlackTreeNode<T>* m_pParent;	//0x08	(inferred — zeroed here, set by the insert helper)
	CRedBlackTreeNode<T>* m_pRight;		//0x10	taken when node.key < search key
	CRedBlackTreeNode<T>* m_pLeft;		//0x18	taken when node.key > search key
	T m_Data;							//0x20
};

// Hw::cRBTreeNodeTemp<T>
template<typename T>
class CRedBlackTreeNodeTemp
{
	BOOL m_bRoot;
	T* m_pRoot;
	T* m_pLeft;
	T* m_pRight;
	char data20[0x90];
	T* m_pNext;
	T* m_pPrevious;
};

template<typename T>
class CRedBlackTree
{

};

template<typename T>
class FunctionExporterF
{
	void* m_pVtbl;
	T m_Function;
};


template<typename R, typename T, typename... Args>
class FunctionExporterF<R(T::*)(Args...)>;

template<typename R, typename... Args>
class FunctionExporterF<R(*)(Args...)>;


// Size of structiure is 0x30 (48) bytes 
struct CReadWriteLock
{
	CRITICAL_SECTION m_CriticalSection;
	BOOL m_bCriticalSectionInitialized;

	CReadWriteLock(void)
	{
		InitializeCriticalSection(&m_CriticalSection);
		m_bCriticalSectionInitialized = TRUE;
	}

	CReadWriteLock(DWORD dwSpinCount)
	{
		// It can fail (before Vista, out of memory); Lock and Unlock then do nothing
		m_bCriticalSectionInitialized = InitializeCriticalSectionAndSpinCount(&m_CriticalSection, dwSpinCount);
	}

	~CReadWriteLock(void)
	{
		DeleteCriticalSection(&m_CriticalSection);
		m_bCriticalSectionInitialized = FALSE;
	}

	bool Lock(void)
	{
		if (m_bCriticalSectionInitialized)
			EnterCriticalSection(&m_CriticalSection);

		return m_bCriticalSectionInitialized;
	}

	void Unlock(void)
	{
		if (m_bCriticalSectionInitialized)
			LeaveCriticalSection(&m_CriticalSection);
	}
};