#pragma once

#include "Protocol.h"
#include "Types.h"
#include "UserManager.h"

#include <memory>
#include <functional>
#include <unordered_map>
#include <thread>
#include <queue>

using namespace std;

class GameManager
{
public:
	void Init(const UINT32 maxSessionCount);

	void Run();

	void End();

	void PushUserData(const INT32 sessionIndex, const UINT32 dataSize, char* pData);

	void PushSystemPacket(PACKET_INFO packet);

	function<void(INT32, UINT32, char*)> SendPacketFunction;

private:
	void clearUser(const INT32 sessionIndex);

	PACKET_INFO popUserPacket();

	PACKET_INFO popSystemPacket();

	void processerThread();

	void processPacket(const INT32 sessionIndex, const UINT8 packetID, const UINT8 packetSize, char* packet);

	void processUserConnect(const INT32 sessionIndex, const UINT8 packetSize, char* packet);
	void processUserDisconnect(const INT32 sessionIndex, const UINT8 packetSize, char* packet);
	void processRequestLogin(const INT32 sessionIndex, const UINT8 packetSize, char* packet);

private:
	using PACKET_PROCESS_FUNCTION = function<void(INT32, UINT8, char*)>;

	unordered_map<UINT8, PACKET_PROCESS_FUNCTION> mPacketIdToFunction;

	unique_ptr<UserManager> mUserManager = nullptr;

	bool mShouldProcesserRun = true;

	thread mProcesserThread;

	Mutex mUserQueueLock;
	queue<INT32> mUserIndexQueue; // �����͸� ���� ������ �ε����� ���

	Mutex mSystemQueueLock;
	queue<PACKET_INFO> mSystemPacketQueue;
};

