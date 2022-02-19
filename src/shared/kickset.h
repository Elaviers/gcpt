#pragma once
#include "defs.h"
#include "t_hashmap.h"
#include "t_buf.h"
#include "t_list.h"
#include "t_vec2.h"

enum class EKickRotTransfer
{
	START2RIGHT,
	START2FLIP,
	START2LEFT,

	RIGHT2START,
	RIGHT2FLIP,
	RIGHT2LEFT,

	FLIP2START,
	FLIP2RIGHT,
	FLIP2LEFT,

	LEFT2START,
	LEFT2RIGHT,
	LEFT2FLIP,

	_COUNT
};

class RotTransferData
{
	List<Vector2> _tests[(size_t)EKickRotTransfer::_COUNT];

public:
	RotTransferData() {}

	List<Vector2>& TestList(EKickRotTransfer rot) noexcept { return _tests[(size_t)rot]; }
};

class KickSet
{
public:
	Buffer<RotTransferData> rotSets;
	Hashmap<PartID, int> part2rotSetIdx;

	bool Parse(const char* ss);
};

