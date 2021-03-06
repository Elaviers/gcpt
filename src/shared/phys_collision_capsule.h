#pragma once
#include "phys_collision_shape.h"

class CollisionCapsule : public CollisionShape
{
	float _halfHeight;
	float _radius;
	
public:
	CollisionCapsule(float halfHeight, float radius, const Transform &transform = Transform()) : CollisionShape(transform), _halfHeight(halfHeight), _radius(radius) {}
	virtual ~CollisionCapsule() {}

	COLLISIONSHAPE_FUNCS(CollisionCapsule)

	void SetHalfHeight(float halfHeight) { _halfHeight = halfHeight; _onChanged(); }
	void SetRadius(float radius) { _radius = radius; _onChanged(); }

	float GetHalfHeight() const { return _halfHeight; }
	float GetRadius() const { return _radius; }

	virtual float GetBoundingRadius() const override
	{ 
		return Maths::Max(_halfHeight, _radius) * Maths::Max(GetTransform().GetScale().GetData(), 3);
	}

	virtual bool IntersectsRay(const Ray&, RaycastHitInformation&, const Transform & = Transform()) const override;
	virtual OrientedPoint GetFarthestPointInDirection(const Vector3& axis, const Transform&) const override;
};

