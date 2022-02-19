#pragma once
#include "t_vec3.h"

struct Ray
{
	Vector3 origin;
	Vector3 direction;

	Ray(const Vector3& origin, const Vector3& direction) : origin(origin), direction(direction) {}
};
