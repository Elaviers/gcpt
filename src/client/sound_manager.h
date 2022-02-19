#pragma once
#include "shared/asset_manager.h"
#include "sound_sampler.h"

class SoundManager : public AssetManager<SoundClip>
{
	virtual SoundClip* _CreateResource(const Array<byte>& data, const String& name, const String& extension) override;
	virtual void _DestroyResource(SoundClip& sound) override;

	uint32 _filteringTargetSampleRate = 48000;
public:
	SoundManager(uint32 targetSampleRate) : AssetManager<SoundClip>({".txt", ".wav"}), _filteringTargetSampleRate(targetSampleRate) {}
	virtual ~SoundManager() {}

	constexpr uint32 GetTargetSampleRate() const noexcept { return _filteringTargetSampleRate; }

	//Old sounds will need to be reloaded in order for the filtering to take effect
	void SetTargetSampleRate(uint32 targetSampleRate) noexcept { _filteringTargetSampleRate = targetSampleRate;  }
};
