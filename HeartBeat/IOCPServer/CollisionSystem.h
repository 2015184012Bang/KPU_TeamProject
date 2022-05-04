#pragma once

#include "Entity.h"

class GameManager;

class CollisionSystem
{
public:
	CollisionSystem(shared_ptr<GameManager>&& gm);

	void Update();

	bool DoAttack(const INT32 sessionIndex);

	void SetStart(bool value) { mbStart = value; }

private:
	void checkPlayersCollision();
	void checkTankCollision();
	void reposition(BoxComponent& playerBox, Entity&& player, BoxComponent& otherBox);

private:
	shared_ptr<GameManager> mGameManager = nullptr;

	// 충돌 처리 시작 여부
	bool mbStart = false;
};

