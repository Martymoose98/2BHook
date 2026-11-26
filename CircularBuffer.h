#pragma once

#include "WinInclude.h"

#include <stdint.h>
#include <string.h>
#include <type_traits>

// The memory behind a CircularBuffer: a ring of m_cbRing bytes seen twice, back to back, so
// m_pRing[i] and m_pRing[i + m_cbRing] are the same byte. VirtualAlloc2 reserves the 2 * m_cbRing
// as a placeholder and MapViewOfFile3 maps one pagefile section into both halves. Where those are
// missing or fail (before Windows 10 1803, or a Wine without placeholders) it is a plain
// 2 * m_cbRing allocation instead, and the buffer writes everything twice to keep the halves the
// same.
struct CircularBufferMemory
{
	uint8_t*	m_pRing;		// 2 * m_cbRing bytes, null when nothing could be allocated
	size_t		m_cbRing;
	bool		m_bMirrored;	// the second half is the first half's pages, not a copy of them
};

// cbRing must be a multiple of CircularBufferGranularity(), the size views are mapped in.
size_t CircularBufferGranularity(void);
CircularBufferMemory CircularBufferAllocate(size_t cbRing);
void CircularBufferFree(CircularBufferMemory& Memory);

// A fixed-capacity FIFO of T on a mirrored ring. Any run of elements in it is contiguous, even one
// that wraps past the end of the ring, so a reader always gets one pointer and a count, never two
// pieces: Data() is the whole contents, oldest first, and At() is any run from a position on.
//
// Elements are copied in as bytes and dropped without being destroyed, so T has to be trivially
// copyable. Not thread-safe; the owner locks.
template<typename T>
class CircularBuffer
{
	static_assert(std::is_trivially_copyable_v<T>, "CircularBuffer copies elements as bytes, into memory mapped twice");
	static_assert(std::is_trivially_destructible_v<T>, "CircularBuffer drops elements without destroying them");

public:
	// Room for at least uMinCapacity elements. Capacity() is that rounded up to whole views.
	explicit CircularBuffer(size_t uMinCapacity)
		: m_Memory(), m_pItems(nullptr), m_uCapacity(0), m_uBegin(0), m_uEnd(0)
	{
		// The ring has to end on an element boundary as well as a view boundary, or the element
		// across its end wouldn't line up with its twin at the start.
		const size_t cbGranularity = CircularBufferGranularity();
		const size_t cbStep = (cbGranularity / Gcd(cbGranularity, sizeof(T))) * sizeof(T);
		const size_t cbMin = ((uMinCapacity) ? uMinCapacity : 1) * sizeof(T);
		const size_t cbRing = ((cbMin + cbStep - 1) / cbStep) * cbStep;

		m_Memory = CircularBufferAllocate(cbRing);

		if (m_Memory.m_pRing)
		{
			m_pItems = reinterpret_cast<T*>(m_Memory.m_pRing);
			m_uCapacity = cbRing / sizeof(T);
		}
	}

	~CircularBuffer(void)
	{
		CircularBufferFree(m_Memory);
	}

	CircularBuffer(const CircularBuffer&) = delete;
	CircularBuffer& operator=(const CircularBuffer&) = delete;

	// False when no memory could be had at all; every push then fails.
	bool IsValid(void) const { return m_pItems != nullptr; }
	bool IsMirrored(void) const { return m_Memory.m_bMirrored; }

	size_t Capacity(void) const { return m_uCapacity; }
	size_t Size(void) const { return (size_t)(m_uEnd - m_uBegin); }
	size_t Free(void) const { return m_uCapacity - Size(); }
	bool IsEmpty(void) const { return m_uEnd == m_uBegin; }
	bool IsFull(void) const { return Size() == m_uCapacity; }

	// Positions number the elements in the order they were pushed, from 0, and are never reused,
	// so something kept outside the buffer can name its run of elements by where it starts.
	uint64_t BeginPos(void) const { return m_uBegin; }	// the oldest element
	uint64_t EndPos(void) const { return m_uEnd; }		// the next one pushed

	// The element at uPos, from BeginPos() to EndPos() (an empty run there is fine), and
	// everything after it up to EndPos(), contiguous. Only on a valid buffer.
	const T* At(uint64_t uPos) const { return m_pItems + (size_t)(uPos % m_uCapacity); }

	// The whole contents, oldest first: Size() contiguous elements. Null on an invalid buffer.
	const T* Data(void) const { return (m_pItems) ? At(m_uBegin) : nullptr; }

	// i counts from the oldest, i < Size()
	const T& operator[](size_t i) const { return *At(m_uBegin + i); }
	const T& Front(void) const { return *At(m_uBegin); }
	const T& Back(void) const { return *At(m_uEnd - 1); }

	// Appends all uCount elements, or none and returns false when they don't fit in Free(): what
	// to drop to make room is the caller's call.
	bool PushBack(const T* pItems, size_t uCount)
	{
		if (!m_pItems || uCount > Free())
			return false;

		if (!uCount)
			return true;

		const size_t uIndex = (size_t)(m_uEnd % m_uCapacity);

		// One copy does it when the halves are the same pages: a run that goes past the end of the
		// ring carries on into the second half, which is the ring's start.
		memcpy(m_pItems + uIndex, pItems, uCount * sizeof(T));

		if (!m_Memory.m_bMirrored)
			CopyToTwin(uIndex, pItems, uCount);

		m_uEnd += uCount;
		return true;
	}

	bool PushBack(const T& Item)
	{
		return PushBack(&Item, 1);
	}

	// Drops the uCount oldest elements, or all of them if there are fewer.
	void PopFront(size_t uCount = 1)
	{
		const size_t uSize = Size();

		m_uBegin += (uCount < uSize) ? uCount : uSize;
	}

	void Clear(void)
	{
		m_uBegin = m_uEnd;
	}

private:
	static constexpr size_t Gcd(size_t a, size_t b)
	{
		while (b)
		{
			const size_t r = a % b;

			a = b;
			b = r;
		}

		return a;
	}

	// The fallback's second half is a copy, kept by hand: the run just written at
	// [uIndex, uIndex + uCount) goes again a capacity further on, wrapping at twice the capacity,
	// so each slot k and k + capacity hold the same element.
	void CopyToTwin(size_t uIndex, const T* pItems, size_t uCount)
	{
		const size_t uTwin = uIndex + m_uCapacity;
		const size_t uToEnd = (2 * m_uCapacity) - uTwin;
		const size_t uFirst = (uCount < uToEnd) ? uCount : uToEnd;

		memcpy(m_pItems + uTwin, pItems, uFirst * sizeof(T));
		memcpy(m_pItems, pItems + uFirst, (uCount - uFirst) * sizeof(T));
	}

	CircularBufferMemory	m_Memory;
	T*						m_pItems;
	size_t					m_uCapacity;
	uint64_t				m_uBegin;
	uint64_t				m_uEnd;
};
