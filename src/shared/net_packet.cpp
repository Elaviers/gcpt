#include "net_packet.h"
#include "assert.h"
#include <WinSock2.h>

const char* g_netMsgNames[(int)ENetMsg::_COUNT + 1] =
{
	"none",
	"ack",
	"heartbeat",
	"hello",
	"goodbye",
	"ping",
	"pong",
	"config",
	"speaktext",
	"speakvoice",
	"move",
	
	"serverinfo",

	"invalid"
};

byte ReadNetU8(const byte*& data) noexcept
{
	const byte val = *data; 
	data++;
	return val;
}

void WriteNetU8(byte*& data, byte val) noexcept
{
	*data = val;
	++data;
}

uint16 ReadNetU16(const byte*& data) noexcept
{
	const uint16 val = ntohs(*(uint16*)data);
	data += 2;
	return val;
}

void WriteNetU16(byte*& data, uint16 val) noexcept
{
	const uint16 netVal = htons(val);
	const byte* netBytes = (const byte*)&netVal;
	data[0] = netBytes[0];
	data[1] = netBytes[1];
	data += 2;
}

uint32 ReadNetU32(const byte*& data) noexcept
{
	const uint32 val = ntohs(*(uint32*)data);
	data += 4;
	return val;
}

void WriteNetU32(byte*& data, uint32 val) noexcept
{
	const uint32 netVal = htonl(val);
	const byte* netBytes = (const byte*)&netVal;
	data[0] = netBytes[0];
	data[1] = netBytes[1];
	data[2] = netBytes[2];
	data[3] = netBytes[3];
	data += 4;
}

uint64 ReadNetU64(const byte*& data) noexcept
{
	uint64 val = ntohll(*(uint64*)data);
	data += 8;
	return val;
}

void WriteNetU64(byte*& data, uint64 val) noexcept
{
	const uint64 netVal = htonll(val);
	const byte* netBytes = (const byte*)&netVal;
	data[0] = netBytes[0];
	data[1] = netBytes[1];
	data[2] = netBytes[2];
	data[3] = netBytes[3];
	data[4] = netBytes[4];
	data[5] = netBytes[5];
	data[6] = netBytes[6];
	data[7] = netBytes[7];
	data += 8;
}

float ReadNetFloat(const byte*& data) noexcept
{
	const float val = ntohf(*(unsigned int*)data);
	data += 4;
	return val;
}

void WriteNetFloat(byte*& data, float val) noexcept
{
	const float netVal = htonf(val);
	const byte* netBytes = (const byte*)&netVal;
	data[0] = netBytes[0];
	data[1] = netBytes[1];
	data[2] = netBytes[2];
	data[3] = netBytes[3];
	data += 4;
}

int ReadNetString(const byte*& data, char* buffer, int sz)
{
	const byte* const start = data;
	const byte* const end = data + sz;
	while (*data && data != end)
		++data;

	const int outSz = data - start;
	buffer[outSz - 1] = '\0';
	return outSz;
}

String ReadNetString(const byte*& data) noexcept
{
	constexpr const int NET_MAX_STRING_LENGTH = 1024; //Max reasonable string length, don't wanna get stuck on anything malicious
	const byte* const start = data;
	const byte* const end = data + NET_MAX_STRING_LENGTH;
	while (*data && data != end)
		++data;

	return String((const char*)start, data - start);
}

void WriteNetString(byte*& data, const char* string)
{
	while ( *data )
		*data++ = *string++;
}

#define NETPKT_READ_FUNC(CLASS) bool CLASS::Read(const byte* data, int size) noexcept
#define NETPKT_WRITE_FUNC(CLASS) bool CLASS::Write(byte*& data, int size) const noexcept

NETPKT_READ_FUNC(NetPkt_Ack)
{
	if (size < 6)
		return false;

	m_ackMsg = ReadNetU16(data);
	m_ackID = ReadNetU32(data);
	return true;
}

