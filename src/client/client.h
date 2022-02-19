#pragma once
#include "shared/mutex.h"
#include "shared/net_socket_udp.h"
#include "shared/net_address.h"
#include "shared/net_packet.h"

template <typename T>
class ExclusiveHandle : public MutexHolderNoLock
{
	T& _obj;

public:
	ExclusiveHandle(T& obj, Mutex& mutex) : MutexHolderNoLock(mutex), _obj(obj) {}
	ExclusiveHandle(const ExclusiveHandle&) = delete;
	ExclusiveHandle(ExclusiveHandle&& other) : MutexHolderNoLock(std::move(other)), _obj(other->_obj) {}

	constexpr T* operator->() const noexcept { return &_obj; }
	constexpr T& operator*() const noexcept { return _obj; }
};

class Client
{
private:
	class Server* _server;
	
	UDPSocket _socket;
	Address _serverAddr;

	Buffer<byte> _packetMem;

	CLNetPacket _clPacket;
	Mutex _clPacketMutex;

public:
	Client();
	~Client();

	bool Connect(const Address& serverAddr);
	bool ConnectLocal(class Server*); //listen server
	void Disconnect();
	bool IsConnected() const;
	void Send(const byte* data, int sz);
	void Send(const CLNetPacket& pkt);
	bool Receive(SVNetPacket& pkt);

	ExclusiveHandle<CLNetPacket> GetWritablePacket();
};
