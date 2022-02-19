#include "sound_manager.h"
#include "sound_io.h"
#include "sound_utils.h"
#include "shared/utils.h"
#include "shared/debug.h"
#include "shared/maths.h"

#undef CopyMemory

SoundClip* SoundManager::_CreateResource(const Array<byte>& data, const String& name, const String& extension)
{
	SoundClip* sound;
	float volume = 1.f;
	
	if (extension.Equals(".txt", true))
	{
		String filename;
		Buffer<String> lines = String(data.begin(), data.GetSize()).ToLower().Split("\r\n");

		for (const String& line : lines)
		{
			Buffer<String> tokens = line.Split(" ");

			if (tokens[0] == "filename")
				filename = tokens[1];
			else if (tokens[0] == "volume")
				volume = tokens[1].ToFloat();
		}
		
		sound = IO::ReadWaveFile((this->GetRootPath() + filename).begin());
	}
	else
	{
		sound = IO::ReadWaveData(data);
	}

	sound->volume = volume;

	if (sound->sampleRate != _filteringTargetSampleRate)
	{
		byte* originalData = new byte[sound->dataSize];
		Utilities::CopyMemory(sound->data, originalData, sound->dataSize);

		uint32 cutoff = (uint32)(Maths::Min<uint32>(sound->sampleRate, _filteringTargetSampleRate) / 2.f);

		AudioUtilities::LowPassFilter(
			(int16*)originalData,
			(int16*)sound->data,
			sound->channelCount,
			sound->dataSize / sound->frameSize,
			sound->sampleRate,
			cutoff);

		delete[] originalData;

		//sound.Resample(_waveFormat.nSamplesPerSec);
	}

	return sound;
}

void SoundManager::_DestroyResource(SoundClip& sound)
{
	sound.Destroy();
}
