#include "server.h"
#include "console.h"
#include "convar.h"
#include "engine.h"
#include "player.h"

ConVar sv_timeout("sv_timeout", "20", "Max duration in seconds of net inactivity to allow before kicking a client");

static void ServerMsg(const char* fmt, ...)
{
	va_list vargs;

	char fmtBuffer[1024];
	snprintf(fmtBuffer, sizeof(fmtBuffer), "[Server] %s", fmt);

	va_start(vargs, fmt);
	Console_VMsg(fmtBuffer, vargs);
	va_end(vargs);
}

static void ServerWarning(const char* fmt, ...)
{
	va_list vargs;

	char fmtBuffer[1024];
	snprintf(fmtBuffer, sizeof(fmtBuffer), "[Server Warning] %s", fmt);

	va_start(vargs, fmt);
	Console_VMsg(fmtBuffer, vargs);
	va_end(vargs);
}


template <typename T>
void Server_CLNetPktReceived(Server& sv, int uid, const T& packet)
{

}

template<>
void Server_CLNetPktReceived(Server& sv, int uid, const NetPkt_Ack& packet)
{


}

template<>
void Server_CLNetPktReceived(Server& sv, int uid, const CLNetPkt_GoodBye& packet)
{
	ServerMsg("%s [%d] says goodbye. Reason: \"%s\"\n", sv.GetUserData(uid)->name.begin(), uid, packet.m_reason.begin());
	sv.DisconnectUser(uid);
}

bool Server::Start(int port)
{
	_packetMem.SetSize(SERVER_MAX_PACKET_SIZE);
	return _socket.Open(port);
}

void Server::Stop()
{
	_socket.Close();
}

void Server::Frame()
{
	if (!_socket.IsOpen())
		return;

	for (int i = 0; i < _users.GetSize(); ++i)
	{
		UserData* const user = _users[i];
		if (user)
		{
			if (user->isLeaving)
			{
				ServerMsg("%s disconnected\n", user->name.begin());
				delete user;
				_users[i] = nullptr;
				continue;
			}

			for (const CLNetPacket& localpkt : user->queuedIncomingPackets)
			{
				_ProcessCLPacket(localpkt, i, user->localId, &user->addr);
			}

			user->queuedIncomingPackets.Clear();
		}
	}

	Address from;
	int length;
	while (length = _socket.ReceiveFrom(from, _packetMem.begin(), _packetMem.GetSize()))
	{
		if (_clPacket.Read(_packetMem.begin(), _packetMem.GetSize()))
		{
			const LocalUsers* const usr = _addr2uid.TryGet(from);
			
			for (int i = 0; i < MAX_LOCAL_CLIENTS; ++i)
				if (_clPacket.m_players & (1 << i))
					_ProcessCLPacket(_clPacket, usr ? usr->uids[_clPacket.m_players] : INVALID_USER, i, &from);

		}
	}
}

void Server::SendCLPacket(const CLNetPacket& packet, int recipient)
{
	UserData* user = _users[recipient];
	
	if (user)
	{
		int packetSz = packet.Write(_packetMem.begin(), _packetMem.GetSize());
		_socket.SendTo(user->addr, _packetMem.begin(), packetSz);
	}
}

void Server::BroadcastCLPacket(const CLNetPacket& packet, bool (*shouldSendToUser)(int))
{
	int packetSz = packet.Write(_packetMem.begin(), _packetMem.GetSize());
	for (int i = 0; i < SERVER_MAX_USERS; ++i)
	{
		UserData* user = _users[i];
		if (user && shouldSendToUser(i))
		{
			_socket.SendTo(user->addr, _packetMem.begin(), packetSz);
		}
	}
}

#ifdef BUILD_CLIENT
void Server::ReceiveLocalPacket(const CLNetPacket& pkt)
{
	for (int i = 0; i < g_engine.localPlayers.GetSize(); ++i)
	{
		if (pkt.m_players & (1 << i))
		{
			Player* const player = g_engine.localPlayers[i];
			Assert(player);
			Assert(player->m_uid != INVALID_USER);
			AssertI(player->m_uid, <, _users.GetSize());

			/* TODO WHAT
			UserData* user = _users[player->uid];
			Assert(user);
			user->queuedIncomingPackets.Add(pkt);
			*/

			_ProcessCLPacket(pkt, g_engine.localPlayers[i]->m_uid, i, nullptr);
		}
	}
}
#endif

