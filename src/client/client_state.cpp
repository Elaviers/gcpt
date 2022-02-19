#include "client_state.h"
#include "client.h"
#include "shared/console.h"
#include "shared/engine.h"
#include "assert.h"

ClientState::ClientState() : _helloClients(0)
{
	
}

bool ClientState::HandlePacket(const SVNetPacket& pkt)
{
	if (_currentState == EClientState::DISCONNECTED)
		return false;

	switch (pkt.m_type)
	{
	case ENetMsg::HELLO:
		if (_currentState != EClientState::AWAIT_HELLO)
		{
			Console_Warning("Received an unexpected hello message from server!\n");
			return false;
		}

		// todo g_engine.SetPlayerUids(pkt.)

		break;



	}

	switch (_currentState)
	{
	case EClientState::DISCONNECTED:
		return false;
		break;

	case EClientState::AWAIT_HELLO:
		switch (pkt.m_type)
		{
		case ENetMsg::HELLO:






		default:
			return false;
		}

		return true;

	case EClientState::CONNECTED:
		switch (pkt.m_type)
		{




		default:
			return false;
		}

		return true;
	}

	AssertM(0, "Invalid EClientState!");
	return false;
}

void ClientState::ConnectToServer(Client& client, const Player* localPlayers, int numLocalPlayers)
{
	AssertI((int)_currentState, == , (int)EClientState::DISCONNECTED);

	_netClient = &client;

	{
		ExclusiveHandle<CLNetPacket> pkt = _netClient->GetWritablePacket();
		pkt->m_type = ENetMsg::HELLO;
		pkt->m_players = 0;
		pkt->m_data.hello.m_clientCount = numLocalPlayers;

		pkt->m_data.hello.m_clientNames.SetSize(numLocalPlayers);
		for (int i = 0; i < numLocalPlayers; ++i)
			pkt->m_data.hello.m_clientNames[i] = localPlayers->m_name;
		
		_netClient->Send(*pkt);
	}

}
