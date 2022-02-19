#include "client.h"
#include "shared/console.h"
#include "shared/server.h"

static void ClientMsg(const char* fmt, ...)
{
	va_list vargs;

	char fmtBuffer[1024];
	snprintf(fmtBuffer, sizeof(fmtBuffer), "[Client] %s", fmt);

	va_start(vargs, fmt);
	Console_VMsg(fmtBuffer, vargs);
	va_end(vargs);
}

static void ClientWarning(const char* fmt, ...)
{
	va_list vargs;

	char fmtBuffer[1024];
	snprintf(fmtBuffer, sizeof(fmtBuffer), "[Client Warning] %s", fmt);

	va_start(vargs, fmt);
	Console_VMsg(fmtBuffer, vargs);
	va_end(vargs);
}

Client::Client() : _server(nullptr)
{
	_packetMem.SetSize(SERVER_MAX_PACKET_SIZE);
}

Client::~Client()
{
	_socket.Close();
}

bool Client::Connect(const Address& serverAddr)
{
	Disconnect();
	return _socket.Open(7000);
}

bool Client::ConnectLocal(Server* server)
{
	Disconnect();
	_server = server;
	return true;
}

void Client::Disconnect()
{
	_server = nullptr;
	_socket.Close();
}

bool Client::IsConnected() const
{
	return _server || _socket.IsOpen();
}

void Client::Send(const byte* data, int sz)
{
	if (_socket.IsOpen())
	{
		_socket.SendTo(_serverAddr, _packetMem.begin(), sz);
	}

	if (_server) //Listen client
	{
		// todo _server->ReceiveLocalPacket(pkt);
	}
}

void Client::Send(const CLNetPacket& pkt)
{
	if (!IsConnected())
	{
		ClientWarning("Cannot send packet when disconnected\n");
		return;
	}

	const int pktSize = pkt.Write(_packetMem.begin(), _packetMem.GetSize());

	if (_socket.IsOpen())
	{
		_socket.SendTo(_serverAddr, _packetMem.begin(), pktSize);
	}

	if (_server) //Listen client
	{
		_server->ReceiveLocalPacket(pkt);
	}
}

bool Client::Receive(SVNetPacket& pkt)
{
	if (!IsConnected())
		return false;

	if (_socket.IsOpen())
	{
		Address from;
		size_t sz;
		while (sz = _socket.ReceiveFrom(from, _packetMem.begin(), _packetMem.GetSize()))
		{
			if (from == _serverAddr)
			{
				return pkt.Read(_packetMem.begin(), sz);
			}
			else
			{
				ClientWarning("Received data from an unexpected address '%s'\n", from.GetAddr4String());
			}
		}
	}

	if (_server) //Listen client
	{
		//todo



	}

	return false;
}

ExclusiveHandle<CLNetPacket> Client::GetWritablePacket()
{
	_clPacketMutex.Lock();
	return ExclusiveHandle(_clPacket, _clPacketMutex);
}
