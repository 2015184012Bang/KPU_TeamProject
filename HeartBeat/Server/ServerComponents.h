#pragma once

#include "CollisionChecker.h"

struct STransformComponent
{
	STransformComponent();
	STransformComponent(const Vector3& position, const Vector3& rotation = Vector3::Zero);

	Vector3 Position;
	Vector3 Rotation;
};

struct SHealthComponent
{
	SHealthComponent();
	SHealthComponent(int32 maxHealth);

	int32 Health;
};