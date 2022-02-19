#pragma once
#include "shared/types.h"

enum class ERenderChannels : byte
{
	BOARD = 0x01,
	BACKGROUND = 0x02,
	SKY = 0x08,
	UI = 0x10,
	
	NONE = 0,
	ALL = 0xff
};

constexpr ERenderChannels ERENDERCHANNEL_DEBUG = ERenderChannels::UI;
constexpr ERenderChannels ERENDERCHANNEL_ELUI = ERenderChannels::UI;
constexpr ERenderChannels ERENDERCHANNEL_MATERIALGRID = ERenderChannels::BOARD;
constexpr ERenderChannels ERENDERCHANNEL_MATERIALSPRITE = ERenderChannels::BOARD;
constexpr ERenderChannels ERENDERCHANNEL_MATERIALSURFACE = ERenderChannels::BACKGROUND;
constexpr ERenderChannels ERENDERCHANNEL_SKYBOX = ERenderChannels::SKY;

#include "shared/macro_utils.h"
DEFINE_BITMASK_FUNCS(ERenderChannels, byte)
