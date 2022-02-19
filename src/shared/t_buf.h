#pragma once
#include "allocator.h"
#include "t_array.h"
#include "types.h"
#include <cassert>
#include <initializer_list>

template <typename T, typename AllocatorType = DefaultAllocator<T>>
class Buffer
{
public:
	using size_type = size_t;
	using value_type = T;

private:
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
		return (v >> 1) == x ? x : v; // just return x if it's already a power of 2
	}

	constexpr static size_t _GoodMemSizeForCount(size_t elementCount) noexcept
	{
		if (elementCount < 8192)
			return _ClosestPowerOf2((uint32)elementCount);

		return ((elementCount / 256) + 1) * 256;
	}

	template <typename _T, typename _Alloc>
	friend class Buffer;

	class Proxy : public AllocatorType, public Array<T>
	{
		friend Buffer;

	protected:
		size_t _memSize;

	public:
		constexpr Proxy() noexcept : Array<T>(nullptr, 0), _memSize(0) {}
		constexpr Proxy(const AllocatorType& allocator) noexcept : AllocatorType(allocator), Array<T>(nullptr, 0), _memSize(0) {}

		constexpr Proxy(const Proxy& other) : AllocatorType((const AllocatorType&)other), Array<T>(nullptr, other._size), _memSize(0) {}
		
		constexpr Proxy(Proxy&& other) noexcept : AllocatorType(std::move((AllocatorType&)other)), Array<T>(other._elements, other._size), _memSize(other._memSize)
		{
			other._elements = nullptr;
			other._size = 0;
			other._memSize = 0;
		}

		constexpr Proxy& operator=(const Proxy& other) 
		{
			AllocatorType::operator=((const AllocatorType&)other);
			this->_elements = nullptr;
			this->_size = other._size;
			this->_memSize = other._memSize;
			return *this;
		}

		constexpr Proxy& operator=(Proxy&& other) noexcept
		{
			AllocatorType::operator=(std::move((AllocatorType&)other));
			this->_elements = other._elements;
			this->_size = other._size;
			this->_memSize = other._memSize;
			other._elements = nullptr;
			other._size = 0;
			other._memSize = 0;
			return *this;
		}
	} _proxy;

	T*& _elements = _proxy._elements;
	size_type& _size = _proxy._size;
	size_type& _memSize = _proxy._memSize;

	//newMemSize must accommodate _size!!!
	constexpr void _Reallocate(size_type newMemSize)
	{
		if (newMemSize != _memSize)
		{
			assert(newMemSize >= _size);

			T* newElements = _proxy.allocate(newMemSize);

			for (size_type i = 0; i < _size; ++i)
				new (newElements + i) T(std::move(_elements[i]));

			_DestroyElementsAndDeallocate();
			_elements = newElements;
			_memSize = newMemSize;
		}
	}

	constexpr void _ReallocateIfLarger(size_type newMemSize)
	{
		if (newMemSize > _memSize)
			_Reallocate(newMemSize);
	}

	constexpr void _ReallocateIfSmaller(size_type newMemSize)
	{
		if (newMemSize < _memSize)
			_Reallocate(newMemSize);
	}

	constexpr void _DestroyElementsAndDeallocate()
	{
		for (size_type i = 0; i < _size; ++i)
			_elements[i].~T();

		_proxy.deallocate(_elements, _memSize);
	}

