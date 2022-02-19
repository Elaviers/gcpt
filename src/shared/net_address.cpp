#include "net_address.h"
#include <WS2tcpip.h>

Address::Address(const char* ipv4, ushort port) : _sockaddr{}
{
	sockaddr_in& address = (sockaddr_in&)_sockaddr;
	address.sin_family = AF_INET;
	address.sin_port = htons(port);
	inet_pton(AF_INET, ipv4, &address.sin_addr);
}

String Address::GetAddr4String() const
{
	char buffer[64];

	sockaddr_in& inet = (sockaddr_in&)_sockaddr;
	if (inet_ntop(inet.sin_family, &inet.sin_addr, buffer, 64))
		return buffer;

	return "";
}

uint32 Address::GetAddr4() const
{
	return ntohl(((sockaddr_in&)_sockaddr).sin_addr.S_un.S_addr);
}

ushort Address::GetPort() const
{
	return ntohs(((sockaddr_in&)_sockaddr).sin_port);
}

bool Address::operator==(const Address& other) const noexcept
{
	return (_sockaddr.sa_family == other._sockaddr.sa_family) && (memcmp(_sockaddr.sa_data, other._sockaddr.sa_data, sizeof(_sockaddr.sa_data)) == 0);
}
