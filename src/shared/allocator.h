#pragma once
#include "types.h"

template <typename T>
class DefaultAllocator
{
public:
	using value_type = T;

	DefaultAllocator() noexcept {}

	template <typename O>
	DefaultAllocator(const DefaultAllocator<O>&) noexcept {}

	template <typename O>
	DefaultAllocator& operator=(const DefaultAllocator<O>& other) noexcept { return *this; }

	template <typename O>
	DefaultAllocator(DefaultAllocator<O>&&) noexcept {}

	template <typename O>
	DefaultAllocator& operator=(DefaultAllocator<O>&& other) noexcept { return *this; }

	bool operator==(const DefaultAllocator& other) const noexcept { return true; }

	constexpr value_type* allocate(size_t size) { return (value_type*)::operator new(sizeof(value_type) * size); }
	constexpr void deallocate(value_type* data, size_t size) noexcept { return ::operator delete(data); }
};

template <typename T>
class VAllocator
{
public:
	using value_type = T;

	VAllocator() noexcept {}

	VAllocator(const DefaultAllocator<T>&) noexcept {}

	template <typename O>
	VAllocator(const VAllocator<O>&) noexcept {}

	template <typename O>
	VAllocator& operator=(const VAllocator<O>& other) noexcept { return *this; }

	template <typename O>
	VAllocator(VAllocator<O>&&) noexcept {}

	template <typename O>
	VAllocator& operator=(VAllocator<O>&& other) noexcept { return *this; }

	bool operator==(const VAllocator& other) const noexcept { return true; }

	virtual value_type* allocate(size_t size) { return (value_type*)::operator new(sizeof(value_type) * size); }
	virtual void deallocate(value_type* data, size_t size) noexcept { return ::operator delete(data); }
};

#include <iostream>

template <typename T>
class DebugAllocator
{
public:
	using value_type = T;

	DebugAllocator() noexcept {}

	template <typename O>
	DebugAllocator(const DebugAllocator<O>&) noexcept {}

	template <typename O>
	DebugAllocator(DebugAllocator<O>&&) noexcept {}

	value_type* allocate(size_t size)
	{
		std::cout << "allocate " << size << 'x' << typeid(T).name() << std::endl;
		return (value_type*)::operator new(sizeof(value_type) * size);
	}

	void deallocate(value_type* data, size_t size)
	{
		std::cout << "deallocate " << size << 'x' << typeid(T).name() << std::endl;
		::operator delete(data);
	}

	bool operator==(const DebugAllocator& other) const noexcept { return true; }
};


template <typename T, size_t SIZE>
class FixedAllocator
{
private:
	T elements[SIZE];
	byte _state[(SIZE / 4) + 1];

	size_t _firstFree;

	enum class EItemState
	{
		FREE = 0,
		START = 0x01,
		CONT = 0x02
	};

	EItemState GetItemState(size_t idx)
	{
		constexpr int mask = 0x03;
		const int shift = idx & mask;
		const byte& states = _state[idx / 4];
		return (EItemState)((mask << shift) >> shift);
	}

	void SetItemState(size_t idx, EItemState state)
	{
		constexpr int mask = 0x03;
		const int shift = idx & mask;
		byte& states = _state[idx / 4];

		states &= ~(mask << shift);
		states |= ((byte)state << shift);
	}

	void SetContItems(size_t beginIdx, size_t endIdx)
	{
		if (endIdx <= beginIdx) return;
		constexpr const int mask = 0x03;
		constexpr const byte state = (byte)EItemState::CONT;
		constexpr const byte fullByte = state | (state << 2) | (state << 4) | (state << 6);

		for (int i = beginIdx; i < endIdx;)
		{
			const int shift = i & mask;

			if (shift == 0 && endIdx - i >= 4)
				state[i / 4] = fullByte;
			else
			{
				byte& states = _state[i / 4];
				states &= ~(mask << shift);
				states |= ((byte)state << shift);
			}
		}
	}

	byte* FindSpace(size_t size)
	{
		for (size_t i = _firstFree; i < SIZE; )
		{
			if (GetItemState(i) != EItemState::FREE)
			{
				++i;
				continue;
			}

			int iSz = 1;
			while (iSz < size && ItemState(i + iSz) == EItemState::FREE);

			if (iSz == size)
			{
				SetItemState(i, EItemState::START);
				
				if (size > 1)
					SetContItems(i + 1, i + size);

				if (GetItemState(i + size) == EItemState::CONT)
					SetItemState(i + size, EItemState::FREE);

				if (i == _firstFree)
					_firstFree = i + size;

				return elements[i];
			}
		}

		return nullptr;
	}

public:
	using value_type = T;

	FixedAllocator() noexcept : _firstFree(0), _state{ 0 }  { /*for (int i = 0; i < sizeof(state); ++i) state[i] = 0;*/ }

	template <typename O>
	FixedAllocator(const FixedAllocator<O, SIZE>) noexcept {}

	template <typename O>
	FixedAllocator(FixedAllocator<O, SIZE>&&) noexcept {}

	constexpr value_type* allocate(size_t size)
	{
		byte* mem = FindSpace(size);

		if (mem)
			return new (mem) value_type[size];

		return nullptr;
	}

	constexpr void deallocate(value_type* data, size_t size)
	{
		if (data >= elements)
		{
			const size_t idx = data - elements;

			if (idx < SIZE)
				SetItemState(idx, EItemState::FREE);

			if (idx < _firstFree)
				_firstFree = idx;
		}
	}

	constexpr bool owns(value_type* data)
	{
		return data >= elements && data < elements + SIZE;
	}
};

template <typename T, size_t FIX_SZ, typename FallbackAlloc = DefaultAllocator<T>>
class FixedAllocatorWithFallback
{
	FixedAllocator<T, FIX_SZ> _fixed;
	FallbackAlloc _fallback;

public:
	using value_type = T;

	FixedAllocatorWithFallback() noexcept {}

	template <typename O>
	FixedAllocatorWithFallback(const FixedAllocatorWithFallback<O, FIX_SZ, FallbackAlloc>&) noexcept {}

	template <typename O>
	FixedAllocatorWithFallback& operator=(const FixedAllocatorWithFallback<O, FIX_SZ, FallbackAlloc>& other) noexcept { return *this; }

	template <typename O>
	FixedAllocatorWithFallback(FixedAllocatorWithFallback<O, FIX_SZ, FallbackAlloc>&&) noexcept {}

	template <typename O>
	FixedAllocatorWithFallback& operator=(FixedAllocatorWithFallback<O, FIX_SZ, FallbackAlloc>&& other) noexcept { return *this; }

	bool operator==(const FixedAllocatorWithFallback& other) const noexcept { return true; }

	constexpr value_type* allocate(size_t size)
	{  
		value_type* val = _fixed.allocate(size);
		return val ? val : _fallback.allocate(size);
	}

	constexpr void deallocate(value_type* data, size_t size) noexcept
	{
		if (_fixed.owns(data)) _fixed.deallocate(data, size);
		else _fallback.deallocate(data, size);
	}
};
