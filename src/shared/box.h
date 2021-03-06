#pragma once
#include "t_vec3.h"

struct Box
{
	Vector3 centre;
	Vector3 halfSize;

	Box(const Vector3& centre = Vector3(), const Vector3& halfSize = Vector3()) : centre(centre), halfSize(halfSize) {}

	static Box FromMinMax(const Vector3& min, const Vector3& max)
	{
		return Box((max + min) / 2.f, (max - min) / 2.f);
	}

	static Box FromPoints(const Vector3& point1, const Vector3& point2)
	{
		return Box((point1 + point2) / 2.f, (point1 - point2).Abs() / 2.f);
	}
};
