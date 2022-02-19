#pragma once
#include "material.h"
#include "texture.h"
#include "shared/shared_ptr.h"

class MaterialSprite : public Material
{
protected:
	SharedPointer<const Texture> _diffuse;

	void _CMD_tex(const Array<String>& args);

public:
	MaterialSprite(const SharedPointer<const Texture>& diffuse = SharedPointer<const Texture>()) : Material(ERENDERCHANNEL_MATERIALSPRITE), _diffuse(diffuse) {}

	virtual ~MaterialSprite() {}

	const SharedPointer<const Texture>& GetDiffuse() const { return _diffuse; }
	void SetDiffuse(const SharedPointer<const Texture>& diffuse) { _diffuse = diffuse; }

	virtual const PropertyCollection& GetProperties() override;

	virtual void Apply(RenderEntry&, const MaterialParam* param = nullptr) const override;

	virtual bool SetDefaultTexture(const SharedPointer<const Texture>& texture) override { SetDiffuse(texture); return true; }
	virtual const SharedPointer<const Texture>& GetDefaultTexture() const override { return _diffuse; }
};
