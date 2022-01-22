#pragma once

#include "Mesh.h"
#include "Texture.h"
#include "UploadBuffer.h"
#include "Bone.h"
#include "Skeleton.h"
#include "Animation.h"

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
	bool bDirty;
};

struct CameraComponent
{
	CameraComponent();
	CameraComponent(const Vector3& position, const Vector3& target, const Vector3& up = Vector3::UnitY, float fov = 90.0f);

	Vector3 Position;
	Vector3 Target;
	Vector3 Up;
	float FOV;
	UploadBuffer<Matrix> Buffer;
};

struct AnimatorComponent
{
	AnimatorComponent();
	AnimatorComponent(Skeleton* skel);

	MatrixPalette Palette;
	Skeleton* Skel;
	Animation* Anim;
	float AnimPlayRate;
	float AnimTime;
	bool bLoop;
	UploadBuffer<MatrixPalette> Buffer;
};