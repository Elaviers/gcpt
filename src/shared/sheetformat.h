#pragma once
#include "asset.h"
#include "string.h"
#include "t_hashmap.h"

struct SpriteDef
{
	uint16 x, y;
	uint16 w, h;
};

struct SpriteSet
{
	Hashmap<String, SpriteSet> subsets;
	Hashmap<String, SpriteDef> sprites;

	SpriteSet() {}
	SpriteSet(const SpriteSet&) = delete;
};

class SheetFormat : public Asset
{
public:
	SheetFormat() {}
	SheetFormat(const SheetFormat&) = delete;

	SpriteSet sprites;

	bool Parse(const String& ss);
};
