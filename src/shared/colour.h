#pragma once
#include "string.h"
#include "types.h"
#include "vec4.h"

union Colour
{
private:
	Vector4 _data;

public:
	struct
	{
		float r;
		float g;
		float b;
		float a;
	};

	Colour() : _data(0.f, 0.f, 0.f, 1.f) {}
	Colour(const Vector4& data) : _data(data) {}
	Colour(float r, float g, float b, float a = 1.f) : _data(r, g, b, a) {}
	Colour(byte r, byte g, byte b, byte a = 255) : _data(r / 255.f, g / 255.f, b / 255.f, a / 255.f) {}
	
	Colour(const Colour& other) : _data(other._data) {}
	
	~Colour() {}

	const Vector4& GetData() { return _data; }

	operator Vector4& () { return _data; }
	operator const Vector4&() const { return _data; }

	Colour Inverse() const { return Colour(1.f-r, 1.f-g, 1.f-b, a); }

	Colour& operator=(const Colour& other)
	{
		_data = other._data;
		return *this;
	}

	bool operator==(const Colour& other) const { return _data == other._data; }

	String ToColourCode() const;

	static Colour FromColourCode(const char* colourCode);

	static const Colour Invisible, Black, Grey, White, Red, Green, Blue, Yellow, Pink, Cyan;
};
	