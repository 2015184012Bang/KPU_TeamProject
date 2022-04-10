#include "ServerPCH.h"
#include "Server.h"

#include "HeartBeat/PacketType.h"
#include "HeartBeat/Tags.h"
#include "HeartBeat/Random.h"
#include "HeartBeat/Define.h"

#include "AIController.h"
#include "CollisionChecker.h"
#include "EnemyGenerator.h"
#include "ServerComponents.h"
#include "ServerSystems.h"
#include "Tank.h"

Server::Server()
	: mListenSocket(nullptr)
	, mNumCurUsers(0)
	, mEnemyGenerator(nullptr)
	, mCollisionChecker(nullptr)
	, mAIController(nullptr)
{
	mUserReadied.fill(false);
}

bool Server::Init()
{
	HB_LOG("SERVER INIT");

	SocketUtil::Init();
	Timer::Init();
	Random::Init();

	mCollisionChecker = std::make_shared<CollisionChecker>(this);
	mEnemyGenerator = std::make_shared<EnemyGenerator>(this);
	mAIController = std::make_shared<AIController>(this);

	int retVal = bindAndListen();

	if (false == retVal)
	{
		return false;
	}

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
		Timer::Update();

		acceptClients();
		recvFromClients();
		clearIfDisconnected();

		updateEnemyGenerator();
		updateCollisionChecker();
		updateAIController();

		makePacket();
		flushSendQueue();
	}
}

Entity Server::CreateEntity()
{
	Entity e = Entity(GetNewEntity(), this);
	e.AddComponent<STransformComponent>();
	auto& id = e.AddComponent<IDComponent>();

	RegisterEntity(id.ID, e);

	return e;
}

void Server::PushPacket(MemoryStream* packet)
{
	mSendQueue.push(packet);
}

bool Server::bindAndListen()
{
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

	mListenSocket->SetReuseAddress(true);
	mListenSocket->SetNonBlockingMode(true);

	return true;
}

void Server::acceptClients()
{
	if (mNumCurUsers >= MAX_PLAYER)
	{
		return;
	}

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

		if (mNumCurUsers == MAX_PLAYER)
		{
			mListenSocket = nullptr;
		}
	}
}

void Server::recvFromClients()
{
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
}

void Server::clearIfDisconnected()
{
	// 접속이 끊긴 클라이언트 정리
	mSessions.erase(std::remove_if(mSessions.begin(), mSessions.end(), [](const Session& s) {
		return s.bConnect == false;
		}), mSessions.end());
}

void Server::flushSendQueue()
{
	static float elapsed = 0.0f;
	elapsed += Timer::GetDeltaTime();
	if (elapsed > 0.03f)
	{
		elapsed = 0.0f;

		while (!mSendQueue.empty())
		{
			MemoryStream* packet = mSendQueue.front();
			mSendQueue.pop();

			sendToAllSessions(*packet);
			delete packet;
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

		case CSPacket::eUserKeyboardInput:
			processKeyboardUserInput(outPacket);
			break;

		case CSPacket::eUserMouseInput:
			processUserMouseInput(outPacket);
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

		// 스테이지 초기화
		initGameStage();
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
			auto& transform = e.GetComponent<STransformComponent>();
			e.AddComponent<BoxComponent>(mCollisionChecker->GetLocalBox(L"Character"), transform.Position);
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

void Server::processKeyboardUserInput(MemoryStream* outPacket)
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

void Server::processUserMouseInput(MemoryStream* outPacket)
{
	uint64 eid = 0;
	outPacket->ReadUInt64(&eid);

	auto entity = GetEntityByID(eid);
	HB_ASSERT((entity != entt::null), "No entity id : {0}", eid);

	auto& transform = Entity(entity, this).GetComponent<STransformComponent>();
	mCollisionChecker->MakeHitBoxAndCheck(transform.Position, transform.Rotation.y);
}


void Server::sendToAllSessions(const MemoryStream& packet)
{
	for (auto& s : mSessions)
	{
		(s.ClientSocket)->Send(&packet, sizeof(MemoryStream));
	}
}

void Server::updateAIController()
{
	if (!mAIController)
	{
		HB_LOG("AIController is nullptr");
		return;
	}

	mAIController->Update();
}

void Server::initGameStage()
{
	mEnemyGenerator->SetStart(true);

	Entity tank = CreateEntity();
	tank.AddComponent<NameComponent>("Tank");
	tank.AddComponent<AIComponent>(std::make_shared<Tank>(tank));

	auto& transform = tank.GetComponent<STransformComponent>();
	auto& id = tank.GetComponent<IDComponent>();

	MemoryStream* packet = new MemoryStream;
	packet->WriteUByte(static_cast<uint8>(SCPacket::eCreateTank));
	packet->WriteUInt64(id.ID);
	packet->WriteVector3(transform.Position);
	packet->WriteFloat(transform.Rotation.y);

	PushPacket(packet);
}

void Server::updateEnemyGenerator()
{
	if (!mEnemyGenerator)
	{
		HB_LOG("EnemyGenerator is nullptr");
		return;
	}

	mEnemyGenerator->Update();
}

void Server::updateCollisionChecker()
{
	if (!mCollisionChecker)
	{
		HB_LOG("CollisionChecker is nullptr!");
		return;
	}

	mCollisionChecker->Update();
}

void Server::makePacket()
{
	static float elapsed = 0.0f;

	elapsed += Timer::GetDeltaTime();

	if (elapsed > 0.03f)
	{
		elapsed = 0.0f;

		auto view = GetRegistry().view<Tag_UpdateTransform>();
		if (view.empty())
		{
			return;
		}

		MemoryStream* spacket = new MemoryStream;
		for (auto e : view)
		{
			Entity ent = Entity(e, this);
			ent.RemoveComponent<Tag_UpdateTransform>();
			auto& transform = ent.GetComponent<STransformComponent>();
			auto& id = ent.GetComponent<IDComponent>();

			spacket->WriteUByte(static_cast<uint8>(SCPacket::eUpdateTransform));
			spacket->WriteUInt64(id.ID);
			spacket->WriteVector3(transform.Position);
			spacket->WriteFloat(transform.Rotation.y);
		}

		mSendQueue.push(spacket);
	}
}
