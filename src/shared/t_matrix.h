#pragma once
#include "allocator.h"
#include "utils.h"

template <typename T, typename AllocatorType = DefaultAllocator<T>>
class Matrix
{
public:
	using size_type = size_t;

private:
	class Proxy : public AllocatorType
	{
		size_type width;
		size_type height;
		T* elements;

		constexpr Proxy() : AllocatorType(), width(0), height(0), elements(nullptr) {}
		constexpr Proxy(const Proxy&) = delete;
		constexpr Proxy(Proxy&&) = delete;
	} _proxy;

protected:
	size_type& _width = _proxy.width;
	size_type& _height = _proxy.height;
	T*& _elements = _proxy.elements;

public:
	constexpr Matrix() {}
	constexpr ~Matrix() { Clear(); }

	constexpr T& At(size_type x, size_type y) { return _elements[y * _width + x]; }
	constexpr const T& At(size_type x, size_type y) const { return _elements[y * _width + x]; }

	constexpr size_type GetWidth() const { return _width; }
	constexpr size_type GetHeight() const { return _height; }

	constexpr void Clear()
	{
		size_type sz = _width * _height;
		for (size_type i = 0; i < sz; ++i)
			_elements[i].~T();

		_proxy.deallocate(_elements, _width * _height);
		_width = 0;
		_height = 0;
		_elements = nullptr;
	}

	constexpr void SetSize(size_type width, size_type height)
	{
		if (width != _width && height != _height)
		{
			if (width == 0 || height == 0)
			{
				Clear();
				return;
			}

			T* newElements = _proxy.allocate(width * height);

			if (_elements)
			{
				size_type minWidth = width > _width ? _width : width;
				size_type minHeight = height > _height ? _height : height;

				for (size_type x = 0; x < width; ++x)
					for (size_type y = 0; y < height; ++y)
					{
						if (x < minWidth && y < minHeight)
							newElements[y * width + x] = std::move(At(x, y));
						else
							new (newElements + (y * width + x)) T;
					}

				for (size_type x = minWidth; x < _width; ++x)
					for (size_type y = minHeight; y < _height; ++y)
						At(x, y).~T();

			}
			
			_width = width;
			_height = height;
			_elements = newElements;
		}
	}

	constexpr void FlipRows() noexcept
	{
		for (size_type r = 0; r < _height; ++r)
		{
			size_type swapRow = _width - 1 - r;
			for (size_type c = 0; c < _width; ++c)
				Utilities::Swap(At(r, c), At(swapRow, c));
		}
	}

	constexpr void FlipColumns() noexcept
	{
		for (size_type c = 0; c < _width; ++c)
		{
			size_type swapCol = _height - 1 - c;
			for (size_type r = 0; r < _height; ++r)
				Utilities::Swap(At(r, c), At(r, swapCol));
		}
	}

	constexpr void Transpose() noexcept
	{
		for (size_type r = 0; r < _height; ++r)
			for (size_type c = 0; c < _width; ++c)
				Utilities::Swap(At(r, c), At(c, r));

		Utilities::Swap(_width, _height);
	}
};
