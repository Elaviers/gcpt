#pragma once
#include "string.h"


#undef min
#undef max
#undef CopyMemory

namespace Utilities
{
	template <typename Base, typename Param>
	struct ReplaceParam;

	template <template <typename> typename Base, typename Param, typename _P1>
	struct ReplaceParam<Base<_P1>, Param>
	{
		using Type = Base<Param>;
	};

	template <typename T, size_t N>
	constexpr size_t ArrayLength(T(&)[N]) noexcept { return N; }

	template <typename T>
	constexpr void Swap(T &a, T &b)
	{
		T temp = std::move(a);
		a = std::move(b);
		b = std::move(temp);
	}

	constexpr void CopyMemory(const void* src, void* dest, size_t length)
	{
		for (size_t i = 0; i < length; ++i)
			reinterpret_cast<byte*>(dest)[i] = reinterpret_cast<const byte*>(src)[i];
	}

	template <typename T>
	constexpr void CopyTypedMemory(const T*  src, T* dest, size_t length)
	{
		if (std::is_trivially_copy_constructible_v<T>)
			CopyMemory(src, dest, sizeof(T) * length);
		else
			for (size_t i = 0; i < length; ++i)
				new (dest + i) T(src[i]);
	}

	template <typename T>
	constexpr void MoveTypedMemory(const T* src, T* dest, size_t length)
	{
		if (std::is_trivially_move_constructible_v<T>)
			CopyMemory(src, dest, sizeof(T) * length);
		else
			for (size_t i = 0; i < length; ++i)
				new (dest + i) T(std::move(src[i]));
	}

	template <typename T>
	constexpr void ToLittleEndian(T from, byte *to)
	{
		auto sz = sizeof(T);
		T mask = 0xFF;
		for (byte i = 0; i < sz; ++i)
		{
			to[i] = (from & mask) >> (8 * i);
			mask <<= 8;
		}
	}

	template <typename T>
	constexpr void ToBigEndian(T from, byte *to) noexcept
	{
		auto sz = sizeof(T);
		T mask = 0xFF;
		for (byte i = 0; i < sz; ++i)
		{
			to[sz - 1 - i] = (from & mask) >> (8 * i);
			mask <<= 8;
		}
	}

	template <typename T>
	constexpr T ToLittleEndian(byte* from) noexcept
	{
		T result = 0;
		byte shift = 8 * (sizeof(T) - 1);
		for (byte i = 0; i < sizeof(T); ++i)
		{
			result += from[i] << shift;
			shift -= 8;
		}
		return result;
	}

	template <typename T>
	constexpr T ToBigEndian(byte *from) noexcept
	{
		T result = 0;
		byte shift = 0;
		for (byte i = 0; i < sizeof(T); ++i)
		{
			result += from[i] << shift;
			shift += 8;
		}
		return result;
	}

	inline String GetExtension(const String& string)
	{
		for (size_t i = string.GetLength() - 1; i > 0; --i)
			if (string[i] == '.')
				return string.SubString(i, string.GetLength());

		return String();
	}

	inline String DirectoryOf(const String& filepath)
	{
		size_t i = filepath.GetLength() - 1;
		for (; i > 0; --i)
			if (filepath[i] == '/' || filepath[i] == '\\')
				break;

		for (; i > 0; --i)
			if (filepath[i] != '/' && filepath[i] != '\\')
				break;

		return filepath.SubString(0, i + 1);
	}

	inline String WithCarriageReturns(const String& string)
	{
		String result(string);

		for (size_t i = 0; i < result.GetLength(); )
		{
			if (result[i] == '\n')
			{
				if (i <= 0 || result[i - 1] != '\r')
				{
					result.Insert('\r', i);
					i += 2;
					continue;
				}
			}

			++i;
		}

		return result;
	}

	inline void StripExtension(String& string)
	{
		for (size_t i = string.GetLength() - 1; i > 0; --i)
			if (string[i] == '.')
			{
				string.ShrinkTo(i);
				break;
			}
	}

	inline void LowerString(String& string)
	{
		for (unsigned int i = 0; i < string.GetLength(); ++i)
			if (string[i] >= 'A' && string[i] <= 'Z')
				string[i] += ('a' - 'A');
	}
}

namespace Utilities
{
	String GetSystemFontDir();
}
