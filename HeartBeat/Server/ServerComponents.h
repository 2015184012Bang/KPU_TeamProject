#pragma once

struct STransformComponent
{
	STransformComponent();
	STransformComponent(const Vector3& position, const Vector3& rotation = Vector3::Zero);

	Vector3 Position;
	Vector3 Rotation;
};