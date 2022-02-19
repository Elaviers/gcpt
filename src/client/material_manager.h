#pragma once
#include "material.h"
#include "texture_manager.h"
#include "shared/asset_manager.h"
#include "shared/function.h"

class MaterialManager : public AssetManager<Material>
{
private:
	virtual Material* _CreateResource(const Array<byte>& data, const String& name, const String& extension) override;
	virtual bool _CreateAlternative(Material*& material, const String& name) override;

	virtual Buffer<const AssetManagerBase*> _GetFallbackManagers() const override;

public:
	MaterialManager() : AssetManager({".txt", ".tmat", ".mat"}) {}
	virtual ~MaterialManager() {}

	void Initialise(const TextureManager&);
};
 