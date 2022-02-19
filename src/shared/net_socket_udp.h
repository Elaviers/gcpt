#pragma once
#include "types.h"
#include <WinSock2.h>

class Address;

class UDPSocket
{
private:
	SOCKET _socket;

public:
	UDPSocket() : _socket(INVALID_SOCKET) {}
	UDPSocket(const UDPSocket&) = delete;
	UDPSocket(UDPSocket&& other) noexcept : _socket(other._socket) { other._socket = INVALID_SOCKET; }

	bool IsOpen() const { return _socket != INVALID_SOCKET; }

	bool Open(ushort port);
	void Close();

	bool SendTo(const Address& to, const byte* buffer, int size) const;

	//Returns bytes received. If nonzero, from is set to the address we received from
	size_t ReceiveFrom(Address& from, byte* buffer, int size) const;
};
