#include "ClientPCH.h"
#include "Components.h"

#include "Animation.h"
#include "Helpers.h"
#include "Mesh.h"
#include "Skeleton.h"
#include "Texture.h"

using namespace std::string_literals;

MeshRendererComponent::MeshRendererComponent()
	: Mesi(nullptr)
	, Tex(nullptr)
{

}

MeshRendererComponent::MeshRendererComponent(const Mesh* mesh, const Texture* texture)
	: Mesi(mesh)
	, Tex(texture)
{

}


TransformComponent::TransformComponent()
	: Position(Vector3::Zero)
	, Rotation(Vector3::Zero)
	, Scale(1.0f)
	, Buffer(gDevice.Get(), 1, true)
	, bDirty(true)
{

}

TransformComponent::TransformComponent(const Vector3& position, const Vector3& rotation /*= Vector3::Zero*/, float scale /*= 1.0f*/)
	: Position(position)
	, Rotation(rotation)
	, Scale(scale)
	, Buffer(gDevice.Get(), 1, true)
	, bDirty(true)
{

}


RectTransformComponent::RectTransformComponent()
	: Width(0)
	, Height(0)
	, Position(Vector2::Zero)
	, Buffer(gDevice.Get(), 1, true)
	, bDirty(true)
{

}

RectTransformComponent::RectTransformComponent(int width, int height, const Vector2& position /*= Vector2::Zero*/)
	: Width(width)
	, Height(height)
	, Position(position)
	, Buffer(gDevice.Get(), 1, true)
	, bDirty(true)
{

}

CameraComponent::CameraComponent()
	: FOV(XMConvertToRadians(90.0f))
	, Position(Vector3::Zero)
	, Target(Vector3(0.0f, 0.0f, 1.0f))
	, Up(Vector3(0.0f, 1.0f, 0.0f))
	, Buffer(gDevice.Get(), 1, true)
{

}

CameraComponent::CameraComponent(const Vector3& position, const Vector3& target, const Vector3& up /*= Vector3::UnitY*/, float fov /*= 90.0f*/)
	: Position(position)
	, Target(target)
	, Up(up)
	, FOV(XMConvertToRadians(fov))
	, Buffer(gDevice.Get(), 1, true)
{

}

AnimatorComponent::AnimatorComponent()
	: Skel(nullptr)
	, CurAnim(nullptr)
	, PrevAnim(nullptr)
	, AnimPlayRate(0.0f)
	, CurAnimTime(0.0f)
	, PrevAnimTime(0.0f)
	, BlendingTime(0.0f)
	, Buffer(gDevice.Get(), 1, true)
{

}

AnimatorComponent::AnimatorComponent(const Skeleton* skel)
	: Skel(skel)
	, CurAnim(nullptr)
	, PrevAnim(nullptr)
	, AnimPlayRate(0.0f)
	, CurAnimTime(0.0f)
	, PrevAnimTime(0.0f)
	, BlendingTime(0.0f)
	, Buffer(gDevice.Get(), 1, true)
{

}

void AnimatorComponent::SetTrigger(string_view triggerName)
{
	Animation* nextAnim = CurAnim->FindNextAnimation(triggerName);

	if (nextAnim == nullptr)
	{
		return;
	}

	Helpers::PlayAnimation(this, nextAnim, 1.0f);
}

DebugDrawComponent::DebugDrawComponent()
	: Mesi(nullptr)
{

}

DebugDrawComponent::DebugDrawComponent(const Mesh* mesh)
	: Mesi(mesh)
{

}

SpriteRendererComponent::SpriteRendererComponent()
	: Mesi(nullptr)
	, Tex(nullptr)
	, DrawOrder(100)
{

}

SpriteRendererComponent::SpriteRendererComponent(SpriteMesh* mesh, const Texture* texture, int drawOrder /*= 100*/)
	: Mesi(mesh)
	, Tex(texture)
	, DrawOrder(drawOrder)
{

}

SpriteRendererComponent::SpriteRendererComponent(SpriteRendererComponent&& other) noexcept
{
	Mesi = other.Mesi;
	Tex = other.Tex;
	DrawOrder = other.DrawOrder;

	other.Mesi = nullptr;
	other.Tex = nullptr;
}

SpriteRendererComponent& SpriteRendererComponent::operator=(SpriteRendererComponent&& other) noexcept
{
	if (this != &other)
	{
		Mesi = other.Mesi;
		Tex = other.Tex;
		DrawOrder = other.DrawOrder;

		other.Mesi = nullptr;
		other.Tex = nullptr;
	}

	return *this;
}

SpriteRendererComponent::~SpriteRendererComponent()
{
	if (Mesi)
	{
		delete Mesi;
		Mesi = nullptr;
	}
}

ButtonComponent::ButtonComponent()
{

}

ButtonComponent::ButtonComponent(std::function<void(void)> f)
	: CallbackFunc(f)
{

}

ChildComponent::ChildComponent()
	: Parent(entt::null)
	, BoneIndex(-1)
	, BoneName("")
{

}

ChildComponent::ChildComponent(const entt::entity parent, const uint32 boneIndex, string_view boneName)
	: Parent(parent)
	, BoneIndex(boneIndex)
	, BoneName(boneName.data())
{

}

IDComponent::IDComponent()
	: ID(UINT32_MAX)
{
	//HB_LOG("Entity ID: {0}", ID);
}

IDComponent::IDComponent(const uint32 id)
	: ID(id)
{
	//HB_LOG("Entity ID: {0}", ID);
}

BoxComponent::BoxComponent()
	: Local(nullptr)
	, World()
{

}

BoxComponent::BoxComponent(const AABB* localBox, const Vector3& position, float yaw)
	: Local(localBox)
{
	World = *Local;

	World.UpdateWorldBox(position, yaw);
}

NameComponent::NameComponent()
	: Name("default"s)
{

}

NameComponent::NameComponent(string_view name)
	: Name(name.data())
{

}

HealthComponent::HealthComponent()
	: Health(0)
{

}

HealthComponent::HealthComponent(int32 maxHealth)
	: Health(maxHealth)
{

}

ScriptComponent::ScriptComponent()
	: NativeScript(nullptr)
	, bInitialized(false)
{

}

ScriptComponent::ScriptComponent(shared_ptr<Script>&& script)
	: NativeScript(std::move(script))
	, bInitialized(false)
{

}

MovementComponent::MovementComponent()
	: MaxSpeed(0.0f)
	, Direction(Vector3::Zero)
{

}

MovementComponent::MovementComponent(float maxSpeed)
	: MaxSpeed(maxSpeed)
	, Direction(Vector3::Zero)
{

}

LightComponent::LightComponent(const Vector3& ambient, const float specularStrength, const Vector3& lightPos, const Vector3& cameraPos)
	: Buffer(gDevice.Get(), 1, true)
{
	Light.AmbientColor = ambient;
	Light.SpecularStrength = specularStrength;
	Light.LightPosition = lightPos;
	Light.CameraPosition = cameraPos;
}

LightComponent::LightComponent()
	: Buffer(gDevice.Get(), 1, true)
{

}

FollowComponent::FollowComponent(const UINT32 targetID)
	: TargetID{ targetID }
{

}
