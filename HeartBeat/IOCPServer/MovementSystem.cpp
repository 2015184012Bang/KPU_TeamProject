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
	// ���̵� �ش��ϴ� ���͸� �����´�.
	auto actor = GetEntityByID(mRegistry, clientID);
	ASSERT(mRegistry.valid(actor), "Invalid entity!");

	auto& movement = mRegistry.get<MovementComponent>(actor);
	movement.Direction = direction;

	// ������ 0�̸� yaw�� ������Ʈ�� �ʿ� �����Ƿ� ����.
	if (direction == Vector3::Zero)
	{
		return;
	}

	// ������ ����(0, 0, 1)�� ���� ������ ������ ���Ѵ�.
	// ������ 0~180�������� �������Ƿ�, -x�� �������� �̵��� ����
	// ���� ���� -1�� ������� �Ѵ�.
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

	// ��ũ �̵� ��Ŷ ������
	if (mRegistry.valid(tank))
	{
		packet.EntityID = mRegistry.get<IDComponent>(tank).ID;
		packet.Direction = mRegistry.get<MovementComponent>(tank).Direction;
		packet.Position = mRegistry.get<TransformComponent>(tank).Position;
		mOwner->Broadcast(sizeof(packet), reinterpret_cast<char*>(&packet));
	}

	// īƮ �̵� ��Ŷ ������
	auto cart = GetEntityByName(mRegistry, "Cart");
	if (mRegistry.valid(cart))
	{
		packet.EntityID = mRegistry.get<IDComponent>(cart).ID;
		packet.Direction = mRegistry.get<MovementComponent>(cart).Direction;
		packet.Position = mRegistry.get<TransformComponent>(cart).Position;
		mOwner->Broadcast(sizeof(packet), reinterpret_cast<char*>(&packet));
	}

	// �� �̵� ��Ŷ ������
	auto enemies = mRegistry.view<Tag_Enemy>();
	for (auto entity : enemies)
	{
		packet.EntityID = mRegistry.get<IDComponent>(entity).ID;
		packet.Direction = mRegistry.get<MovementComponent>(entity).Direction;
		packet.Position = mRegistry.get<TransformComponent>(entity).Position;
		mOwner->Broadcast(sizeof(packet), reinterpret_cast<char*>(&packet));
	}

	// ���� �̵� ��Ŷ ������
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
	// START_POINT Ÿ�� ��������
	auto startTile = GetEntityByName(mRegistry, "StartPoint");
	ASSERT(mRegistry.valid(startTile), "There is no start point.");
	const auto& startPos = mRegistry.get<TransformComponent>(startTile).Position;

	// �÷��̾���� ���� ��ġ�� �ʱ�ȭ
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
