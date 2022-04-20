#pragma once

#include "Protocol.h"
#include "UserManager.h"

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

	void swapQueues();

	PACKET_INFO popUserPacket();

	PACKET_INFO popSystemPacket();

	void logicThread();

	void processPacket(const INT32 sessionIndex, const UINT8 packetID, const UINT8 packetSize, char* packet);
	void processUserConnect(const INT32 sessionIndex, const UINT8 packetSize, char* packet);
	void processUserDisconnect(const INT32 sessionIndex, const UINT8 packetSize, char* packet);
	void processRequestLogin(const INT32 sessionIndex, const UINT8 packetSize, char* packet);

private:
	using PACKET_PROCESS_FUNCTION = function<void(INT32, UINT8, char*)>;

	// ��Ŷ ó�� �Լ� ��
	unordered_map<UINT8, PACKET_PROCESS_FUNCTION> mPacketIdToFunction;

	// ���� �Ŵ���
	unique_ptr<UserManager> mUserManager = nullptr;

	// ���� ������ ���� ����
	bool mShouldLogicRun = true;

	// ���� ������
	thread mLogicThread;

	// �� ť ����ȭ�� ��
	Lock mLock;

	// �����͸� ���� ������ �ε����� ���
	queue<INT32> mIndexQueueA; 
	queue<INT32> mIndexQueueB; 
	queue<INT32>* mBackIndexQueue = nullptr; 
	queue<INT32>* mFrontIndexQueue = nullptr;
	
	// ���� ����, ���� �� �ý��� ��Ŷ�� ��� ť
	queue<PACKET_INFO> mSystemQueueA;
	queue<PACKET_INFO> mSystemQueueB;
	queue<PACKET_INFO>* mBackSystemQueue = nullptr;
	queue<PACKET_INFO>* mFrontSystemQueue = nullptr;
};

