#include "mesh.h"
#include "mesh_static.h"
#include "mesh_skeletal.h"

//static
Mesh* Mesh::FromData(const Buffer<byte>& data)
{
	Mesh* mesh = nullptr;

#ifdef BUILD_CLIENT
	ByteReader reader = data;
	switch (reader.Peek_byte())
	{
	case ASSET_MESH_STATIC:
		mesh = new Mesh_Static();
		break;
	case ASSET_MESH_SKELETAL:
		mesh = new Mesh_Skeletal();
		break;
	}

	if (mesh)
	{
		mesh->Read(reader);
	}
#endif

	return mesh;
}
