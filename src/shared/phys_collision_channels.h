#pragma once
#include "types.h"

enum class ECollisionChannels : byte
{
	NONE =		0,
	ALL =		0xFF,
	STATIC =	0x01,		//Static Geometry
	DYNAMIC =	0x02,		//Dynamic (movable) Geometry
	PROP =		0x04,		//Physics / small object
	PLAYER =	0x08,

	SURFACE = STATIC | DYNAMIC | PROP | PLAYER
};

#include "macro_utils.h"
DEFINE_BITMASK_FUNCS(ECollisionChannels, byte)
