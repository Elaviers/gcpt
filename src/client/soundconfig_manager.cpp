#include "soundconfig_manager.h"
#include "shared/debug.h"

SoundConfig* SoundConfigManager::_CreateResource(const Array<byte>& data, const String& name, const String& extension)
{
	SoundConfig* soundConfig = new SoundConfig();
	if (!soundConfig->Parse(name, (const char*)data.begin()))
		Debug::Error(CSTR("Error loading sound configuration \"", name, "\""));

	return soundConfig;
}
