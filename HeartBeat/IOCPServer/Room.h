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

	// �ش� �ε����� Ÿ�� Ÿ���� ROAD�� �ٲ۴�.
	void ChangeTileToRoad(INT32 row, INT32 col);

	UINT32 CreateCell(const Vector3& position, bool bWhiteCell = false);

	function<void(INT32, UINT32, char*)> SendPacketFunction;

	void SendDeleteEntityPacket(const UINT32 id, EntityType eType);

	void SendEventOccurPacket(const INT32 addtionalData, EventType eType);

	void UpdatePlayerHpInState(const INT32 hp, const UINT32 id);

	void UpdateScore(INT32 delta);

	void GenerateEnemyRandomly(const Vector3& controlPoint);

	void GenerateBoss();

public:
	RoomState GetState() { return mRoomState; }
	list<User*>& GetUsers() { return mUsers; }
	UINT32 GetEntityID() { return mEntityID++; }
	size_t GetCurrentUsers() { return mUsers.size(); }

	void SetState(RoomState state) { mRoomState = state; }

private:
	void checkGameState();

	bool canEnterRoom();

	void notifyNewbie(User* newbie);

	void createSystems();

	void createTiles(string_view fileName);
	void createTile(const Tile& tile);
	void createTankAndCart();
	void createCells();
	
	void createGameState();

	void clearAllUser();
	void clearGame();

	Vector3 getCellStartPosition(const INT32 index);

private:
	INT32 mRoomIndex = -1;

	RoomState mRoomState = RoomState::Waiting;
	
	list<INT8> mClientIDs; // 0, 1, 2

	list<User*> mUsers;

	entt::registry mRegistry;

	UINT32 mEntityID = 3; // 0~2�� �÷��̾ ���

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

// Ÿ�� Ÿ�Կ� ���� y ��ġ ��ȯ
float GetTileYPos(TileType ttype);