#pragma once

template <typename T>
class Array
{
public:
	using size_type = size_t;

protected:
	T* _elements;
	size_type _size;

public:
	constexpr Array(T* elements, size_type size) noexcept : _elements(elements), _size(size) {}

	template <size_type SZ>
	constexpr Array(T (&elements)[SZ]) noexcept : _elements(elements), _size(SZ) {}
	
	constexpr size_type GetSize() const noexcept { return _size; }
	
	constexpr T& operator[](size_type index) noexcept { return _elements[index]; }
	constexpr const T& operator[](size_type index) const noexcept { return _elements[index]; }

	constexpr T* begin() noexcept { return _elements; }
	constexpr T* end() noexcept { return _elements + _size; }
	constexpr const T* begin() const noexcept { return _elements; }
	constexpr const T* end() const noexcept { return _elements + _size; }

	constexpr bool operator==(const Array& other) const noexcept
	{
		if (_size != other._size)
			return false;

		for (size_t i = 0; i < _size; ++i)
			if (_elements[i] != other._elements[i])
				return false;

		return true;
	}
};
