#pragma once

template <class T> class IList
{
public:
	virtual const bool Insert(T, size_t) = 0;
	virtual const bool Erase(T, size_t) = 0;
	virtual bool Append(T) = 0;
	virtual bool Prepend(T) = 0;
	virtual bool Push(T) = 0;
	virtual T Pop() = 0;
	virtual T At(unsigned int) = 0;
	virtual const bool Get(size_t, T*) = 0;
	virtual size_t Size() const = 0;
	virtual T Head() = 0;
	virtual T Tail() = 0;
	virtual int Find(T) = 0;
	virtual const bool Resize(size_t) = 0;
	virtual void Clear() = 0;
	virtual const bool Empty() const = 0;
	virtual void Sort(int (__cdecl *pfnCompare)(const T*, const T*)) = 0;
	virtual T operator[](size_t) = 0;
};
