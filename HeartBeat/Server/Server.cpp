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

	MemoryStream buffer;

	while (true)
	{
		int retVal = clientSocket->Recv(&buffer, sizeof(buffer));

		if (retVal == 0)
		{
			HB_LOG("Client num({0}) disconnected.", clientNum);
			break;
		}
		else if (retVal == SOCKET_ERROR)
		{
			HB_LOG("Client num({0}) error");
			break;
		}

		uint16 totalDataLength = buffer.GetLength();
		buffer.SetLength(0);
		uint32 foo = 0;
		buffer.ReadUInt(&foo);
		HB_LOG("Buffer read: {0}", foo);
		HB_LOG("Buffer length: {0}", buffer.GetLength());
	}
}
