#include "pch.h"
#include "MovementSystem.h"

#include "Entity.h"
#include "Timer.h"
#include "Tags.h"
#include "Random.h"
#include "Room.h"

MovementSystem::MovementSystem(entt::registry& registry, shared_ptr<Room>&& room)
	: mRegistry{ registry }
	, mOwner{ move(room) }
{

}

void MovementSystem::Update()
{
	auto view = mRegistry.view<MovementComponent, TransformComponent>(entt::exclude<Tag_Stop>);

	for (auto [entity, movement, transform] : view.each())
	{
		transform.Position += movement.Direction * movement.Speed * Timer::GetDeltaTime();
	}

	checkMidPoint();
	checkBattleTrigger();
	checkBossTrigger();

	SendNotifyMovePackets();
}

void MovementSystem::SetDirection(const INT8 clientID, const Vector3& direction)
{
	// 아이디에 해당하는 액터를 가져온다.
	auto actor = GetEntityByID(mRegistry, clientID);
	ASSERT(mRegistry.valid(actor), "Invalid entity!");

	auto& movement = mRegistry.get<MovementComponent>(actor);
	movement.Direction = direction;

	// 방향이 0이면 yaw를 업데이트할 필요 없으므로 리턴.
	if (direction == Vector3::Zero)
	{
		return;
	}

	// 액터의 전방(0, 0, 1)과 방향 사이의 각도를 구한다.
	// 각도는 0~180도까지만 구해지므로, -x축 방향으로 이동할 때는
	// 구한 각에 -1을 곱해줘야 한다.
	Vector3 rotation = XMVector3AngleBetweenVectors(Vector3::UnitZ, direction);
	float scalar = 1.0f;

	if (direction.x < 0.0f)
	{
		scalar = -1.0f;
	}

	auto& transform = mRegistry.get<TransformComponent>(actor);
	transform.Yaw = scalar * XMConvertToDegrees(rotation.y);
}

void MovementSystem::SendNotifyMovePackets()
{
	auto tank = GetEntityByName(mRegistry, "Tank");

	NOTIFY_MOVE_PACKET packet = {};
	packet.PacketID = NOTIFY_MOVE;
	packet.PacketSize = sizeof(packet);

	// 탱크 이동 패킷 보내기
	if (mRegistry.valid(tank))
	{
		packet.EntityID = mRegistry.get<IDComponent>(tank).ID;
		packet.Direction = mRegistry.get<MovementComponent>(tank).Direction;
		packet.Position = mRegistry.get<TransformComponent>(tank).Position;
		mOwner->Broadcast(sizeof(packet), reinterpret_cast<char*>(&packet));
	}

	// 카트 이동 패킷 보내기
	auto cart = GetEntityByName(mRegistry, "Cart");
	if (mRegistry.valid(cart))
	{
		packet.EntityID = mRegistry.get<IDComponent>(cart).ID;
		packet.Direction = mRegistry.get<MovementComponent>(cart).Direction;
		packet.Position = mRegistry.get<TransformComponent>(cart).Position;
		mOwner->Broadcast(sizeof(packet), reinterpret_cast<char*>(&packet));
	}

	// 적 이동 패킷 보내기
	auto enemies = mRegistry.view<Tag_Enemy>();
	for (auto entity : enemies)
	{
		packet.EntityID = mRegistry.get<IDComponent>(entity).ID;
		packet.Direction = mRegistry.get<MovementComponent>(entity).Direction;
		packet.Position = mRegistry.get<TransformComponent>(entity).Position;
		mOwner->Broadcast(sizeof(packet), reinterpret_cast<char*>(&packet));
	}

	// 적혈구 이동 패킷 보내기
	auto redCells = mRegistry.view<Tag_RedCell>();
	for (auto entity : redCells)
	{
		packet.EntityID = mRegistry.get<IDComponent>(entity).ID;
		packet.Direction = mRegistry.get<MovementComponent>(entity).Direction;
		packet.Position = mRegistry.get<TransformComponent>(entity).Position;
		mOwner->Broadcast(sizeof(packet), reinterpret_cast<char*>(&packet));
	}
}

