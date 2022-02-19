#pragma once
#include "t_matrix.h"

class ShapeDef
{
public:
	Matrix<byte> data;
	class ShapeRotator* rotator;
};
