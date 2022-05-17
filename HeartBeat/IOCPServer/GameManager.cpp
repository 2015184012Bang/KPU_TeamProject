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
	mPacketIdToFunction[REQUEST_ATTACK] = std::bind(&GameManager::processRequestAttack, this,
		std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	mPacketIdToFunction[REQUEST_SKILL] = std::bind(&GameManager::processRequestSkill, this,
		std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

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
	auto user = mUserManager->GetUserByIndex(sessionIndex);
	auto& room = mRoomManager->GetRoom(rerPacket->RoomNumber);
	room->DoEnterRoom(user);
}

void GameManager::processRequestLeaveRoom(const INT32 sessionIndex, const UINT8 packetSize, char* packet)
{
	if (sizeof(REQUEST_LEAVE_ROOM_PACKET) != packetSize)
	{
		return;
	}

	auto user = mUserManager->GetUserByIndex(sessionIndex);
	auto& room = mRoomManager->GetRoom(user->GetRoomIndex());
	
	room->DoLeaveRoom(user);

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

	REQUEST_MOVE_PACKET* rmPacket = reinterpret_cast<REQUEST_MOVE_PACKET*>(packet);
	room->DoSetDirection(user, rmPacket->Direction);
}

void GameManager::processRequestUpgrade(const INT32 sessionIndex, const UINT8 packetSize, char* packet)
{
	if (sizeof(REQUEST_UPGRADE_PACKET) != packetSize)
	{
		return;
	}

	auto user = mUserManager->GetUserByIndex(sessionIndex);
	auto& room = mRoomManager->GetRoom(user->GetRoomIndex());

	REQUEST_UPGRADE_PACKET* ruPacket = reinterpret_cast<REQUEST_UPGRADE_PACKET*>(packet);
	room->DoSetPreset(user, static_cast<UpgradePreset>(ruPacket->UpgradePreset));
}

void GameManager::processRequestEnterGame(const INT32 sessionIndex, const UINT8 packetSize, char* packet)
{
	if (sizeof(REQUEST_ENTER_GAME_PACKET) != packetSize)
	{
		return;
	}

	auto user = mUserManager->GetUserByIndex(sessionIndex);
	auto& room = mRoomManager->GetRoom(user->GetRoomIndex());
	room->DoEnterGame();
}

void GameManager::processRequestAttack(const INT32 sessionIndex, const UINT8 packetSize, char* packet)
{
	if (sizeof(REQUEST_ATTACK_PACKET) != packetSize)
	{
		return;
	}

	auto user = mUserManager->GetUserByIndex(sessionIndex);
	auto& room = mRoomManager->GetRoom(user->GetRoomIndex());
	room->DoAttack(user);
}


void GameManager::processRequestSkill(const INT32 sessionIndex, const UINT8 packetSize, char* packet)
{
	if (sizeof(REQUEST_SKILL_PACKET) != packetSize)
	{
		return;
	}

	auto user = mUserManager->GetUserByIndex(sessionIndex);
	auto& room = mRoomManager->GetRoom(user->GetRoomIndex());
	room->DoSkill(user);
}

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
