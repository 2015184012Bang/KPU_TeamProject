#pragma once

#include "Mesh.h"
#include "Texture.h"
#include "UploadBuffer.h"
#include "Bone.h"
#include "Skeleton.h"
#include "Animation.h"
#include "AABB.h"
#include "Script.h"

struct MeshRendererComponent
{
	MeshRendererComponent();
	MeshRendererComponent(const Mesh* mesh, const Texture* texture);

	const Mesh* Mesi;
	const Texture* Tex;
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

struct RectTransformComponent
{
	RectTransformComponent();
	RectTransformComponent(const Vector2& position, int width, int height);

	Vector2 Position;
	int Width;
	int Height;
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
	AnimatorComponent(const Skeleton* skel);

	void SetTrigger(const string& triggerName);

	MatrixPalette Palette;
	const Skeleton* Skel;
	const Animation* CurAnim;
	const Animation* PrevAnim;
	float AnimPlayRate;
	float CurAnimTime;
	float PrevAnimTime;
	float BlendingTime;
	UploadBuffer<MatrixPalette> Buffer;
};

struct BoxComponent
{
	BoxComponent();
	BoxComponent(const AABB* Local, const Vector3& position, float yaw);
	
	const AABB* Local;
	AABB World;
};

struct DebugDrawComponent
{
	DebugDrawComponent();
	DebugDrawComponent(const Mesh* mesh);

	const Mesh* Mesi;
};

struct ScriptComponent
{
	ScriptComponent();
	ScriptComponent(Script* s);
	~ScriptComponent();

	Script* NativeScript;
	bool bInitialized;
};
