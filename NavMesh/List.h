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

	bool Push(T const &item)
	{
		int elem = AddToTail(item);
		Assert(IsValidIndex(elem));
		return IsValidIndex(elem);
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
		Assert(std::is_pointer<T>::value);
		for(int i = 0; i < Count(); i++)
			delete Element(i);
		Purge();
	}

	bool IsEmpty() const { return Count() == 0; }

	// stdlib support and C++ 11 range looping
	typedef T* iterator;
	typedef T const* const_iterator;
	iterator begin() { return Base(); }
	const_iterator begin() const { return Base(); }
	iterator end() { return Base() + Count(); }
	const_iterator end() const { return Base() + Count(); }
};

#endif