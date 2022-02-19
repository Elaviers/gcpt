#include "net.h"
#include "string.h"
#include "debug.h"
#include <WinSock2.h>

void Networking::Initialise()
{
	WSADATA wsaData;
	if (int wsaStatus = WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		Debug::Error(CSTR("WSAStartup failed with error code ", wsaStatus));
	}
}

void Networking::Cleanup()
{
	WSACleanup();
}
