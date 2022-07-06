#pragma once

class Room;

class MovementSystem
{
public:
	MovementSystem(entt::registry& registry, shared_ptr<Room>&& room);

	void Update();

	void SetDirection(const INT8 clientID, const Vector3& direction);

	void SendNotifyMovePackets();

	void Start();

private:
	void checkMidPoint();
	void checkBattleTrigger();
	void checkBossTrigger();
	void generateEnemyBossBattle();

	entt::entity getClosestDoor(const Vector3& midPointPos);

private:
	entt::registry& mRegistry;
	shared_ptr<Room> mOwner = nullptr;

	bool mbBattleProgressed = false;
	bool mbBossProgressed = false;
	bool mbBossBattleEnemyGen = false;
	float mEnemyGenTracker = 0.0f;

	INT32 mNumRedCells = -1;
};

