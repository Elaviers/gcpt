#include "phys_collider.h"
#include "phys_collision.h"
#include "phys_collision_box.h"
#include "phys_collision_sphere.h"
#include "t_list.h"
#include "t_paged_memory.h"
#include "line_segment.h"
#include "ray.h"
#include "transform.h"
#include "t_vec2.h"

constexpr const int GJK_MAX_ITERATIONS = 100;

constexpr const int EPA_MAX_ITERATIONS = 250;

constexpr const int GJKDIST_MAX_ITERATIONS = 100;

//GJK will terminate if the dot product between a point and an edge normal is lower than this
constexpr const float GJK_TOUCH_TOLERANCE = 0.001f;

//GJKDist will terminate if a the dot product between an existing point and the direction and the dot product between the new point and the direction is lower than this
constexpr const double GJK_TOLERANCE = 1e-50;

constexpr const double EPA_TOLERANCE = 1e-50;

constexpr const double GJKDIST_TOLERANCE = 1e-50;

struct GJKInfo
{
	const CollisionShape& shapeA;
	const CollisionShape& shapeB;
	const Transform& transformA;
	const Transform& transformB;

	const LineSegment* sweepA = nullptr;

	bool calculatePenetration = true;
	Vector3 penetrationVector;

	float expand = 0.f;

	GJKInfo(const CollisionShape& shapeA, const CollisionShape& shapeB, const Transform& transformA, const Transform& transformB) :
		shapeA(shapeA), shapeB(shapeB), transformA(transformA), transformB(transformB)
	{}
};

//#define GJK_DEBUG 1
#define GJK_MATHEMATICA_DEBUG 0

#if GJK_MATHEMATICA_DEBUG
inline void MathematicaDebugState(const Vector3& a, const Vector3& b, const Vector3& c, const Vector3& d, const Vector3& closestPoint)
{
	Debug::PrintLine(CSTR("{{", a, "},{", b, "},{", c, "},{", d, "},{", closestPoint, "}},"));
}
#else
__forceinline void MathematicaDebugState(const Vector3& a, const Vector3& b, const Vector3& c, const Vector3& d, const Vector3& closestPoint) {}
#endif

inline Vector3 ClosestPointToOriginOnPlane(const Vector3& planePoint, const Vector3& planeNormal)
{
	//Assert(Maths::AlmostEquals(planeNormal.LengthSquared(), 1.f, .001f), "ClosestPointToOriginOnPlane requires a normalised normal vector...");

	return planePoint.Dot(planeNormal) * planeNormal;
}

inline Vector3 ClosestPointToOriginOnLineSegment(const Vector3& a, const Vector3& ab)
{
	float length = ab.Length();

	if (length == 0.f)
		return a;

	Vector3 d = ab / length;
	float dot = (-a).Dot(d);

	if (dot <= 0.f) return a;
	if (dot >= length) return a + ab;
	return a + d * dot;
}

Vector3 ClosestPointToOriginOnTriangle(const Vector3& a, const Vector3& b, const Vector3& c)
{
	if (a == b) return ClosestPointToOriginOnLineSegment(a, c - a);
	if (a == c || b == c) return ClosestPointToOriginOnLineSegment(a, b - a);

	Vector3 ab = b - a;
	Vector3 bc = c - b;
	Vector3 ca = a - c;

	//A.-N[AB]
	if (a.Dot(Vector3::TripleCross(ab, ca, ab)) <= 0.f)
	{
		if (a.Dot(-ab) <= 0.f)				//A.BA (outside a on ab)
		{
			if (a.Dot(ca) <= 0.f) return a;	//A.CA (outside a on ca)
			return ClosestPointToOriginOnLineSegment(c, ca);
		}

		if (b.Dot(ab) <= 0.f)					//B.AB (outside b on ab)
		{
			if (b.Dot(-bc) <= 0.f) return b;	//B.CB (outside b on bc)
			return ClosestPointToOriginOnLineSegment(b, bc);
		}

		return ClosestPointToOriginOnLineSegment(a, ab);
	}

	//B.-N[BC]
	if (b.Dot(Vector3::TripleCross(bc, ab, bc)) <= 0.f)
	{
		if (b.Dot(-bc) <= 0.f) return b;		//B.CB (outside b on bc)

		if (c.Dot(bc) <= 0.f)					//B.BC (outside c on bc)
		{
			if (c.Dot(-ca) <= 0.f) return c;	//C.AC (outside c on ca)

			return ClosestPointToOriginOnLineSegment(c, ca);
		}

		return ClosestPointToOriginOnLineSegment(b, bc);
	}

	//C.-N[CA]
	if (c.Dot(Vector3::TripleCross(ca, -ab, ca)) <= 0.f)
	{
		if (a.Dot(ca) <= 0.f) return a;		//A.CA (outside a on ca)
		if (c.Dot(-ca) <= 0.f) return c;		//C.AC (outside c on ca)
		return ClosestPointToOriginOnLineSegment(c, ca);
	}

	return ClosestPointToOriginOnPlane(a, (ab).Cross(bc).Normalised());
}

