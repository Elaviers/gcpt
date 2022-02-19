#pragma once
#include "shared/t_array.h"
#include "shared/types.h"
class SoundClip;

namespace IO
{
	SoundClip* ReadWaveData(const Array<byte>& data);
	SoundClip* ReadWaveFile(const char* filename);
}
