#include "PCH.h"
#include "SocketUtil.h"

#include "StringUtils.h"

void SocketUtil::Init()
{
	WSADATA wsa;

	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		ReportError(L"SocketUtil::Init");
		HB_ASSERT(false, "ASSERTION FAILED");
	}
}

void SocketUtil::Shutdown()
{
	WSACleanup();
}

void SocketUtil::ReportError(const wstring& desc)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	wstring msg((wchar_t*)lpMsgBuf);
	HB_LOG("Error {0}: {1}", ws2s(desc), ws2s(msg));
	LocalFree(lpMsgBuf);
}

void SocketUtil::ReportError(const wstring& desc, int err)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, err,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	wstring msg((wchar_t*)lpMsgBuf);
	HB_LOG("Error {0}: {1}", ws2s(desc), ws2s(msg));
	LocalFree(lpMsgBuf);
}

TCPSocketPtr SocketUtil::CreateTCPSocket()
{
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);

	if (sock == INVALID_SOCKET)
	{
		ReportError(L"SocketUtil::CreateTCPSocket");
		return nullptr;
	}
	else
	{
		return TCPSocketPtr(new TCPSocket(sock));
	}
}
