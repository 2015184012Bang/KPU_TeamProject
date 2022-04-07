#include "ServerPCH.h"
#include "Server.h"

#include "HeartBeat/PacketType.h"

Server::Server()
	: mListenSocket(nullptr)
	, mbFullUsers(false)
	, mNumCurUsers(0)
{
	mUserReadied.fill(false);
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
		if (!mbFullUsers)
		{
			if (mNumCurUsers < NUM_MAX_PLAYER)
			{
				acceptClients();
			}
		}

		MemoryStream packet;

		for (auto& s : mSessions)
		{
			packet.Reset();

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
					s.bConnect = false;
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

void Server::acceptClients()
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
			mbFullUsers = true;
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
		uint8 packetType = 0;

		outPacket->ReadUByte(&packetType);

		switch (static_cast<CSPacket>(packetType))
		{
		case CSPacket::eLoginRequest:
			processLoginRequest(outPacket, session);
			break;

		case CSPacket::eImReady:
			processImReady(outPacket, session);
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
	MemoryStream spacket;
	int clientID = mNumCurUsers - 1;

	mIdToNickname[clientID] = nickname;

	spacket.WriteUByte(static_cast<int>(SCPacket::eLoginConfirmed));
	spacket.WriteInt(clientID);
	spacket.WriteInt(nameLen);
	spacket.WriteString(nickname);
	(session.ClientSocket)->Send(&spacket, sizeof(MemoryStream));

	spacket.Reset();
	// Send UserConnected packet to the others
	for (auto& s : mSessions)
	{
		spacket.WriteUByte(static_cast<int>(SCPacket::eUserConnected));

		int clientID = s.ClientID;
		const string& name = mIdToNickname[clientID];

		spacket.WriteInt(clientID);
		spacket.WriteInt(static_cast<int>(name.size()));
		spacket.WriteString(name);
	}

	auto numReadied = std::count(mUserReadied.begin(), mUserReadied.end(), true);
	spacket.WriteUByte(static_cast<int>(SCPacket::eReadyPressed));
	spacket.WriteInt(static_cast<int>(numReadied));

	for (auto& s : mSessions)
	{
		(s.ClientSocket)->Send(&spacket, sizeof(MemoryStream));
	}
}

void Server::processImReady(MemoryStream* outPacket, const Session& session)
{
	int clientID = -1;
	outPacket->ReadInt(&clientID);

	if (mUserReadied[clientID])
	{
		return;
	}
	
	mUserReadied[clientID] = true;
	auto numReadied = std::count(mUserReadied.begin(), mUserReadied.end(), true);

	MemoryStream spacket;
	if (numReadied == mUserReadied.size())
	{
		// Send GameStart packet if all players pressed ready button
		spacket.WriteUByte(static_cast<int>(SCPacket::eGameStart));
	}
	else
	{
		spacket.WriteUByte(static_cast<int>(SCPacket::eReadyPressed));
		spacket.WriteInt(static_cast<int>(numReadied));
	}

	for (auto& s : mSessions)
	{
		(s.ClientSocket)->Send(&spacket, sizeof(MemoryStream));
	}
}
