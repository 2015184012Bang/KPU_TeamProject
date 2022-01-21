#pragma once

class SocketUtil
{
public:
	
	static bool StaticInit();
	static void CleanUp();

	static void ReportError(const char* InOperationDesc);
	static int GetLastError();

	static TCPSocketPtr CreateTCPSocket();

};