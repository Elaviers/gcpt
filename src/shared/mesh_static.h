#pragma once
#include "mesh.h"
#include "t_buf.h"
#include "mesh_vertex.h"

class Mesh_Static : public Mesh
{
private:
	VBox _box;

public:
	Mesh_Static(const Buffer<Vertex17F>& vertices = Buffer<Vertex17F>(), const Buffer<uint32>& elements = Buffer<uint32>());
	virtual ~Mesh_Static() {}

	VBox& BoundingBox() { return _box; }
	const VBox& BoundingBox() const { return _box; }

	Buffer<Vertex17F> vertices;
	Buffer<uint32> elements;

	virtual bool IsValid() const { return vertices.GetSize() != 0; }

	virtual void Read(ByteReader& data) override;
	virtual void Write(ByteWriter& data) const override;

#ifdef BUILD_CLIENT
	virtual void UpdateRenderer() override;
#endif
};
