#include "ServerPCH.h"
#include "Server.h"



Server::Server()
{

}

bool Server::Init()
{
	HB_LOG("SERVER INIT");

	SocketUtil::Init();

	waitPlayers();

	return true;
}

void Server::Shutdown()
{
	HB_LOG("SERVER SHUTDOWN");

	SocketUtil::Shutdown();

	for (auto& t : mClientThreads)
	{
		t.join();
	}
}

void Server::Run()
{
	while (true)
	{
		
	}
}

void Server::waitPlayers()
{
	TCPSocketPtr listenSocket = SocketUtil::CreateTCPSocket();
	SocketAddress serveraddr(SERVER_PORT);

	if (listenSocket->Bind(serveraddr) == SOCKET_ERROR)
	{
		HB_ASSERT(false, "ASSERTION FAILED");
	}

	if (listenSocket->Listen() == SOCKET_ERROR)
	{
		HB_ASSERT(false, "ASSERTION FAILED");
	}

	SocketAddress clientAddr;
	int clientNum = 0;

	while (clientNum < MAX_PLAYER_NUM)
	{
		TCPSocketPtr clientSocket = listenSocket->Accept(&clientAddr);
		
		HB_LOG("Client Connected: {0}", clientAddr.ToString());

		mClientThreads.emplace_back(&Server::clientThreadFunc, this, clientSocket, clientNum);
		mClientSockets.push_back(clientSocket);
		++clientNum;
	}
}

void Server::clientThreadFunc(const TCPSocketPtr& clientSocket, int clientNum)
{
	HB_LOG("Client Thread : {0}", clientNum);
}