NETPKT_WRITE_FUNC(NetPkt_Ack)
{
	if (size < 6)
		return false;

	WriteNetU16(data, m_ackMsg);
	WriteNetU32(data, m_ackID);
	return true;
}

NETPKT_READ_FUNC(CLNetPkt_Hello)
{
	if (size < 1)
		return false;

	m_clientCount = ReadNetU8(data);
	m_clientNames.SetSize(m_clientCount);
	for (int i = 0; i < m_clientCount; ++i)
		m_clientNames[i] = ReadNetString(data);

	return true;
}

NETPKT_WRITE_FUNC(CLNetPkt_Hello)
{
	WriteNetU8(data, m_clientCount);

	AssertI(m_clientCount, ==, m_clientNames.GetSize());
	for (int i = 0; i < m_clientCount; ++i)
		WriteNetString(data, m_clientNames[i].begin());

	return true;
}

NETPKT_READ_FUNC(SVNetPkt_Hello)
{
	if (size < 2)
		return false;

	m_clientId = ReadNetU16(data);
	return true;
}

NETPKT_WRITE_FUNC(SVNetPkt_Hello)
{
	WriteNetU16(data, m_clientId);

	return true;
}

NETPKT_READ_FUNC(CLNetPkt_GoodBye)
{
	if (size < 2)
		return true;

	const uint16 length = ReadNetU16(data);
	m_reason = String((const char*)data);
}

NETPKT_WRITE_FUNC(CLNetPkt_GoodBye)
{
	if (size < 2 + m_reason.GetLength() || m_reason.GetLength() > 0xFFFF)
		return false;

	WriteNetU16(data, m_reason.GetLength());
	for (char c : m_reason)
		WriteNetU8(data, c);

	return true;
}

NETPKT_READ_FUNC(SVNetPkt_GoodBye)
{
	if (size < 2)
		return true;

	const uint16 length = ReadNetU16(data);
	m_reason = String((const char*)data);
}

NETPKT_WRITE_FUNC(SVNetPkt_GoodBye)
{
	if (size < 2 + m_reason.GetLength() || m_reason.GetLength() > 0xFFFF)
		return false;

	WriteNetU16(data, m_reason.GetLength());
	for (char c : m_reason)
		WriteNetU8(data, c);

	return true;
}

NETPKT_READ_FUNC(NetPkt_Pong)
{
	if (size < 8)
	{
		m_timeSent = 0;
		return true;
	}
	
	m_timeSent = ReadNetU64(data);
	return true;
}

NETPKT_WRITE_FUNC(NetPkt_Pong)
{
	if (m_timeSent)
		WriteNetU64(data, m_timeSent);

	return true;
}

NETPKT_READ_FUNC(CLNetPkt_Config)
{
	return true;
}

NETPKT_WRITE_FUNC(CLNetPkt_Config)
{
	return true;
}

NETPKT_READ_FUNC(SVNetPkt_Config)
{
	return true;
}

NETPKT_WRITE_FUNC(SVNetPkt_Config)
{
	return true;
}

NETPKT_READ_FUNC(NetPkt_SpeakText)
{
	if (size < 2)
		return true;

	const uint16 length = ReadNetU16(data);
	m_text = String((const char*)data);
}

NETPKT_WRITE_FUNC(NetPkt_SpeakText)
{
	if (size < 2 + m_text.GetLength() || m_text.GetLength() > 0xFFFF)
		return false;

	WriteNetU16(data, m_text.GetLength());
	for (char c : m_text)
		WriteNetU8(data, c);

	return true;
}

NETPKT_READ_FUNC(NetPkt_SpeakVoice)
{
	return true;
}

NETPKT_WRITE_FUNC(NetPkt_SpeakVoice)
{
	return true;
}

NETPKT_READ_FUNC(NetPkt_Move)
{
	return true;
}

NETPKT_WRITE_FUNC(NetPkt_Move)
{
	return true;
}

