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
	auto view = mRegistry.view<MovementComponent, TransformComponent>();

	for (auto [entity, movement, transform] : view.each())
	{
		transform.Position += movement.Direction * movement.Speed * Timer::GetDeltaTime();
	}
	
	if (!mbMidPointFlag)
	{
		checkArriveAtMidPoint();
	}

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

	// 세포 이동 패킷 보내기
	auto cells = mRegistry.view<Tag_RedCell>();
	for (auto entity : cells)
	{
		packet.EntityID = mRegistry.get<IDComponent>(entity).ID;
		packet.Direction = mRegistry.get<MovementComponent>(entity).Direction;
		packet.Position = mRegistry.get<TransformComponent>(entity).Position;
		mOwner->Broadcast(sizeof(packet), reinterpret_cast<char*>(&packet));
	}
}

void MovementSystem::Start()
{
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

void MovementSystem::Reset()
{
	mbMidPointFlag = false;
}

void MovementSystem::checkArriveAtMidPoint()
{
	auto tank = GetEntityByName(mRegistry, "Tank");
	if (!mRegistry.valid(tank))
	{
		return;
	}

	auto midPoint = GetEntityByName(mRegistry, "MidPoint");
	if (!mRegistry.valid(midPoint))
	{
		return;
	}

	const auto& tankPos = mRegistry.get<TransformComponent>(tank).Position;
	auto midPos = mRegistry.get<TransformComponent>(midPoint).Position;
	midPos.y = 0.0f;

	float dist = Vector3::DistanceSquared(tankPos, midPos);

	if (dist < 100.0f)
	{
		mRegistry.get<MovementComponent>(tank).Direction = Vector3::Zero;
		auto cart = GetEntityByName(mRegistry, "Cart");
		mRegistry.get<MovementComponent>(cart).Direction = Vector3::Zero;
		mbMidPointFlag = true;

		mRegistry.emplace<Tag_Stop>(tank);
		mRegistry.emplace<Tag_Stop>(cart);

		NOTIFY_EVENT_OCCUR_PACKET packet = {};
		packet.EventType = static_cast<UINT8>(EventType::DOOR_DOWN);
		packet.PacketID = NOTIFY_EVENT_OCCUR;
		packet.PacketSize = sizeof(packet);
		mOwner->Broadcast(packet.PacketSize, reinterpret_cast<char*>(&packet));

		Timer::AddEvent(5.0f, [this, tank, cart]()
			{
				mRegistry.remove<Tag_Stop>(tank);
				mRegistry.remove<Tag_Stop>(cart);
				auto door = GetEntityByName(mRegistry, "Door");
				DestroyEntity(mRegistry, door);
			});
	}
}
