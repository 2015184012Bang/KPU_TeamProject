#include "pch.h"
#include "CollisionSystem.h"

#include "Box.h"
#include "Entity.h"
#include "Tags.h"
#include "Protocol.h"
#include "Values.h"
#include "Room.h"
#include "GameMap.h"


CollisionSystem::CollisionSystem(entt::registry& registry, shared_ptr<Room>&& room)
	: mRegistry{ registry }
	, mOwner{ move(room) }
{
	
}

void CollisionSystem::Update()
{
	if (!mbStart)
	{
		return;
	}

	// 동적인 객체들의 박스를 업데이트한다.
	auto view = mRegistry.view<BoxComponent, MovementComponent, TransformComponent>();

	for (auto [entity, box, movement, transform] : view.each())
	{
		box.WorldBox = *box.LocalBox;
		box.WorldBox.Update(transform.Position, transform.Yaw);
	}

	// 플레이어와 다른 액터들과의 충돌을 검사한다.
	checkPlayersCollision();

	// 탱크와 FAT 타일의 충돌을 검사한다.
	checkTankCollision();

	// 플레이어가 맵의 경계 부분을 벗어났는지 검사한다.
	checkPlayerOutOfBound();
}

bool CollisionSystem::CheckAttackHit(const INT8 clientID)
{
	auto character = GetEntityByID(mRegistry, clientID);
	ASSERT(mRegistry.valid(character), "Invalid entity!");

	auto& transform = mRegistry.get<TransformComponent>(character);
	
	// 공격을 시도한 플레이어의 Transform으로
	// 히트박스를 업데이트한다.
	Box hitbox = Box::GetBox("Hitbox");
	hitbox.Update(transform.Position, transform.Yaw);

	// 히트박스와 바이러스의 충돌 체크
	auto viruses = mRegistry.view<Tag_Virus, BoxComponent>();
	for (auto [entity, enemyBox] : viruses.each())
	{
		if (Intersects(hitbox, enemyBox.WorldBox))
		{
			auto& health = mRegistry.get<HealthComponent>(entity);
			--health.Health;

			if (health.Health <= 0)
			{
				NOTIFY_DELETE_ENTITY_PACKET packet = {};
				packet.EntityID = mRegistry.get<IDComponent>(entity).ID;
				packet.EntityType = static_cast<UINT8>(EntityType::VIRUS);
				packet.PacketID = NOTIFY_DELETE_ENTITY;
				packet.PacketSize = sizeof(packet);
				mOwner->Broadcast(sizeof(packet), reinterpret_cast<char*>(&packet));

				DestroyEntity(mRegistry, entity);
			}

			return true;
		}
	}

	// 히트박스 - 개
	auto dogs = mRegistry.view<Tag_Dog, BoxComponent>();
	for (auto [entity, enemyBox] : dogs.each())
	{
		if (Intersects(hitbox, enemyBox.WorldBox))
		{
			auto& health = mRegistry.get<HealthComponent>(entity);
			--health.Health;

			if (health.Health <= 0)
			{
				NOTIFY_DELETE_ENTITY_PACKET packet = {};
				packet.EntityID = mRegistry.get<IDComponent>(entity).ID;
				packet.EntityType = static_cast<UINT8>(EntityType::DOG);
				packet.PacketID = NOTIFY_DELETE_ENTITY;
				packet.PacketSize = sizeof(packet);
				mOwner->Broadcast(sizeof(packet), reinterpret_cast<char*>(&packet));

				DestroyEntity(mRegistry, entity);
			}

			return true;
		}
	}

	// 버프가 켜져 있으면 타일 공격력 5(지방 타일의 최대 체력)
	auto& combat = mRegistry.get<CombatComponent>(character);
	INT32 tileAttackDmg = combat.BuffDuration > 0.0f ? 5 : 1;

	// 히트박스와 부술 수 있는 타일과의 충돌 체크
	auto tiles = mRegistry.view<Tag_BreakableTile, BoxComponent>();
	for (auto [entity, tileBox] : tiles.each())
	{
		if (Intersects(hitbox, tileBox.WorldBox))
		{
			auto& health = mRegistry.get<HealthComponent>(entity);
			health.Health -= tileAttackDmg;

			if (health.Health <= 0)
			{
				// 그래프 속 타일 타입 갱신
				changeTileTypeInGraph(entity);

				// 엔티티 삭제 패킷 송신
				NOTIFY_DELETE_ENTITY_PACKET packet = {};
				packet.PacketID = NOTIFY_DELETE_ENTITY;
				packet.PacketSize = sizeof(packet);
				packet.EntityID = mRegistry.get<IDComponent>(entity).ID;
				packet.EntityType = static_cast<UINT8>(EntityType::FAT);
				mOwner->Broadcast(sizeof(packet), reinterpret_cast<char*>(&packet));

				// 레지스트리에서 엔티티 제거
				DestroyEntity(mRegistry, entity);
			}

			return true;
		}
	}

	return false;
}

