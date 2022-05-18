#pragma once

#include "Protocol.h"
#include "UserManager.h"

#include "MovementSystem.h"
#include "CombatSystem.h"
#include "CollisionSystem.h"
#include "ScriptSystem.h"
#include "EnemySystem.h"
#include "RoomManager.h"
#include "GameMap.h"

class GameManager
{
public:
	void Init(const UINT32 maxSessionCount);

	void Run();

	void End();

	void PushUserData(const INT32 sessionIndex, const UINT32 dataSize, char* pData);

	void PushSystemPacket(PACKET_INFO packet);

	function<void(INT32, UINT32, char*)> SendPacketFunction;

	// 게임오버 처리
	//void DoGameOver();

private:
	void swapQueues();

	void logicThread();

	void processPacket(const INT32 sessionIndex, const UINT8 packetID, const UINT8 packetSize, char* packet);
	void processUserConnect(const INT32 sessionIndex, const UINT8 packetSize, char* packet);
	void processUserDisconnect(const INT32 sessionIndex, const UINT8 packetSize, char* packet);
	void processRequestLogin(const INT32 sessionIndex, const UINT8 packetSize, char* packet);
	void processRequestRoom(const INT32 sessionIndex, const UINT8 packetSize, char* packet);
	void processRequestEnterRoom(const INT32 sessionIndex, const UINT8 packetSize, char* packet);
	void processRequestLeaveRoom(const INT32 sessionIndex, const UINT8 packetSize, char* packet);
	void processRequestEnterUpgrade(const INT32 sessionIndex, const UINT8 packetSize, char* packet);
	void processRequestMove(const INT32 sessionIndex, const UINT8 packetSize, char* packet);
	void processRequestUpgrade(const INT32 sessionIndex, const UINT8 packetSize, char* packet);
	void processRequestEnterGame(const INT32 sessionIndex, const UINT8 packetSize, char* packet);
	void processRequestAttack(const INT32 sessionIndex, const UINT8 packetSize, char* packet);
	void processRequestSkill(const INT32 sessionIndex, const UINT8 packetSize, char* packet);

	// 스테이지 초기화
	//void clearStage();

private:
	using PACKET_PROCESS_FUNCTION = function<void(INT32, UINT8, char*)>;

	// 패킷 처리 함수 맵
	unordered_map<UINT8, PACKET_PROCESS_FUNCTION> mPacketIdToFunction;

	// 유저 매니저
	unique_ptr<UserManager> mUserManager = nullptr;

	// 룸 매니저
	unique_ptr<RoomManager> mRoomManager = nullptr;

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
};

