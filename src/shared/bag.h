#pragma once
#include "utils.h"
#include "random.h"

template <typename T>
class Bag
{
private:
	T* _items;
	int _size;
	int _taken;

public:
	Bag() : _size(0), _taken(0), _items(nullptr) {}
	Bag(const Bag&) = delete;
	~Bag() { delete _items; }

	void Clear()
	{
		delete[] _items;
		_taken = 0;
		_size = 0;
	}

	void AddSequence(Random& rand, int size)
	{
		T* newItems = new T[size];
		Utilities::CopyTypedMemory<T>(_items, newItems, _size);

		for (int i = _size; i < size; ++i)
			_items[i] = i;

		for (int i = _size; i < size; ++i)
			Utilities::Swap(_items[i], _items[rand.Next(i, size)]);
		
		delete[] _items;
		_items = newItems;
		_size = size;
	}

	const T* Take() noexcept
	{
		return (_taken < _size) ? _items[_taken++] : nullptr;
	}
};