void CollisionSystem::DoWhirlwind(const INT8 clientID)
{
	// 휠윈드 유효범위
	static float WHIRLWIND_RANGE_SQ = 600.0f * 600.0f;

	auto character = GetEntityByID(mRegistry, clientID);
	ASSERT(mRegistry.valid(character), "Invalid entity!");
	const auto& characterPosition = mRegistry.get<TransformComponent>(character).Position;

	auto viruses = mRegistry.view<Tag_Virus, TransformComponent>();
	for (auto [entity, transform] : viruses.each())
	{
		float distSq = Vector3::DistanceSquared(characterPosition, transform.Position);

		if (distSq < WHIRLWIND_RANGE_SQ)
		{
			NOTIFY_DELETE_ENTITY_PACKET packet = {};
			packet.EntityID = mRegistry.get<IDComponent>(entity).ID;
			packet.EntityType = static_cast<UINT8>(EntityType::VIRUS);
			packet.PacketID = NOTIFY_DELETE_ENTITY;
			packet.PacketSize = sizeof(packet);
			mOwner->Broadcast(sizeof(packet), reinterpret_cast<char*>(&packet));

			DestroyEntity(mRegistry, entity);
		}
	}

	auto dogs = mRegistry.view<Tag_Dog, TransformComponent>();
	for (auto [entity, transform] : dogs.each())
	{
		float distSq = Vector3::DistanceSquared(characterPosition, transform.Position);

		if (distSq < WHIRLWIND_RANGE_SQ)
		{
			NOTIFY_DELETE_ENTITY_PACKET packet = {};
			packet.EntityID = mRegistry.get<IDComponent>(entity).ID;
			packet.EntityType = static_cast<UINT8>(EntityType::DOG);
			packet.PacketID = NOTIFY_DELETE_ENTITY;
			packet.PacketSize = sizeof(packet);
			mOwner->Broadcast(sizeof(packet), reinterpret_cast<char*>(&packet));

			DestroyEntity(mRegistry, entity);
		}
	}
}

