#include "skybox.h"
#include "shared/gfx_io.h"
#include "shared/debug.h"
#include "shared/engine.h"
#include "client/render_cmd.h"
#include "client/render_queue.h"
#include "client/texture_manager.h"
#include <utility> //move

Skybox::Skybox()
{
}


Skybox::~Skybox()
{
}

void Skybox::Load(const char *faceFilenames[6])
{
	TextureManager* const textureManager = g_engine.textures;
	Assert(textureManager);

	Buffer<byte> facebuffers[6];
	TextureData faces[6] = {};

	for (int i = 0; i < 6; ++i) {
		bool success = IO::ReadPNGFile(CSTR(textureManager->GetRootPath(), faceFilenames[i]), facebuffers[i], faces[i].width, faces[i].height);
		if (!success)
			return;

		faces[i].data = facebuffers[i].begin();
	}

	_cubemap.Create(faces);
}

void Skybox::Render(RenderQueue& q) const
{
	{
		auto& e = q.CreateEntry(ERENDERCHANNEL_SKYBOX);
		e.AddSetCubemap(_cubemap, 0);
		e.AddCommand(RCMDRenderMesh::INV_CUBE);
	}

	{
		auto& e = q.CreateEntry(ERenderChannels::ALL, -1);
		e.AddSetCubemap(_cubemap, 100);
	}
}