void MovementSystem::Start()
{
	mbBattleProgressed = false;
	mbBossProgressed = false;
	mNumRedCells = -1;

	// START_POINT 타일 가져오기
	auto startTile = GetEntityByName(mRegistry, "StartPoint");
	ASSERT(mRegistry.valid(startTile), "There is no start point.");
	const auto& startPos = mRegistry.get<TransformComponent>(startTile).Position;

	// 플레이어들을 시작 위치로 초기화
	auto view = mRegistry.view<Tag_Player, TransformComponent, IDComponent>();
	for (auto [entity, transform, id] : view.each())
	{
		transform.Position = Vector3{
			startPos.x + id.ID * 300.0f,
			transform.Position.y,
			startPos.z + 800.f
		};

		NOTIFY_MOVE_PACKET packet = {};
		packet.Direction = mRegistry.get<MovementComponent>(entity).Direction;
		packet.EntityID = id.ID;
		packet.PacketID = NOTIFY_MOVE;
		packet.PacketSize = sizeof(packet);
		packet.Position = transform.Position;

		mOwner->Broadcast(sizeof(packet), reinterpret_cast<char*>(&packet));
	}
}

entt::entity MovementSystem::getClosestDoor(const Vector3& midPointPos)
{
	auto doors = mRegistry.view<Tag_Door>();

	float minDistance = FLT_MAX;
	entt::entity cloestDoor = entt::null;
	for (auto door : doors)
	{
		const auto& doorPos = mRegistry.get<TransformComponent>(door).Position;
		
		float dist = fabs(doorPos.x - midPointPos.x);

		if (dist < minDistance)
		{
			minDistance = dist;
			cloestDoor = door;
		}
	}

	return cloestDoor;
}

void MovementSystem::checkMidPoint()
{
	auto tank = GetEntityByName(mRegistry, "Tank");
	if (!mRegistry.valid(tank))
	{
		return;
	}

	auto midPoints = mRegistry.view<Tag_MidPoint>();

	for (auto midPoint : midPoints)
	{
		const auto& tankPos = mRegistry.get<TransformComponent>(tank).Position;
		auto midPos = mRegistry.get<TransformComponent>(midPoint).Position;
		midPos.y = 0.0f;

		float dist = Vector3::DistanceSquared(tankPos, midPos);

		if (dist < 100.0f)
		{
			auto cart = GetEntityByName(mRegistry, "Cart");

			mRegistry.emplace<Tag_Stop>(tank);
			mRegistry.emplace<Tag_Stop>(cart);

			mOwner->SendEventOccurPacket(0, EventType::DOOR_DOWN);

			mRegistry.remove<Tag_MidPoint>(midPoint);

			auto door = getClosestDoor(midPos);

			Timer::AddEvent(5.0f, [this, tank, cart, door]()
				{
					if (mRegistry.valid(tank) &&
						mRegistry.valid(cart))
					{
						mRegistry.remove<Tag_Stop>(tank);
						mRegistry.remove<Tag_Stop>(cart);
						DestroyEntity(mRegistry, door);
					}
				});

			// 적혈구 3체 추가
			NOTIFY_CREATE_ENTITY_PACKET createPacket = {};
			createPacket.PacketSize = sizeof(createPacket);
			createPacket.PacketID = NOTIFY_CREATE_ENTITY;
			const auto& cartPosition = mRegistry.get<TransformComponent>(cart).Position;
			auto cellPosition = Vector3{ cartPosition.x - 400.0f, cartPosition.y, cartPosition.z };
			for (int i = 0; i < 3; ++i)
			{
				cellPosition.z += 100.0f * i;
				auto entityID = mOwner->CreateCell(cellPosition);
				createPacket.EntityID = entityID;
				createPacket.EntityType = static_cast<UINT8>(EntityType::RED_CELL);
				createPacket.Position = cellPosition;
				mOwner->Broadcast(createPacket.PacketSize, reinterpret_cast<char*>(&createPacket));
			}

			break;
		}
	}
}

