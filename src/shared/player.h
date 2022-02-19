#pragma once
#include "string.h"

#ifdef BUILD_CLIENT
#include "client/client.h"
#endif

class Player
{
public:
	String m_name;
	int m_uid;
	int m_localId;

	bool m_pendingDisconnect;

#ifdef BUILD_CLIENT
private:
	Client* _netClient;

public:
	bool IsConnected() const { return _netClient != nullptr; }
	bool Connect(Client& netClient);
	void Disconnect();

#endif

	Player(int localId);
	~Player();

	void Frame();
};

