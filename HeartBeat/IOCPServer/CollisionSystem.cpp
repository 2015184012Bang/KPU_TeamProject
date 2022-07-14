#include "pch.h"
#include "CollisionSystem.h"

#include "Box.h"
#include "Entity.h"
#include "Tags.h"
#include "Protocol.h"
#include "Values.h"
#include "Room.h"
#include "GameMap.h"
#include "Random.h"
#include "Timer.h"

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

	// ������ ��ü���� �ڽ��� ������Ʈ�Ѵ�.
	auto view = mRegistry.view<BoxComponent, MovementComponent, TransformComponent>();

	for (auto [entity, box, movement, transform] : view.each())
	{
		box.WorldBox = *box.LocalBox;
		box.WorldBox.Update(transform.Position, transform.Yaw);
	}

	// �÷��̾�� �ٸ� ���͵���� �浹�� �˻��Ѵ�.
	checkPlayersCollision();

	// ��ũ�� FAT Ÿ���� �浹�� �˻��Ѵ�.
	checkTankCollision();

	// �÷��̾ ���� ��� �κ��� ������� �˻��Ѵ�.
	checkPlayerOutOfBound();
}

tuple<bool, EntityType, UINT32> CollisionSystem::CheckAttackHit(const INT8 clientID)
{
	auto character = GetEntityByID(mRegistry, clientID);
	ASSERT(mRegistry.valid(character), "Invalid entity!");

	auto& transform = mRegistry.get<TransformComponent>(character);

	// ������ �õ��� �÷��̾��� Transform����
	// ��Ʈ�ڽ��� ������Ʈ�Ѵ�.
	Box hitbox = Box::GetBox("Hitbox");
	hitbox.Update(transform.Position, transform.Yaw);

	auto& combat = mRegistry.get<CombatComponent>(character);
	INT32 baseAttackDmg = combat.BaseAttackDmg;

	// ��Ʈ�ڽ��� ���̷����� �浹 üũ
	auto viruses = mRegistry.view<Tag_Virus, BoxComponent>();
	for (auto [entity, enemyBox] : viruses.each())
	{
		if (Intersects(hitbox, enemyBox.WorldBox))
		{
			auto& health = mRegistry.get<HealthComponent>(entity);
			health.Health -= baseAttackDmg;

			const auto victimID = mRegistry.get<IDComponent>(entity).ID;

			if (health.Health <= 0)
			{
				mOwner->SendDeleteEntityPacket(victimID, EntityType::VIRUS);
				DestroyEntity(mRegistry, entity);
			}

			return { true, EntityType::VIRUS, victimID };
		}
	}

	// ��Ʈ�ڽ� - ��
	auto dogs = mRegistry.view<Tag_Dog, BoxComponent>();
	for (auto [entity, enemyBox] : dogs.each())
	{
		if (Intersects(hitbox, enemyBox.WorldBox))
		{
			auto& health = mRegistry.get<HealthComponent>(entity);
			health.Health -= baseAttackDmg;

			const auto victimID = mRegistry.get<IDComponent>(entity).ID;

			if (health.Health <= 0)
			{
				mOwner->SendDeleteEntityPacket(victimID, EntityType::DOG);
				DestroyEntity(mRegistry, entity);
			}

			return { true, EntityType::DOG, victimID };
		}
	}

	// ��Ʈ�ڽ� - ����
	auto boss = mRegistry.view<Tag_Boss, BoxComponent>();
	for (auto [entity, enemyBox] : boss.each())
	{
		if (mRegistry.any_of<Tag_Invincible>(entity))
		{
			break;
		}

		if (Intersects(hitbox, enemyBox.WorldBox))
		{
			auto& health = mRegistry.get<HealthComponent>(entity);
			health.Health -= 1; // ��� ������ �������� 1�� ���ظ� �ش�.

			if (health.Health <= 0)
			{
				mOwner->DoBossDie();
			}

			const auto victimID = mRegistry.get<IDComponent>(entity).ID;
			return { true, EntityType::BOSS, victimID };
		}
	}

	// ������ ���� ������ (���ݷ� == Ÿ�� �ִ� ü��)
	INT32 tileAttackDmg = combat.BuffDuration > 0.0f ? Values::TileMaxHealth : 1;

	// ��Ʈ�ڽ��� �μ� �� �ִ� Ÿ�ϰ��� �浹 üũ
	auto tiles = mRegistry.view<Tag_BreakableTile, BoxComponent>();
	for (auto [entity, tileBox] : tiles.each())
	{
		if (Intersects(hitbox, tileBox.WorldBox))
		{
			auto& health = mRegistry.get<HealthComponent>(entity);
			health.Health -= tileAttackDmg;

			const auto victimID = mRegistry.get<IDComponent>(entity).ID;

			if (health.Health <= 0)
			{
				// �׷��� �� Ÿ�� Ÿ�� ����
				changeTileTypeInGraph(entity);

				// Ÿ���� �μ��� ���� Ȯ���� ������ ����(��Ÿ��, ī����)
				if (Random::RandInt(1, 10) <= Values::ItemDrop)
				{
					auto& transform = mRegistry.get<TransformComponent>(entity);
					createItem(transform.Position);
				}

				
				mOwner->SendDeleteEntityPacket(victimID, EntityType::FAT);
				DestroyEntity(mRegistry, entity);
			}

			return { true, EntityType::FAT, victimID };
		}
	}

	return { false, EntityType::END, 0 };
}

