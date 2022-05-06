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
	void Init(const INT32 index, function<void(INT32, UINT32, char*)> sendFunc);

	enum class RoomState
	{
		Waiting,
		Waiting_Full,
		Playing,
	};

	bool ExistsFreeSlot();

	bool CanEnter();

	void AddUser(User* user);

	void RemoveUser(User* user);

	void Broadcast(const UINT32 packetSize, char* packet);

	void DoEnterUpgrade();

	void DoEnterGame();

	void NotifyNewbie(User* newbie);

	void SetDirection(const INT8 clientID, const Vector3& direction);

	void Update();

	void SetPreset(const INT8 clientID, CombatSystem::UpgradePreset preset);

	bool CanBaseAttack(const INT8 clientID);

	bool DoAttack(const INT8 clientID);

	function<void(INT32, UINT32, char*)> SendPacketFunction;

public:
	RoomState GetState() { return mRoomState; }
	list<User*>& GetUsers() { return mUsers; }
	UINT32 GetEntityID() { return mEntityID++; }

	void SetState(RoomState state) { mRoomState = state; }

private:
	void createSystems();

	void createTiles(string_view fileName);

	void createTankAndCart();

	void addTagToTile(entt::entity tile, TileType ttype);

private:
	INT32 mRoomIndex = -1;

	RoomState mRoomState = RoomState::Waiting;
	
	list<INT8> mClientIDs;
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

float GetTileYPos(TileType ttype);