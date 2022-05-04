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
	
	void SetBorder(const Vector3& border) { mBorder = border; }

private:
	void checkPlayersCollision();
	void checkTankCollision();
	void checkPlayerOutOfBound();

	void reposition(BoxComponent& playerBox, Entity&& player, BoxComponent& otherBox);

private:
	shared_ptr<GameManager> mGameManager = nullptr;

	// �浹 ó�� ���� ����
	bool mbStart = false;

	// �� ���
	Vector3 mBorder = Vector3::Zero;
};

