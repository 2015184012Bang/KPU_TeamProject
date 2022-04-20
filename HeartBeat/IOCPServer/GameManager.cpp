#include "pch.h"
#include "GameManager.h"

void GameManager::Init(const UINT32 maxSessionCount)
{
	// 패킷 처리 함수들 등록
	mPacketIdToFunction[SYS_USER_CONNECT] = std::bind(&GameManager::processUserConnect, this,
		std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	mPacketIdToFunction[SYS_USER_DISCONNECT] = std::bind(&GameManager::processUserDisconnect, this,
		std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	mPacketIdToFunction[REQUEST_LOGIN] = std::bind(&GameManager::processRequestLogin, this,
		std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

	// 유저 매니저 생성
	mUserManager = make_unique<UserManager>();
	mUserManager->Init(maxSessionCount);

	// Back은 IO 워커 스레드들이 패킷을 쓰는 큐를 가리킴.
	// Front는 로직 스레드가 처리할 패킷을 담은 큐.
	mBackIndexQueue = &mIndexQueueA;
	mFrontIndexQueue = &mIndexQueueB;
	mBackSystemQueue = &mSystemQueueA;
	mFrontSystemQueue = &mSystemQueueB;
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
	mBackIndexQueue->push(sessionIndex);
}

void GameManager::PushSystemPacket(PACKET_INFO packet)
{
	WriteLockGuard guard(mLock);
	mBackSystemQueue->push(packet);
}

void GameManager::clearUser(const INT32 sessionIndex)
{
	auto user = mUserManager->FindUserByIndex(sessionIndex);
	mUserManager->DeleteUser(user);
}

void GameManager::swapQueues()
{
	WriteLockGuard guard(mLock);
	swap(mBackIndexQueue, mFrontIndexQueue);
	swap(mBackSystemQueue, mFrontSystemQueue);
}

PACKET_INFO GameManager::popUserPacket()
{
	// 로직 스레드에서 유일하게 접근하므로 락 불필요.
	INT32 userIndex = mFrontIndexQueue->front();
	mFrontIndexQueue->pop();

	auto user = mUserManager->FindUserByIndex(userIndex);
	return user->GetPacket();
}

PACKET_INFO GameManager::popSystemPacket()
{
	// 로직 스레드에서 유일하게 접근하므로 락 불필요.
	PACKET_INFO info = mFrontSystemQueue->front();
	mFrontSystemQueue->pop();
	return info;
}

void GameManager::logicThread()
{
	while (mShouldLogicRun)
	{
		bool isIdle = true;

		while (!mFrontIndexQueue->empty())
		{
			isIdle = false;

			PACKET_INFO packet = popUserPacket();
			processPacket(packet.SessionIndex, packet.PacketID, packet.DataSize, packet.DataPtr);
		}

		while (!mFrontSystemQueue->empty())
		{
			isIdle = false;

			PACKET_INFO packet = popSystemPacket();
			processPacket(packet.SessionIndex, packet.PacketID, packet.DataSize, packet.DataPtr);
		}

		swapQueues();

		if (isIdle)
		{
			this_thread::sleep_for(1s);
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
	auto user = mUserManager->FindUserByIndex(sessionIndex);
	user->Reset();
}

void GameManager::processUserDisconnect(const INT32 sessionIndex, const UINT8 packetSize, char* packet)
{
	LOG("Process user disconnect packet. Session Index: {0}", sessionIndex);
	clearUser(sessionIndex);
}

void GameManager::processRequestLogin(const INT32 sessionIndex, const UINT8 packetSize, char* packet)
{
	LOG("Process user login packet. Session Index: {0}", sessionIndex);

	if (sizeof(REQUEST_LOGIN_PACKET) != packetSize)
	{
		return;
	}
}
