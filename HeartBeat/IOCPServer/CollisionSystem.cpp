#include "pch.h"
#include "CollisionSystem.h"

#include "Box.h"
#include "Entity.h"
#include "Tags.h"
#include "Protocol.h"
#include "Values.h"
#include "Room.h"


CollisionSystem::CollisionSystem(entt::registry& registry, shared_ptr<Room>&& room)
	: mRegistry{ registry }
	, mOwner{ move(room) }
{
	// �÷��̾ ������ �� ����� ��Ʈ�ڽ� ����
	Box hitbox;
	hitbox.SetMin(Vector3{ -100.0f, 0.0f, 0.0f });
	hitbox.SetMax(Vector3{ 100.0f, 0.0f, Values::BaseAttackRange });
	Box::SetBox(hitbox, "Hitbox");
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

bool CollisionSystem::DoAttack(const INT32 clientID)
{
	auto character = GetEntityByID(mRegistry, clientID);
	ASSERT(mRegistry.valid(character), "Invalid entity!");

	auto& transform = mRegistry.get<TransformComponent>(character);

	Box hitbox = Box::GetBox("Hitbox");
	hitbox.Update(transform.Position, transform.Yaw);

	auto tiles = mRegistry.view<Tag_BreakableTile, BoxComponent>();
	for (auto [entity, tileBox] : tiles.each())
	{
		if (Intersects(hitbox, tileBox.WorldBox))
		{
			auto& health = mRegistry.get<HealthComponent>(entity);
			--health.Health;

			if (health.Health <= 0)
			{
				// ��ƼƼ ���� ��Ŷ �۽�
				NOTIFY_DELETE_ENTITY_PACKET packet = {};
				packet.PacketID = NOTIFY_DELETE_ENTITY;
				packet.PacketSize = sizeof(packet);
				packet.EntityID = mRegistry.get<IDComponent>(entity).ID;
				packet.EntityType = static_cast<UINT8>(EntityType::FAT);
				mOwner->Broadcast(sizeof(packet), reinterpret_cast<char*>(&packet));

				// ������Ʈ������ ��ƼƼ ����
				DestroyEntity(mRegistry, entity);
			}

			return true;
		}
	}

	return false;
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
			//mGameManager->DoGameOver();
			break;
		}
	}

	// TODO : ��ũ - �� �浹 Ž��&ó��
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