//Returns vector perpendicular to the two sides in the direction of dir
inline Vector3 GetNormalForFace(const Vector3& side1, const Vector3& side2, const Vector3& dir)
{
	Vector3 result = (side1).Cross(side2);

	if (result.Dot(dir) < 0.f)
		return -result;

	return result;
}

struct Face
{
	Vector3 a, b, c;
	Vector3 closestPointToOrigin;
	float closestPointLengthSq;

	Vector3 normal;

	Face() : closestPointLengthSq(0.f) {}
	Face(const Vector3& a, const Vector3& b, const Vector3& c) :
		a(a), b(b), c(c)
	{
		closestPointToOrigin = ClosestPointToOriginOnTriangle(a, b, c);
		closestPointLengthSq = closestPointToOrigin.LengthSquared();
		normal = GetNormalForFace(b - a, c - a, a);
	}
};

template <typename AllocType>
void AddEdge(List<Pair<Vector3>, AllocType>& edges, const Vector3& a, const Vector3& b)
{
	for (auto it = edges.begin(); it; ++it)
		if (a == it->second && b == it->first)
			return;

	edges.EmplaceBack(a, b);
}

template <typename AllocType>
void InsertFace(List<Face, AllocType>& closestFaces, const Face& face)
{
	for (auto it = closestFaces.begin(); it; ++it)
		if (it->closestPointLengthSq > face.closestPointLengthSq)
		{
			closestFaces.Add(it, face);
			return;
		}

	closestFaces.EmplaceBack(face);
}

inline Vector3 Support(const GJKInfo& info, const Vector3& dir)
{
	CollisionShape::OrientedPoint a = info.shapeA.GetFarthestPointInDirection(dir, info.transformA);
	CollisionShape::OrientedPoint b = info.shapeB.GetFarthestPointInDirection(-dir, info.transformB);

	if (info.sweepA)
	{
		Vector3 farthestLinePoint = dir.Dot(info.sweepA->end - info.sweepA->start) > 0.f ? info.sweepA->end : info.sweepA->start;
		a.position += farthestLinePoint;
	}

	if (info.expand != 0.f)
		return (a.position + a.normal * info.expand) - (b.position + b.normal * info.expand);

	return a.position - b.position;
}

