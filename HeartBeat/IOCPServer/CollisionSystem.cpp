#include "pch.h"
#include "CollisionSystem.h"

#include "Box.h"
#include "Entity.h"
#include "Tags.h"
#include "Protocol.h"
#include "GameManager.h"
#include "Values.h"

CollisionSystem::CollisionSystem(shared_ptr<GameManager>&& gm)
	: mGameManager(move(gm))
{
	// 플레이어가 공격할 때 사용할 히트박스 생성
	Box hitbox;
	hitbox.SetMin(Vector3{ -100.0f, 0.0f, 0.0f });
	hitbox.SetMax(Vector3{ 100.0f, 0.0f, Values::BaseAttackRange });
	Box::SetBox(hitbox, "Hitbox");
}

void CollisionSystem::Update()
{
	// 동적인 객체들의 박스를 업데이트한다.
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

	// 플레이어와 타일의 충돌을 검사한다.
	checkPlayerAndTile();
}

bool CollisionSystem::DoAttack(const INT32 sessionIndex)
{
	Entity character = GetEntity(sessionIndex);
	auto& transform = character.GetComponent<TransformComponent>();

	Box hitbox = Box::GetBox("Hitbox");
	hitbox.Update(transform.Position, transform.Yaw);

	auto tiles = gRegistry.view<Tag_BreakableTile, BoxComponent>();
	for (auto [entity, tileBox] : tiles.each())
	{
		if (Intersects(hitbox, tileBox.WorldBox))
		{
			Entity tile = Entity{ entity };
			auto& health = tile.GetComponent<HealthComponent>();
			--health.Health;

			if (health.Health <= 0)
			{
				// 엔티티 삭제 패킷 송신
				NOTIFY_DELETE_ENTITY_PACKET packet = {};
				packet.PacketID = NOTIFY_DELETE_ENTITY;
				packet.PacketSize = sizeof(packet);
				packet.EntityID = tile.GetComponent<IDComponent>().ID;
				packet.EntityType = static_cast<UINT8>(EntityType::FAT);
				mGameManager->SendToAll(sizeof(packet), reinterpret_cast<char*>(&packet));

				// 레지스트리에서 엔티티 제거
				gRegistry.destroy(entity);
			}

			return true;
		}
	}

	return false;
}

void CollisionSystem::checkPlayerAndTile()
{
	auto players = gRegistry.view<Tag_Player, BoxComponent>();
	auto blockingTiles = gRegistry.view<Tag_BlockingTile, BoxComponent>();

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

	// Position이 변했으므로 박스도 업데이트 해준다.
	playerBox.WorldBox = *playerBox.LocalBox;
	playerBox.WorldBox.Update(playerTransform.Position, playerTransform.Yaw);

	// Position이 변했으므로 노티파이 패킷을 날려준다.
	NOTIFY_MOVE_PACKET packet = {};
	packet.PacketID = NOTIFY_MOVE;
	packet.PacketSize = sizeof(packet);
	packet.EntityID = player.GetComponent<IDComponent>().ID;
	packet.Direction = player.GetComponent<MovementComponent>().Direction;
	packet.Position = playerTransform.Position;

	mGameManager->SendToAll(sizeof(packet), reinterpret_cast<char*>(&packet));
}
