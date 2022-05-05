#include "PCH.h"
#include "TCPSocket.h"

#include "SocketUtil.h"

TCPSocket::~TCPSocket()
{
	closesocket(mSocket);
}

int TCPSocket::Bind(const SocketAddress& addr)
{
	int error = bind(mSocket, &addr.mSockAddr, addr.GetSize());
	return error;
}

int TCPSocket::Listen(int backlog /*= SOMAXCONN*/)
{
	int error = listen(mSocket, backlog);
	return error;
}

int TCPSocket::Connect(const SocketAddress& addr)
{
	int error = connect(mSocket, &addr.mSockAddr, addr.GetSize());
	return error;
}

TCPSocketPtr TCPSocket::Accept(SocketAddress* outAddr)
{
	int addrlen = outAddr->GetSize();

	SOCKET newSock = accept(mSocket, &outAddr->mSockAddr, &addrlen);

	if (newSock == INVALID_SOCKET)
	{
		return nullptr;
	}

	return TCPSocketPtr(new TCPSocket(newSock));
}

int TCPSocket::Send(const void* data, int len, int flags /*= 0*/)
{
	int sent = send(mSocket, static_cast<const char*>(data), len, flags);
	return sent;
}

int TCPSocket::Recv(void* outData, int len, int flags /*= 0*/)
{
	int read = recv(mSocket, static_cast<char*>(outData), len, flags);
	return read;
}

void TCPSocket::SetNonBlockingMode(bool value)
{
	u_long mode = value ? 1 : 0;
	ioctlsocket(mSocket, FIONBIO, &mode);
}

void TCPSocket::SetReuseAddress(bool value)
{
	setsockopt(mSocket, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char*>(&value), sizeof(value));
}
