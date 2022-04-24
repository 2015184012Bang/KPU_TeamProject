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
class Text;


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
	TextComponent();
	TextComponent(Text* text);
	~TextComponent();
	TextComponent(TextComponent&& other) noexcept;
	TextComponent& operator=(TextComponent&& other) noexcept;

	Text* Txt;
};

struct AttachmentChildComponent
{
	AttachmentChildComponent();
	AttachmentChildComponent(MatrixPalette* palette, uint32 index, TransformComponent* transform);

	MatrixPalette* ParentPalette;
	uint32 BoneIndex;
	TransformComponent* ParentTransform;
};

struct IDComponent
{
	IDComponent();
	IDComponent(const uint32 id);

	uint32 ID;
};

struct AttachmentParentComponent
{
	AttachmentParentComponent();
	AttachmentParentComponent(const uint32 id);

	uint32 ChildID;
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
	NameComponent(wstring_view name);

	wstring Name;
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