void MovementSystem::checkBattleTrigger()
{
	if (mbBattleProgressed)
	{
		return;
	}

	auto tank = GetEntityByName(mRegistry, "Tank");
	if (!mRegistry.valid(tank))
	{
		return;
	}

	auto trigger = GetEntityByName(mRegistry, "BattleTrigger");
	if (!mRegistry.valid(trigger))
	{
		return;
	}

	const auto& tankPos = mRegistry.get<TransformComponent>(tank).Position;
	auto triggerPos = mRegistry.get<TransformComponent>(trigger).Position;
	triggerPos.y = 0.0f;

	float distSq = Vector3::DistanceSquared(tankPos, triggerPos);

	if (distSq < 100.0f)
	{
		mbBattleProgressed = true;

		auto cart = GetEntityByName(mRegistry, "Cart");
		const auto& cartPos = mRegistry.get<TransformComponent>(cart).Position;

		mRegistry.emplace<Tag_Stop>(tank);
		mRegistry.emplace<Tag_Stop>(cart);

		auto redCells = mRegistry.view<Tag_RedCell>();
		mNumRedCells = static_cast<INT32>(redCells.size());
		for (auto entity : redCells)
		{
			DestroyEntity(mRegistry, entity);
		}

		mOwner->SendEventOccurPacket(0, EventType::BATTLE);

		auto centerPos = (tankPos + cartPos) / 2.0f;
		Timer::AddEvent(10.5f, [this, centerPos]() {
			if (mOwner->GetState() == Room::RoomState::Playing)
			{
				NOTIFY_CREATE_ENTITY_PACKET packet = {};
				packet.PacketID = NOTIFY_CREATE_ENTITY;
				packet.PacketSize = sizeof(packet);

				// 탱크 앞 백혈구
				for (int i = 0; i < 3; ++i)
				{
					auto cellPos = centerPos;
					cellPos.x += 1200.0f;
					cellPos.z += (i - 1) * 300.0f;
					auto eid = mOwner->CreateCell(cellPos, true);

					packet.EntityID = eid;
					packet.EntityType = static_cast<UINT8>(EntityType::WHITE_CELL);
					packet.Position = cellPos;
					mOwner->Broadcast(packet.PacketSize, reinterpret_cast<char*>(&packet));
				}

				// 탱크 뒤 백혈구
				for (int i = 0; i < 3; ++i)
				{
					auto cellPos = centerPos;
					cellPos.x -= 1200.0f;
					cellPos.z += (i - 1) * 300.0f;
					auto eid = mOwner->CreateCell(cellPos, true);

					packet.EntityID = eid;
					packet.EntityType = static_cast<UINT8>(EntityType::WHITE_CELL);
					packet.Position = cellPos;
					mOwner->Broadcast(packet.PacketSize, reinterpret_cast<char*>(&packet));
				}

				// 탱크 왼쪽 백혈구
				for (int i = 0; i < 6; ++i)
				{
					auto cellPos = centerPos;
					cellPos.x += (i * 300.0f) - 800.0f;
					cellPos.z += 800.0f;
					auto eid = mOwner->CreateCell(cellPos, true);

					packet.EntityID = eid;
					packet.EntityType = static_cast<UINT8>(EntityType::WHITE_CELL);
					packet.Position = cellPos;
					mOwner->Broadcast(packet.PacketSize, reinterpret_cast<char*>(&packet));
				}

				// 탱크 오른쪽 백혈구
				for (int i = 0; i < 6; ++i)
				{
					auto cellPos = centerPos;
					cellPos.x += (i * 300.0f) - 800.0f;
					cellPos.z -= 800.0f;
					auto eid = mOwner->CreateCell(cellPos, true);

					packet.EntityID = eid;
					packet.EntityType = static_cast<UINT8>(EntityType::WHITE_CELL);
					packet.Position = cellPos;
					mOwner->Broadcast(packet.PacketSize, reinterpret_cast<char*>(&packet));
				}
			}
			});

		Timer::AddEvent(11.0f, [this, centerPos]() {
			mOwner->GenerateEnemyRandomly(centerPos);
			});

		Timer::AddEvent(26.0f, [this, centerPos]() {
			mOwner->GenerateEnemyRandomly(centerPos);
			});

		Timer::AddEvent(41.0f, [this, centerPos]() {
			mOwner->GenerateEnemyRandomly(centerPos);
			});

		Timer::AddEvent(56.0f, [this, centerPos]() {
			mOwner->GenerateEnemyRandomly(centerPos);
			});

		Timer::AddEvent(71.0f, [this]() {
			auto enemies = mRegistry.view<Tag_Enemy, IDComponent>();
			for (auto [entity, id] : enemies.each())
			{
				EntityType eType = mRegistry.any_of<Tag_Virus>(entity) ?
					EntityType::VIRUS : EntityType::DOG;
				mOwner->SendDeleteEntityPacket(id.ID, eType);
				DestroyEntity(mRegistry, entity);
			}

			mOwner->SendEventOccurPacket(0, EventType::BATTLE_END);

			auto whiteCells = mRegistry.view<Tag_WhiteCell>();
			for (auto entity : whiteCells)
			{
				DestroyEntity(mRegistry, entity);
			}
			});

		Timer::AddEvent(84.0f, [this, tank, cart, centerPos]() {
			auto wall = GetEntityByName(mRegistry, "Wall");
			DestroyEntity(mRegistry, wall);

			mRegistry.remove<Tag_Stop>(tank);
			mRegistry.remove<Tag_Stop>(cart);

			for (int i = 0; i < mNumRedCells; ++i)
			{
				auto pos = centerPos;
				pos.x = Random::RandFloat(centerPos.x - 600.0f, centerPos.x + 600.0f);
				auto id = mOwner->CreateCell(pos);
				
				NOTIFY_CREATE_ENTITY_PACKET packet = {};
				packet.EntityID = id;
				packet.EntityType = static_cast<UINT8>(EntityType::RED_CELL);
				packet.PacketID = NOTIFY_CREATE_ENTITY;
				packet.PacketSize = sizeof(packet);
				packet.Position = pos;
				mOwner->Broadcast(packet.PacketSize, reinterpret_cast<char*>(&packet));
			}
			});
	}
}

