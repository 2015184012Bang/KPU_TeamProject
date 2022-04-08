#include "ServerPCH.h"
#include "ServerSystems.h"

constexpr float PLAYER_MOVE_SPEED = 300.0f;

void ServerSystems::UpdatePlayerTransform(Vector3* outPosition, float* outRotationY, const Vector3& direction)
{
	*outPosition += direction * PLAYER_MOVE_SPEED * 0.016f;

	if (direction.z > 0.0f) *outRotationY = 0.0f;
	if (direction.z < 0.0f) *outRotationY = 180.0f;
	if (direction.x > 0.0f) *outRotationY = 90.0f;
	if (direction.x < 0.0f) *outRotationY = -90.0f;
}
