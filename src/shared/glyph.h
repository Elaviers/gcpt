#pragma once
#include "t_vec2.h"

struct Glyph
{
	Vector2 uv;
	Vector2 uvSize;

	int width = 0;
	int advance = 0;
};
