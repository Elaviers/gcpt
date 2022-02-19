#pragma once
#include "shared/asset.h"
#include "shared/types.h"

class SoundClip : public Asset
{
public:
	uint16 format = 0;
	uint16 channelCount = 0;
	uint32 sampleRate = 0;
	uint32 byteRate = 0;
	uint16 frameSize = 0;
	uint16 bitsPerSample = 0;

	byte* data = nullptr;
	uint32 dataSize = 0;

	float volume = 0.f;

	virtual ~SoundClip() { delete data; }

	void MakeStereo();

	void Destroy() { delete[] data; }
};
