#include "pch.h"
#include "GameManager.h"

void GameManager::Init(const UINT32 maxSessionCount)
{
	// ��Ŷ ó�� �Լ��� ���
	mPacketIdToFunction[SYS_USER_CONNECT] = std::bind(&GameManager::processUserConnect, this,
		std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	mPacketIdToFunction[SYS_USER_DISCONNECT] = std::bind(&GameManager::processUserDisconnect, this,
		std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	mPacketIdToFunction[REQUEST_LOGIN] = std::bind(&GameManager::processRequestLogin, this,
		std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

	// ���� �Ŵ��� ����
	mUserManager = make_unique<UserManager>();
	mUserManager->Init(maxSessionCount);

	// Back�� IO ��Ŀ ��������� ��Ŷ�� ���� ť�� ����Ŵ.
	// Front�� ���� �����尡 ó���� ��Ŷ�� ���� ť.
	mBackPacketQueue = &mPacketQueueA;
	mFrontPacketQueue = &mPacketQueueB;
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
		bool isIdle = true;

		while (!mFrontPacketQueue->empty())
		{
			isIdle = false;

			PACKET_INFO packet = popPacket();
			processPacket(packet.SessionIndex, packet.PacketID, packet.DataSize, packet.DataPtr);
		}

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
	auto userID = reqPacket->ID;

	// ���� �߰�
	mUserManager->AddUser(sessionIndex, userID);

	LOG("User Name[{0}] entered.", userID);

	ANSWER_LOGIN_PACKET ansPacket;
	ansPacket.PacketID = ANSWER_LOGIN;
	ansPacket.PacketSize = sizeof(ANSWER_LOGIN_PACKET);
	ansPacket.Result = ERROR_CODE::SUCCESS;
	ansPacket.ClientID = mUserManager->GetCurrentUserCount() - 1;
	SendPacketFunction(sessionIndex, sizeof(ansPacket), reinterpret_cast<char*>(&ansPacket));
}
