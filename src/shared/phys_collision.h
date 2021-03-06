#pragma once
#include "t_vec3.h"

struct Ray;
class Rotation;
class Transform;

namespace Collision
{
	bool SphereOverlapsSphere(const Transform& t1, float r1, const Transform& t2, float r2);
	bool SphereOverlapsAABB(const Transform& t1, float r1, const Vector3& min2, const Vector3& max2);
	bool SphereOverlapsBox(const Transform& t1, float r1, const Transform& t2);
	bool BoxOverlapsBox(const Transform& t1, const Transform& t2);

	//returns time along ray
	float IntersectRayPlane(const Ray& ray, const Vector3& planePoint, const Vector3& planeNormal);
	float IntersectRayDisc(const Ray& ray, const Vector3& centre, const	 Vector3& normal, float radius);
	float IntersectRayRing(const Ray& ray, const Vector3& centre, const Vector3& normal, float innerRadius, float outerRadius);

	//dim is Vector3 from centre to corner
	float IntersectRayRect(const Ray& ray, const Vector3& centre, const Vector3& normal, const Vector3& w, const Vector3& h);

	Vector3 ClosestPointOnLineSegment(const Vector3& a, const Vector3& b, const Vector3& x);
	Vector3 ClosestPointOnPlane(const Vector3& planePoint, const Vector3& normal, const Vector3& x);
}