void CollisionSystem::DoWhirlwind(const INT8 clientID)
{
	// ������ ��ȿ����
	static float WHIRLWIND_RANGE_SQ = 800.0f * 800.0f;

	auto character = GetEntityByID(mRegistry, clientID);
	ASSERT(mRegistry.valid(character), "Invalid entity!");
	const auto& characterPosition = mRegistry.get<TransformComponent>(character).Position;

	auto viruses = mRegistry.view<Tag_Virus, TransformComponent>();
	for (auto [entity, transform] : viruses.each())
	{
		float distSq = Vector3::DistanceSquared(characterPosition, transform.Position);

		if (distSq < WHIRLWIND_RANGE_SQ)
		{
			const auto id = mRegistry.get<IDComponent>(entity).ID;
			mOwner->SendDeleteEntityPacket(id, EntityType::VIRUS);
			DestroyEntity(mRegistry, entity);
		}
	}

	auto dogs = mRegistry.view<Tag_Dog, TransformComponent>();
	for (auto [entity, transform] : dogs.each())
	{
		float distSq = Vector3::DistanceSquared(characterPosition, transform.Position);

		if (distSq < WHIRLWIND_RANGE_SQ)
		{
			const auto id = mRegistry.get<IDComponent>(entity).ID;
			mOwner->SendDeleteEntityPacket(id, EntityType::DOG);
			DestroyEntity(mRegistry, entity);
		}
	}

	auto boss = mRegistry.view<Tag_Boss, TransformComponent>();
	for (auto [entity, transform] : boss.each())
	{
		if (mRegistry.any_of<Tag_Invincible>(entity))
		{
			return;
		}

		float distSq = Vector3::DistanceSquared(characterPosition, transform.Position);

		if (transform.Position.x - characterPosition.x <= 1300.0f)
		{
			auto& health = mRegistry.get<HealthComponent>(entity);
			health.Health -= 1; // ��� ������ �������� 1�� ���ظ� �ش�.

			if (health.Health <= 0)
			{
				mOwner->DoBossDie();
			}
		}
	}
}

void CollisionSystem::Start()
{
	mbStart = true;
	mPlayState = GetEntityByName(mRegistry, "PlayState");
	ASSERT(mRegistry.valid(mPlayState), "Invalid entity!");
}

void CollisionSystem::Reset()
{
	mbStart = false;
}

