#pragma once
#include "asset.h"
#include "concepts.h"
#include "volume.h"

#ifdef BUILD_CLIENT
#include "client/mesh_renderer.h"
#endif

class Mesh : public Asset
{
protected:
#ifdef BUILD_CLIENT
	MeshRenderer _renderer;
#endif

	Volume* _volume;

	Mesh(Volume* volume) : _volume(volume) {}

public:
	virtual ~Mesh() {}

	const Volume* GetVolume() const { return _volume; }

	static Mesh* FromData(const Buffer<byte>&);

#ifdef BUILD_CLIENT
	void SetRenderMode(ERenderMode mode) { _renderer.SetRenderMode(mode); }
	void Render() const { _renderer.Render(); }
	
	virtual bool IsValid() const { return false; }
	virtual void UpdateRenderer() {}
#endif
};
