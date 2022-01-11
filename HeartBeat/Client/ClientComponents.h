#pragma once

#include "Mesh.h"
#include "Texture.h"
#include "UploadBuffer.h"

struct MeshRendererComponent
{
	MeshRendererComponent(Mesh* mesh, Texture* texture)
		: Mesi(mesh)
		, Tex(texture) {}

	Mesh* Mesi;
	Texture* Tex;
};

struct TransformComponent
{
	TransformComponent();

	TransformComponent(const Vector3& position, const Vector3& rotation = Vector3::Zero, float scale = 1.0f);
	
	Vector3 Position;
	Vector3 Rotation;
	float Scale;
	UploadBuffer<Matrix> Buffer;
};