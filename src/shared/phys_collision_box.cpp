#include "phys_collision_box.h"
#include "phys_collision.h"
#include "phys_raycast_hit_info.h"
#include "utils.h"
#include "maths.h"
#include "ray.h"

#include <float.h>

inline void FindT(const float &originComponent, const float &directionComponent, const float &minComponent, const float &maxComponent, float &minT, float &maxT)
{
	if (directionComponent == 0.f)
	{
		if (originComponent == minComponent)
			minT = 0.f;
		else if (originComponent > minComponent)
			minT = -FLT_MAX;
		else
			minT = FLT_MAX;

		if (originComponent == maxComponent)
			maxT = 0.f;
		else if (originComponent > maxComponent)
			maxT = -FLT_MAX;
		else
			maxT = FLT_MAX;
	}
	else if (directionComponent > 0.f)
	{
		minT = (minComponent - originComponent) / directionComponent;
		maxT = (maxComponent - originComponent) / directionComponent;
	}
	else
	{
		minT = (maxComponent - originComponent) / directionComponent;
		maxT = (minComponent - originComponent) / directionComponent;
	}
}

bool CollisionBox::IntersectsRay(const Ray &ray, RaycastHitInformation &result, const Transform &worldTransform) const
{
	Transform t = _transform * worldTransform;

	Matrix4 mat = Matrix4::Rotation(t.GetRotation().GetQuat().Inverse());

	Vector3 origin = (Vector4(ray.origin - t.GetPosition(), 1.f) * mat).GetXYZ();
	Vector3 dir = (Vector4(ray.direction, 1.f) * mat).GetXYZ();
	
	Vector3 scaledMin = -1.f * t.GetScale();
	Vector3 scaledMax = t.GetScale();

	float minT, maxT;
	float minT_y, maxT_y;
	FindT(origin.x, dir.x, scaledMin.x, scaledMax.x, minT, maxT);
	FindT(origin.y, dir.y, scaledMin.y, scaledMax.y, minT_y, maxT_y);

	if (minT > maxT_y || minT_y > maxT)
		return false;

	minT = Maths::Max(minT, minT_y);
	maxT = Maths::Min(maxT, maxT_y);

	float minT_z, maxT_z;
	FindT(origin.z, dir.z, scaledMin.z, scaledMax.z, minT_z, maxT_z);

	if (minT > maxT_z || minT_z > maxT)
		return false;

	minT = Maths::Max(minT, minT_z);
	maxT = Maths::Min(maxT, maxT_z);

	if (maxT < 0.f) return false; //Should probably check if behind a bit earlier
	
	result.time = minT < 0.f ? 0.f : minT;
	return true;
}

/*
	R = O + dt

	(d.x)(t) + O.x = min.x
	d.x(t) = min.x - O.x
	t = (min.x - O.x)/d.x
*/

CollisionShape::OrientedPoint CollisionBox::GetFarthestPointInDirection(const Vector3& axisIn, const Transform& worldTransform) const
{
	Transform ft = _transform * worldTransform;
	Matrix4 transform = ft.GetMatrix();
	Vector3 axis = (Vector4(axisIn, 0.f) * ft.GetInverseMatrix()).GetXYZ();

	const static Vector3 points[8] = {
		Vector3(-1, -1, -1),
		Vector3(-1, -1, 1),
		Vector3(-1, 1, -1),
		Vector3(-1, 1, 1),
		Vector3(1, -1, -1),
		Vector3(1, -1, 1),
		Vector3(1, 1, -1),
		Vector3(1, 1, 1)
	};

	int octant = ((axis.x >= 0.f) << 2) | ((axis.y >= 0.f) << 1) | (axis.z >= 0.f);
	CollisionShape::OrientedPoint point;
	point.position = (Vector4(points[octant], 1.f) * transform).GetXYZ();
	point.normal = (point.position - worldTransform.GetPosition()).Normalised() * Maths::SquareRoot(3.f);
	return point;
}
