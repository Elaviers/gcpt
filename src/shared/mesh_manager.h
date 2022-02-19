#pragma once
#include "asset_manager.h"
#include "mesh_static.h"

class MeshManager : public AssetManager<Mesh>
{
private:
	virtual Mesh* _CreateResource(const Array<byte>&, const String&, const String&) override;

	Mesh_Static* _line = nullptr;
	Mesh_Static* _cube = nullptr;
	Mesh_Static* _invCube = nullptr;
	Mesh_Static* _plane = nullptr;

	SharedPointer<const Mesh> _tLine, _tCube, _tInvCube, _tPlane;

public:
	MeshManager() : AssetManager({ ".mesh", ".obj" }) {}
	virtual ~MeshManager() {}

	virtual void Initialise() override;

	const SharedPointer<const Mesh>& Line() const			{ return _tLine; }
	const SharedPointer<const Mesh>& Cube() const			{ return _tCube; }
	const SharedPointer<const Mesh>& InverseCube() const	{ return _tInvCube; }
	const SharedPointer<const Mesh>& Plane() const			{ return _tPlane; }
};