constexpr const int minValidSvInfoSz = 1 + 1 + 4 + 4;

NETPKT_READ_FUNC(NetPkt_ServerInfo)
{
	if (size < minValidSvInfoSz)
		return false;

	m_name = ReadNetString(data);
	m_gamemode = ReadNetString(data);
	m_currentPlayerCount = ReadNetU16(data);
	m_maxPlayerCount = ReadNetU16(data);

	m_playerNames.SetSize(m_currentPlayerCount);
	for (int i = 0; i < m_currentPlayerCount; ++i)
		m_playerNames[i] = ReadNetString(data);

	return true;
}

NETPKT_WRITE_FUNC(NetPkt_ServerInfo)
{
	WriteNetString(data, m_name.begin());
	WriteNetString(data, m_gamemode.begin());
	WriteNetU16(data, m_currentPlayerCount);
	WriteNetU16(data, m_maxPlayerCount);

	for (int i = 0; i < m_currentPlayerCount; ++i)
		WriteNetString(data, m_playerNames[i].begin());

	return true;
}

template <typename T>
bool PlaceNewAndRead(void* placeAt, const byte* data, int size) noexcept
{
	T* pkt = new (placeAt) T();
	return pkt->Read(data, size);
}

bool CLNetPacket::Read(const byte* data, int size) noexcept
{
	if (size < 4) return false;

	m_type = (ENetMsg)ReadNetU16(data);
	m_unused = ReadNetU16(data);
	
	switch (m_type)
	{
	case ENetMsg::ACK: return PlaceNewAndRead<NetPkt_Ack>(&m_data, data, size);
	case ENetMsg::HELLO: return PlaceNewAndRead<CLNetPkt_Hello>(&m_data, data, size);
	case ENetMsg::GOODBYE: return PlaceNewAndRead<CLNetPkt_GoodBye>(&m_data, data, size);
	case ENetMsg::PONG: return PlaceNewAndRead<NetPkt_Pong>(&m_data, data, size);
	case ENetMsg::CONFIG: return PlaceNewAndRead<CLNetPkt_Config>(&m_data, data, size);
	case ENetMsg::SPEAKTEXT: return PlaceNewAndRead<NetPkt_SpeakText>(&m_data, data, size);
	case ENetMsg::SPEAKVOICE: return PlaceNewAndRead<NetPkt_SpeakVoice>(&m_data, data, size);
	case ENetMsg::MOVE: return PlaceNewAndRead<NetPkt_Move>(&m_data, data, size);
	}

	return false;
}

int CLNetPacket::Write(byte* data, int size) const noexcept
{
	if (size < 4) return false;
	const byte* const start = data;

	WriteNetU16(data, (uint16)m_type);
	WriteNetU32(data, m_unused);
	
	switch (m_type)
	{
	case ENetMsg::ACK: return m_data.ack.Write(data, size);
	case ENetMsg::HELLO: return m_data.hello.Write(data, size);
	case ENetMsg::GOODBYE: return m_data.goodbye.Write(data, size);
	case ENetMsg::PONG: return m_data.pong.Write(data, size);
	case ENetMsg::CONFIG: return m_data.config.Write(data, size);
	case ENetMsg::SPEAKTEXT: return m_data.speakText.Write(data, size);
	case ENetMsg::SPEAKVOICE: return m_data.speakVoice.Write(data, size);
	case ENetMsg::MOVE: return m_data.move.Write(data, size);
	}

	return data - start;
}

