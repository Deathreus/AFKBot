#ifndef __war3source_ilist_h__
#define __war3source_ilist_h__

template <class T>
class IList
{
public:
	virtual bool Insert(T item, unsigned int index) = 0;
	virtual void Append(T item) = 0;
	virtual void Prepend(T item) = 0;
	virtual T At(unsigned int index) = 0;
	virtual size_t Size() = 0;
	virtual T Head() = 0;
	virtual T Tail() = 0;
	virtual unsigned int Find(T item) = 0;
	virtual bool Push(T item) = 0;
	virtual void PopList() = 0;
	virtual bool Resize(size_t newSize) = 0;
};

#endif