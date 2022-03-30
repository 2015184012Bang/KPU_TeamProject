#include "ServerPCH.h"
#include "Server.h"

#include "HeartBeat/PacketType.h"

Server::Server()
	: mListenSocket(nullptr)
	, mbGameStart(false)
	, mNumCurUsers(0)
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
	for (auto& s : mSessions)
	{
		s.ClientSocket = nullptr;
	}
	
	SocketUtil::Shutdown();
}

void Server::Run()
{
	while (!ShouldClose())
	{
		if (!mbGameStart)
		{
			if (mNumCurUsers < NUM_MAX_PLAYER)
			{
				accpetClients();
			}
		}

		MemoryStream packet;

		for (auto& s : mSessions)
		{
			packet.Reset();

			if (s.ClientSocket == nullptr)
			{
				HB_LOG("nullptr");
			}

			int retVal = (s.ClientSocket)->Recv(&packet, sizeof(MemoryStream));

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
				processPacket(&packet, s);
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
		HB_LOG("Client[{0}] Connected: {1}", mNumCurUsers, clientAddr.ToString());
		mSessions.emplace_back(true, clientSocket, clientAddr, mNumCurUsers++);

		if (mNumCurUsers == NUM_MAX_PLAYER)
		{
			mbGameStart = true;
			mListenSocket = nullptr;
		}
	}
}

void Server::processPacket(MemoryStream* outPacket, const Session& session)
{
	uint16 totalLen = outPacket->GetLength();
	outPacket->SetLength(0);

	while (outPacket->GetLength() < totalLen)
	{
		int packetType = 0;

		outPacket->ReadInt(&packetType);

		switch (static_cast<CSPacket>(packetType))
		{
		case CSPacket::eLoginRequest:
			processLoginRequest(outPacket, session);
			break;

		default:
			break;
		}
	}
}

void Server::processLoginRequest(MemoryStream* outPacket, const Session& session)
{
	int nameLen = 0;
	outPacket->ReadInt(&nameLen);

	string nickname;
	outPacket->ReadString(&nickname, nameLen);

	// Send LoginConfirmed packet to newly connected client
	MemoryStream packet;
	int clientID = mNumCurUsers - 1;

	mIdToNickname[clientID] = nickname;

	packet.WriteInt(static_cast<int>(SCPacket::eLoginConfirmed));
	packet.WriteInt(clientID);
	packet.WriteInt(nameLen);
	packet.WriteString(nickname);
	(session.ClientSocket)->Send(&packet, sizeof(MemoryStream));

	packet.Reset();
	// Send UserConnected packet to the others
	for (auto& s : mSessions)
	{
		packet.WriteInt(static_cast<int>(SCPacket::eUserConnected));

		int clientID = s.ClientID;
		const string& name = mIdToNickname[clientID];

		packet.WriteInt(clientID);
		packet.WriteInt(name.size());
		packet.WriteString(name);
	}

	for (auto& s : mSessions)
	{
		(s.ClientSocket)->Send(&packet, sizeof(MemoryStream));
	}
}
