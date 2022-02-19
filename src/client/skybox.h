#pragma once
#include "shared/mesh_manager.h"
#include "gl_cubemap.h"

class RenderQueue;

class Skybox
{
private:
	GLCubemap _cubemap;

public:
	Skybox();
	~Skybox();

	void Load(const char *faces[6]);

	void Bind(byte unit) const
	{
		_cubemap.Bind(unit);
	}

	void Render(RenderQueue& q) const;
};

