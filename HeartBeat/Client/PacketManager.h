#pragma once

#include <queue>

#include "../IOCPServer/Protocol.h"

using std::queue;

constexpr UINT32 PACKET_BUFFER_SIZE = 1024;

class PacketManager
{
public:
	~PacketManager();

	void Init();

	void Shutdown();

	bool Connect(string_view ip, const UINT16 port);

	void Recv();

	bool GetPacket(PACKET& packet);

	void SetNonblocking(bool opt);

	void Send(char* packet, UINT32 dataSize);

private:
	SOCKET mSocket = INVALID_SOCKET;

	char mRecvBuffer[PACKET_BUFFER_SIZE * 2];
	UINT32 mWritePos = 0;
	UINT32 mReadPos = 0;

	queue<PACKET> mPackets;

	bool mbConnected = false;
	bool mbShutdown = false;
};