void MovementSystem::checkBossTrigger()
{
	if (mbBossProgressed)
	{
		return;
	}

	auto tank = GetEntityByName(mRegistry, "Tank");
	if (!mRegistry.valid(tank))
	{
		return;
	}

	auto trigger = GetEntityByName(mRegistry, "BossTrigger");
	if (!mRegistry.valid(trigger))
	{
		return;
	}

	const auto& tankPos = mRegistry.get<TransformComponent>(tank).Position;
	auto triggerPos = mRegistry.get<TransformComponent>(trigger).Position;
	triggerPos.y = 0.0f;

	float distSq = Vector3::DistanceSquared(tankPos, triggerPos);

	if (distSq < 100.0f)
	{
		mbBossProgressed = true;

		mRegistry.emplace<Tag_Stop>(tank);
		auto cart = GetEntityByName(mRegistry, "Cart");
		mRegistry.emplace<Tag_Stop>(cart);

		auto redCells = mRegistry.view<Tag_RedCell>();
		mNumRedCells = static_cast<INT32>(redCells.size());
		for (auto entity : redCells)
		{
			DestroyEntity(mRegistry, entity);
		}

		mOwner->SendEventOccurPacket(0, EventType::BOSS_BATTLE);

		NOTIFY_CREATE_ENTITY_PACKET packet = {};
		packet.PacketSize = sizeof(packet);
		packet.PacketID = NOTIFY_CREATE_ENTITY;
		packet.EntityType = static_cast<UINT8>(EntityType::BOSS);
		packet.EntityID = mOwner->GetEntityID();
		auto bossScar = GetEntityByName(mRegistry, "BossSpawnPoint");
		const auto& bsPosition = mRegistry.get<TransformComponent>(bossScar).Position;
		packet.Position = Vector3{ bsPosition.x, 0.0f, bsPosition.z };
		mOwner->Broadcast(packet.PacketSize, reinterpret_cast<char*>(&packet));
	}
}
