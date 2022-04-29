#pragma once

#include "Protocol.h"
#include "UserManager.h"

#include "MovementSystem.h"
#include "CombatSystem.h"
#include "CollisionSystem.h"

extern float gTileSide;
extern float gPlayerSpeed;
extern float gBaseAttackCooldown;

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
	void loadValuesFromXML(string_view fileName);

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

	// 노티파이 패킷을 보낼 때 사용.
	// indexToExclude : 이 세션 인덱스를 제외한 다른 접속 유저들에게 패킷을 보냄
	void sendPacketExclude(const INT32 userIndexToExclude, const UINT32 packetSize, char* packet);

	// 접속 유저 모두에게 보내기
	void sendToAll(const INT32 packetSize, char* packet);

private:
	using PACKET_PROCESS_FUNCTION = function<void(INT32, UINT8, char*)>;

	// 패킷 처리 함수 맵
	unordered_map<UINT8, PACKET_PROCESS_FUNCTION> mPacketIdToFunction;

	// 유저 매니저
	unique_ptr<UserManager> mUserManager = nullptr;

	// 로직 스레드 실행 여부
	bool mShouldLogicRun = true;

	// 로직 스레드
	thread mLogicThread;

	// 백 큐 동기화용 락
	Lock mLock;

	// 패킷 큐
	queue<PACKET_INFO> mPacketQueueA;
	queue<PACKET_INFO> mPacketQueueB;
	queue<PACKET_INFO>* mBackPacketQueue = nullptr;
	queue<PACKET_INFO>* mFrontPacketQueue = nullptr;

	// 시스템
	unique_ptr<MovementSystem> mMovementSystem = nullptr;
	unique_ptr<CombatSystem> mCombatSystem = nullptr;
	unique_ptr<CollisionSystem> mCollisionSystem = nullptr;
};

