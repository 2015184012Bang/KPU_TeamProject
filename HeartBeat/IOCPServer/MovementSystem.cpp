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
	// ���̵� �ش��ϴ� ���͸� �����´�.
	auto actor = GetEntity(eid);
	ASSERT(actor, "Invalid entity!");

	auto& movement = actor.GetComponent<MovementComponent>();
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

	auto& transform = actor.GetComponent<TransformComponent>();
	transform.Yaw = scalar * XMConvertToDegrees(rotation.y);
}
