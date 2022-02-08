#include "ServerPCH.h"
#include "Server.h"



Server::Server()
{

}

bool Server::Init()
{
	HB_LOG("SERVER INIT");

	SocketUtil::Init();

	TCPSocketPtr listenSock = SocketUtil::CreateTCPSocket();
	SocketAddress serveraddr(9000);

	if (listenSock->Bind(serveraddr) == SOCKET_ERROR)
	{
		HB_ASSERT(false, "ASSERTION FAILED");
	}

	if (listenSock->Listen() == SOCKET_ERROR)
	{
		HB_ASSERT(false, "ASSERTION FAILED");
	}

	SocketAddress clientaddr;
	int clientNum = 0;

	while (clientNum != 1)
	{
		TCPSocketPtr clientSocket = listenSock->Accept(&clientaddr);
		
		HB_LOG("클라이언트 접속: {0}", clientaddr.ToString());

		clientNum++;
	}

	return true;
}

void Server::Shutdown()
{
	HB_LOG("SERVER SHUTDOWN");

	SocketUtil::Shutdown();
}

void Server::Run()
{
	while (true)
	{
		//HB_LOG("SERVER RUN");
	}
}