public:
	constexpr Buffer() noexcept : _proxy() {}

	constexpr Buffer(const AllocatorType& allocator) noexcept : _proxy(allocator) {}
	
	constexpr Buffer(const std::initializer_list<T>& elements, const AllocatorType& allocator = AllocatorType()) :
		_proxy(allocator)
	{
		if (_size = elements.size())
		{
			_memSize = _GoodMemSizeForCount(_size);
			_elements = _proxy.allocate(_memSize);

			size_type i = 0;
			for (const T& elem : elements)
				new (_elements + i++) T(elem);
		}
	}

	constexpr Buffer(const T* elements, size_type size, const AllocatorType& allocator = AllocatorType()) : _proxy(allocator)
	{
		if (_size = size)
		{
			_memSize = _GoodMemSizeForCount(_size);
			_elements = _proxy.allocate(_memSize);

			for (size_type i = 0; i < _size; ++i)
				new (_elements + i) T(elements[i]);
		}
	}

	constexpr Buffer(const Array<T>& array, const AllocatorType& allocator = AllocatorType()) : Buffer(array.begin(), array.GetSize(), allocator) {}

	constexpr Buffer(const Buffer& other) : _proxy(other._proxy)
	{
		if (_size)
		{
			_memSize = _GoodMemSizeForCount(_size);
			_elements = _proxy.allocate(_memSize);
			for (size_type i = 0; i < _size; ++i)
				new (_elements + i) T(other._elements[i]);
		}
	}

	constexpr Buffer(Buffer&& other) noexcept : _proxy(std::move(other._proxy)) {}

	constexpr ~Buffer() noexcept
	{
		_DestroyElementsAndDeallocate();
	}

	constexpr Buffer& operator=(const Buffer& other)
	{
		_proxy = other._proxy;
		if (_size)
		{
			_memSize = _GoodMemSizeForCount(_size);
			_elements = _proxy.allocate(_memSize);
			for (size_type i = 0; i < _size; ++i)
				new (_elements + i) T(other._elements[i]);
		}

		return *this;
	}

	constexpr Buffer& operator=(Buffer&& other) noexcept
	{
		_proxy = std::move(other._proxy);
		return *this;
	}

	constexpr operator Array<T>&() noexcept
	{
		return _proxy;
	}

	constexpr operator const Array<T>&() const noexcept
	{
		return _proxy;
	}

	/*
		SIZE
	*/

	//Amount of elements
	constexpr size_type GetSize() const noexcept { return _size; }
	
	//Amount of elements we have room for
	constexpr size_type GetAllocatedSize() const noexcept { return _memSize; }

	constexpr void Clear()
	{
		_DestroyElementsAndDeallocate();
		_elements = nullptr;
		_size = _memSize = 0;
	}

	constexpr void Shrink(size_type amount)
	{
		if (amount)
		{
			if (amount >= _size)
				return Clear();

			_ReallocateIfSmaller(_GoodMemSizeForCount(_size - amount));

			for (size_type i = _size - 1 - amount; i < _size; ++i)
				_elements[i].~T();
			
			_size -= amount;
		}
	}

	template <typename... Args>
	constexpr void Grow(size_type amount, Args&&... defaultCtorArgs)
	{
		if (amount)
		{
			_ReallocateIfLarger(_GoodMemSizeForCount(_size + amount));
			
			for (size_type i = _size; i < _size + amount; ++i)
				new (_elements + i) T(std::forward<Args>(defaultCtorArgs)...);

			_size += amount;
		}
	}

	template <typename... Args>
	constexpr void SetSize(size_type size, Args&&... defaultCtorArgs)
	{
		if (size > _size) Grow(size - _size, std::forward<Args>(defaultCtorArgs)...);
		else if (size < _size) Shrink(_size - size);
	}

	constexpr void Reserve(size_type count) noexcept
	{
		_ReallocateIfLarger(_size + count);
	}

	/*
		INSERTION
	*/

	template <typename... Args>
	constexpr T& Insert(size_type index, Args&&... ctorArgs)
	{
		_ReallocateIfLarger(_GoodMemSizeForCount(_size + 1));

		for (size_type i = _size; i > index; --i)
			new (_elements + i) T(std::move(_elements[i - 1]));

		new (_elements + index) T(std::forward<Args>(ctorArgs)...);
		
		++_size;
		return _elements[_size - 1];
	}

	template <typename... Args>
	constexpr T& Emplace(Args&&... ctorArgs)
	{
		_ReallocateIfLarger(_GoodMemSizeForCount(_size + 1));
		new (_elements + _size++) T(std::forward<Args>(ctorArgs)...);
		return _elements[_size - 1];
	}

	constexpr T& Add(const T& element)
	{
		return Emplace(element);
	}

	constexpr T& Add(T&& element)
	{
		return Emplace(std::move(element));
	}

	constexpr Buffer& AddMultiple(const T* elements, size_type count)
	{
		if (count)
		{
			_ReallocateIfLarger(_GoodMemSizeForCount(_size + count));

			T* firstAddr = _elements + _size;
			for (size_type i = 0; i < count; ++i)
				new (firstAddr + i) T(elements[i]);

			_size += count;
		}

		return *this;
	}

	constexpr Buffer& AddMultiple(const Array<T>& arr)
	{
		AddMultiple(arr.begin(), arr.GetSize());
		return *this;
	}

	/*
		REMOVAL
	*/

	//from is inclusive, to is exclusive
	constexpr void Remove(size_type from, size_type to)
	{
		if (from > to)
			std::swap(from, to);
		else if (from == to)
			return;

		if (from > _size) return;
		if (to > _size)
		{
			Shrink(_size - from);
			return;
		}

		const size_type elementsRemoved = to - from;
		const size_type newMemSize = _GoodMemSizeForCount(_size - elementsRemoved);
		if (newMemSize != _memSize)
		{
			T* newElements = _proxy.allocate(newMemSize);

			for (size_type i = 0; i < from; ++i)
				new (newElements + i) T(std::move(_elements[i]));

			for (size_type i = to; i < _size; ++i)
				new (newElements + i - elementsRemoved) T(std::move(_elements[i]));

			_DestroyElementsAndDeallocate();
			_elements = newElements;
			_memSize = newMemSize;
		}
		else
		{
			for (size_type i = to; i < _size; ++i)
				new (_elements + i - elementsRemoved) T(std::move(_elements[i]));
		}

		_size -= elementsRemoved;
	}

	constexpr void Remove(size_type index)
	{
		Remove(index, index + 1);
	}

	/*
		MEMBER ACCESS
	*/

	constexpr T& operator[](size_type index) { return _elements[index]; }
	constexpr const T& operator[](size_type index) const { return _elements[index]; }

	/*
		ITERATION
	*/

	constexpr T* begin() noexcept { return _elements; }
	constexpr const T* begin() const noexcept { return _elements; }
	constexpr T* end() noexcept { return _elements ? (_elements + _size) : nullptr; }
	constexpr const T* end() const noexcept { return _elements ? (_elements + _size) : nullptr; }

	/*
		OTHER
	*/

	constexpr bool Matches(const Buffer& other) const
	{
		if (other.GetSize() != _size) return false;

		for (size_type i = 0; i < _size; ++i)
			if (_elements[i] != other._elements[i])
				return false;

		return true;
	}
};

template <typename T>
using VBuffer = Buffer<T, VAllocator<T>>;
