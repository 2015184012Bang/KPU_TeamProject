#pragma once

#include "UploadBuffer.h"

class ClientSystems
{
public:
	static void Move(Vector3* outPosition, const Vector3& velocity, float deltaTime);

	static void SetWorldMatrix(const Vector3& position, const Vector3& rotaion, float scale, UploadBuffer<Matrix>& buffer);
};