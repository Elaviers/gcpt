#pragma once
#include "t_buf.h"
#include "string.h"

struct UserData
{
	bool connected;

	Address addr;
	String name;

	uint64 lastHeartbeat;

	bool isLeaving;

	int localId;

	Buffer<CLNetPacket, FixedAllocatorWithFallback<CLNetPacket, 8>> queuedIncomingPackets;
	Buffer<byte, FixedAllocatorWithFallback<byte, 512>> queuedIncomingData;
};
