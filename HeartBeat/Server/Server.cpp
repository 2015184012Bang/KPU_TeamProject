#include "ServerPCH.h"
#include "Server.h"



Server::Server()
	: mListenSocket(nullptr)
	, mbGameStart(false)
{

}

bool Server::Init()
{
	HB_LOG("SERVER INIT");

	SocketUtil::Init();

	mListenSocket = SocketUtil::CreateTCPSocket();
	SocketAddress serveraddr(SERVER_PORT);

	if (mListenSocket->Bind(serveraddr) == SOCKET_ERROR)
	{
		SocketUtil::ReportError(L"Server::Init");
		return false;
	}

	if (mListenSocket->Listen() == SOCKET_ERROR)
	{
		SocketUtil::ReportError(L"Server::Init");
		return false;
	}

	mListenSocket->SetNonBlockingMode(true);

	return true;
}

void Server::Shutdown()
{
	SocketUtil::Shutdown();
}

void Server::Run()
{
	while (!ShouldClose())
	{
		if (!mbGameStart)
		{
			auto numCurUsers = mSessions.size();
			if (numCurUsers < NUM_MAX_PLAYER)
			{
				accpetClients();
			}
		}

		MemoryStream buf;

		for (auto& s : mSessions)
		{
			buf.Reset();

			int retVal = (s.ClientSocket)->Recv(&buf, sizeof(MemoryStream));

			if (retVal == SOCKET_ERROR)
			{
				int error = WSAGetLastError();

				if (error == WSAEWOULDBLOCK)
				{
					continue;
				}
				else
				{
					SocketUtil::ReportError(L"Server::Run");
				}
			}
			else if (retVal == 0)
			{
				HB_LOG("Client[{0}] disconnected.", s.ClientAddr.ToString());
				s.bConnect = false;
			}
			else
			{
				processPacket(&buf);
			}
		}

		mSessions.erase(std::remove_if(mSessions.begin(), mSessions.end(), [](const Session& s) {
			return s.bConnect == false;
			}), mSessions.end());
	}
}

void Server::accpetClients()
{
	SocketAddress clientAddr;

	TCPSocketPtr clientSocket = mListenSocket->Accept(&clientAddr);

	if (!clientSocket)
	{
		int error = WSAGetLastError();

		if (error != WSAEWOULDBLOCK)
		{
			SocketUtil::ReportError(L"Server::waitPlayers", error);
		}
	}
	else
	{
		clientSocket->SetNonBlockingMode(true);
		HB_LOG("Client Connected: {0}", clientAddr.ToString());
		mSessions.emplace_back(true, clientSocket, clientAddr);

		auto numCurUsers = mSessions.size();
		if (numCurUsers == NUM_MAX_PLAYER)
		{
			mbGameStart = true;
		}
	}
}

void Server::processPacket(MemoryStream* outPacket)
{
	
}
