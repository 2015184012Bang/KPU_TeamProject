#include "ServerPCH.h"
#include "ServerSystems.h"

constexpr float PLAYER_MOVE_SPEED = 300.0f;
constexpr float TANK_MOVE_SPEED = 100.0f;

void ServerSystems::UpdatePlayerTransform(Vector3* outPosition, float* outRotationY, const Vector3& direction)
{
	*outPosition += direction * PLAYER_MOVE_SPEED * 0.016f;

	if (direction.z > 0.0f) *outRotationY = 0.0f;
	if (direction.z < 0.0f) *outRotationY = 180.0f;
	if (direction.x > 0.0f) *outRotationY = 90.0f;
	if (direction.x < 0.0f) *outRotationY = -90.0f;
}

void ServerSystems::MoveToward(STransformComponent* outTransform, const Vector3& to, float deltaTime)
{
	Vector3 direction = to - outTransform->Position;
	direction.Normalize();
	Vector3 forward = Vector3{ 0.0f, 0.0f, 1.0f };
	Vector3 angle = XMVector3AngleBetweenNormals(direction, forward);

	outTransform->Position += direction * TANK_MOVE_SPEED * deltaTime;
	outTransform->Rotation.y = XMConvertToDegrees(angle.y);
}

bool ServerSystems::Intersects(const AABB& a, const AABB& b)
{
	const Vector3& aMin = a.GetMin();
	const Vector3& aMax = a.GetMax();

	const Vector3& bMin = b.GetMin();
	const Vector3& bMax = b.GetMax();

	bool no = aMax.x < bMin.x ||
		aMax.y < bMin.y ||
		aMax.z < bMin.z ||
		bMax.x < aMin.x ||
		bMax.y < aMin.y ||
		bMax.z < aMin.z;

	return !no;
}

void ServerSystems::UpdateBox(const AABB* localBox, AABB* outWorldBox, const Vector3& position, float yaw)
{
	*outWorldBox = *localBox;
	outWorldBox->UpdateWorldBox(position, yaw);
}
