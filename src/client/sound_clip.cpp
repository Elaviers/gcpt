#include "sound_clip.h"
#include "shared/utils.h"

void SoundClip::MakeStereo()
{
	if (this->channelCount == 2) return;

	if (this->channelCount == 1)
	{
		uint32 newSize = this->dataSize * 2;
		byte* newData = new byte[newSize];

		byte sampleSize = this->bitsPerSample / 8;

		for (uint32 i = 0; i < this->dataSize; i += sampleSize) {
			Utilities::CopyMemory(&this->data[i], &newData[i * 2], sampleSize);
			Utilities::CopyMemory(&this->data[i], &newData[(i * 2) + sampleSize], sampleSize);
		}

		this->data = newData;
		this->dataSize = newSize;
		this->channelCount = 2;
		this->frameSize *= 2;
		this->byteRate *= 2;
	}

}
