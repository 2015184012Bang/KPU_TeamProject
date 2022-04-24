#include "pch.h"
#include "GameManager.h"
#include "Timer.h"

void GameManager::Init(const UINT32 maxSessionCount)
{
	// ��Ŷ ó�� �Լ��� ���
	mPacketIdToFunction[SYS_USER_CONNECT] = std::bind(&GameManager::processUserConnect, this,
		std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	mPacketIdToFunction[SYS_USER_DISCONNECT] = std::bind(&GameManager::processUserDisconnect, this,
		std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	mPacketIdToFunction[REQUEST_LOGIN] = std::bind(&GameManager::processRequestLogin, this,
		std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	mPacketIdToFunction[REQUEST_GAME_START] = std::bind(&GameManager::processRequestGameStart, this,
		std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	mPacketIdToFunction[REQUEST_MOVE] = std::bind(&GameManager::processRequestMove, this,
		std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

	// ���� �Ŵ��� ����
	mUserManager = make_unique<UserManager>();
	mUserManager->Init(maxSessionCount);

	// Back�� IO ��Ŀ ��������� ��Ŷ�� ���� ť�� ����Ŵ.
	// Front�� ���� �����尡 ó���� ��Ŷ�� ���� ť.
	mBackPacketQueue = &mPacketQueueA;
	mFrontPacketQueue = &mPacketQueueB;

	// Ÿ�̸� �ʱ�ȭ
	Timer::Init();
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

	WriteLockGuard guard(mLock);
	if (auto packet = user->GetPacket(); packet.SessionIndex != -1)
	{
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
	// ���� �����忡�� �����ϰ� �����ϹǷ� �� ���ʿ�.
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

		// ������ �̵� ���⿡ ���� ��ġ�� ����
		mUserManager->UpdateUserTransforms();

		// �� ť�� ����Ʈ ť�� ����
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
	ansPacket.ClientID = sessionIndex;	// Ŭ���̾�Ʈ ID�� ���� �ε����� ��ġ��Ų��.
	SendPacketFunction(sessionIndex, sizeof(ansPacket), reinterpret_cast<char*>(&ansPacket));

	sendNotifyLoginPacket(sessionIndex);
}

void GameManager::processRequestGameStart(const INT32 sessionIndex, const UINT8 packetSize, char* packet)
{
	if (sizeof(REQUEST_GAME_START_PACKET) != packetSize)
	{
		return;
	}

	ANSWER_GAME_START_PACKET ansPacket;
	ansPacket.PacketID = ANSWER_GAME_START;
	ansPacket.PacketSize = sizeof(ANSWER_GAME_START_PACKET);
	ansPacket.Result = START_GAME;

	auto connectedUsers = mUserManager->GetAllConnectedUsersIndex();

	if (connectedUsers.empty())
	{
		return;
	}

	for (auto userIndex : connectedUsers)
	{
		SendPacketFunction(userIndex, sizeof(ansPacket), reinterpret_cast<char*>(&ansPacket));
	}
}

void GameManager::processRequestMove(const INT32 sessionIndex, const UINT8 packetSize, char* packet)
{
	if (sizeof(REQUEST_MOVE_PACKET) != packetSize)
	{
		return;
	}

	REQUEST_MOVE_PACKET* rmPacket = reinterpret_cast<REQUEST_MOVE_PACKET*>(packet);
	auto user = mUserManager->FindUserByIndex(sessionIndex);
	user->SetMoveDirection(rmPacket->Direction);

	ANSWER_MOVE_PACKET amPacket = {};
	amPacket.PacketID = ANSWER_MOVE;
	amPacket.PacketSize = sizeof(amPacket);
	amPacket.Position = user->GetPosition();
	SendPacketFunction(sessionIndex, sizeof(amPacket), reinterpret_cast<char*>(&amPacket));

	auto connectedUsers = mUserManager->GetAllConnectedUsersIndex();

	if (connectedUsers.empty())
	{
		return;
	}

	NOTIFY_MOVE_PACKET anmPacket = {};
	anmPacket.PacketID = NOTIFY_MOVE;
	anmPacket.PacketSize = sizeof(NOTIFY_MOVE_PACKET);
	anmPacket.Direction = rmPacket->Direction;
	anmPacket.EntityID = sessionIndex;

	for (auto userIndex : connectedUsers)
	{
		if (userIndex == sessionIndex)
		{
			continue;
		}

		SendPacketFunction(userIndex, sizeof(NOTIFY_MOVE_PACKET), reinterpret_cast<char*>(&anmPacket));
	}
}

void GameManager::sendNotifyLoginPacket(const INT32 newlyConnectedIndex)
{
	auto connectedUsers = mUserManager->GetAllConnectedUsersIndex();

	if (connectedUsers.empty())
	{
		return;
	}

	NOTIFY_LOGIN_PACKET nofityPacket;
	nofityPacket.PacketID = NOTIFY_LOGIN;
	nofityPacket.PacketSize = sizeof(NOTIFY_LOGIN_PACKET);
	nofityPacket.ClientID = newlyConnectedIndex;

	// ������ ������ �ִ� �����鿡�� ���� ������ ������ �˸���.
	for (auto userIndex : connectedUsers)
	{
		// userIndex�� sessionIndex�� �����ȴ�.
		if (userIndex == newlyConnectedIndex)
		{
			continue;
		}

		SendPacketFunction(userIndex, sizeof(nofityPacket), reinterpret_cast<char*>(&nofityPacket));
	}

	// ���� ������ �������� ���� �������� �˸���.
	for (auto userIndex : connectedUsers)
	{
		if (userIndex == newlyConnectedIndex)
		{
			continue;
		}

		nofityPacket.ClientID = userIndex;
		SendPacketFunction(newlyConnectedIndex, sizeof(nofityPacket), reinterpret_cast<char*>(&nofityPacket));
	}
}
