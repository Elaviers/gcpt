#pragma once
#include "io.h"
#include "mesh_static.h"

#ifdef BUILD_CLIENT
#include "client/texture.h"
#endif

namespace IO
{
	bool ReadPNGFile(const Buffer<byte>& data, Buffer<byte>& outData, unsigned int& outWidth, unsigned int& outHeight);
	inline bool ReadPNGFile(const char* filename, Buffer<byte>& outData, unsigned int& outWidth, unsigned int& outHeight) { return ReadPNGFile(IO::ReadFile(filename), outData, outWidth, outHeight); }

	Mesh_Static* ReadOBJFile(const String& textData);
	inline Mesh_Static* ReadOBJFile(const char* filename) { return ReadOBJFile(IO::ReadFileString(filename)); }

#ifdef BUILD_CLIENT
	Texture* ReadPNGFile(const Buffer<byte> &data, int maxMipLevels = 0, int maxAnisotropy = 0);
	inline Texture* ReadPNGFile(const char* filename) { return ReadPNGFile(IO::ReadFile(filename)); }
#endif
}
