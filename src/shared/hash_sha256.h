#pragma once
#include "hash_functor.h"
#include "types.h"

class SHA256 : public Hasher<byte[32]>
{
public:
	void Hash(const void* in, size_t length, byte (&out)[32]) const;

};
