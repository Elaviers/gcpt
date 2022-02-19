#pragma once
#include "shared/t_vec2.h"

struct MaterialParam
{
	enum class EType
	{
		GRID_PARAM, //Parameters for a MaterialGrid
		SPRITESHEET_PARAM
	} type;

	union
	{
		struct
		{
			int row;
			int column;

			Vector2 uvScale;
		} gridData;

		struct
		{
			float time;

		} spritesheetData;
	};

	MaterialParam() {}
};