//Note: not the best for very shallow overlaps (can have erroneous results due to division by incredibly small amounts) (can return NaN in its current incarnation)
//Not incredibly reliable right now...
Vector3 EPA(const Vector3 simplex[4], const GJKInfo& info)
{
	//todo: this will cause problems with multithreading
	static PagedMemory<> pool;

	pool.Clear();

	//Array of closest faces, closest first, farthest last
	List<Face, PagedMemory<>::Allocator<Face>> closestFaces(pool.GetAllocator<Face>());

	closestFaces.EmplaceBack(simplex[3], simplex[0], simplex[1]);
	InsertFace(closestFaces, Face(simplex[3], simplex[1], simplex[2]));
	InsertFace(closestFaces, Face(simplex[3], simplex[2], simplex[0]));
	InsertFace(closestFaces, Face(simplex[0], simplex[1], simplex[2]));

	for (int i = 0; i < EPA_MAX_ITERATIONS; ++i)
	{
		Face& closestFace = *closestFaces.begin();
		Vector3 dir = closestFace.normal;

		Vector3 newPoint = Support(info, dir);

		double newDot = ((Vector3T<double>)dir).Dot((Vector3T<double>)newPoint);
		double oldDot = ((Vector3T<double>)dir).Dot((Vector3T<double>)closestFace.a);
		if (newDot - oldDot <= EPA_TOLERANCE)
			return closestFace.closestPointToOrigin;

		List<Pair<Vector3>, PagedMemory<>::Allocator<Pair<Vector3>>> edges(pool.GetAllocator<Face>());

		//Remove any faces that the new point is in front of
		for (auto it = closestFaces.begin(); it;)
		{
			//Point on iterator face -> new point . iterator normal
			if ((newPoint - it->a).Dot(it->normal) > 0.f)
			{
				AddEdge(edges, it->a, it->b);
				AddEdge(edges, it->b, it->c);
				AddEdge(edges, it->c, it->a);

				it = closestFaces.Remove(it);
			}
			else
				++it;
		}

		for (const Pair<Vector3>& edge : edges)
		{
			InsertFace(closestFaces, Face(newPoint, edge.first, edge.second));
		}
	}

	return closestFaces.begin()->closestPointToOrigin;
}

//Overlap Query w/ EPA penetration solving
EOverlapResult GJK(GJKInfo& info)
{
	//The simplex is a tetrahedron inside the minkowski difference
	Vector3 simplex[4];
	Vector3& a = simplex[0], & b = simplex[1], & c = simplex[2], & d = simplex[3];

	Vector3 dir;
	int i = 0;
	for (int _iteration = 0; _iteration < GJK_MAX_ITERATIONS; ++_iteration)
	{
		switch (i)
		{
		case 0:
			//First point, search along A->B
			dir = (info.shapeB.GetTransform().GetPosition() + info.transformB.GetPosition()) - (info.shapeA.GetTransform().GetPosition() + info.transformA.GetPosition());

			if (dir.LengthSquared() == 0.f)
				dir = Vector3(1, 0, 0);

			break;
		case 1:
			//The first point is past the origin
			//Look for the second support in the opposite direction
			dir *= -1.f;
			break;
		case 2:
		{
			//The simplex is a line with points on either side of the origin
			//Look for the third support perpendicular to the line, facing the origin

			Vector3 ba = a - b;

			//(BA X B0) X BA
			dir = Vector3::TripleCross(ba, -b, ba);
			break;
		}
		case 3:
		{
			//The simplex is a triangle that contains the origin in its respective space
			//Look for the fourth support along the normal which is facing the origin

			dir = GetNormalForFace(b - c, a - c, -c);
			break;
		}
		case 4:
		{
			//The simplex is a tetrahedron whose base (points 0, 1, 2) is on the opposite side of the origin from its peak (point 3)
			//Check if the tetrahedron contains the origin by checking the dot products of the normals of the 3 faces related to the peak
			//The base's normal doesn't have to be checked as it is on the opposite side of the origin from the peak

			//Edges
			Vector3 da = a - d;
			Vector3 db = b - d;
			Vector3 dc = c - d;

			Vector3 bcd = GetNormalForFace(dc, db, -da);
			float dotBCD = bcd.Dot(d);
			if (dotBCD < 0.f)
			{
				//Origin is outside BCD
				//Remove point A (0)
				simplex[0] = simplex[1];
				simplex[1] = simplex[2];
				simplex[2] = simplex[3];
				i = 3;

				dir = bcd;
				break;
			}

			Vector3 abd = GetNormalForFace(db, da, -dc);
			float dotABD = abd.Dot(d);
			if (dotABD < 0.f)
			{
				//Origin is outside ABD
				//Remove point C (2)
				simplex[2] = simplex[3];
				i = 3;

				dir = abd;
				break;
			}

			Vector3 acd = GetNormalForFace(da, dc, -db);
			float dotACD = acd.Dot(d);
			if (dotACD < 0.f)
			{
				//Origin is outside ACD
				//Remove point B (1)
				simplex[1] = simplex[2];
				simplex[2] = simplex[3];
				i = 3;

				dir = acd;
				break;
			}

			if (info.calculatePenetration)
				info.penetrationVector = EPA(simplex, info);

			if (dotBCD <= GJK_TOUCH_TOLERANCE || dotABD <= GJK_TOUCH_TOLERANCE || dotACD <= GJK_TOUCH_TOLERANCE)
				return EOverlapResult::TOUCHING;

			return EOverlapResult::OVERLAPPING;
		}
		}

		//Find new support point
		simplex[i] = Support(info, dir);

		//Fail if the new point did not go past the origin
		if (simplex[i].Dot(dir) < 0.f)
			return EOverlapResult::SEPERATE;

		++i;
	}

	//Failsafe
	return EOverlapResult::OVERLAPPING;
}

