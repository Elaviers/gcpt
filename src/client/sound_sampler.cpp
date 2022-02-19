#include "sound_sampler.h"
#include "sound_utils.h"
#include "shared/utils.h"
#include "shared/maths.h"
#include "shared/debug.h"
#include <limits>

uint32 Sampler::ReadToSoundBuffer(byte* dest, uint32 destFrameCount, uint32 destSampleRate, uint16 destChannelCount, float mix)
{
	if (!_playing) return 0;
	if (_sound->bitsPerSample != 16) return 0;

	mix *= _volume * _sound->volume;

	uint32 framesWritten = 0;

	if (mix > 0.f)
	{
		do
		{
			uint32 offset = _currentFrame * _sound->frameSize;

			framesWritten = AudioUtilities::ResampleTo(
				(int16*)(_sound->data + offset), _sound->sampleRate, _sound->channelCount, _sound->dataSize / _sound->frameSize - _currentFrame,
				(int16*)dest, destSampleRate, destChannelCount, destFrameCount, mix, 3);
		} while (_loop && framesWritten < destFrameCount);
	}

	_currentFrame += (uint32)((float)framesWritten * (float)_sound->sampleRate / (float)destSampleRate);

	return framesWritten;
}
