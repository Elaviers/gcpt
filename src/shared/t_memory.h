#pragma once
#include "allocator.h"
#include "utils.h"
#include "assert.h"

#undef CopyMemory

template <typename T, typename Allocator = DefaultAllocator<T>>
class AllocatedMemory
{
	struct Data : public Allocator
	{
		T* memory;
		size_t size;

		Data(const Allocator& allocator) : Allocator(allocator), memory(nullptr), size(0) {}
		Data(Allocator&& allocator, T* memory, size_t size) : Allocator(std::move(allocator)), memory(memory), size(size) {}
	} _data;

	//ex. (x = 64 -> 64 ... x = 65 -> 128...)
	constexpr static uint32 _ClosestPowerOf2(uint32 x) noexcept
	{
		//fill in all lower bits, then add 1
		uint32 v = x | (x >> 1);
		v |= v >> 2;
		v |= v >> 4;
		v |= v >> 8;
		v |= v >> 16;
		++v;
		return (v >> 1) == x ? x : v;
	}

	constexpr static size_t _GoodMemSizeForCount(size_t elementCount) noexcept
	{
		if (elementCount < 8192)
			return _ClosestPowerOf2((uint32)elementCount);

		return ((elementCount / 256) + 1) * 256;
	}

public:
	AllocatedMemory(const Allocator& allocator = Allocator()) : _data(allocator) {}
	AllocatedMemory(Allocator&& allocator) : _data(std::forward<Allocator>(allocator), nullptr, 0) {}
	AllocatedMemory(const AllocatedMemory&) = delete;
	AllocatedMemory(AllocatedMemory<T, Allocator>&& other) : _data(std::move(other._data), other._data.memory, other._data.size)
	{
		other._data.memory = nullptr;
		other._data.size = nullptr;
	}

	~AllocatedMemory() { delete _data.memory; }

	constexpr T* GetBase() const noexcept { return _data.memory; }
	constexpr size_t GetSize() const noexcept { return _data.size; }

	bool Grow(size_t spaceUsed, size_t spaceRequired)
	{
		if (spaceRequired > _data.size)
		{
			size_t newSize = _GoodMemSizeForCount(spaceRequired);
			T* newMemory = _data.allocate(newSize);
			if (!newMemory)
				return false;

			if (std::is_trivially_move_constructible_v<T>)
			{
				Utilities::CopyMemory(_data.memory, newMemory, sizeof(T) * spaceUsed);
			}
			else
			{
				for (size_t i = 0; i < spaceUsed; ++i)
					new (newMemory + i) T(std::move(_data.memory[i]));
			}

			_data.deallocate(_data.memory, _data.size);
			_data.memory = newMemory;
			_data.size = newSize;
		}

		return true;
	}

	bool MakeGap(size_t spaceUsed, size_t gapIdx, size_t gapSize)
	{
		Assert(gapIdx <= spaceUsed);
		const size_t spaceRequired = spaceUsed + gapSize;
		if (spaceRequired > _data.size)
		{
			size_t newSize = _GoodMemSizeForCount(spaceRequired);
			T* newMemory = _data.allocate(newSize);
			if (!newMemory)
				return false;

			if (std::is_trivially_move_constructible_v<T>)
			{
				Utilities::CopyMemory(_data.memory, newMemory, sizeof(T) * gapIdx);
				Utilities::CopyMemory(_data.memory, newMemory + sizeof(T) * (gapIdx + gapSize), sizeof(T) * (spaceUsed - gapIdx));
			}
			else
			{
				for (size_t i = 0; i < gapIdx; ++i)
					new (newMemory + i) T(std::move(_data.memory[i]));

				for (size_t i = gapIdx; i < spaceUsed; ++i)
					new (newMemory + gapIdx + gapSize) T(std::move(_data.memory[i]));
			}

			_data.deallocate(_data.memory, _data.size);
			_data.memory = newMemory;
			_data.size = newSize;
		}
		else
		{
			for (size_t i = spaceUsed; i > gapIdx; --i)
				new (_data.memory + i - 1 + gapSize) T(std::move(_data.memory[i - 1]));
		}

		return true;
	}

	void Shrink(size_t spaceUsed)
	{
		Assert(spaceUsed < _data.size);
		const size_t newSize = _GoodMemSizeForCount(spaceUsed);
		if (newSize < _data.size)
		{
			if (newSize == 0)
			{
				_data.deallocate(_data.memory, _data.size);
				_data.memory = nullptr;
				_data.size = 0;
				return;
			}

			T* newMemory = _data.allocate(newSize);
			if (!newMemory)
				return;

			const size_t moveCount = spaceUsed < newSize ? spaceUsed : newSize;
			if (std::is_trivially_move_constructible_v<T>)
			{
				Utilities::CopyMemory(_data.memory, newMemory, sizeof(T) * moveCount);
			}
			else
			{
				for (size_t i = 0; i < moveCount; ++i)
					new (newMemory + i) T(std::move(_data.memory[i]));
			}

			_data.deallocate(_data.memory, _data.size);
			_data.memory = newMemory;
			_data.size = newSize;
		}
	}
};

template <typename T, size_t SIZE>
class StaticMemory
{
	T _memory[SIZE];

public:
	constexpr T* GetBase() const noexcept { return const_cast<T*>(_memory); }
	constexpr size_t GetSize() noexcept { return SIZE; }
	bool Grow(size_t spaceUsed, size_t spaceRequired) { return spaceRequired < SIZE; }
	bool MakeGap(size_t spaceUsed, size_t gapIdx, size_t gapSize)
	{
		Assert(gapIdx <= spaceUsed);
		if (spaceUsed + gapSize < SIZE) return false;
		for (size_t i = spaceUsed; i > gapIdx; ++i)
			new (_memory + i - 1 + gapSize) T(std::move(_memory[i - 1]));

		return true;
	}
	void Shrink(size_t spaceUsed) { spaceUsed; }
};

template <typename T, size_t SIZE, typename Allocator = DefaultAllocator<T>>
class StaticGrowableMemory
{
	StaticMemory<T, SIZE> _staticMemory;
	AllocatedMemory<T, Allocator> _allocedMemory;
	bool _usingAlloc;

public:
	constexpr T* GetBase() { return _usingAlloc ? _allocedMemory.GetBase() : _staticMemory.GetBase(); }
	constexpr size_t GetSize() { return _usingAlloc ? _allocedMemory.GetSize() : _staticMemory.GetSize(); }
	bool Grow(size_t spaceUsed, size_t spaceRequired)
	{
		if (!_usingAlloc && _staticMemory.Grow(spaceUsed, spaceRequired))
			return true;

		_usingAlloc = true;
		return _allocedMemory.Grow(spaceUsed, spaceRequired);
	}

	bool MakeGap(size_t spaceUsed, size_t gapIdx, size_t gapSize)
	{
		if (!_usingAlloc && _staticMemory.MakeGap(spaceUsed, gapIdx, gapSize))
			return true;

		_usingAlloc = true;
		return _allocedMemory.MakeGap(spaceUsed, gapIdx, gapSize);
	}

	void Shrink(size_t spaceUsed)
	{
		if (_usingAlloc)
		{
			if (_staticMemory.Grow(spaceUsed, spaceUsed))
			{
				Utilities::MoveTypedMemory<T>(_allocedMemory.GetBase(), _staticMemory.GetBase(), spaceUsed);
				_allocedMemory.Shrink(0);
				_usingAlloc = false;
			}
			else
				_allocedMemory.Shrink(spaceUsed);
		}

		if (!_usingAlloc)
		{
			_staticMemory.Shrink(spaceUsed);
		}
	}
};
