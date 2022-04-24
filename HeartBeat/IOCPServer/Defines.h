#pragma once

#include <WinSock2.h>

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