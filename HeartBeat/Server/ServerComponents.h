#pragma once

struct STransformComponent
{
	STransformComponent();

	STransformComponent(const Vector3& position, const Vector3& rotation = Vector3::Zero, float scale = 1.0f);

	Vector3 Position;
	Vector3 Rotation;
	float Scale;
	bool bDirty;
};