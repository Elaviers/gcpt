#pragma once
#include "assert.h"
#include "t_memory.h"
#include "t_array.h"
#include "types.h"
#include "utils.h"
#include <initializer_list>
#include <type_traits>
#include <utility>

class NoSpaceAvailableException : public std::exception
{
public:
	NoSpaceAvailableException() : std::exception("No memory space available!") {}
};

template <typename T, typename Memory = AllocatedMemory<T>>
class TVec
{
private:
	Memory _memory;
	size_t _size;

public:
	TVec() : _size(0) {}
	TVec(const std::initializer_list<T>& items) : TVec()
	{
		if (_memory.Grow(items.size()))
		{
			_size = items.size();
			for (size_t i = 0; i < _size; ++i)
				new (_memory.GetBase() + i) T(items[i]);
		}
		else throw NoSpaceAvailableException();
	}

	template <typename M>
	TVec(const TVec<T, M>& other) : TVec()
	{
		if (_memory.Grow(_size, other._size))
		{
			_size = other._size;
			for (size_t i = 0; i < _size; ++i)
				new (_memory.GetBase() + i) T(other._memory.GetBase()[i]);
		}
		else throw NoSpaceAvailableException();
	}

	template <typename M>
	TVec(TVec<T, M>&& other)
	{
		if (_memory.Grow(_size, other._size))
		{
			_size = other._size;
			for (size_t i = 0; i < _size; ++i)
				new (_memory.GetBase() + i) T(std::move(other._memory.GetBase()[i]));

			other._memory.Clear();
			other._size = 0;
		}
		else throw NoSpaceAvailableException();
	}

	~TVec() { Clear(); }

	TVec& operator=(const TVec& other)
	{
		Clear();
		if (_memory.Grow(0, other._size))
		{
			_size = other._size;
			for (size_t i = 0; i < _size; ++i)
				new (_memory.GetBase() + i) T(other[i]);
		}
		else throw NoSpaceAvailableException();

		return *this;
	}

	TVec& operator=(TVec&& other)
	{
		Clear();
		_size = other._size;
		other._size = 0;
		if (std::is_move_assignable_v<Memory>)
			_memory = std::move(other._memory);
		else
		{
			if (_memory.Grow(0, other._size))
			{
				for (size_t i = 0; i < _size; ++i)
					new (_memory.GetBase() + i) T(other[i]);
			}
			else throw NoSpaceAvailableException();
		}

		return *this;
	}

	constexpr T* begin() noexcept { return _memory.GetBase(); }
	constexpr T* end() noexcept { return _memory.GetBase() + _size; }
	constexpr const T* begin() const noexcept { return _memory.GetBase(); }
	constexpr const T* end() const noexcept { return _memory.GetBase() + _size; }
	constexpr T& operator[](size_t idx) { return _memory.GetBase()[idx]; }
	constexpr const T& operator[](size_t idx) const { return _memory.GetBase()[idx]; }

	constexpr size_t GetSize() const noexcept { return _size; }
	constexpr size_t GetMemorySize() const noexcept { return _memory.GetSize(); }

	bool Reserve(size_t size) { return _memory.Grow(_size, size); }
	void Clear() 
	{ 
		for (size_t i = 0; i < _size; ++i)
			_memory.GetBase()[i].~T();

		_memory.Shrink(0);
		_size = 0;
	}

	template <typename... Args>
	T& Emplace(Args&&... args)
	{
		const size_t newSize = _size + 1;
		if (newSize <= _memory.GetSize() || _memory.Grow(_size, newSize))
		{
			new (_memory.GetBase() + _size++) T(std::forward<Args>(args)...);
			return _memory.GetBase()[_size - 1];
		}

		throw NoSpaceAvailableException();
	}

	template <typename... Args>
	T& EmplaceAt(size_t idx, Args&&... args)
	{
		assert(idx <= _size);
		const size_t newSize = _size + 1;
		if (_memory.MakeGap(_size, idx, 1))
		{
			new (_memory.GetBase() + idx) T(std::forward<Args>(args)...);
			return _memory.GetBase()[idx];
		}

		throw NoSpaceAvailableException();
	}

	T& Add(const T& item) { return Emplace(item); }
	T& Add(T&& item) { return Emplace(std::move(item)); }
	T& Insert(size_t idx, const T& item) { return EmplaceAt(idx, item); }
	T& Insert(size_t idx, T&& item) { return EmplaceAt(idx, std::move(item)); }

	void InsertMultiple(size_t idx, const T* items, size_t count)
	{
		Assert(idx < _size);
		Assert(count > 0);
		const size_t newSize = _size + count;
		if (_memory.MakeGap(_size, idx, count))
		{
			for (size_t i = 0; i < count; ++i)
				new (_memory.GetBase() + idx + i) T(items[i]);
		}
		else throw NoSpaceAvailableException();
	}

	void AddMultiple(const T* items, size_t count) { InsertMultiple(_size, items, count); }

	void RemoveMultiple(size_t beginIdx, size_t count)
	{
		Assert(beginIdx < _size);
		Assert(count > 0);
		const size_t newSize = _size - count;
		for (size_t i = beginIdx; i < beginIdx + count; ++i)
		{
			_memory.GetData()[i].~T();
			new (_memory.GetData() + i) T(std::move(_memory.GetData()[i + count]));
		}

		_size = newSize;
		_memory.Shrink(_size);
	}

	void Remove(size_t idx)
	{
		RemoveMultiple(idx, 1);
	}
};

template <typename T, size_t SIZE>
using TVecStatic = TVec<T, StaticMemory<T, SIZE>>;

template <typename T, size_t SIZE, typename Allocator = DefaultAllocator<T>>
using TVecStaticGrowable = TVec<T, StaticGrowableMemory<T, SIZE, Allocator>>;
