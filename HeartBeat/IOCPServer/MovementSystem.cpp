#include "pch.h"
#include "MovementSystem.h"

#include "Entity.h"
#include "Timer.h"

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
