#include "sound_io.h"
#include "sound_clip.h"
#include "shared/byte_reader.h"
#include "shared/io.h"

SoundClip* IO::ReadWaveData(const Array<byte>& data)
{
	ByteReader reader(data);

	if (!(reader.Read_byte() == 'R' && reader.Read_byte() == 'I' && reader.Read_byte() == 'F' && reader.Read_byte() == 'F'))
		return nullptr;

	uint32 chunkSize = reader.Read_uint32_little();

	if (!(reader.Read_byte() == 'W' && reader.Read_byte() == 'A' && reader.Read_byte() == 'V' && reader.Read_byte() == 'E'))
		return nullptr;

	if (!(reader.Read_byte() == 'f' && reader.Read_byte() == 'm' && reader.Read_byte() == 't' && reader.Read_byte() == ' '))
		return nullptr;

	SoundClip* aud = new SoundClip;

	uint32 fmtChunkSize = reader.Read_uint32_little();
	aud->format = reader.Read_uint16_little();
	aud->channelCount = reader.Read_uint16_little();
	aud->sampleRate = reader.Read_uint32_little();
	aud->byteRate = reader.Read_uint32_little();
	aud->frameSize = reader.Read_uint16_little();
	aud->bitsPerSample = reader.Read_uint16_little();
	aud->data = NULL;

	if (!(reader.Read_byte() == 'd' && reader.Read_byte() == 'a' && reader.Read_byte() == 't' && reader.Read_byte() == 'a'))
		return aud;

	aud->dataSize = reader.Read_uint32_little();
	aud->data = new byte[aud->dataSize];

	//ReadTo is kinda slow here..
	//size_t bytesRead = reader.ReadTo(aud.data, aud.dataSize);

	Utilities::CopyMemory(reader.Ptr(), aud->data, aud->dataSize);

	return aud;
}

SoundClip* IO::ReadWaveFile(const char* filename)
{
	return ReadWaveData(IO::ReadFile(filename));
}
