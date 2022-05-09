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

	// ���� ���°� Playing�� �� Tick���� ȣ��Ǵ� �Լ�
	void Update();

	// ���� ���� ���θ� ����
	bool CanEnter();

	// �濡 ���� ���
	void AddUser(User* user);

	// �濡�� ���� ����
	void RemoveUser(User* user);

	// �� ���� ��� �����鿡�� ��Ŷ �۽�
	void Broadcast(const UINT32 packetSize, char* packet);

	// ���׷��̵� ���� ������ �� �������� �ؾ��� �� ����
	void DoEnterUpgrade();

	// ���� ���� ������ �� �������� �ؾ��� �� ����
	void DoEnterGame();

	// �� ������ �� ������ �������� �� ȣ��
	void NotifyNewbie(User* newbie);

	// ������ �̵� �Է� ó�� �Լ�
	void SetDirection(const INT8 clientID, const Vector3& direction);

	// ���׷��̵� ������ ������ ��ȣ�ۿ� Ű�� ������ �� ȣ��Ǵ� �Լ�
	void SetPreset(const INT8 clientID, CombatSystem::UpgradePreset preset);

	// ������ ���� ���� ���θ� ����
	bool CanBaseAttack(const INT8 clientID);

	// ������ �⺻ ���� Ű(A)�� ������ �� ȣ��Ǵ� �Լ�
	bool DoAttack(const INT8 clientID);

	// ��Ŷ �۽� �Լ�
	function<void(INT32, UINT32, char*)> SendPacketFunction;

public:
	RoomState GetState() { return mRoomState; }
	list<User*>& GetUsers() { return mUsers; }
	UINT32 GetEntityID() { return mEntityID++; }

	void SetState(RoomState state) { mRoomState = state; }

private:
	// �� �ȿ� ���� �ڸ��� �ִ� �� ����
	bool existsFreeSlot();

	void createSystems();

	void createTiles(string_view fileName);

	void createTankAndCart();

	// �Ű������� ���޵� Ÿ�� ��ƼƼ�� Ÿ�� Ÿ�Կ� �ش��ϴ� ������Ʈ ����
	void addTagToTile(entt::entity tile, TileType ttype);

	// ���� ���� �ʱ�ȭ
	void clearGame();

private:
	INT32 mRoomIndex = -1;

	RoomState mRoomState = RoomState::Waiting;
	
	list<INT8> mClientIDs; // 0, 1, 2

	list<User*> mUsers;

	entt::registry mRegistry;

	UINT32 mEntityID = 3; // 0~2�� �÷��̾ ���

	// �ý��۵�
	unique_ptr<MovementSystem> mMovementSystem = nullptr;
	unique_ptr<ScriptSystem> mScriptSystem = nullptr;
	unique_ptr<EnemySystem> mEnemySystem = nullptr;
	unique_ptr<CombatSystem> mCombatSystem = nullptr;
	unique_ptr<CollisionSystem> mCollisionSystem = nullptr;
};

// Ÿ�� Ÿ�Կ� ���� y ��ġ ��ȯ
float GetTileYPos(TileType ttype);