inline Vector2 Cartesian2Barycentric(const Vector3& p, const Vector3& a, const Vector3& b)
{
	Vector2 result;
	result.x = (p - b).Length() / (a - b).Length();
	result.y = 1.f - result.x;
	return result;
}

Vector3 Cartesian2Barycentric(const Vector3& p, const Vector3& a, const Vector3& b, const Vector3& c)
{
	if (a == b)
	{
		Vector2 bary2D = Cartesian2Barycentric(p, a, c);
		return Vector3(bary2D.x, 0.f, bary2D.y);
	}

	if (a == c || b == c)
	{
		Vector2 bary2D = Cartesian2Barycentric(p, a, b);
		return Vector3(0.f, bary2D.x, bary2D.y);
	}

	Vector3 result;
	Vector3 ab = b - a, ac = c - a, ap = p - a;
	float ab_ab = (ab).Dot(ab);
	float ab_ac = (ab).Dot(ac);
	float ac_ac = (ac).Dot(ac);
	float ap_ab = (ap).Dot(ab);
	float ap_ac = (ap).Dot(ac);
	float denom = ab_ab * ac_ac - ab_ac * ab_ac;

	if (denom == 0.f)
	{
		Vector2 bary2D = Cartesian2Barycentric(p, a, b);
		return Vector3(bary2D.x, bary2D.y, 0.f);
	}

	result.y = (ac_ac * ap_ab - ab_ac * ap_ac) / denom;
	result.z = (ab_ab * ap_ac - ab_ac * ap_ab) / denom;
	result.x = 1.0f - result.y - result.z;
	return result;
}

