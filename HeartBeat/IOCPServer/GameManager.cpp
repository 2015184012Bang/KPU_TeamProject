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
	// 패킷 처리 함수들 등록
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

	// Back은 IO 워커 스레드들이 패킷을 쓰는 큐를 가리킴.
	// Front는 로직 스레드가 처리할 패킷을 담은 큐.
	mBackPacketQueue = &mPacketQueueA;
	mFrontPacketQueue = &mPacketQueueB;

	// 유저 매니저 생성
	mUserManager = make_unique<UserManager>();
	mUserManager->Init(maxSessionCount);

	// 룸 매니저 생성
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

	// 유저 등록
	REQUEST_LOGIN_PACKET* reqPacket = reinterpret_cast<REQUEST_LOGIN_PACKET*>(packet);
	auto userName = reqPacket->ID;
	mUserManager->AddUser(sessionIndex, userName);

	// ANSWER 패킷 반송
	ANSWER_LOGIN_PACKET ansPacket = {};
	ansPacket.PacketID = ANSWER_LOGIN;
	ansPacket.PacketSize = sizeof(ANSWER_LOGIN_PACKET);
	ansPacket.Result = RESULT_CODE::SUCCESS;
	SendPacketFunction(sessionIndex, sizeof(ansPacket), reinterpret_cast<char*>(&ansPacket));

	// Room 정보 클라이언트에게 전송
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

	// Room Scene으로 돌아간 유저에게 이용 가능한 방 목록 전송
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
//	// 게임 오버 패킷 송신
//	NOTIFY_GAME_OVER_PACKET packet = {};
//	packet.PacketID = NOTIFY_GAME_OVER;
//	packet.PacketSize = sizeof(packet);
//	packet.Result = RESULT_CODE::STAGE_FAIL;
//	SendToAll(sizeof(packet), reinterpret_cast<char*>(&packet));
//
//	// 엔티티 초기화
//	clearStage();
//}
//

//void GameManager::clearStage()
//{
//	// 엔티티 ID 초기화
//	Values::EntityID = 3;
//
//	// 플레이어 엔티티를 제외한 엔티티 삭제
//	DestroyExclude<Tag_Player>();
//
//	// 시스템 작동 정지
//	mCollisionSystem->SetStart(false);
//	mEnemySystem->SetGenerate(false);
//
//	// 이 함수에서 Spawn 컴포넌트를 부착한 엔티티를 생성하므로
//	// 플레이어 엔티티를 제외한 전체 삭제는 미리 해둬야 한다.
//	mEnemySystem->LoadStageFile("../Assets/Stages/Stage1.csv");
//
//	// 플레이어의 이동 방향, 위치, 전투 능력 초기화하고
//	// 위치를 초기화시키기 위한 패킷을 보낸다.
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
