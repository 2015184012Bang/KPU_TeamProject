#include "pch.h"
#include "GameManager.h"

void GameManager::Init(const UINT32 maxSessionCount)
{
	mPacketIdToFunction[SYS_USER_CONNECT] = std::bind(&GameManager::processUserConnect, this,
		std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	mPacketIdToFunction[SYS_USER_DISCONNECT] = std::bind(&GameManager::processUserDisconnect, this,
		std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	mPacketIdToFunction[REQUEST_LOGIN] = std::bind(&GameManager::processRequestLogin, this,
		std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

	mUserManager = make_unique<UserManager>();
	mUserManager->Init(maxSessionCount);
}

void GameManager::Run()
{
	mProcesserThread = thread([this]() { processerThread(); });
}

void GameManager::End()
{
	mShouldProcesserRun = false;

	if (mProcesserThread.joinable())
	{
		mProcesserThread.join();
	}
}

void GameManager::PushUserData(const INT32 sessionIndex, const UINT32 dataSize, char* pData)
{
	auto user = mUserManager->FindUserByIndex(sessionIndex);
	user->SetData(dataSize, pData);

	WriteLockGuard guard(mUserQueueLock);
	mUserIndexQueue.push(sessionIndex);
}

void GameManager::PushSystemPacket(PACKET_INFO packet)
{
	WriteLockGuard guard(mSystemQueueLock);
	mSystemPacketQueue.push(packet);
}

void GameManager::clearUser(const INT32 sessionIndex)
{
	auto user = mUserManager->FindUserByIndex(sessionIndex);
	mUserManager->DeleteUser(user);
}

PACKET_INFO GameManager::popUserPacket()
{
	INT32 userIndex = -1;

	{
		WriteLockGuard guard(mUserQueueLock);
		if (mUserIndexQueue.empty())
		{
			return PACKET_INFO();
		}

		userIndex = mUserIndexQueue.front();
		mUserIndexQueue.pop();
	}

	auto user = mUserManager->FindUserByIndex(userIndex);
	return user->GetPacket();
}

PACKET_INFO GameManager::popSystemPacket()
{
	WriteLockGuard guard(mSystemQueueLock);
	if (mSystemPacketQueue.empty())
	{
		return PACKET_INFO();
	}

	PACKET_INFO info = mSystemPacketQueue.front();
	mSystemPacketQueue.pop();
	return info;
}

void GameManager::processerThread()
{
	while (mShouldProcesserRun)
	{
		bool isIdle = true;

		if (auto packet = popUserPacket(); packet.SessionIndex != -1)
		{
			isIdle = false;
			processPacket(packet.SessionIndex, packet.PacketID, packet.DataSize, packet.DataPtr);
		}

		if (auto packet = popSystemPacket(); packet.SessionIndex != -1)
		{
			isIdle = false;
			processPacket(packet.SessionIndex, packet.PacketID, packet.DataSize, packet.DataPtr);
		}

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
