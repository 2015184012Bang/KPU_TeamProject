#pragma once

#include "UploadBuffer.h"
#include "Bone.h"
#include "AABB.h"

class Mesh;
class Texture;
class SpriteMesh;
class Skeleton;
class Animation;
class Script;

struct MeshRendererComponent
{
	MeshRendererComponent();
	MeshRendererComponent(const Mesh* mesh, const Texture* texture);

	const Mesh* Mesi;
	const Texture* Tex;
};

struct SpriteRendererComponent
{
	SpriteRendererComponent();
	SpriteRendererComponent(SpriteMesh* mesh, const Texture* texture, int drawOrder = 100);
	~SpriteRendererComponent();
	SpriteRendererComponent(SpriteRendererComponent&& other) noexcept;
	SpriteRendererComponent& operator=(SpriteRendererComponent&& other) noexcept;

	SpriteMesh* Mesi;
	const Texture* Tex;
	int DrawOrder;
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
	RectTransformComponent(int width, int height, const Vector2& position = Vector2::Zero);

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

	void SetTrigger(string_view triggerName);

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

struct DebugDrawComponent
{
	DebugDrawComponent();
	DebugDrawComponent(const Mesh* mesh);

	const Mesh* Mesi;
};

struct ButtonComponent
{
	ButtonComponent();
	ButtonComponent(std::function<void(void)> f);

	std::function<void(void)> CallbackFunc;
};

struct TextComponent
{
	TextComponent() = default;

	std::wstring Sentence = L"";
	float X = 0.0f;
	float Y = 0.0f;
	int FontSize = 40;
};

struct ParentComponent
{
	ParentComponent() = default;

	vector<entt::entity> Children;
};

struct ChildComponent
{
	ChildComponent();
	ChildComponent(const entt::entity parent, const uint32 boneIndex, string_view boneName);

	entt::entity Parent;
	uint32 BoneIndex;
	string BoneName;
};

struct IDComponent
{
	IDComponent();
	IDComponent(const uint32 id);

	uint32 ID;
};

struct BoxComponent
{
	BoxComponent();
	BoxComponent(const AABB* Local, const Vector3& position, float yaw = 0.0f);

	const AABB* Local;
	AABB World;
};

struct NameComponent
{
	NameComponent();
	NameComponent(string_view name);

	string Name;
};

struct HealthComponent
{
	HealthComponent();
	HealthComponent(int32 maxHealth);

	int32 Health;
};

struct ScriptComponent
{
	ScriptComponent();
	ScriptComponent(shared_ptr<Script>&& script);

	shared_ptr<Script> NativeScript;
	bool bInitialized;
};

struct MovementComponent
{
	MovementComponent();
	MovementComponent(float maxSpeed);

	float MaxSpeed;
	Vector3 Direction;
};

struct LightComponent
{
	LightComponent();
	LightComponent(const Vector3& ambient, const float specularStrength,
		const Vector3& lightPos, const Vector3& cameraPos);

	struct LightInfo
	{
		Vector3 AmbientColor = Vector3::Zero;
		float SpecularStrength = 0.0f;
		Vector3 LightPosition = Vector3::Zero;
		Vector3 CameraPosition = Vector3::Zero;
	};

	LightInfo Light = {};
	UploadBuffer<LightInfo> Buffer;
};

struct FollowComponent
{
	FollowComponent() = default;
	FollowComponent(const UINT32 targetID);

	UINT32 TargetID = 0;
};