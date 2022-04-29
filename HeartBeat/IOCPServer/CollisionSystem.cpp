#include "pch.h"
#include "CollisionSystem.h"

#include "Box.h"
#include "Entity.h"
#include "Tags.h"

void CollisionSystem::Update()
{
	// ������ ��ü���� �ڽ��� ������Ʈ�Ѵ�.
	auto view = gRegistry.view<BoxComponent, MovementComponent, TransformComponent>();

	for (auto [entity, box, movement, transform] : view.each())
	{
		if (movement.Direction == Vector3::Zero)
		{
			continue;
		}

		box.WorldBox = *box.LocalBox;
		box.WorldBox.Update(transform.Position, transform.Yaw);
	}

	// �÷��̾�� Ÿ���� �浹�� �˻��Ѵ�.
	checkPlayerAndTile();
}

void CollisionSystem::TestCollision()
{
	auto p2 = GetEntityByName("Player2");
	auto p1 = GetEntityByName("Player1");

	if (p2 && p1)
	{
		auto& p2Box = p2.GetComponent<BoxComponent>();
		auto& p1Box = p1.GetComponent<BoxComponent>();

		if (Intersects(p2Box.WorldBox, p1Box.WorldBox))
		{
			LOG("Collision Occured!");
		}
	}
}

void CollisionSystem::checkPlayerAndTile()
{
	auto players = gRegistry.view<Tag_Player, BoxComponent>();
	auto blockingTiles = gRegistry.view<Tag_Blocked, BoxComponent>();

	for (auto [pEnt, playerBox] : players.each())
	{
		for (auto [tEnt, tileBox] : blockingTiles.each())
		{
			if (Intersects(playerBox.WorldBox, tileBox.WorldBox))
			{
				LOG("Collision!");
			}
		}
	}
}
