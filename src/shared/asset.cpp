#include "asset.h"
#include "byte_writer.h"

Buffer<byte> Asset::GetAsData() const
{
	Buffer<byte> data;
	ByteWriter writer(data);
	Write(writer);
	return data;
}