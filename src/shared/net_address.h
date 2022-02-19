#pragma once
#include "string.h"
#include "types.h"
#include <WinSock2.h>

class Address
{
	sockaddr _sockaddr;

public:
	Address() : _sockaddr{} {}
	Address(const char* ipv4, ushort port);
	
	uint32 GetAddr4() const; //host byte order
	String GetAddr4String() const;
	
	ushort GetPort() const; //host byte order

	bool operator==(const Address& other) const noexcept;
};
