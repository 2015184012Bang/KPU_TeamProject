#pragma once

#include "Components.h"

class Room;

class CollisionSystem
{
	friend class Timer;

public:
	CollisionSystem(entt::registry& registry, shared_ptr<Room>&& room);

	void Update();

	tuple<bool, EntityType, UINT32> CheckAttackHit(const INT8 clientID);

	void DoWhirlwind(const INT8 clientID);

	void Start();

	void Reset();
	
	void SetBorder(const Vector3& border) { mBorder = border; }

private:
	void checkPlayersCollision();
	void checkTankCollision();
	void checkPlayerOutOfBound();

	void reposition(BoxComponent& playerBox, entt::entity player, BoxComponent& otherBox);

	void changeTileTypeInGraph(entt::entity tile);

	void createItem(const Vector3& position);

	void doItemUse(const entt::entity item, const entt::entity player);

	// 카페인 섭취로 증가한 플레이어 능력 원상복귀
	void backPlayerStatus(const entt::entity player);

private:
	entt::registry& mRegistry;
	shared_ptr<Room> mOwner = nullptr;

	// 충돌 처리 시작 여부
	bool mbStart = false;

	// 맵 경계
	Vector3 mBorder = Vector3::Zero;

	entt::entity mPlayState = entt::null;
};

INT32 GetBaseAttackDmg(UpgradePreset preset);