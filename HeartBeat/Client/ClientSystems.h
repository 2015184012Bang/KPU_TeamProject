#pragma once

#include "UploadBuffer.h"

class ClientSystems
{
public:
	static void MovePosition(Vector3* outPosition, const Vector3& velocity, float deltaTime, bool* outDirty);
	static void RotateY(Vector3* outRotation, float speed, float deltaTime, bool* outDirty);
	static void BindWorldMatrix(const Vector3& position, const Vector3& rotaion, float scale, UploadBuffer<Matrix>& buffer, bool* outDirty);
	static void BindViewProjectionMatrix(const Vector3& cameraPosition, const Vector3& cameraTarget, const Vector3& cameraUp, float fov, UploadBuffer<Matrix>& buffer);
};