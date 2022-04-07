#include "ServerPCH.h"
#include "Server.h"

#include "HeartBeat/PacketType.h"

#include "ServerComponents.h"
#include "ServerSystems.h"

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

		// 접속이 끊긴 클라이언트 정리
		mSessions.erase(std::remove_if(mSessions.begin(), mSessions.end(), [](const Session& s) {
			return s.bConnect == false;
			}), mSessions.end());

		{
			MemoryStream spacket;

			auto view = GetRegistry().view<Tag_UpdateTransform>();

			for (auto e : view)
			{
				Entity ent = Entity(e, this);
				ent.RemoveComponent<Tag_UpdateTransform>();
				auto& transform = ent.GetComponent<STransformComponent>();
				auto& id = ent.GetComponent<IDComponent>();

				spacket.WriteUByte(static_cast<uint8>(SCPacket::eUpdateTransform));
				spacket.WriteUInt64(id.ID);
				spacket.WriteVector3(transform.Position);
				spacket.WriteFloat(transform.Rotation.y);
			}

			if (spacket.GetLength() > 0)
			{
				for (auto& s : mSessions)
				{
					(s.ClientSocket)->Send(&spacket, sizeof(MemoryStream));
				}
			}
		}
	}
}

Entity Server::CreateEntity()
{
	Entity e = Entity(getNewEntt(), this);
	e.AddComponent<STransformComponent>();
	auto& id = e.AddComponent<IDComponent>();

	RegisterEntity(id.ID, e);

	return e;
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

		case CSPacket::eUserInput:
			processUserInput(outPacket);
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

	spacket.WriteUByte(static_cast<uint8>(SCPacket::eLoginConfirmed));
	spacket.WriteInt(clientID);
	spacket.WriteInt(nameLen);
	spacket.WriteString(nickname);
	(session.ClientSocket)->Send(&spacket, sizeof(MemoryStream));

	spacket.Reset();
	// Send UserConnected packet to the others
	for (auto& s : mSessions)
	{
		spacket.WriteUByte(static_cast<uint8>(SCPacket::eUserConnected));

		int clientID = s.ClientID;
		const string& name = mIdToNickname[clientID];

		spacket.WriteInt(clientID);
		spacket.WriteInt(static_cast<int>(name.size()));
		spacket.WriteString(name);
	}

	auto numReadied = std::count(mUserReadied.begin(), mUserReadied.end(), true);
	spacket.WriteUByte(static_cast<uint8>(SCPacket::eReadyPressed));
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

	// 이미 레디를 누른 클라이언트인가?
	if (mUserReadied[clientID])
	{
		return;
	}
	
	mUserReadied[clientID] = true;
	auto numReadied = std::count(mUserReadied.begin(), mUserReadied.end(), true);

	MemoryStream spacket;

	bool bAllReady = false;
	// 모든 유저가 레디를 눌렀는가?
	if (numReadied == mUserReadied.size())
	{
		// 모든 유저가 레디했다면 게임 시작
		spacket.WriteUByte(static_cast<uint8>(SCPacket::eGameStart));
		bAllReady = true;
	}
	else
	{
		// 그렇지 않다면 레디한 유저 수를 담아 전송
		spacket.WriteUByte(static_cast<uint8>(SCPacket::eReadyPressed));
		spacket.WriteInt(static_cast<int>(numReadied));
	}

	for (auto& s : mSessions)
	{
		(s.ClientSocket)->Send(&spacket, sizeof(MemoryStream));
	}

	if (bAllReady)
	{
		spacket.Reset();
		spacket.WriteUByte(static_cast<uint8>(SCPacket::eCreateCharacter));
		for (int i = 0; i < mNumCurUsers; ++i)
		{
			Entity e = CreateEntity();
			e.AddTag<Tag_Player>();
			auto& id = e.GetComponent<IDComponent>();
			mSessions[i].CharacterID = id.ID;
			spacket.WriteInt(i);			// 클라이언트 ID
			spacket.WriteUInt64(id.ID);		// 캐릭터 아이디
		}

		for (auto& s : mSessions)
		{
			(s.ClientSocket)->Send(&spacket, sizeof(MemoryStream));
		}
	}
}

void Server::processUserInput(MemoryStream* outPacket)
{
	uint64 eid = 0;
	Vector3 direction = Vector3::Zero;

	outPacket->ReadUInt64(&eid);
	outPacket->ReadVector3(&direction);

	auto e = GetEntityByID(eid);
	if (entt::null == e)
	{
		HB_ASSERT(false, "No entity id: {0}", eid);
	}

	Entity character(e, this);
	auto& transform = character.GetComponent<STransformComponent>();

	ServerSystems::UpdatePlayerTransform(&transform.Position, &transform.Rotation.y, direction);
	character.AddTag<Tag_UpdateTransform>();
}
