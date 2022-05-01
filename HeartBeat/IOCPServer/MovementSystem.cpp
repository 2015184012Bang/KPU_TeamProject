#include "pch.h"
#include "MovementSystem.h"

#include "Entity.h"
#include "Timer.h"
#include "Tags.h"
#include "GameManager.h"
#include "Random.h"

MovementSystem::MovementSystem(shared_ptr<GameManager>&& gm)
	: mGameManager(move(gm))
{

}

void MovementSystem::Update()
{
	auto view = gRegistry.view<MovementComponent, TransformComponent>();

	for (auto [entity, movement, transform] : view.each())
	{
		transform.Position += movement.Direction * movement.Speed * Timer::GetDeltaTime();
	}
}

void MovementSystem::SetDirection(const UINT32 eid, const Vector3& direction)
{
	// 아이디에 해당하는 액터를 가져온다.
	auto actor = GetEntity(eid);
	ASSERT(actor, "Invalid entity!");

	auto& movement = actor.GetComponent<MovementComponent>();
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

	auto& transform = actor.GetComponent<TransformComponent>();
	transform.Yaw = scalar * XMConvertToDegrees(rotation.y);
}

void MovementSystem::SendNotifyMovePackets()
{
	auto tank = GetEntityByName("Tank");

	if (!tank)
	{
		return;
	}

	NOTIFY_MOVE_PACKET packet = {};
	packet.PacketID = NOTIFY_MOVE;
	packet.PacketSize = sizeof(packet);
	packet.EntityID = tank.GetComponent<IDComponent>().ID;
	packet.Direction = tank.GetComponent<MovementComponent>().Direction;
	packet.Position = tank.GetComponent<TransformComponent>().Position;
	mGameManager->SendToAll(sizeof(packet), reinterpret_cast<char*>(&packet));
}

void MovementSystem::SetPlayersStartPos()
{
	// START_POINT 타일 가져오기
	auto startTile = GetEntityByName("StartPoint");
	ASSERT(startTile, "There is no start point.");
	const auto& startPos = startTile.GetComponent<TransformComponent>().Position;

	auto view = gRegistry.view<Tag_Player, TransformComponent, IDComponent>();

	for (auto [entity, transform, id] : view.each())
	{
		transform.Position = Vector3{
			startPos.x + id.ID * 300.0f,
			transform.Position.y,
			startPos.z + id.ID * 300.0f
		};

		Entity player = Entity{ entity };
		NOTIFY_MOVE_PACKET packet = {};
		packet.Direction = player.GetComponent<MovementComponent>().Direction;
		packet.EntityID = id.ID;
		packet.PacketID = NOTIFY_MOVE;
		packet.PacketSize = sizeof(packet);
		packet.Position = transform.Position;

		mGameManager->SendToAll(sizeof(packet), reinterpret_cast<char*>(&packet));
	}
}
