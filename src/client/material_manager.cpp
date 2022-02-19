#include "material_manager.h"
#include "material_grid.h"
#include "material_surface.h"
#include "material_sprite.h"
#include "shared/utils.h"
#include "shared/debug.h"
#include "shared/engine.h"

#ifdef BUILD_CLIENT
#include "client/texture_manager.h"
#endif

Material* MaterialManager::_CreateResource(const Array<byte>& data, const String& name, const String& ext)
{
	Material* mat = Material::FromText(String(data.begin(), data.GetSize()));

	if (mat == nullptr)
		Debug::Error(CSTR("Could not load material \"", name, '\"'));

	return mat;
}

bool MaterialManager::_CreateAlternative(Material*& material, const String& name)
{
	if (material)
	{
		//The material has been created by an _all file
		//Look in the texturemanager for a texture with the same name
		
		SharedPointer<const Texture> texture = g_engine.textures->Get(name);
		if (texture && material->SetDefaultTexture(texture))
			return true;
	}

	return false; //Can't do anything now
}

Buffer<const AssetManagerBase*> MaterialManager::_GetFallbackManagers() const
{
	return { (const AssetManagerBase*)g_engine.textures };
}

void MaterialManager::Initialise(const TextureManager& tm)
{
	_MapValue("white").SetPtr(new MaterialSurface(tm.White(), tm.UnitNormal(), tm.White()));
}
