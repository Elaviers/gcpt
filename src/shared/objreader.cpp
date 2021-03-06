#include "gfx_io.h"
#include "mesh.h"
#include "mesh_static.h"
#include "t_buf.h"
#include "utils.h"
#include "debug.h"
#include "mesh_vertex.h"
#include <stdlib.h>

struct OBJVertexDef
{
	int pos_index;
	int normal_index;
	int uv_index;
};

OBJVertexDef ParseOBJVertex(const String &term)
{
	int uvStringIndex = 0;
	int normalStringIndex = 0;

	for (unsigned int i = 0; i < term.GetLength(); ++i)
	{
		if (term[i] == '/')
		{
			i++;

			if (term[i] != '/')
				if (uvStringIndex == 0)
					uvStringIndex = i;
				else
				{
					normalStringIndex = i;
					break;
				}
			else
			{
				normalStringIndex = i + 1;
				break;
			}
		}

	}

	OBJVertexDef result = {};
	result.pos_index = atoi(term.begin());
	if (uvStringIndex)
		result.uv_index = atoi(term.begin() + uvStringIndex);
	if (normalStringIndex)
		result.normal_index = atoi(term.begin() + normalStringIndex);

	return result;
}

template<typename T>
inline const size_t OBJIndexToCIndex(const Buffer<T> &buf, size_t objIndex)
{
	if (objIndex == 0)
		return 0;

	if (objIndex < 0)
		return buf.GetSize() + objIndex;

	return objIndex - 1;
}

uint32 GetVertexIndex(Buffer<Vertex17F> &vertices, uint32 &vertexCount, const Buffer<Vector3> &positions, const Buffer<Vector3> &normals, const Buffer<Vector2> &uvs, const char *vertex)
{
	OBJVertexDef objVertex = ParseOBJVertex(vertex);
	const Vector3 &pos = positions[OBJIndexToCIndex(vertices, objVertex.pos_index)];
	const Vector3 &normal = normals[OBJIndexToCIndex(normals, objVertex.normal_index)];
	const Vector2 &uv = uvs[OBJIndexToCIndex(uvs, objVertex.uv_index)];

	uint32 i = 0;
	for (; i < vertexCount; ++i)
		if (vertices[i].pos == pos && vertices[i].normal == normal && vertices[i].uv == uv)
			return i;

	if (i == vertices.GetSize())
		vertices.Emplace();

	vertices[i].pos = pos;
	vertices[i].normal = normal;
	vertices[i].uv = uv;

	vertexCount++;
	return i;
}

Mesh_Static* IO::ReadOBJFile(const String& objSrc)
{
	const float scale = 1;

	Mesh_Static* mesh = nullptr;
	
	if (objSrc.GetLength())
	{
		mesh = new Mesh_Static();

		Buffer<Vector3> positions;
		Buffer<Vector3> normals;
		Buffer<Vector2> uvs;

		uint32 vertexCount = 0;

		Buffer<String> lines = objSrc.Split("\r\n");

		Vector3 bmin, bmax;

		for (uint32 i = 0; i < lines.GetSize(); ++i)
		{
			Buffer<String> tokens = lines[i].Split(" ");

			if (tokens.GetSize() >= 3)
			{
				if (tokens[0][0] == 'v')
				{
					if (tokens[0].GetLength() == 1) //Vertex position
					{
						if (tokens.GetSize() >= 4)
						{
							positions.Add(Vector3(tokens[1].ToFloat() * scale, tokens[2].ToFloat() * scale, tokens[3].ToFloat() * scale));

							const Vector3 &newPos = positions[positions.GetSize() - 1];

							if (newPos.x < bmin.x)
								bmin.x = newPos.x;

							if (newPos.y < bmin.y)
								bmin.y = newPos.y;

							if (newPos.z < bmin.z)
								bmin.z = newPos.z;

							if (newPos.x > bmax.x)
								bmax.x = newPos.x;

							if (newPos.y > bmax.y)
								bmax.y = newPos.y;

							if (newPos.z > bmax.z)
								bmax.z = newPos.z;
						}
					}

					else if (tokens[0][1] == 'n')	//Vertex normal
					{
						if (tokens.GetSize() >= 4)
							normals.Add(Vector3(tokens[1].ToFloat(), tokens[2].ToFloat(), tokens[3].ToFloat()));
					}

					else if (tokens[0][1] == 't')	//Vertex texture coordinates
						uvs.Add(Vector2(1.f - tokens[1].ToFloat(), 1.f - tokens[2].ToFloat()));
				}
				else if (tokens[0][0] == 'f')		//Face
				{
					if (mesh->vertices.GetSize() == 0)
					{
						size_t maxSizeSoFar = Maths::Max(Maths::Max(positions.GetSize(), normals.GetSize()), uvs.GetSize());
						mesh->vertices.SetSize(maxSizeSoFar);
					}

					if (tokens.GetSize() >= 4)		//Triangle
					{
						uint32 v1 = GetVertexIndex(mesh->vertices, vertexCount, positions, normals, uvs, tokens[1].begin());
						uint32 v2 = GetVertexIndex(mesh->vertices, vertexCount, positions, normals, uvs, tokens[2].begin());
						uint32 v3 = GetVertexIndex(mesh->vertices, vertexCount, positions, normals, uvs, tokens[3].begin());

						size_t last = mesh->elements.GetSize();
						mesh->elements.Grow(3);

						mesh->elements[last] = v1;
						mesh->elements[last + 1] = v3;
						mesh->elements[last + 2] = v2;
						Vertex17F::CalculateTangents(mesh->vertices[v1], mesh->vertices[v3], mesh->vertices[v2]);

						if (tokens.GetSize() == 5)	//Quad, add another triangle
						{
							uint32 v4 = GetVertexIndex(mesh->vertices, vertexCount, positions, normals, uvs, tokens[4].begin());

							size_t last = mesh->elements.GetSize();
							mesh->elements.Grow(3);

							mesh->elements[last] = v1;
							mesh->elements[last + 1] = v4;
							mesh->elements[last + 2] = v3;
							Vertex17F::CalculateTangents(mesh->vertices[v1], mesh->vertices[v4], mesh->vertices[v3]);
						}
					}
				}
			}
		}
		
		mesh->BoundingBox().SetTransform(Transform((bmin + bmax) / 2.f, Rotation(), bmax - bmin));

#ifdef BUILD_CLIENT
		mesh->UpdateRenderer();
#endif
	}

	return mesh;
}
