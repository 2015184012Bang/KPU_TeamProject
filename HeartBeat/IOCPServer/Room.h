#pragma once

#include "Entity.h"
#include "MovementSystem.h"
#include "ScriptSystem.h"
#include "EnemySystem.h"
#include "CombatSystem.h"
#include "CollisionSystem.h"
#include "GameMap.h"

class User;

constexpr INT32 ROOM_MAX_USER = 3;

class Room 
	: public enable_shared_from_this<Room>
{
public:
	enum class RoomState
	{
		Waiting,
		Waiting_Full,
		Playing,
	};

public:
	void Init(const INT32 index, function<void(INT32, UINT32, char*)> sendFunc);

	// 방의 상태가 Playing일 때 Tick마다 호출되는 함수
	void Update();

	// 입장 가능 여부를 리턴
	bool CanEnter();

	// 방에 유저 등록
	void AddUser(User* user);

	// 방에서 유저 제거
	void RemoveUser(User* user);

	// 방 안의 모든 유저들에게 패킷 송신
	void Broadcast(const UINT32 packetSize, char* packet);

	// 업그레이드 씬에 입장할 때 서버에서 해야할 일 수행
	void DoEnterUpgrade();

	// 게임 씬에 입장할 때 서버에서 해야할 일 수행
	void DoEnterGame();

	// 룸 씬에서 새 유저가 참가했을 때 호출
	void NotifyNewbie(User* newbie);

	// 유저의 이동 입력 처리 함수
	void SetDirection(const INT8 clientID, const Vector3& direction);

	// 업그레이드 씬에서 유저가 상호작용 키를 눌렀을 때 호출되는 함수
	void SetPreset(const INT8 clientID, CombatSystem::UpgradePreset preset);

	// 유저의 공격 가능 여부를 리턴
	bool CanBaseAttack(const INT8 clientID);

	// 유저가 기본 공격 키(A)를 눌렀을 때 호출되는 함수
	bool DoAttack(const INT8 clientID);

	// 패킷 송신 함수
	function<void(INT32, UINT32, char*)> SendPacketFunction;

public:
	RoomState GetState() { return mRoomState; }
	list<User*>& GetUsers() { return mUsers; }
	UINT32 GetEntityID() { return mEntityID++; }

	void SetState(RoomState state) { mRoomState = state; }

private:
	// 방 안에 남은 자리가 있는 지 리턴
	bool existsFreeSlot();

	void createSystems();

	void createTiles(string_view fileName);

	void createTankAndCart();

	// 매개변수로 전달된 타일 엔티티에 타일 타입에 해당하는 컴포넌트 부착
	void addTagToTile(entt::entity tile, TileType ttype);

	// 게임 상태 초기화
	void clearGame();

private:
	INT32 mRoomIndex = -1;

	RoomState mRoomState = RoomState::Waiting;
	
	list<INT8> mClientIDs; // 0, 1, 2

	list<User*> mUsers;

	entt::registry mRegistry;

	UINT32 mEntityID = 3; // 0~2는 플레이어가 사용

	// 시스템들
	unique_ptr<MovementSystem> mMovementSystem = nullptr;
	unique_ptr<ScriptSystem> mScriptSystem = nullptr;
	unique_ptr<EnemySystem> mEnemySystem = nullptr;
	unique_ptr<CombatSystem> mCombatSystem = nullptr;
	unique_ptr<CollisionSystem> mCollisionSystem = nullptr;
};

// 타일 타입에 따른 y 위치 반환
float GetTileYPos(TileType ttype);