#pragma once

#include "CollisionChecker.h"

struct STransformComponent
{
	STransformComponent();
	STransformComponent(const Vector3& position, const Vector3& rotation = Vector3::Zero);

	Vector3 Position;
	Vector3 Rotation;
};

struct AIComponent
{
	AIComponent();
	AIComponent(shared_ptr<Script>&& ai);

	shared_ptr<Script> AIScript;
	bool bInitialized;
};