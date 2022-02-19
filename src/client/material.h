#pragma once
#include "render_channels.h"
#include "render_entry.h"
#include "texture.h"
#include "shared/asset.h"
#include "shared/string.h"
#include "shared/shared_ptr.h"

struct MaterialParam;

class Material : public Asset
{
protected:
	ERenderChannels _RenderChannels;

	Material(ERenderChannels channels) : _RenderChannels(channels) { }

public:
	virtual ~Material() {}

	static Material* FromText(const String&);

	virtual void Apply(RenderEntry&, const MaterialParam* param = nullptr) const = 0;

	virtual bool SetDefaultTexture(const SharedPointer<const Texture>& texture) { return false; }
	virtual const SharedPointer<const Texture>& GetDefaultTexture() const { static SharedPointer<const Texture> null; return null; }

	ERenderChannels GetRenderChannels() const { return _RenderChannels; }
};
