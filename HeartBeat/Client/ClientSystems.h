#pragma once

#include "UploadBuffer.h"

class ClientSystems
{
public:
	static void Move(Vector3* outPosition, const Vector3& velocity, float deltaTime);
	static void BindWorldMatrix(const Vector3& position, const Vector3& rotaion, float scale, UploadBuffer<Matrix>& buffer);
	static void BindViewProjectionMatrix(const Vector3& cameraPosition, const Vector3& cameraTarget, const Vector3& cameraUp, float fov, UploadBuffer<Matrix>& buffer);
};