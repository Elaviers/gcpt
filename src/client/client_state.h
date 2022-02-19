#pragma once
#include "shared/net_packet.h"

enum class EClientState
{
	DISCONNECTED,
	AWAIT_HELLO,
	CONNECTED,
};

class ClientState
{
	EClientState _currentState;

	byte _helloClients; static_assert(MAX_LOCAL_CLIENTS == 8); //Bitmask of local clients that we have received a uid for

	class Client* _netClient;

public:
	ClientState();

	constexpr EClientState GetCurrentState() const noexcept { return _currentState; }

	//returns true if packet is used
	bool HandlePacket(const SVNetPacket&);

	void ConnectToServer(class Client&, const class Player* localPlayers, int numLocalPlayers);

};
