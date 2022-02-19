#pragma once
#include "t_matrix.h"
#include "t_vec2.h"

class ShapeRotator
{
	//Positive amount = clockwise, negative = counterclockwise
	virtual bool Rotate(const Matrix<byte>& boardData, Vector2& shapePos, Matrix<byte>& shapeData, int amount) noexcept = 0;
};

class TetrisRotator_SRS
{
	

	virtual bool Rotate(const Matrix<byte>& boardData, Vector2& shapePos, Matrix<byte>& shapeData, int amount) noexcept;
};
