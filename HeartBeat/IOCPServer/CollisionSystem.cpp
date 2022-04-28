#include "pch.h"
#include "CollisionSystem.h"

#include "Box.h"
#include "Entity.h"

void CollisionSystem::Update()
{
	auto view = gRegistry.view<BoxComponent, MovementComponent,TransformComponent>();

	for (auto [entity, box, movement, transform] : view.each())
	{
		if (movement.Direction == Vector3::Zero)
		{
			continue;
		}

		auto& world = box.WorldBox;
		world.Update(transform.Position, transform.Yaw);
	}
}
