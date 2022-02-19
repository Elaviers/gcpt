#include "animation_manager.h"

Animation* AnimationManager::_CreateResource(const Array<byte>& data, const String& name, const String& ext)
{
	Animation* anim = Animation::FromData(data);

	if (anim == nullptr)
		Debug::Error(CSTR("Could not load animation \"", name, '\"'));

	return anim;
}
