#include "mesh_manager.h"
#include "gfx_io.h"

Mesh* MeshManager::_CreateResource(const Array<byte>& data, const String& name, const String& ext)
{
	if (ext == ".obj")
	{
		Mesh* mesh = IO::ReadOBJFile(String(data.begin(), data.GetSize()));

		if (mesh == nullptr)
			Debug::Error(CSTR("Could not load OBJ \"", name, "\""));

		return mesh;
	}

	Mesh* mesh = Mesh::FromData(data);

	if (mesh == nullptr)
		Debug::Error(CSTR("Could not load mesh \"", name, "\""));

	return Mesh::FromData(data);
}

void MeshManager::Initialise()
{
	const Vector3 white(1.f, 1.f, 1.f);

	//Line
	_line = new Mesh_Static(
		Buffer<Vertex17F>({
			//pos						uv			_colour		tangent			bitangent			normal
			{Vector3(0.f, -.5f, 0.f), Vector2(0, 0), white, Vector3(1, 0, 0), Vector3(0, 1, 0), Vector3(0, 0, -1)},
			{Vector3(0.f, .5f, 0.f), Vector2(1, 0), white, Vector3(1, 0, 0), Vector3(0, 1, 0), Vector3(0, 0, -1)}
			})
	);

	//Plane
	_plane = new Mesh_Static(
		Buffer<Vertex17F>({
			{ Vector3(-.5f, -.5f, 0), Vector2(0, 1),	white, Vector3(1, 0, 0), Vector3(0, 1, 0), Vector3(0, 0, -1) },
			{ Vector3(.5f, -.5f, 0), Vector2(1, 1),		white, Vector3(1, 0, 0), Vector3(0, 1, 0), Vector3(0, 0, -1) },
			{ Vector3(-.5f, .5f, 0), Vector2(0, 0),		white, Vector3(1, 0, 0), Vector3(0, 1, 0), Vector3(0, 0, -1) },
			{ Vector3(.5f, .5f, 0), Vector2(1, 0),		white, Vector3(1, 0, 0), Vector3(0, 1, 0), Vector3(0, 0, -1) }
			}),
		Buffer<uint32>({
			0, 1, 2,
			3, 2, 1
			}));

	//Cube
	_cube = new Mesh_Static(
		Buffer<Vertex17F>({
			//Bottom
			VERT14F_TRI(Vector3(.5f, -.5f, -.5f),	Vector3(-.5f, -.5f, -.5f),	Vector3(.5f, -.5f, .5f),	Vector2(0.f, 1.f),	Vector2(1.f, 1.f),	Vector2(0.f, 0.f),	Vector3(0, -1, 0)),
			VERT14F_TRI(Vector3(-.5f, -.5f, .5f),	Vector3(.5f, -.5f, .5f),	Vector3(-.5f, -.5f, -.5f),	Vector2(1.f, 0.f),	Vector2(0.f, 0.f),	Vector2(1.f, 1.f),	Vector3(0, -1, 0)),

			//Top
			VERT14F_TRI(Vector3(-.5f, .5f, -.5f),	Vector3(.5f, .5f, -.5f),	Vector3(-.5f, .5f, .5f),	Vector2(0.f, 1.f),	Vector2(1.f, 1.f),	Vector2(0.f, 0.f),	Vector3(0, 1, 0)),
			VERT14F_TRI(Vector3(.5f, .5f, .5f),		Vector3(-.5f, .5f, .5f),	Vector3(.5f, .5f, -.5f),	Vector2(1.f, 0.f),	Vector2(0.f, 0.f),	Vector2(1.f, 1.f),	Vector3(0, 1, 0)),

			//Front
			VERT14F_TRI(Vector3(-.5f, -.5f, -.5f),	Vector3(.5f, -.5f, -.5f),	Vector3(-.5f, .5f, -.5f),	Vector2(0.f, 1.f),	Vector2(1.f, 1.f),	Vector2(0.f, 0.f),	Vector3(0, 0, -1)),
			VERT14F_TRI(Vector3(.5f, .5f, -.5f),	Vector3(-.5f, .5f, -.5f),	Vector3(.5f, -.5f, -.5f),	Vector2(1.f, 0.f),	Vector2(0.f, 0.f),	Vector2(1.f, 1.f),	Vector3(0, 0, -1)),

			//Right
			VERT14F_TRI(Vector3(.5f, -.5f, -.5f),	Vector3(.5f, -.5f, .5f),	Vector3(.5f, .5f, -.5f),	Vector2(0.f, 1.f),	Vector2(1.f, 1.f),	Vector2(0.f, 0.f),	Vector3(1, 0, 0)),
			VERT14F_TRI(Vector3(.5f, .5f, .5f),		Vector3(.5f, .5f, -.5f),	Vector3(.5f, -.5f, .5f),	Vector2(1.f, 0.f),	Vector2(0.f, 0.f),	Vector2(1.f, 1.f),	Vector3(1, 0, 0)),

			//Back
			VERT14F_TRI(Vector3(.5f, -.5f, .5f),	Vector3(-.5f, -.5f, .5f),	Vector3(.5f, .5f, .5f),		Vector2(0.f, 1.f),	Vector2(1.f, 1.f),	Vector2(0.f, 0.f),	Vector3(0, 0, 1)),
			VERT14F_TRI(Vector3(-.5f, .5f, .5f),	Vector3(.5f, .5f, .5f),		Vector3(-.5f, -.5f, .5f),	Vector2(1.f, 0.f),	Vector2(0.f, 0.f),	Vector2(1.f, 1.f),	Vector3(0, 0, 1)),

			//Left
			VERT14F_TRI(Vector3(-.5f, -.5f, .5f),	Vector3(-.5f, -.5f, -.5f),	Vector3(-.5f, .5f, .5f),	Vector2(0.f, 1.f),	Vector2(1.f, 1.f),	Vector2(0.f, 0.f),	Vector3(-1, 0, 0)),
			VERT14F_TRI(Vector3(-.5f, .5f, -.5f),	Vector3(-.5f, .5f, .5f),	Vector3(-.5f, -.5f, -.5f),	Vector2(1.f, 0.f),	Vector2(0.f, 0.f),	Vector2(1.f, 1.f),	Vector3(-1, 0, 0))
		})
	);

	for (size_t i = 0; i < 36; i += 3)
		Vertex17F::CalculateTangents(_cube->vertices[i], _cube->vertices[i + 1], _cube->vertices[i + 2]);

	_line->BoundingBox().SetScale(Vector3(0.f, 1.f, 0.f));
	_plane->BoundingBox().SetScale(Vector3(1.f, 1.f, 0.f));
	//default bounds are cube, same for invcube

#ifdef BUILD_CLIENT
	_line->SetRenderMode(ERenderMode::LINES);
	_line->UpdateRenderer();
	_plane->UpdateRenderer();
	_cube->UpdateRenderer();
#endif
	
	_invCube = new Mesh_Static(_cube->vertices, _cube->elements);
	
	for (size_t i = 0; i < 36; i += 3) {
		_invCube->vertices[i].normal *= -1;
		_invCube->vertices[i + 1].normal *= -1;
		_invCube->vertices[i + 2].normal *= -1;

		Utilities::Swap(_invCube->vertices[i + 1], _invCube->vertices[i + 2]);
		Vertex17F::CalculateTangents(_invCube->vertices[i], _invCube->vertices[i + 1], _invCube->vertices[i + 2]);
	}

#ifdef BUILD_CLIENT
	_invCube->UpdateRenderer();
#endif

	SharedPointerData<Mesh>& dLine = _MapValue("line"), & dPlane = _MapValue("plane"), & dCube = _MapValue("cube"), & dInvCube = _MapValue("invcube");

	dLine.SetPtr(_line);
	dPlane.SetPtr(_plane);
	dCube.SetPtr(_cube);
	dInvCube.SetPtr(_invCube);

	_tLine = SharedPointer<Mesh>(dLine);
	_tPlane = SharedPointer<Mesh>(dPlane);
	_tCube = SharedPointer<Mesh>(dCube);
	_tInvCube = SharedPointer<Mesh>(dInvCube);
}