float GJKDist(GJKInfo& info, Vector3& out_pointA, Vector3& out_pointB)
{
#if GJK_DEBUG
	Debug::PrintLine("STARTING GJKDIST...");
#endif

	struct SimplexPoint
	{
		Vector3 aSupport;
		Vector3 bSupport;
		Vector3 difference;

		SimplexPoint() {}
		SimplexPoint(const Vector3& aSupport, const Vector3& bSupport) : aSupport(aSupport), bSupport(bSupport), difference(aSupport - bSupport) {}
	};

	SimplexPoint simplex[4];

	SimplexPoint& a = simplex[0];
	SimplexPoint& b = simplex[1];
	SimplexPoint& c = simplex[2];
	SimplexPoint& d = simplex[3];

	//minkowski points
	const Vector3& mA = a.difference;
	const Vector3& mB = b.difference;
	const Vector3& mC = c.difference;
	const Vector3& mD = d.difference;

	Vector3 closestPoint;
	Vector3 dir;
	int i = 0;
	for (int iteration = 0; iteration < GJKDIST_MAX_ITERATIONS; ++iteration)
	{
		switch (i)
		{
		case 0:
			//First point, search along A->B
			dir = (info.shapeB.GetTransform().GetPosition() + info.transformB.GetPosition()) - (info.shapeA.GetTransform().GetPosition() + info.transformA.GetPosition());

			if (dir.LengthSquared() == 0.f)
				dir = Vector3(1.f, 0.f, 0.f);

			break;
		case 1:
			closestPoint = mA;
			dir *= -1.f;
			break;
		case 2:
		{
			closestPoint = ClosestPointToOriginOnLineSegment(mA, mB);

			dir = -closestPoint;
			break;
		}
		case 3:
		{
			closestPoint = ClosestPointToOriginOnTriangle(mA, mB, mC);

			dir = GetNormalForFace(mB - mC, mA - mC, -mC);
			break;
		}
		case 4:
		{
			{
				Vector3 da = mA - mD;
				Vector3 db = mB - mD;
				Vector3 dc = mC - mD;
				//I did have a more efficient way of doing this but it is way uglier and has a small issue so it is not used here, see UnusedCollider.txt

				if (mD.Dot(GetNormalForFace(da, db, -dc)) > 0.f &&	//Inside DAB
					mD.Dot(GetNormalForFace(db, dc, -da)) > 0.f &&	//Inside DBC
					mD.Dot(GetNormalForFace(dc, da, -db)) > 0.f &&	//Inside DCA
					mA.Dot(GetNormalForFace(mB - mA, mC - mA, da)) > 0.f)//Inside ABC
				{
					//Inside all faces of tetrahedron
					return 0.f;
				}


				//Outside tetrahedron, find each triangle's closest point and use the triangle with the lowest one
				Vector3 triPoints[4] = {
					ClosestPointToOriginOnTriangle(mD, mB, mC),
					ClosestPointToOriginOnTriangle(mD, mC, mA),
					ClosestPointToOriginOnTriangle(mD, mA, mB),
					ClosestPointToOriginOnTriangle(mA, mB, mC)
				};

				int smallestPoint = 0;
				float smallestLengthSq = triPoints[smallestPoint].LengthSquared();
				for (int i = 1; i < 4; ++i)
				{
					float lengthSq = triPoints[i].LengthSquared();
					if (lengthSq < smallestLengthSq)
					{
						smallestPoint = i;
						smallestLengthSq = lengthSq;
					}
				}

				MathematicaDebugState(mA, mB, mC, mD, triPoints[smallestPoint]);

				//Remove point not on closest triangle
				//Note: cases fall through
				switch (smallestPoint)
				{
				case 0: //DBC
					a = b;
				case 1: //DCA
					b = c;
				case 2:	//DAB
					c = d;
					break;
				}

				closestPoint = triPoints[smallestPoint];
				dir = -closestPoint;
				i = 3;
				break;
			}
		}
		}

		//Find new support point
		CollisionShape::OrientedPoint a = info.shapeA.GetFarthestPointInDirection(dir, info.transformA);
		CollisionShape::OrientedPoint b = info.shapeB.GetFarthestPointInDirection(-dir, info.transformB);

		if (info.sweepA)
		{
			Vector3 farthestLinePoint = dir.Dot(info.sweepA->end - info.sweepA->start) > 0.f ? info.sweepA->end : info.sweepA->start;
			a.position += farthestLinePoint;
		}

		if (info.expand != 0.f)
			simplex[i] = SimplexPoint(a.position + a.normal * info.expand, b.position + b.normal * info.expand);
		else
			simplex[i] = SimplexPoint(a.position, b.position);

#if GJK_DEBUG
		Debug::PrintLine(CSTR("Current closest point is (", closestPoint, ")"));

		Debug::PrintLine(CSTR(
			iteration, "|point ", i, ", direction (", dir,
			") : a-support(", simplex[i].aSupport,
			") - b-support(", simplex[i].bSupport,
			") = minkowski(", simplex[i].difference, ")"));
#endif

		if (i >= 1)
		{
			bool stopIterating = false;

			double dotD = ((Vector3T<double>)dir).Dot(simplex[i].difference);
			for (int j = 0; j < i; ++j)
			{
				double dotP = ((Vector3T<double>)dir).Dot(simplex[j].difference);

				if (dotD - dotP <= GJKDIST_TOLERANCE)
				{
					stopIterating = true;
					break;
				}
			}

			if (stopIterating) break;
		}

		++i;
	}

	switch (i)
	{
	case 1:
	{
		out_pointA = a.aSupport;
		out_pointB = a.bSupport;
		break;
	}
	case 2:
	{
		Vector2 bary = Cartesian2Barycentric(closestPoint, mA, mB);
		out_pointA = a.aSupport * bary.x + b.aSupport * bary.y;
		out_pointB = a.bSupport * bary.x + b.bSupport * bary.y;
		break;
	}
	case 3:
	case 4:
	{
		Vector3 bary = Cartesian2Barycentric(closestPoint, mA, mB, mC);
		out_pointA = a.aSupport * bary.x + b.aSupport * bary.y + c.aSupport * bary.z;
		out_pointB = a.bSupport * bary.x + b.bSupport * bary.y + c.bSupport * bary.z;
		break;
	}
	}

	float result = closestPoint.Length();

#if GJK_DEBUG
	Debug::PrintLine(CSTR("DONE (", result, ")"));
#endif

	return result;
}

