#pragma once

#include "Components.h"

class Room;

class CollisionSystem
{
public:
	CollisionSystem(entt::registry& registry, shared_ptr<Room>&& room);

	void Update();

	bool DoAttack(const INT8 clientID);

	void SetStart(bool value) { mbStart = value; }
	
	void SetBorder(const Vector3& border) { mBorder = border; }

private:
	void checkPlayersCollision();
	void checkTankCollision();
	void checkPlayerOutOfBound();

	void reposition(BoxComponent& playerBox, entt::entity player, BoxComponent& otherBox);

	void changeTileTypeInGraph(entt::entity tile);

private:
	entt::registry& mRegistry;
	shared_ptr<Room> mOwner = nullptr;

	// �浹 ó�� ���� ����
	bool mbStart = false;

	// �� ���
	Vector3 mBorder = Vector3::Zero;
};

