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
}

void Server::Run()
{
	while (true)
	{
		MemoryStream buf;

		for (auto& c : mConnections)
		{
			buf.Reset();

			int retVal = (c.ClientSocket)->Recv(&buf, sizeof(MemoryStream));

			if (retVal == SOCKET_ERROR)
			{
				continue;
			}
			else if (retVal == 0)
			{
				HB_LOG("Client[{0}] disconnected.", c.ClientAddr.ToString());
				c.bConnect = false;
			}
			else
			{
				processPacket(&buf);
			}
		}

		mConnections.erase(std::remove_if(mConnections.begin(), mConnections.end(), [](const Session& c) {
			return c.bConnect == false;
			}), mConnections.end());
	}
}

void Server::waitPlayers()
{
	TCPSocketPtr listenSocket = SocketUtil::CreateTCPSocket();
	SocketAddress serveraddr(SERVER_PORT);

	if (listenSocket->Bind(serveraddr) == SOCKET_ERROR)
	{
		SocketUtil::ReportError(L"Server::waitPlayers()");
		HB_ASSERT(false, "");
	}

	if (listenSocket->Listen() == SOCKET_ERROR)
	{
		SocketUtil::ReportError(L"Server::waitPlayers()");
		HB_ASSERT(false, "");
	}

	SocketAddress clientAddr;
	int clientNum = 0;

	while (clientNum < NUM_MAX_PLAYER)
	{
		TCPSocketPtr clientSocket = listenSocket->Accept(&clientAddr);
		clientSocket->SetNonBlockingMode(true);
		
		HB_LOG("Client Connected: {0}", clientAddr.ToString());

		Session c;
		c.bConnect = true;
		c.ClientSocket = clientSocket;
		c.ClientAddr = clientAddr;

		mConnections.push_back(c);
		++clientNum;
	}
}

void Server::processPacket(MemoryStream* outPacket)
{
	uint16 totalLen = outPacket->GetLength();
	outPacket->SetLength(0);

	while (outPacket->GetLength() < totalLen)
	{
		uint64 id = 0;
		outPacket->ReadUInt64(&id);

		HB_LOG("Client sent : {0}", id);
	}
}
