#ifndef __war3source_list_h__
#define __war3source_list_h__

#include "tier1/utlvector.h"
#include "tier0/dbg.h"


/*
 * CUtlVector wrapper that has some helper functions
 * and automatically frees itself and the contained data
 */
template <class T> class CList : public CCopyableUtlVector<T>
{
	typedef CCopyableUtlVector<T> BaseClass;
public:
	CList() : BaseClass() {}
	CList(int count) : BaseClass() { EnsureCapacity(count); SetCount(count); }
	~CList() { Purge(); }

	bool Push(T item) { return !!AddToTail(item); }

	T Pop()
	{
		T item = Tail();
		FindAndRemove(item);
		return item;
	}

	const bool Resize(int newSize)
	{
		SetCount(newSize);
		Assert(IsValidIndex(newSize));
		return Count() == newSize;
	}
	
	// Shorthand for PurgeAndDeleteElements()
	void Clear()
	{
		Assert(Element(0));
		for(int i = 0; i < Count(); i++)
			delete Element(i);
		Purge();
	}

	const bool Empty() const { return Count() == 0; }

	const bool operator!() const
	{
		return !Base();
	}

	// stdlib support and C++ 11 range looping
	typedef T* iterator;
	iterator begin() { return Base(); }
	const iterator begin() const { return Base(); }
	iterator end() { return Base() + Count(); }
	const iterator end() const { return Base() + Count(); }
};

#endif