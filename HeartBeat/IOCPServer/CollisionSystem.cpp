#include "pch.h"
#include "CollisionSystem.h"

#include "Box.h"
#include "Entity.h"
#include "Tags.h"
#include "Protocol.h"
#include "GameManager.h"

CollisionSystem::CollisionSystem(shared_ptr<GameManager>&& gm)
	: mGameManager(move(gm))
{
	
}

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
				reposition(playerBox, Entity{ pEnt }, tileBox);
				break;
			}
		}
	}
}

void CollisionSystem::reposition(BoxComponent& playerBox, Entity&& player, BoxComponent& otherBox)
{
	const auto& playerMin = playerBox.WorldBox.GetMin();
	const auto& playerMax = playerBox.WorldBox.GetMax();

	const auto& otherMin = otherBox.WorldBox.GetMin();
	const auto& otherMax = otherBox.WorldBox.GetMax();

	float dx1 = otherMax.x - playerMin.x;
	float dx2 = otherMin.x - playerMax.x;
	float dz1 = otherMax.z - playerMin.z;
	float dz2 = otherMin.z - playerMax.z;

	float dx = abs(dx1) < abs(dx2) ? dx1 : dx2;
	float dz = abs(dz1) < abs(dz2) ? dz1 : dz2;

	auto& playerTransform = player.GetComponent<TransformComponent>();

	if (abs(dx) <= abs(dz))
	{
		playerTransform.Position.x += dx * 2.5f;
	}
	else
	{
		playerTransform.Position.z += dz * 2.5f;
	}

	// Position�� �������Ƿ� �ڽ��� ������Ʈ ���ش�.
	playerBox.WorldBox = *playerBox.LocalBox;
	playerBox.WorldBox.Update(playerTransform.Position, playerTransform.Yaw);

	// Position�� �������Ƿ� ��Ƽ���� ��Ŷ�� �����ش�.
	NOTIFY_MOVE_PACKET packet = {};
	packet.PacketID = NOTIFY_MOVE;
	packet.PacketSize = sizeof(packet);
	packet.EntityID = player.GetComponent<IDComponent>().ID;
	packet.Direction = player.GetComponent<MovementComponent>().Direction;
	packet.Position = playerTransform.Position;

	mGameManager->SendToAll(sizeof(packet), reinterpret_cast<char*>(&packet));
}