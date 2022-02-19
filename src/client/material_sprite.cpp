#include "material_sprite.h"
#include "shared/macro_utils.h"
#include "shared/engine.h"

#ifdef BUILD_CLIENT
#include "client/texture_manager.h"
#endif

const PropertyCollection& MaterialSprite::GetProperties()
{
	static PropertyCollection properties;

	IF_FIRST_EXEC_DO(properties.AddCommand("tex", MemberCommandPtr<MaterialSprite>(&MaterialSprite::_CMD_tex)));

	return properties;
}

void MaterialSprite::_CMD_tex(const Array<String>& args)
{
	if (args.GetSize() > 0)
	{
		TextureManager* const textureManager = g_engine.textures;
		if (textureManager)
			_diffuse = textureManager->Get(args[0]);
	}
}

void MaterialSprite::Apply(RenderEntry& e, const MaterialParam* param) const
{
	if (_diffuse) e.AddSetTexture(*_diffuse, 0);
	else e.AddSetTexture(RCMDSetTexture::Type::WHITE, 0);
}
