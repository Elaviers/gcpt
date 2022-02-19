#include "player.h"
#include "server.h"

#ifdef BUILD_CLIENT
bool Player::Connect(Client& netClient)
{
	if (IsConnected())
		return false;

	_netClient = &netClient;

	{
		ExclusiveHandle<CLNetPacket> pkt = _netClient->GetWritablePacket();
		pkt->m_type = ENetMsg::HELLO;
		pkt->m_players = m_localId;
		
		//TODO TODO TODO CLIENT NAME
		//pkt->m_data.hello.m_name = name;
		_netClient->Send(*pkt);
	}
}

void Player::Disconnect()
{
	if (!IsConnected())
		return;

	{
		ExclusiveHandle<CLNetPacket> pkt = _netClient->GetWritablePacket();
		pkt->m_type = ENetMsg::GOODBYE;
		pkt->m_players = m_localId;
		pkt->m_data.goodbye.m_reason = "disconnected";
		_netClient->Send(*pkt);
	}

	_netClient->Disconnect();
}

void Player::Frame()
{
	if (!IsConnected())
		return;



}

#endif

Player::Player(int localId) : m_uid(INVALID_USER), m_localId(localId)
{
#ifdef BUILD_CLIENT
	_netClient = nullptr;
#endif
}

Player::~Player()
{

}

#ifndef BUILD_CLIENT
void Player::Frame()
{
	

}
#endif
