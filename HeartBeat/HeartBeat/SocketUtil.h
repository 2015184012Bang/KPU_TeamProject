#pragma once

#include "TCPSocket.h"

class SocketUtil
{
public:
	static void Init();
	static void Shutdown();
	static void ReportError(const wstring& desc);
	static TCPSocketPtr CreateTCPSocket();
};