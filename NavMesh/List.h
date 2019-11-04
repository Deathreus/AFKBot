#ifndef __war3source_list_h__
#define __war3source_list_h__

#include "tier1/utlvector.h"
#include "tier0/dbg.h"


/*
 * CUtlVector wrapper that has some helper functions
 */
template <class T> class CList : public CUtlVector<T>
{
	typedef CUtlVector<T> BaseClass;
public:
	CList() : BaseClass() {}
	CList(int count) : BaseClass() { EnsureCapacity(count); SetCount(count); }

	bool Push(T const &item) { return !!AddToTail(item); }

	T &Pop()
	{
		T item = Tail();
		FastRemove(m_Size);
		return item;
	}

	bool Resize(int newSize)
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

	bool Empty() const { return Count() == 0; }

	const bool operator!() const
	{
		return !Base();
	}

	// stdlib support and C++ 11 range looping
	typedef T* iterator;
	typedef T const* const_iterator;
	iterator begin() { return Base(); }
	const_iterator begin() const { return Base(); }
	iterator end() { return Base() + Count(); }
	const_iterator end() const { return Base() + Count(); }
};

#endif