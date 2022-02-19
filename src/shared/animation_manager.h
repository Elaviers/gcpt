#pragma once
#include "asset_manager.h"
#include "animation.h"

class AnimationManager : public AssetManager<Animation>
{
	virtual Animation* _CreateResource(const Array<byte>& data, const String& name, const String& extension) override;

public:
	AnimationManager() : AssetManager({ ".anim" }) {}
	virtual ~AnimationManager() {}
};
