#pragma once

#include "Entity.h"

class GameManager;

class CollisionSystem
{
public:
	CollisionSystem(shared_ptr<GameManager>&& gm);

	void Update();

	bool DoAttack(const INT32 sessionIndex);

private:
	void checkPlayerAndTile();
	void reposition(BoxComponent& playerBox, Entity&& player, BoxComponent& otherBox);

private:
	shared_ptr<GameManager> mGameManager = nullptr;
};

