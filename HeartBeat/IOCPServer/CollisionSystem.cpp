#include "pch.h"
#include "CollisionSystem.h"

#include "Box.h"
#include "Entity.h"
#include "Tags.h"

void CollisionSystem::Update()
{
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
}

void CollisionSystem::TestCollision()
{
	auto p2 = GetEntityByName("Player2");
	auto p1 = GetEntityByName("Player1");

	if (p2 && p1)
	{
		auto& p2Box = p2.GetComponent<BoxComponent>();
		auto& p1Box = p1.GetComponent<BoxComponent>();

		if (Intersects(p2Box.WorldBox, p1Box.WorldBox))
		{
			LOG("Collision Occured!");
		}
	}
}
