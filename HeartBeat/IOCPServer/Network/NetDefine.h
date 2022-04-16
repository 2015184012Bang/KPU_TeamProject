#pragma once

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <MSWSock.h>

#pragma comment(lib, "ws2_32")
#pragma comment(lib, "Mswsock")

enum class IOOperation
{
	NONE,
	ACCEPT,
	RECV,
	SEND
};

struct OVERLAPPEDEX
{
	WSAOVERLAPPED Over = {};
	WSABUF WsaBuf = {};
	IOOperation Operation = IOOperation::NONE;
	INT32 SessionIndex = -1;
};