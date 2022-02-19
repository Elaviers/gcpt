#pragma once
#include "t_hashmap.h"
#include "string.h"
#include "net_address.h"
#include "net_socket_udp.h"
#include "net_packet.h"
#include "User.h"

constexpr const int SERVER_MAX_USERS = 2048;
constexpr const int SERVER_MAX_PACKET_SIZE = 32000;

class Server
{
private:
	UDPSocket _socket;
	Buffer<UserData*> _users;
	Buffer<byte> _packetMem;

	struct LocalUsers
	{
		int uids[MAX_LOCAL_CLIENTS];
	};

	Hashmap<Address, LocalUsers> _addr2uid;

public:
	Server() {}
	Server(const Server&) = delete;
	Server(Server&&) = delete;
	~Server() { Stop(); }

	bool Start(int port);
	void Stop();

	void Frame();

	void DisconnectUser(int uid);

	const UserData* GetUserData(int uid);

	int NewUser(); //First message sent to this uid will need to be a HELLO message, just like a net client

private:
	void _ProcessCLPacketForEachLocalId(const CLNetPacket& pkt, int fromUid, const Address* fromAddr);
	void _ProcessCLPacket(const CLNetPacket& pkt, int fromUid, int localId, const Address* fromAddr);
	
	void HeartbeatReceived(int fromUser);


private:
	CLNetPacket _clPacket;
	SVNetPacket _svPacket;

public:
	void SendSVPacket(const SVNetPacket&, int recipient);
	void SendCLPacket(const CLNetPacket&, int recipient);

	void BroadcastSVPacket(const SVNetPacket&, bool (*shouldSendToUser)(int uid));
	void BroadcastCLPacket(const CLNetPacket&, bool (*shouldSendToUser)(int uid));

#ifdef BUILD_CLIENT
	void ReceiveLocalPacket(const CLNetPacket& pkt);
#endif
};
