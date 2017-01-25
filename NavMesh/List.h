#ifndef __war3source_list_h__
#define __war3source_list_h__

#include "public\IList.h"

#include <sh_vector.h>


template <class T>
class CList : public IList<T>
{
public:
	CList() { this->items = new SourceHook::CVector<T>(); }
	~CList() { delete this->items; }

	bool Insert(T item, unsigned int index)
	{
		size_t size = this->items->size();

		if (index < 0 || index > size)
			return false;

		this->items->insert(this->items->iterAt(index), item);
		return true;
	}

	void Append(T item) { this->items->insert(this->items->end(), item); }

	void Prepend(T item) { this->items->insert(this->items->begin(), item); }

	T At(unsigned int index) { return this->items->at(index); }

	size_t Size() { return this->items->size(); }

	T Head() { return this->items->front(); }

	T Tail() { return this->items->back(); }

	unsigned int Find(T item)
	{
		size_t size = this->items->size();

		for (unsigned int i = 0; i < size; i++)
		{
			if (this->items->at(i) != item)
				continue;

			return i;
		}

		return -1;
	}

	bool Push(T item) { return this->items->push_back(item); }

	void PopList() { this->items->pop_back(); }

	bool Resize(size_t newSize) { return this->items->resize(newSize); }

private:
	SourceHook::CVector<T> *items;
};

#endif