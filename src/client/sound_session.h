#pragma once
#include "sound_sampler.h"
#include "sound_clip.h"
#include "shared/t_list.h"
#include <Windows.h>
#include <Audioclient.h>
#include <mmdeviceapi.h>

#undef PlaySound

class AudioSession
{
private:
	IMMDeviceEnumerator* _enumerator;
	IMMDevice* _audioDevice;
	IAudioClient* _audioClient;
	IAudioRenderClient* _renderClient;

	WAVEFORMATEX _waveFormat;

	uint32 _bufferFrameCount;

	List<Sampler> _playingSounds;

public:
	AudioSession() : _waveFormat{ 0 } {}
	~AudioSession();

	Sampler& PlaySound(const SoundClip& sound);

	void Initialise(uint32 minimumBufferDurationMillis = 100);

	void FillBuffer();

	void ImGui();

	constexpr uint32 GetSampleRate() const noexcept { return _waveFormat.nSamplesPerSec; }
};
