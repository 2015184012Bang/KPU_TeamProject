#include "PCH.h"
#include "SocketUtil.h"

void SocketUtil::Init()
{
	WSADATA wsa;

	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		ReportError("SocketUtil::Init");
		HB_ASSERT(false, "ASSERTION FAILED");
	}
}

void SocketUtil::Shutdown()
{
	WSACleanup();
}

void SocketUtil::ReportError(const char* desc)
{
	LPVOID lpMsgBuf;
	DWORD errorNum = GetLastError();

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		errorNum,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, NULL);


	HB_LOG("Error {0}: {1} - {2}", desc, errorNum, lpMsgBuf);
}

int SocketUtil::GetLastError()
{
	return WSAGetLastError();
}

TCPSocketPtr SocketUtil::CreateTCPSocket()
{
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);

	if (sock == INVALID_SOCKET)
	{
		ReportError("SocketUtil::CreateTCPSocket");
		return nullptr;
	}
	else
	{
		return TCPSocketPtr(new TCPSocket(sock));
	}
}
