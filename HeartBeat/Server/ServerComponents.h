#pragma once

#include "CollisionChecker.h"

struct STransformComponent
{
	STransformComponent();
	STransformComponent(const Vector3& position, const Vector3& rotation = Vector3::Zero);

	Vector3 Position;
	Vector3 Rotation;
};

struct SBoxComponent
{
	SBoxComponent();
	SBoxComponent(const AABB* localBox, const Vector3& position, float yaw = 0.0f);
	
	const AABB* LocalBox;
	AABB MyBox;
};