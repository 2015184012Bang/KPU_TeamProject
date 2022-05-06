#include "pch.h"
#include "GameManager.h"
#include "Timer.h"

#include "Box.h"
#include "Entity.h"
#include "Tags.h"
#include "Random.h"
#include "Values.h"
#include "Tank.h"
#include "Room.h"

//float GetTileYPos(TileType ttype);
//void AddTagToTile(entt::entity tile, TileType ttype);

void GameManager::Init(const UINT32 maxSessionCount)
{
	// ��Ŷ ó�� �Լ��� ���
	mPacketIdToFunction[SYS_USER_CONNECT] = std::bind(&GameManager::processUserConnect, this,
		std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	mPacketIdToFunction[SYS_USER_DISCONNECT] = std::bind(&GameManager::processUserDisconnect, this,
		std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	mPacketIdToFunction[REQUEST_LOGIN] = std::bind(&GameManager::processRequestLogin, this,
		std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	mPacketIdToFunction[REQUEST_ENTER_ROOM] = std::bind(&GameManager::processRequestEnterRoom, this,
		std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	mPacketIdToFunction[REQUEST_LEAVE_ROOM] = std::bind(&GameManager::processRequestLeaveRoom, this,
		std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	mPacketIdToFunction[REQUEST_ENTER_UPGRADE] = std::bind(&GameManager::processRequestEnterUpgrade, this,
		std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	mPacketIdToFunction[REQUEST_MOVE] = std::bind(&GameManager::processRequestMove, this,
		std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	mPacketIdToFunction[REQUEST_UPGRADE] = std::bind(&GameManager::processRequestUpgrade, this,
		std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	mPacketIdToFunction[REQUEST_ENTER_GAME] = std::bind(&GameManager::processRequestEnterGame, this,
		std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	//mPacketIdToFunction[REQUEST_ATTACK] = std::bind(&GameManager::processRequestAttack, this,
	//	std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

	// Back�� IO ��Ŀ ��������� ��Ŷ�� ���� ť�� ����Ŵ.
	// Front�� ���� �����尡 ó���� ��Ŷ�� ���� ť.
	mBackPacketQueue = &mPacketQueueA;
	mFrontPacketQueue = &mPacketQueueB;

	// ���� �Ŵ��� ����
	mUserManager = make_unique<UserManager>();
	mUserManager->Init(maxSessionCount);

	// �� �Ŵ��� ����
	mRoomManager = make_unique<RoomManager>();
	mRoomManager->SendPacketFunction = SendPacketFunction;
	mRoomManager->Init(Values::MaxRoomNum);
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
			PACKET_INFO& packet = mFrontPacketQueue->front();
			processPacket(packet.SessionIndex, packet.PacketID, packet.DataSize, packet.DataPtr);
			mFrontPacketQueue->pop();
		}

		mRoomManager->Update();

		while (duration_cast<milliseconds>(high_resolution_clock::now() - start).count() < 33);

		// �� ť�� ����Ʈ ť�� ����
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

	auto roomIndex = user->GetRoomIndex();

	if (roomIndex != -1)
	{
		auto& room = mRoomManager->GetRoom(roomIndex);
		room->RemoveUser(user);
	}
	
	mUserManager->DeleteUser(user);
}

void GameManager::processRequestLogin(const INT32 sessionIndex, const UINT8 packetSize, char* packet)
{
	if (sizeof(REQUEST_LOGIN_PACKET) != packetSize)
	{
		return;
	}

	// ���� ���
	REQUEST_LOGIN_PACKET* reqPacket = reinterpret_cast<REQUEST_LOGIN_PACKET*>(packet);
	auto userName = reqPacket->ID;
	mUserManager->AddUser(sessionIndex, userName);
	
	// ANSWER ��Ŷ �ݼ�
	ANSWER_LOGIN_PACKET ansPacket = {};
	ansPacket.PacketID = ANSWER_LOGIN;
	ansPacket.PacketSize = sizeof(ANSWER_LOGIN_PACKET);
	ansPacket.Result = RESULT_CODE::SUCCESS;
	SendPacketFunction(sessionIndex, sizeof(ansPacket), reinterpret_cast<char*>(&ansPacket));

	// Room ���� Ŭ���̾�Ʈ���� ����
	mRoomManager->SendAvailableRoom(sessionIndex);
}

void GameManager::processRequestEnterRoom(const INT32 sessionIndex, const UINT8 packetSize, char* packet)
{
	if (sizeof(REQUEST_ENTER_ROOM_PACKET) != packetSize)
	{
		return;
	}

	REQUEST_ENTER_ROOM_PACKET* rerPacket = reinterpret_cast<REQUEST_ENTER_ROOM_PACKET*>(packet);
	auto& room = mRoomManager->GetRoom(rerPacket->RoomNumber);
	bool bEnter = room->CanEnter();

	ANSWER_ENTER_ROOM_PACKET aerPacket = {};
	aerPacket.PacketID = ANSWER_ENTER_ROOM;
	aerPacket.PacketSize = sizeof(aerPacket);

	if (!bEnter)
	{
		aerPacket.Result = RESULT_CODE::ROOM_ENTER_DENY;
		SendPacketFunction(sessionIndex, sizeof(aerPacket), reinterpret_cast<char*>(&aerPacket));
	}
	else
	{
		auto user = mUserManager->GetUserByIndex(sessionIndex);
		ASSERT(user, "User is nullptr!");
		room->AddUser(user);

		aerPacket.Result = RESULT_CODE::ROOM_ENTER_SUCCESS;
		aerPacket.ClientID = user->GetClientID();
		SendPacketFunction(sessionIndex, sizeof(aerPacket), reinterpret_cast<char*>(&aerPacket));

		// ������ �濡 ���� �������� ������ �濡 ������ �ִ� �������� ���� �۽�
		// ���� �����鿡�� ���ο� ������ ���� �۽�
		room->NotifyNewbie(user);
	}
}

void GameManager::processRequestLeaveRoom(const INT32 sessionIndex, const UINT8 packetSize, char* packet)
{
	if (sizeof(REQUEST_LEAVE_ROOM_PACKET) != packetSize)
	{
		return;
	}

	auto user = mUserManager->GetUserByIndex(sessionIndex);
	
	NOTIFY_LEAVE_ROOM_PACKET nlrPacket = {};
	nlrPacket.ClientID = user->GetClientID();
	nlrPacket.PacketID = NOTIFY_LEAVE_ROOM;
	nlrPacket.PacketSize = sizeof(nlrPacket);

	auto& room = mRoomManager->GetRoom(user->GetRoomIndex());

	// �ش� �� �����鿡�� ��ε�ĳ��Ʈ
	room->Broadcast(sizeof(nlrPacket), reinterpret_cast<char*>(&nlrPacket));

	// ���� ���� ó��
	room->RemoveUser(user);

	// Room Scene���� ���ư� �������� �̿� ������ �� ��� ����
	mRoomManager->SendAvailableRoom(sessionIndex);
}

void GameManager::processRequestEnterUpgrade(const INT32 sessionIndex, const UINT8 packetSize, char* packet)
{
	if (sizeof(REQUEST_ENTER_UPGRADE_PACKET) != packetSize)
	{
		return;
	}

	auto user = mUserManager->GetUserByIndex(sessionIndex);
	auto& room = mRoomManager->GetRoom(user->GetRoomIndex());

	room->DoEnterUpgrade();
}

void GameManager::processRequestMove(const INT32 sessionIndex, const UINT8 packetSize, char* packet)
{
	if (sizeof(REQUEST_MOVE_PACKET) != packetSize)
	{
		return;
	}

	auto user = mUserManager->GetUserByIndex(sessionIndex);
	auto& room = mRoomManager->GetRoom(user->GetRoomIndex());

	// ��Ŷ�� ���� ������ Direction ����
	REQUEST_MOVE_PACKET* rmPacket = reinterpret_cast<REQUEST_MOVE_PACKET*>(packet);
	room->SetDirection(user->GetClientID(), rmPacket->Direction);

	// �̵� ��Ƽ���� ��Ŷ ����
	NOTIFY_MOVE_PACKET anmPacket = {};
	anmPacket.PacketID = NOTIFY_MOVE;
	anmPacket.PacketSize = sizeof(NOTIFY_MOVE_PACKET);
	anmPacket.Direction = user->GetMoveDirection();
	anmPacket.Position = user->GetPosition();
	anmPacket.EntityID = user->GetClientID();
	room->Broadcast(sizeof(anmPacket), reinterpret_cast<char*>(&anmPacket));
}

void GameManager::processRequestUpgrade(const INT32 sessionIndex, const UINT8 packetSize, char* packet)
{
	if (sizeof(REQUEST_UPGRADE_PACKET) != packetSize)
	{
		return;
	}

	REQUEST_UPGRADE_PACKET* ruPacket = reinterpret_cast<REQUEST_UPGRADE_PACKET*>(packet);
	auto user = mUserManager->GetUserByIndex(sessionIndex);
	auto& room = mRoomManager->GetRoom(user->GetRoomIndex());

	// ���� ���ݷ�, ����, ȸ���� ����
	room->SetPreset(user->GetClientID(), static_cast<CombatSystem::UpgradePreset>(ruPacket->UpgradePreset));

	// �ش� ������ ����� �ٸ� �����鿡�� �˸�
	NOTIFY_UPGRADE_PACKET nuPacket = {};
	nuPacket.PacketID = NOTIFY_UPGRADE;
	nuPacket.PacketSize = sizeof(nuPacket);
	nuPacket.EntityID = user->GetClientID();
	nuPacket.UpgradePreset = ruPacket->UpgradePreset;
	room->Broadcast(sizeof(nuPacket), reinterpret_cast<char*>(&nuPacket));
}

void GameManager::processRequestEnterGame(const INT32 sessionIndex, const UINT8 packetSize, char* packet)
{
	if (sizeof(REQUEST_ENTER_GAME_PACKET) != packetSize)
	{
		return;
	}

	auto user = mUserManager->GetUserByIndex(sessionIndex);
	auto& room = mRoomManager->GetRoom(user->GetRoomIndex());

	NOTIFY_ENTER_GAME_PACKET negPacket = {};
	negPacket.PacketID = NOTIFY_ENTER_GAME;
	negPacket.PacketSize = sizeof(negPacket);
	negPacket.Result = RESULT_CODE::SUCCESS;
	room->Broadcast(sizeof(negPacket), reinterpret_cast<char*>(&negPacket));

	room->DoEnterGame();
}

//void GameManager::processRequestAttack(const INT32 sessionIndex, const UINT8 packetSize, char* packet)
//{
//	if (sizeof(REQUEST_ATTACK_PACKET) != packetSize)
//	{
//		return;
//	}
//
//	bool canAttack = mCombatSystem->CanBaseAttack(sessionIndex);
//	if (!canAttack)
//	{
//		return;
//	}
//
//	bool bHit = mCollisionSystem->DoAttack(sessionIndex);
//
//	// ������ �㰡�ƴٸ�, �ش� ������ ������ �ٸ� �����鿡�� �˷��ش�.
//	NOTIFY_ATTACK_PACKET naPacket = {};
//	naPacket.EntityID = sessionIndex;
//	naPacket.Result = bHit ? RESULT_CODE::ATTACK_SUCCESS : RESULT_CODE::ATTACK_MISS;
//	naPacket.PacketID = NOTIFY_ATTACK;
//	naPacket.PacketSize = sizeof(naPacket);
//	SendToAll(sizeof(naPacket), reinterpret_cast<char*>(&naPacket));
//}


//void GameManager::DoGameOver()
//{
//	// ���� ���� ��Ŷ �۽�
//	NOTIFY_GAME_OVER_PACKET packet = {};
//	packet.PacketID = NOTIFY_GAME_OVER;
//	packet.PacketSize = sizeof(packet);
//	packet.Result = RESULT_CODE::STAGE_FAIL;
//	SendToAll(sizeof(packet), reinterpret_cast<char*>(&packet));
//
//	// ��ƼƼ �ʱ�ȭ
//	clearStage();
//}
//

//void GameManager::clearStage()
//{
//	// ��ƼƼ ID �ʱ�ȭ
//	Values::EntityID = 3;
//
//	// �÷��̾� ��ƼƼ�� ������ ��ƼƼ ����
//	DestroyExclude<Tag_Player>();
//
//	// �ý��� �۵� ����
//	mCollisionSystem->SetStart(false);
//	mEnemySystem->SetGenerate(false);
//
//	// �� �Լ����� Spawn ������Ʈ�� ������ ��ƼƼ�� �����ϹǷ�
//	// �÷��̾� ��ƼƼ�� ������ ��ü ������ �̸� �ص־� �Ѵ�.
//	mEnemySystem->LoadStageFile("../Assets/Stages/Stage1.csv");
//
//	// �÷��̾��� �̵� ����, ��ġ, ���� �ɷ� �ʱ�ȭ�ϰ�
//	// ��ġ�� �ʱ�ȭ��Ű�� ���� ��Ŷ�� ������.
//	auto view = gRegistry.view<Tag_Player>();
//	for (auto entity : view)
//	{
//		Entity player{ entity };
//
//		auto& movement = player.GetComponent<MovementComponent>();
//		movement.Direction = Vector3::Zero;
//
//		auto& transform = player.GetComponent<TransformComponent>();
//		transform.Position = Vector3::Zero;
//
//		auto& combat = player.GetComponent<CombatComponent>();
//		combat.Armor = 0;
//		combat.BaseAttackCooldown = 0.0f;
//		combat.BaseAttackDmg = 0;
//		combat.BaseAttackTracker = 0.0f;
//		combat.Regeneration = 0;
//
//		NOTIFY_MOVE_PACKET packet = {};
//		packet.Direction = movement.Direction;
//		packet.EntityID = player.GetComponent<IDComponent>().ID;
//		packet.PacketID = NOTIFY_MOVE;
//		packet.PacketSize = sizeof(packet);
//		packet.Position = transform.Position;
//		SendToAll(sizeof(packet), reinterpret_cast<char*>(&packet));
//	}
//}
