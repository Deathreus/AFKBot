#pragma once

#include "tier1/utlvector.h"
#include "tier0/dbg.h"


#define ForEachItem(items, iter) \
	for (int iter = 0, iSize = items.Count(); iter < iSize; iter++)

/*
 * CUtlVector wrapper that has some helper functions
 * and automatically frees itself and the contained data
 */
template <class T> class CList : public CUtlVector<T>
{
public:
	CList() : CUtlVector() {}
	CList(const CList &other) : CUtlVector() { AddVectorToTail(other); }
	CList(int count) : CUtlVector() { EnsureCapacity(count); SetCount(count); }
	~CList() { Purge(); }

	bool Push(T item) { return !!AddToTail(item); }

	T Pop()
	{
		T item = Tail();
		FindAndRemove(item);
		return item;
	}

	const bool Resize(size_t newSize)
	{
		SetCount(newSize);
		return m_Size == newSize;
	}
	
	void Clear(bool bDelete)
	{
		if (bDelete)
		{
			for (int i = 0; i < m_Size; i++)
				delete Element(i);
		}
		Purge();
	}

	const bool Empty() const { return Count() == 0; }

	const bool operator!() const
	{
		return !Base();
	}
};