void CollisionSystem::checkPlayersCollision()
{
	// 플레이어 - 타일
	auto players = mRegistry.view<Tag_Player, BoxComponent>();
	auto blockingTiles = mRegistry.view<Tag_BlockingTile, BoxComponent>();
	for (auto [pEnt, playerBox] : players.each())
	{
		for (auto [tEnt, tileBox] : blockingTiles.each())
		{
			if (Intersects(playerBox.WorldBox, tileBox.WorldBox))
			{
				reposition(playerBox, pEnt, tileBox);
				break;
			}
		}
	}

	// 플레이어 - 탱크
	auto tank = GetEntityByName(mRegistry, "Tank");
	ASSERT(mRegistry.valid(tank), "Invalid entity!");
	auto& tankBox = mRegistry.get<BoxComponent>(tank);
	for (auto [pEnt, playerBox] : players.each())
	{
		if (Intersects(playerBox.WorldBox, tankBox.WorldBox))
		{
			reposition(playerBox, pEnt, tankBox);
		}
	}

	// 플레이어 - 카트
	auto cart = GetEntityByName(mRegistry, "Cart");
	ASSERT(mRegistry.valid(cart), "Invalid entity!");
	auto& cartBox = mRegistry.get<BoxComponent>(cart);
	for (auto [pEnt, playerBox] : players.each())
	{
		if (Intersects(playerBox.WorldBox, cartBox.WorldBox))
		{
			reposition(playerBox, pEnt, cartBox);
		}
	}

	// 플레이어 - 적
	auto enemies = mRegistry.view<Tag_Enemy, BoxComponent>();
	for (auto [pEnt, playerBox] : players.each())
	{
		for (auto [eEnt, enemyBox] : enemies.each())
		{
			if (Intersects(playerBox.WorldBox, enemyBox.WorldBox))
			{
				reposition(playerBox, pEnt, enemyBox);
			}
		}
	}

	// 플레이어 - 산소공급소
	auto houses = mRegistry.view<Tag_HouseTile, BoxComponent>();
	for (auto [pEnt, playerBox] : players.each())
	{
		for (auto [hEnt, houseBox] : houses.each())
		{
			if (Intersects(playerBox.WorldBox, houseBox.WorldBox))
			{
				reposition(playerBox, pEnt, houseBox);
			}
		}
	}
}

void CollisionSystem::reposition(BoxComponent& playerBox, entt::entity player, BoxComponent& otherBox)
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

	auto& playerTransform = mRegistry.get<TransformComponent>(player);

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
	packet.EntityID = mRegistry.get<IDComponent>(player).ID;
	packet.Direction = mRegistry.get<MovementComponent>(player).Direction;
	packet.Position = playerTransform.Position;

	mOwner->Broadcast(sizeof(packet), reinterpret_cast<char*>(&packet));
}

void CollisionSystem::changeTileTypeInGraph(entt::entity tile)
{
	auto& tilePosition = mRegistry.get<TransformComponent>(tile).Position;
	
	// 오차 방지용으로 1.0 을 더해준다.
	INT32 row = static_cast<INT32>((tilePosition.z + 1.0f) / Values::TileSide);
	INT32 col = static_cast<INT32>((tilePosition.x + 1.0f) / Values::TileSide);

	mOwner->ChangeTileToRoad(row, col);
}

void CollisionSystem::checkTankCollision()
{
	auto tank = GetEntityByName(mRegistry, "Tank");
	ASSERT(mRegistry.valid(tank), "Invalid entity!");
	auto& tankBox = mRegistry.get<BoxComponent>(tank);

	// FAT과 TANK_FAT엔 BreakableTile 태그가 붙어 있다.
	auto tiles = mRegistry.view<Tag_BreakableTile, BoxComponent>();
	for (auto [entity, tileBox] : tiles.each())
	{
		if (Intersects(tankBox.WorldBox, tileBox.WorldBox))
		{
			mOwner->DoGameOver();
			break;
		}
	}
}

void CollisionSystem::checkPlayerOutOfBound()
{
	if (mBorder == Vector3::Zero)
	{
		return;
	}

	auto view = mRegistry.view<Tag_Player, TransformComponent>();

	for (auto [entity, transform] : view.each())
	{
		auto& position = transform.Position;
		bool bOut = false;

		if (position.x < 0) { position.x = 0; bOut = true; }
		else if (position.x > mBorder.x) { position.x = mBorder.x; bOut = true; }

		if (position.z < 0) { position.z = 0; bOut = true; }
		else if (position.z > mBorder.z) { position.z = mBorder.z; bOut = true; }

		if (bOut)
		{
			NOTIFY_MOVE_PACKET packet = {};
			packet.Direction = mRegistry.get<MovementComponent>(entity).Direction;
			packet.EntityID = mRegistry.get<IDComponent>(entity).ID;
			packet.PacketID = NOTIFY_MOVE;
			packet.PacketSize = sizeof(packet);
			packet.Position = position;

			mOwner->Broadcast(sizeof(packet), reinterpret_cast<char*>(&packet));
		}
	}
}
