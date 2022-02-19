#include "shared/asset_manager.h"
#include "sound.h"

class SoundConfigManager : public AssetManager<SoundConfig>
{
	virtual SoundConfig* _CreateResource(const Array<byte>& data, const String& name, const String& extension);

};