EOverlapResult Collider::NarrowOverlapCheck(const Transform& transform, const Collider& other, const Transform& otherTransform, const LineSegment* lineA, Vector3* out_PenetrationVector) const
{
	if (CanCollideWith(other))
	{
		const float shallowContactRadius = 0.2f;
		bool isTouching = false;

		Vector3 cpA;
		Vector3 cpB;

		for (size_t i = 0; i < GetShapeCount(); ++i)
			for (size_t j = 0; j < other.GetShapeCount(); ++j)
			{
				const CollisionShape& shape = GetShape(i);
				const CollisionShape& otherShape = other.GetShape(j);

				float distanceSq = ((shape.GetTransform().GetPosition() + transform.GetPosition()) - (otherShape.GetTransform().GetPosition() + otherTransform.GetPosition())).LengthSquared();
				float shapeRadius = shape.GetBoundingRadius() * Maths::Max(transform.GetScale().GetData(), 3);
				float otherShapeRadius = otherShape.GetBoundingRadius() * Maths::Max(otherTransform.GetScale().GetData(), 3);
				if (distanceSq <= shapeRadius * shapeRadius + otherShapeRadius * otherShapeRadius)
				{
					GJKInfo info(shape, otherShape, transform, otherTransform);
					info.sweepA = lineA;
					info.calculatePenetration = out_PenetrationVector != nullptr;
					info.expand = -shallowContactRadius;

					float d = GJKDist(info, cpA, cpB);

					if (d < 0.01f || d > shallowContactRadius * 2.f)
					{
						info.expand = 0.f;
						EOverlapResult result = GJK(info);
						if (result == EOverlapResult::OVERLAPPING)
						{
							if (out_PenetrationVector)
								*out_PenetrationVector = info.penetrationVector;

							return EOverlapResult::OVERLAPPING;
						}

						if (!isTouching && result == EOverlapResult::TOUCHING)
						{
							if (out_PenetrationVector)
								*out_PenetrationVector = (cpB - cpA).Normalise();

							isTouching = true;
						}
					}
					else
					{
						float pa = shallowContactRadius * 2.f - d;
						Vector3 pd = (cpB - cpA).Normalise();

						if (pa < 0.0001f)
						{
							isTouching = true;
							if (out_PenetrationVector)
								*out_PenetrationVector = pd;

							continue;
						}

						if (out_PenetrationVector)
							*out_PenetrationVector = pd * pa;

						return EOverlapResult::OVERLAPPING;
					}
				}
			}

		if (isTouching)
		{
			return EOverlapResult::TOUCHING;
		}
	}

	return EOverlapResult::SEPERATE;
}

float Collider::MinimumDistanceTo(
	const Transform& transform,
	const Collider& other, const Transform& otherTransform,
	Vector3& out_PointA, Vector3& out_PointB, const LineSegment* pLineA) const
{
	float minDist = -1.f;
	Vector3 a, b;

	if (CanCollideWith(other))
		for (size_t i = 0; i < GetShapeCount(); ++i)
			for (size_t j = 0; j < GetShapeCount(); ++j)
			{
				GJKInfo info(GetShape(i), other.GetShape(j), transform, otherTransform);
				info.sweepA = pLineA;

				float dist = GJKDist(info, a, b);
				if (minDist < 0.f || dist < minDist)
				{
					out_PointA = a;
					out_PointB = b;

					if (dist <= 0.f)
						return 0.f;

					minDist = dist;
				}
			}

	return minDist;
}
