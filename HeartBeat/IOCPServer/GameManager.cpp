#include "pch.h"
#include "GameManager.h"
#include "Timer.h"

#include "Box.h"
#include "Entity.h"
#include "tinyxml2.h"
#include "Tags.h"
#include "Random.h"

float gTileSide;
float gPlayerSpeed;
float gBaseAttackCooldown;
float gBaseAttackRange;

float GetTileYPos(TileType ttype);
void AddTagToTile(Entity& tile, TileType ttype);

void GameManager::Init(const UINT32 maxSessionCount)
{
	// XML 파일에서 여러 변수 값 읽어오기
	loadValuesFromXML("settings.xml");

	// 패킷 처리 함수들 등록
	mPacketIdToFunction[SYS_USER_CONNECT] = std::bind(&GameManager::processUserConnect, this,
		std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	mPacketIdToFunction[SYS_USER_DISCONNECT] = std::bind(&GameManager::processUserDisconnect, this,
		std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	mPacketIdToFunction[REQUEST_LOGIN] = std::bind(&GameManager::processRequestLogin, this,
		std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	mPacketIdToFunction[REQUEST_ENTER_UPGRADE] = std::bind(&GameManager::processRequestEnterUpgrade, this,
		std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	mPacketIdToFunction[REQUEST_MOVE] = std::bind(&GameManager::processRequestMove, this,
		std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	mPacketIdToFunction[REQUEST_UPGRADE] = std::bind(&GameManager::processRequestUpgrade, this,
		std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	mPacketIdToFunction[REQUEST_ENTER_GAME] = std::bind(&GameManager::processRequestEnterGame, this,
		std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	mPacketIdToFunction[REQUEST_ATTACK] = std::bind(&GameManager::processRequestAttack, this,
		std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

	// 유저 매니저 생성
	mUserManager = make_unique<UserManager>();
	mUserManager->Init(maxSessionCount);

	// Back은 IO 워커 스레드들이 패킷을 쓰는 큐를 가리킴.
	// Front는 로직 스레드가 처리할 패킷을 담은 큐.
	mBackPacketQueue = &mPacketQueueA;
	mFrontPacketQueue = &mPacketQueueB;

	Timer::Init();
	Random::Init();
	Box::Init();

	// 시스템 생성
	initSystems();

	// 게임 맵 생성
	mGameMap = make_unique<GameMap>();
	mGameMap->LoadMap("../Assets/Maps/Map01.csv");
}

void GameManager::Run()
{
	mLogicThread = thread([this]() { logicThread(); });
}

void GameManager::End()
{
	mShouldLogicRun = false;

	if (mLogicThread.joinable())
	{
		mLogicThread.join();
	}
}

void GameManager::PushUserData(const INT32 sessionIndex, const UINT32 dataSize, char* pData)
{
	auto user = mUserManager->GetUserByIndex(sessionIndex);
	user->SetData(dataSize, pData);

	if (auto packet = user->GetPacket(); packet.SessionIndex != -1)
	{
		WriteLockGuard guard(mLock);
		mBackPacketQueue->push(packet);
	}
}

void GameManager::PushSystemPacket(PACKET_INFO packet)
{
	WriteLockGuard guard(mLock);
	mBackPacketQueue->push(packet);
}

void GameManager::loadValuesFromXML(string_view fileName)
{
	tinyxml2::XMLDocument doc;
	tinyxml2::XMLError error = doc.LoadFile(fileName.data());
	ASSERT(error == tinyxml2::XML_SUCCESS, "Failed to read xml file: {0}", fileName.data());

	auto root = doc.RootElement();

	// 타일 한 변의 길이
	auto elem = root->FirstChildElement("Values")->FirstChildElement("TileSide");
	string tileSide = elem->GetText();
	gTileSide = stof(tileSide);

	// 플레이어 이동 속도
	elem = elem->NextSiblingElement();
	string playerSpeed = elem->GetText();
	gPlayerSpeed = stof(playerSpeed);

	// 기본 공격 재사용 대기 시간
	elem = elem->NextSiblingElement();
	string baCooldown = elem->GetText();
	gBaseAttackCooldown = stof(baCooldown);

	// 기본 공격 전방 사거리
	elem = elem->NextSiblingElement();
	string baRange = elem->GetText();
	gBaseAttackRange = stof(baRange);
}

void GameManager::initSystems()
{
	mMovementSystem = make_unique<MovementSystem>(shared_from_this());
	mCombatSystem = make_unique<CombatSystem>(shared_from_this());
	mCollisionSystem = make_unique<CollisionSystem>(shared_from_this());
}

void GameManager::swapQueues()
{
	WriteLockGuard guard(mLock);
	swap(mBackPacketQueue, mFrontPacketQueue);
}

void GameManager::logicThread()
{
	while (mShouldLogicRun)
	{
		Timer::Update();

		auto start = high_resolution_clock::now();

		while (!mFrontPacketQueue->empty())
		{
			PACKET_INFO packet = mFrontPacketQueue->front();
			mFrontPacketQueue->pop();
			processPacket(packet.SessionIndex, packet.PacketID, packet.DataSize, packet.DataPtr);
		}

		mMovementSystem->Update();
		mCombatSystem->Update();
		mCollisionSystem->Update();

		while (duration_cast<milliseconds>(high_resolution_clock::now() - start).count() < 33);

		// 백 큐와 프론트 큐를 스왑
		swapQueues();
	}
}

void GameManager::processPacket(const INT32 sessionIndex, const UINT8 packetID, const UINT8 packetSize, char* packet)
{
	if (auto iter = mPacketIdToFunction.find(packetID); iter != mPacketIdToFunction.end())
	{
		(iter->second)(sessionIndex, packetSize, packet);
	}
}

void GameManager::processUserConnect(const INT32 sessionIndex, const UINT8 packetSize, char* packet)
{
	LOG("Process user connect packet. Session Index: {0}", sessionIndex);
}

void GameManager::processUserDisconnect(const INT32 sessionIndex, const UINT8 packetSize, char* packet)
{
	LOG("Process user disconnect packet. Session Index: {0}", sessionIndex);
	auto user = mUserManager->GetUserByIndex(sessionIndex);
	mUserManager->DeleteUser(user);
}

void GameManager::processRequestLogin(const INT32 sessionIndex, const UINT8 packetSize, char* packet)
{
	if (sizeof(REQUEST_LOGIN_PACKET) != packetSize)
	{
		return;
	}

	REQUEST_LOGIN_PACKET* reqPacket = reinterpret_cast<REQUEST_LOGIN_PACKET*>(packet);
	auto userName = reqPacket->ID;
	mUserManager->AddUser(sessionIndex, userName);

	ANSWER_LOGIN_PACKET ansPacket;
	ansPacket.PacketID = ANSWER_LOGIN;
	ansPacket.PacketSize = sizeof(ANSWER_LOGIN_PACKET);
	ansPacket.Result = ERROR_CODE::SUCCESS;
	ansPacket.ClientID = sessionIndex;	// 클라이언트 ID와 세션 인덱스를 일치시킨다.
	SendPacketFunction(sessionIndex, sizeof(ansPacket), reinterpret_cast<char*>(&ansPacket));

	sendNotifyLoginPacket(sessionIndex);
}

void GameManager::processRequestEnterUpgrade(const INT32 sessionIndex, const UINT8 packetSize, char* packet)
{
	if (sizeof(REQUEST_ENTER_UPGRADE_PACKET) != packetSize)
	{
		return;
	}

	NOTIFY_ENTER_UPGRADE_PACKET ansPacket;
	ansPacket.PacketID = NOTIFY_ENTER_UPGRADE;
	ansPacket.PacketSize = sizeof(NOTIFY_ENTER_UPGRADE_PACKET);
	ansPacket.Result = ERROR_CODE::SUCCESS;

	SendToAll(sizeof(ansPacket), reinterpret_cast<char*>(&ansPacket));
}

void GameManager::processRequestMove(const INT32 sessionIndex, const UINT8 packetSize, char* packet)
{
	if (sizeof(REQUEST_MOVE_PACKET) != packetSize)
	{
		return;
	}

	// 패킷을 보낸 유저의 Direction 변경
	REQUEST_MOVE_PACKET* rmPacket = reinterpret_cast<REQUEST_MOVE_PACKET*>(packet);
	mMovementSystem->SetDirection(sessionIndex, rmPacket->Direction);

	auto user = mUserManager->GetUserByIndex(sessionIndex);

	// 이동 노티파이 패킷 전송
	NOTIFY_MOVE_PACKET anmPacket = {};
	anmPacket.PacketID = NOTIFY_MOVE;
	anmPacket.PacketSize = sizeof(NOTIFY_MOVE_PACKET);
	anmPacket.Direction = user->GetMoveDirection();
	anmPacket.Position = user->GetPosition();
	anmPacket.EntityID = sessionIndex;
	SendToAll(sizeof(anmPacket), reinterpret_cast<char*>(&anmPacket));
}

void GameManager::processRequestUpgrade(const INT32 sessionIndex, const UINT8 packetSize, char* packet)
{
	if (sizeof(REQUEST_UPGRADE_PACKET) != packetSize)
	{
		return;
	}

	REQUEST_UPGRADE_PACKET* ruPacket = reinterpret_cast<REQUEST_UPGRADE_PACKET*>(packet);
	auto user = mUserManager->GetUserByIndex(sessionIndex);

	// 유저 공격력, 방어력, 회복력 설정
	mCombatSystem->SetPreset(sessionIndex,
		static_cast<CombatSystem::UpgradePreset>(ruPacket->UpgradePreset));

	// 해당 유저를 비롯한 다른 유저들에게 알림
	NOTIFY_UPGRADE_PACKET nuPacket = {};
	nuPacket.PacketID = NOTIFY_UPGRADE;
	nuPacket.PacketSize = sizeof(nuPacket);
	nuPacket.EntityID = sessionIndex;
	nuPacket.UpgradePreset = ruPacket->UpgradePreset;
	SendToAll(sizeof(nuPacket), reinterpret_cast<char*>(&nuPacket));
}

void GameManager::processRequestEnterGame(const INT32 sessionIndex, const UINT8 packetSize, char* packet)
{
	if (sizeof(REQUEST_ENTER_GAME_PACKET) != packetSize)
	{
		return;
	}

	NOTIFY_ENTER_GAME_PACKET negPacket = {};
	negPacket.PacketID = NOTIFY_ENTER_GAME;
	negPacket.PacketSize = sizeof(negPacket);
	negPacket.Result = ERROR_CODE::SUCCESS;

	SendToAll(sizeof(negPacket), reinterpret_cast<char*>(&negPacket));

	// 게임이 시작되면, 서버에서도 맵 타일을 생성한다.
	createMapTiles();
}

void GameManager::processRequestAttack(const INT32 sessionIndex, const UINT8 packetSize, char* packet)
{
	if (sizeof(REQUEST_ATTACK_PACKET) != packetSize)
	{
		return;
	}

	bool canAttack = mCombatSystem->CanBaseAttack(sessionIndex);
	if (!canAttack)
	{
		return;
	}

	bool bHit = mCollisionSystem->DoAttack(sessionIndex);

	// 공격이 허가됐다면, 해당 유저를 포함한 다른 유저들에게 알려준다.
	NOTIFY_ATTACK_PACKET naPacket = {};
	naPacket.EntityID = sessionIndex;
	naPacket.Result = bHit ? ERROR_CODE::ATTACK_SUCCESS : ERROR_CODE::ATTACK_MISS;
	naPacket.PacketID = NOTIFY_ATTACK;
	naPacket.PacketSize = sizeof(naPacket);
	SendToAll(sizeof(naPacket), reinterpret_cast<char*>(&naPacket));
}

void GameManager::sendNotifyLoginPacket(const INT32 newlyConnectedIndex)
{
	auto connectedUsers = mUserManager->GetAllConnectedUsersIndex();

	if (connectedUsers.empty())
	{
		return;
	}

	NOTIFY_LOGIN_PACKET notifyPacket;
	notifyPacket.PacketID = NOTIFY_LOGIN;
	notifyPacket.PacketSize = sizeof(NOTIFY_LOGIN_PACKET);
	notifyPacket.ClientID = newlyConnectedIndex;

	// 기존에 접속해 있던 유저들에게 새로 접속한 유저를 알린다.
	SendPacketExclude(newlyConnectedIndex, sizeof(notifyPacket), reinterpret_cast<char*>(&notifyPacket));

	// 새로 접속한 유저에게 기존 유저들을 알린다.
	for (auto userIndex : connectedUsers)
	{
		if (userIndex == newlyConnectedIndex)
		{
			continue;
		}

		notifyPacket.ClientID = userIndex;
		SendPacketFunction(newlyConnectedIndex, sizeof(notifyPacket), reinterpret_cast<char*>(&notifyPacket));
	}
}

void GameManager::SendPacketExclude(const INT32 userIndexToExclude, const UINT32 packetSize, char* packet)
{
	auto connectedUsers = mUserManager->GetAllConnectedUsersIndex();

	if (connectedUsers.empty())
	{
		return;
	}

	for (auto userIndex : connectedUsers)
	{
		if (userIndex == userIndexToExclude)
		{
			continue;
		}

		SendPacketFunction(userIndex, packetSize, packet);
	}
}

void GameManager::SendToAll(const INT32 packetSize, char* packet)
{
	auto connectedUsers = mUserManager->GetAllConnectedUsersIndex();

	if (connectedUsers.empty())
	{
		return;
	}

	for (auto userIndex : connectedUsers)
	{
		SendPacketFunction(userIndex, packetSize, packet);
	}
}

void GameManager::createMapTiles()
{
	const auto& tiles = mGameMap->GetTiles();

	for (const auto& tile : tiles)
	{
		Entity obj = Entity{ gRegistry.create() };
		AddTagToTile(obj, tile.TType);

		auto& transform = obj.AddComponent<TransformComponent>();
		transform.Position = { tile.X, GetTileYPos(tile.TType), tile.Z };

		obj.AddComponent<BoxComponent>(&Box::GetBox("../Assets/Boxes/Cube.box"), transform.Position, transform.Yaw);
	}
}

float GetTileYPos(TileType ttype)
{
	switch (ttype)
	{
	case TileType::BLOCKED:
	case TileType::FAT:
	case TileType::TANK_FAT:
		return 0.0f;

	case TileType::MOVABLE:
	case TileType::RAIL:
	case TileType::SCAR:
		return -gTileSide;

	default:
		ASSERT(false, "Unknown tile type!");
		return 0.0;
	}
}

void AddTagToTile(Entity& tile, TileType ttype)
{
	switch (ttype)
	{
	case TileType::BLOCKED:
		tile.AddTag<Tag_Tile>();
		tile.AddTag<Tag_Blocked>();
		break;

	case TileType::FAT:
	case TileType::TANK_FAT:
		tile.AddTag<Tag_Tile>();
		tile.AddTag<Tag_Blocked>();
		tile.AddTag<Tag_Breakable>();
		tile.AddComponent<HealthComponent>(Random::RandInt(1, 5)); // FAT 종류는 부술 수 있으므로 체력 컴포넌트 부착
		break;

	case TileType::MOVABLE:
	case TileType::RAIL:
	case TileType::SCAR:
		tile.AddTag<Tag_Tile>();
		break;

	default:
		ASSERT(false, "Unknown tile type!");
		break;
	}
}