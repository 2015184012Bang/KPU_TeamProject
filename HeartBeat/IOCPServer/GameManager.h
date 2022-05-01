#pragma once

#include "Protocol.h"
#include "UserManager.h"

#include "MovementSystem.h"
#include "CombatSystem.h"
#include "CollisionSystem.h"
#include "ScriptSystem.h"
#include "GameMap.h"

class GameManager : public enable_shared_from_this<GameManager>
{
public:
	void Init(const UINT32 maxSessionCount);

	void Run();

	void End();

	void PushUserData(const INT32 sessionIndex, const UINT32 dataSize, char* pData);

	void PushSystemPacket(PACKET_INFO packet);

	function<void(INT32, UINT32, char*)> SendPacketFunction;

	// ��Ƽ���� ��Ŷ�� ���� �� ���.
	// indexToExclude : �� ���� �ε����� ������ �ٸ� ���� �����鿡�� ��Ŷ�� ����
	void SendPacketExclude(const INT32 userIndexToExclude, const UINT32 packetSize, char* packet);

	// ���� ���� ��ο��� ������
	void SendToAll(const INT32 packetSize, char* packet);

private:
	void initSystems();

	void swapQueues();

	void logicThread();

	void processPacket(const INT32 sessionIndex, const UINT8 packetID, const UINT8 packetSize, char* packet);
	void processUserConnect(const INT32 sessionIndex, const UINT8 packetSize, char* packet);
	void processUserDisconnect(const INT32 sessionIndex, const UINT8 packetSize, char* packet);
	void processRequestLogin(const INT32 sessionIndex, const UINT8 packetSize, char* packet);
	void processRequestEnterUpgrade(const INT32 sessionIndex, const UINT8 packetSize, char* packet);
	void processRequestMove(const INT32 sessionIndex, const UINT8 packetSize, char* packet);
	void processRequestUpgrade(const INT32 sessionIndex, const UINT8 packetSize, char* packet);
	void processRequestEnterGame(const INT32 sessionIndex, const UINT8 packetSize, char* packet);
	void processRequestAttack(const INT32 sessionIndex, const UINT8 packetSize, char* packet);

	void sendNotifyLoginPacket(const INT32 newlyConnectedIndex);

	// �������� �ʱ�ȭ �Լ�
	void initStage(string_view mapFile);

	void createMapTiles(string_view mapFile);
	void createTankAndCart();

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

	// ��Ŷ ť
	queue<PACKET_INFO> mPacketQueueA;
	queue<PACKET_INFO> mPacketQueueB;
	queue<PACKET_INFO>* mBackPacketQueue = nullptr;
	queue<PACKET_INFO>* mFrontPacketQueue = nullptr;

	// �ý���
	unique_ptr<MovementSystem> mMovementSystem = nullptr;
	unique_ptr<CombatSystem> mCombatSystem = nullptr;
	unique_ptr<CollisionSystem> mCollisionSystem = nullptr;
	unique_ptr<ScriptSystem> mScriptSystem = nullptr;

	// ���� ��
	unique_ptr<GameMap> mGameMap = nullptr;
};

