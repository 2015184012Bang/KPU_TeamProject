#include "ServerPCH.h"
#include "Server.h"



Server::Server()
	: mCS()
{

}

bool Server::Init()
{
	HB_LOG("SERVER INIT");

	SocketUtil::Init();
	InitializeCriticalSection(&mCS);

	waitPlayers();

	return true;
}

void Server::Shutdown()
{
	HB_LOG("SERVER SHUTDOWN");

	DeleteCriticalSection(&mCS);
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
		if (!mPackets.empty())
		{
			EnterCriticalSection(&mCS);
			MemoryStream packet = mPackets.front();
			mPackets.pop_front();
			LeaveCriticalSection(&mCS);

			uint16 totalDataLength = packet.GetLength();
			packet.SetLength(0);

			Vector3 data; 
			packet.ReadVector3(&data);

			HB_LOG("Data Received: {0} {1} {2}", data.x, data.y, data.z);
		}
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

		EnterCriticalSection(&mCS);
		mPackets.push_back(buffer);
		LeaveCriticalSection(&mCS);

		buffer.Reset();
	}

	auto iter = std::find(mClientSockets.begin(), mClientSockets.end(), clientSocket);
	if (iter != mClientSockets.end())
	{
		mClientSockets.erase(iter);
	}
}
