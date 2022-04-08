#pragma once

#include "CollisionChecker.h"

class ServerSystems
{
public:
	static void UpdatePlayerTransform(Vector3* outPosition, float* outRotationY, const Vector3& direction);
	static bool Intersects(const AABB& a, const AABB& b);
	static void UpdateBox(const AABB* localBox, AABB* outWorldBox, const Vector3& position, float yaw);
};