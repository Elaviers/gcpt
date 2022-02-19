#include "ShapeRotator.h"

bool TetrisRotator_SRS::Rotate(const Matrix<byte>& boardData, Vector2& shapePos, Matrix<byte>& shapeData, int amount) noexcept
{
	if (amount == 0) return true;

	while (amount > 0)
	{
		--amount;

		//CW
		shapeData.Transpose();
		shapeData.FlipColumns();

	}

	while (amount < 0)
	{
		++amount;

		//CCW
		shapeData.Transpose();
		shapeData.FlipRows();
	}

	bool overlaps = false;
	for (auto x = 0; x < shapeData.GetWidth(); ++x)
		for (auto y = 0; y < shapeData.GetHeight(); ++y)
			if (shapeData.At(x, y) != 0 && shapeData.At(shapePos.x + x, shapePos.y + y) != 0)
				overlaps = true;

	if (overlaps)
	{
		//Kick


	}

	return true;
}
