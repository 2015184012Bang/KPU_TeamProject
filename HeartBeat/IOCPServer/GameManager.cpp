#include "pch.h"
#include "GameManager.h"
#include "Timer.h"

#include "Box.h"

void GameManager::Init(const UINT32 maxSessionCount)
{
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

	// 타이머 초기화
	Timer::Init();

	// 모든 박스 로드
	Box::Init();
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
	auto user = mUserManager->FindUserByIndex(sessionIndex);
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

PACKET_INFO GameManager::popPacket()
{
	// 로직 스레드에서 유일하게 접근하므로 락 불필요.
	PACKET_INFO info = mFrontPacketQueue->front();
	mFrontPacketQueue->pop();
	return info;
}

void GameManager::logicThread()
{
	while (mShouldLogicRun)
	{
		Timer::Update();

		bool isIdle = true;

		while (!mFrontPacketQueue->empty())
		{
			isIdle = false;

			PACKET_INFO packet = popPacket();
			processPacket(packet.SessionIndex, packet.PacketID, packet.DataSize, packet.DataPtr);
		}

		// 유저의 이동 방향에 따라 위치를 갱신
		mUserManager->UpdateUserTransforms();

		checkCollision();

		// 백 큐와 프론트 큐를 스왑
		swapQueues();

		if (isIdle)
		{
			this_thread::sleep_for(1ms);
		}
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
	auto user = mUserManager->FindUserByIndex(sessionIndex);
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

	sendToAll(sizeof(ansPacket), reinterpret_cast<char*>(&ansPacket));
}

void GameManager::processRequestMove(const INT32 sessionIndex, const UINT8 packetSize, char* packet)
{
	if (sizeof(REQUEST_MOVE_PACKET) != packetSize)
	{
		return;
	}

	// 패킷을 보낸 유저의 Direction 변경
	REQUEST_MOVE_PACKET* rmPacket = reinterpret_cast<REQUEST_MOVE_PACKET*>(packet);
	auto user = mUserManager->FindUserByIndex(sessionIndex);
	user->SetMoveDirection(rmPacket->Direction);

	// 패킷을 보낸 유저에게 서버가 유지하고 있는 Position 전송
	ANSWER_MOVE_PACKET amPacket = {};
	amPacket.PacketID = ANSWER_MOVE;
	amPacket.PacketSize = sizeof(amPacket);
	amPacket.Position = user->GetPosition();
	amPacket.Direction = user->GetMoveDirection();
	SendPacketFunction(sessionIndex, sizeof(amPacket), reinterpret_cast<char*>(&amPacket));

	// 이동 노티파이 패킷 전송
	NOTIFY_MOVE_PACKET anmPacket = {};
	anmPacket.PacketID = NOTIFY_MOVE;
	anmPacket.PacketSize = sizeof(NOTIFY_MOVE_PACKET);
	anmPacket.Direction = amPacket.Direction;
	anmPacket.Position = amPacket.Position;
	anmPacket.EntityID = sessionIndex;
	sendPacketExclude(sessionIndex, sizeof(anmPacket), reinterpret_cast<char*>(&anmPacket));
}

void GameManager::processRequestUpgrade(const INT32 sessionIndex, const UINT8 packetSize, char* packet)
{
	if (sizeof(REQUEST_UPGRADE_PACKET) != packetSize)
	{
		return;
	}

	REQUEST_UPGRADE_PACKET* ruPacket = reinterpret_cast<REQUEST_UPGRADE_PACKET*>(packet);
	auto user = mUserManager->FindUserByIndex(sessionIndex);
	
	// 유저 공격력, 방어력, 회복력 설정
	user->SetUpgrade(static_cast<User::UpgradePreset>(ruPacket->UpgradePreset));
	
	// 해당 유저를 비롯한 다른 유저들에게 알림
	NOTIFY_UPGRADE_PACKET nuPacket = {};
	nuPacket.PacketID = NOTIFY_UPGRADE;
	nuPacket.PacketSize = sizeof(nuPacket);
	nuPacket.EntityID = sessionIndex;
	nuPacket.UpgradePreset = ruPacket->UpgradePreset;
	sendToAll(sizeof(nuPacket), reinterpret_cast<char*>(&nuPacket));
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

	sendToAll(sizeof(negPacket), reinterpret_cast<char*>(&negPacket));
}

void GameManager::processRequestAttack(const INT32 sessionIndex, const UINT8 packetSize, char* packet)
{
	if (sizeof(REQUEST_ATTACK_PACKET) != packetSize)
	{
		return;
	}

	auto user = mUserManager->FindUserByIndex(sessionIndex);
	bool bAttack = user->CanAttack();

	ANSWER_ATTACK_PACKET aaPacket = {};
	aaPacket.PacketID = ANSWER_ATTACK;
	aaPacket.PacketSize = sizeof(aaPacket);

	if (!bAttack)
	{
		aaPacket.Result = ERROR_CODE::ATTACK_NOT_YET;
	}
	else
	{
		aaPacket.Result = ERROR_CODE::SUCCESS;
	}

	SendPacketFunction(sessionIndex, sizeof(aaPacket), reinterpret_cast<char*>(&aaPacket));

	if (aaPacket.Result == ERROR_CODE::SUCCESS)
	{
		// 공격이 허가됐다면, 다른 유저들에게 알려준다.
		NOTIFY_ATTACK_PACKET naPacket = {};
		naPacket.EntityID = sessionIndex;
		naPacket.PacketID = NOTIFY_ATTACK;
		naPacket.PacketSize = sizeof(naPacket);
		sendPacketExclude(sessionIndex, sizeof(naPacket), reinterpret_cast<char*>(&naPacket));
	}
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
	sendPacketExclude(newlyConnectedIndex, sizeof(notifyPacket), reinterpret_cast<char*>(&notifyPacket));

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

void GameManager::sendPacketExclude(const INT32 userIndexToExclude, const UINT32 packetSize, char* packet)
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

void GameManager::sendToAll(const INT32 packetSize, char* packet)
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

void GameManager::checkCollision()
{
	auto connectedUsers = mUserManager->GetAllConnectedUsersIndex();

	for (auto i : connectedUsers)
	{
		auto user1 = mUserManager->FindUserByIndex(i);

		for (auto j : connectedUsers)
		{
			if (i == j) continue;

			auto user2 = mUserManager->FindUserByIndex(j);

			if (Intersects(user1->GetBox(), user2->GetBox()))
			{
				LOG("Collision!!");
			}
		}
	}
}
