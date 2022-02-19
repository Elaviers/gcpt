#pragma once
#include "defs.h"
#include "string.h"

enum class ENetMsg : uint16
{
	NONE = 0,
	ACK,
	HEARTBEAT,
	HELLO,
	GOODBYE,
	PING,
	PONG,

	CONFIG,
	
	SPEAKTEXT,
	SPEAKVOICE,
	MOVE,

	SERVERINFO,

	_COUNT
};

extern const char* g_netMsgNames[(int)ENetMsg::_COUNT + 1];

#define _NETPKT_FUNCS \
	bool Read(const byte* data, int size) noexcept; \
	bool Write(byte*& data, int size) const noexcept;

struct NetPkt_Ack
{
	_NETPKT_FUNCS;

	uint16 m_ackMsg;
	uint32 m_ackID;
};

struct CLNetPkt_Hello
{
	_NETPKT_FUNCS;

	int m_clientCount;
	Buffer<String> m_clientNames;
};

struct SVNetPkt_Hello
{
	_NETPKT_FUNCS;

	int m_clientId;
};

struct CLNetPkt_GoodBye
{
	_NETPKT_FUNCS;

	String m_reason;
};

struct SVNetPkt_GoodBye
{
	_NETPKT_FUNCS;

	String m_reason;
};

struct NetPkt_Pong
{
	_NETPKT_FUNCS;

	uint64 m_timeSent; //msecs, unix epoch
};

struct CLNetPkt_Config
{
	_NETPKT_FUNCS;

};

struct SVNetPkt_Config
{
	_NETPKT_FUNCS;

};

struct NetPkt_Move
{
	_NETPKT_FUNCS;


};

struct NetPkt_SpeakText
{
	_NETPKT_FUNCS;

	String m_text;
};


struct NetPkt_SpeakVoice
{
	_NETPKT_FUNCS;

	Buffer<byte> m_voiceData;
};

struct NetPkt_ServerInfo
{
	_NETPKT_FUNCS;

	String m_name;
	String m_gamemode;
	int m_currentPlayerCount;
	int m_maxPlayerCount;

	Buffer<String> m_playerNames;
};

struct CLNetPacket
{
	ENetMsg m_type;
	byte m_players; static_assert(MAX_LOCAL_CLIENTS <= 8); //bitmask of local players affected by message
	byte m_unused;

	union CLNetPacket_DataUnion
	{
		NetPkt_Ack ack;
		CLNetPkt_Hello hello;
		CLNetPkt_GoodBye goodbye;
		NetPkt_Pong pong;
		CLNetPkt_Config config;
		NetPkt_SpeakText speakText;
		NetPkt_SpeakVoice speakVoice;
		NetPkt_Move move;

		CLNetPacket_DataUnion() { memset(this, 0, sizeof(CLNetPacket_DataUnion)); }
		~CLNetPacket_DataUnion() {}
	} m_data;

	CLNetPacket() : m_type(ENetMsg::NONE), m_players(0) {}
	~CLNetPacket();

	bool Read(const byte* data, int size) noexcept;
	int Write(byte* data, int size) const noexcept;
};

struct SVNetPacket
{
	ENetMsg m_type;
	uint16 m_uid;

	union SVNetPacket_DataUnion
	{
		NetPkt_Ack ack;
		SVNetPkt_Hello hello;
		SVNetPkt_GoodBye goodbye;
		NetPkt_Pong pong;
		SVNetPkt_Config config;
		NetPkt_SpeakText speakText;
		NetPkt_SpeakVoice speakVoice;
		NetPkt_Move move;
		NetPkt_ServerInfo serverInfo;

		SVNetPacket_DataUnion() { memset(this, 0, sizeof(SVNetPacket_DataUnion)); }
		~SVNetPacket_DataUnion() {}
	} m_data;

	SVNetPacket() : m_type(ENetMsg::NONE), m_uid(INVALID_USER) {}
	~SVNetPacket();

	bool Read(const byte* data, int size) noexcept;
	int Write(byte* data, int size) const noexcept;
};
