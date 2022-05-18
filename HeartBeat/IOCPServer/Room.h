#pragma once

#include "Entity.h"
#include "MovementSystem.h"
#include "ScriptSystem.h"
#include "EnemySystem.h"
#include "CombatSystem.h"
#include "CollisionSystem.h"
#include "PathSystem.h"
#include "GameMap.h"

class User;

constexpr INT32 ROOM_MAX_USER = 3;
constexpr INT32 MAX_CELL_COUNT = 6;

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

	void Update();

	void AddUser(User* user);

	void RemoveUser(User* user);

	void Broadcast(const UINT32 packetSize, char* packet);

	void DoEnterRoom(User* user);

	void DoLeaveRoom(User* user);

	void DoEnterUpgrade();

	void DoEnterGame();

	void DoSetDirection(User* user, const Vector3& direction);

	void DoSetPreset(User* user, UpgradePreset preset);

	void DoAttack(User* user);

	void DoSkill(User* user);

	void DoGameOver();

	// 해당 인덱스의 타일 타입을 ROAD로 바꾼다.
	void ChangeTileToRoad(INT32 row, INT32 col);

	function<void(INT32, UINT32, char*)> SendPacketFunction;

public:
	RoomState GetState() { return mRoomState; }
	list<User*>& GetUsers() { return mUsers; }
	UINT32 GetEntityID() { return mEntityID++; }

	void SetState(RoomState state) { mRoomState = state; }

private:
	void checkGameState();

	bool canEnterRoom();

	void notifyNewbie(User* newbie);

	void createSystems();

	void createTiles(string_view fileName);
	void createTankAndCart();
	void createCells();
	void createGameState();

	void addTagToTile(entt::entity tile, TileType ttype);

	void clearAllUser();
	void clearGame();

	Vector3 getCellStartPosition(INT32 index);

private:
	INT32 mRoomIndex = -1;

	RoomState mRoomState = RoomState::Waiting;
	
	list<INT8> mClientIDs; // 0, 1, 2

	list<User*> mUsers;

	entt::registry mRegistry;

	UINT32 mEntityID = 3; // 0~2는 플레이어가 사용

	unique_ptr<MovementSystem> mMovementSystem = nullptr;
	unique_ptr<ScriptSystem> mScriptSystem = nullptr;
	unique_ptr<EnemySystem> mEnemySystem = nullptr;
	unique_ptr<CombatSystem> mCombatSystem = nullptr;
	unique_ptr<CollisionSystem> mCollisionSystem = nullptr;
	unique_ptr<PathSystem> mPathSystem = nullptr;

	entt::entity mPlayState = entt::null;

	float mPlayTimeSec = 0.0f;
	bool bGameStart = false;
};

// 타일 타입에 따른 y 위치 반환
float GetTileYPos(TileType ttype);