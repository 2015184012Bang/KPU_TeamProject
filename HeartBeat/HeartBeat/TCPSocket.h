#pragma once

#include "SocketAddress.h"

using TCPSocketPtr = shared_ptr<class TCPSocket>;

class TCPSocket
{
	friend class SocketUtil;

public:
	~TCPSocket();

	int Bind(const SocketAddress& addr);
	int Listen(int backlog = SOMAXCONN);
	int Connect(const SocketAddress& addr);
	TCPSocketPtr Accept(SocketAddress* outAddr);
	int Send(const void* data, int len, int flags = 0);
	int Recv(void* outData, int len, int flags = 0);

private:
	TCPSocket(SOCKET sock)
		: mSocket(sock) {}

private:
	SOCKET mSocket;
};