void Server::SendSVPacket(const SVNetPacket& packet, int recipient)
{
	UserData* user = _users[recipient];

	if (user)
	{
		int packetSz = packet.Write(_packetMem.begin(), _packetMem.GetSize());
		_socket.SendTo(user->addr, _packetMem.begin(), packetSz);
	}
}

void Server::BroadcastSVPacket(const SVNetPacket& packet, bool (*shouldSendToUser)(int))
{
	int packetSz = packet.Write(_packetMem.begin(), _packetMem.GetSize());
	for (int i = 0; i < SERVER_MAX_USERS; ++i)
	{
		UserData* user = _users[i];
		if (user && shouldSendToUser(i))
		{
			_socket.SendTo(user->addr, _packetMem.begin(), packetSz);
		}
	}
}

void Server::DisconnectUser(int uid)
{
	if (_users[uid])
		_users[uid]->isLeaving = true; //will be disconnected on next server frame
}

const UserData* Server::GetUserData(int uid)
{
	return _users[uid];
}

int Server::NewUser()
{
	UserData* user = new UserData();
	user->connected = false;
	user->lastHeartbeat = 0;

	int uid = INVALID_USER;
	for (int i = 0; i < _users.GetSize(); ++i)
	{
		if (!_users[i])
		{
			_users[i] = user;
			uid = i;
			break;
		}
	}

	if (uid == INVALID_USER)
	{
		_users.Add(user);
		uid = _users.GetSize() - 1;
	}

	return uid;
}

void Server::_ProcessCLPacket(const CLNetPacket& pkt, int fromUid, int localId, const Address* fromAddr)
{
	const uint64 now = g_engine.GetTimeMillis();

	if (fromUid == INVALID_USER)
	{
		if (pkt.m_type == ENetMsg::HELLO)
		{
			if (localId != INVALID_USER)
				ServerWarning("Received a hello from %s, but the localId is valid? This shouldn't happen.\n");

			const int uid = NewUser();
			UserData* user = _users[uid];
			user->lastHeartbeat = now;
			
			//TODO TODO TODO CLIENT NAME
			//user->name = pkt.m_data.hello.m_clientNames;
			user->connected = true;

			if (fromAddr)
			{
				user->addr = *fromAddr;
				_addr2uid[user->addr].uids[localId] = fromUid;
			}

			ServerMsg("Received hello from %s - client connected (id:%d, name:%s)\n", fromAddr->GetAddr4String().begin(), uid, user->name.begin());
		}
		else
		{
			ServerWarning("Received [%s] from a disconnected client!\n", g_netMsgNames[(int)pkt.m_type]);
		}
	}
	else
	{
		if (UserData* user = _users[fromUid])
		{
			switch (pkt.m_type)
			{
			case ENetMsg::ACK: Server_CLNetPktReceived<NetPkt_Ack>(*this, fromUid, pkt.m_data.ack); break;
			case ENetMsg::HEARTBEAT: HeartbeatReceived(fromUid); break;
			case ENetMsg::HELLO: ServerWarning("Received [%s] from a connected client!\n", g_netMsgNames[(int)pkt.m_type]); break;
			case ENetMsg::GOODBYE: Server_CLNetPktReceived<CLNetPkt_GoodBye>(*this, fromUid, pkt.m_data.goodbye); break;
			case ENetMsg::CONFIG: Server_CLNetPktReceived<CLNetPkt_Config>(*this, fromUid, pkt.m_data.config); break;
			case ENetMsg::SPEAKTEXT: Server_CLNetPktReceived<NetPkt_SpeakText>(*this, fromUid, pkt.m_data.speakText); break;
			case ENetMsg::SPEAKVOICE: Server_CLNetPktReceived<NetPkt_SpeakVoice>(*this, fromUid, pkt.m_data.speakVoice); break;
			case ENetMsg::MOVE: Server_CLNetPktReceived<NetPkt_Move>(*this, fromUid, pkt.m_data.move); break;
			}
		}
	}
}

void Server::HeartbeatReceived(int fromUser)
{
	_users[fromUser]->lastHeartbeat = g_engine.GetTimeMillis();
}