CLNetPacket::~CLNetPacket()
{
	switch (m_type)
	{
	case ENetMsg::ACK: m_data.ack.~NetPkt_Ack(); break;
	case ENetMsg::HELLO: m_data.hello.~CLNetPkt_Hello(); break;
	case ENetMsg::GOODBYE: m_data.goodbye.~CLNetPkt_GoodBye(); break;
	case ENetMsg::PONG: m_data.pong.~NetPkt_Pong(); break;
	case ENetMsg::CONFIG: m_data.config.~CLNetPkt_Config(); break;
	case ENetMsg::SPEAKTEXT: m_data.speakText.~NetPkt_SpeakText(); break;
	case ENetMsg::SPEAKVOICE: m_data.speakVoice.~NetPkt_SpeakVoice(); break;
	case ENetMsg::MOVE: m_data.move.~NetPkt_Move(); break;
	}
}

bool SVNetPacket::Read(const byte* data, int size) noexcept
{
	if (size < 4) return false;

	m_type = (ENetMsg)ReadNetU16(data);
	m_uid = ReadNetU16(data);

	switch (m_type)
	{
	case ENetMsg::ACK: return PlaceNewAndRead<NetPkt_Ack>(&m_data, data, size);
	case ENetMsg::HELLO: return PlaceNewAndRead<SVNetPkt_Hello>(&m_data, data, size);
	case ENetMsg::GOODBYE: return PlaceNewAndRead<SVNetPkt_GoodBye>(&m_data, data, size);
	case ENetMsg::PONG: return PlaceNewAndRead<NetPkt_Pong>(&m_data, data, size);
	case ENetMsg::CONFIG: return PlaceNewAndRead<SVNetPkt_Config>(&m_data, data, size);
	case ENetMsg::SPEAKTEXT: return PlaceNewAndRead<NetPkt_SpeakText>(&m_data, data, size);
	case ENetMsg::SPEAKVOICE: return PlaceNewAndRead<NetPkt_SpeakVoice>(&m_data, data, size);
	case ENetMsg::MOVE: return PlaceNewAndRead<NetPkt_Move>(&m_data, data, size);
	case ENetMsg::SERVERINFO: return PlaceNewAndRead<NetPkt_ServerInfo>(&m_data, data, size);
	}

	return false;
}

int SVNetPacket::Write(byte* data, int size) const noexcept
{
	if (size < 4) return false;
	const byte* const start = data;

	WriteNetU16(data, (uint16)m_type);
	WriteNetU32(data, m_uid);

	switch (m_type)
	{
	case ENetMsg::ACK: return m_data.ack.Write(data, size);
	case ENetMsg::HELLO: return m_data.hello.Write(data, size);
	case ENetMsg::GOODBYE: return m_data.goodbye.Write(data, size);
	case ENetMsg::PONG: return m_data.pong.Write(data, size);
	case ENetMsg::CONFIG: return m_data.config.Write(data, size);
	case ENetMsg::SPEAKTEXT: return m_data.speakText.Write(data, size);
	case ENetMsg::SPEAKVOICE: return m_data.speakVoice.Write(data, size);
	case ENetMsg::MOVE: return m_data.move.Write(data, size);
	case ENetMsg::SERVERINFO: return m_data.serverInfo.Write(data, size);
	}

	return data - start;
}

SVNetPacket::~SVNetPacket()
{
	switch (m_type)
	{
	case ENetMsg::ACK: m_data.ack.~NetPkt_Ack(); break;
	case ENetMsg::HELLO: m_data.hello.~SVNetPkt_Hello(); break;
	case ENetMsg::GOODBYE: m_data.goodbye.~SVNetPkt_GoodBye(); break;
	case ENetMsg::PONG: m_data.pong.~NetPkt_Pong(); break;
	case ENetMsg::CONFIG: m_data.config.~SVNetPkt_Config(); break;
	case ENetMsg::SPEAKTEXT: m_data.speakText.~NetPkt_SpeakText(); break;
	case ENetMsg::SPEAKVOICE: m_data.speakVoice.~NetPkt_SpeakVoice(); break;
	case ENetMsg::MOVE: m_data.move.~NetPkt_Move(); break;
	case ENetMsg::SERVERINFO: m_data.serverInfo.~NetPkt_ServerInfo(); break;
	}
}