void CollisionSystem::checkPlayersCollision()
{
	// �÷��̾� - Ÿ��
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

	// �÷��̾� - ��ũ
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

	// �÷��̾� - īƮ
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

	// �÷��̾� - ��
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

	// �÷��̾� - ��Ұ��޼�
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

	// �÷��̾� - ������
	auto items = mRegistry.view<Tag_Item, BoxComponent>();
	for (auto [pEnt, playerBox] : players.each())
	{
		for (auto [iEnt, itemBox] : items.each())
		{
			if (Intersects(playerBox.WorldBox, itemBox.WorldBox))
			{
				doItemUse(iEnt, pEnt);
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

	// Position�� �������Ƿ� �ڽ��� ������Ʈ ���ش�.
	playerBox.WorldBox = *playerBox.LocalBox;
	playerBox.WorldBox.Update(playerTransform.Position, playerTransform.Yaw);

	// Position�� �������Ƿ� ��Ƽ���� ��Ŷ�� �����ش�.
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

	// ���� ���������� 1.0 �� �����ش�.
	INT32 row = static_cast<INT32>((tilePosition.z + 1.0f) / Values::TileSide);
	INT32 col = static_cast<INT32>((tilePosition.x + 1.0f) / Values::TileSide);

	mOwner->ChangeTileToRoad(row, col);
}

void CollisionSystem::createItem(const Vector3& position)
{
	auto item = mRegistry.create();
	mRegistry.emplace<Tag_Item>(item);
	auto& id = mRegistry.emplace<IDComponent>(item, mOwner->GetEntityID());
	auto& transform = mRegistry.emplace<TransformComponent>(item);
	transform.Position = position;

	EntityType eType = EntityType::END;
	if (Random::RandInt(0, 1) == 0)
	{
		mRegistry.emplace<Tag_Vitamin>(item);
		mRegistry.emplace<BoxComponent>(item, &Box::GetBox("Sphere.box"),
			transform.Position, transform.Yaw);
		eType = EntityType::VITAMIN;
	}
	else
	{
		mRegistry.emplace<Tag_Caffeine>(item);
		mRegistry.emplace<BoxComponent>(item, &Box::GetBox("Sphere.box"),
			transform.Position, transform.Yaw);
		eType = EntityType::CAFFEINE;
	}

	NOTIFY_CREATE_ENTITY_PACKET packet = {};
	packet.EntityID = id.ID;
	packet.EntityType = static_cast<UINT8>(eType);
	packet.PacketID = NOTIFY_CREATE_ENTITY;
	packet.PacketSize = sizeof(packet);
	packet.Position = transform.Position;
	mOwner->Broadcast(packet.PacketSize, reinterpret_cast<char*>(&packet));
}

void CollisionSystem::doItemUse(const entt::entity item, const entt::entity player)
{
	EntityType itemType = mRegistry.any_of<Tag_Vitamin>(item) ?
		EntityType::VITAMIN :
		EntityType::CAFFEINE;

	const auto id = mRegistry.get<IDComponent>(item).ID;
	mOwner->SendDeleteEntityPacket(id, itemType);
	DestroyEntity(mRegistry, item);

	if (EntityType::VITAMIN == itemType)
	{
		mOwner->UpdateScore(10);
	}
	else
	{
		auto& health = mRegistry.get<HealthComponent>(player);
		++health.Health;
		if (health.Health > Values::PlayerHealth)
		{
			health.Health = Values::PlayerHealth;
		}
		mOwner->UpdatePlayerHpInState(health.Health, mRegistry.get<IDComponent>(player).ID);

		auto& combat = mRegistry.get<CombatComponent>(player);
		if (!combat.bEatCaffeine)
		{
			combat.bEatCaffeine = true;
			combat.BaseAttackDmg += 5;
		}

		Timer::AddEvent(5.0f, [this, player]() {
			if (mRegistry.valid(player))
			{
				backPlayerStatus(player);
			}
			});
	}
}

void CollisionSystem::backPlayerStatus(const entt::entity player)
{
	if (!mRegistry.valid(player))
	{
		return;
	}

	auto& combat = mRegistry.get<CombatComponent>(player);
	combat.bEatCaffeine = false;
	combat.BaseAttackDmg = GetBaseAttackDmg(combat.Preset);
}

void CollisionSystem::checkTankCollision()
{
	auto tank = GetEntityByName(mRegistry, "Tank");
	ASSERT(mRegistry.valid(tank), "Invalid entity!");
	auto& tankBox = mRegistry.get<BoxComponent>(tank);

	// FAT�� TANK_FAT�� BreakableTile �±װ� �پ� �ִ�.
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

INT32 GetBaseAttackDmg(UpgradePreset preset)
{
	switch (preset)
	{
	case UpgradePreset::ATTACK:
		return 3;

	case UpgradePreset::HEAL:
		return 1;

	case UpgradePreset::SUPPORT:
		return 2;

	default:
		ASSERT(false, "Unknown preset!");
		return 0;
	}
}
