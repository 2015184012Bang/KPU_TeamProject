#include "ClientPCH.h"
#include "ClientComponents.h"

#include "ClientSystems.h"

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

void AnimatorComponent::SetTrigger(const string& triggerName)
{
	Animation* nextAnim = CurAnim->FindNextAnimation(triggerName);

	if (nextAnim == nullptr)
	{
		return;
	}

	ClientSystems::PlayAnimation(this, nextAnim, 1.0f);
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

DebugDrawComponent::DebugDrawComponent()
	: Mesi(nullptr)
{

}

DebugDrawComponent::DebugDrawComponent(const Mesh* mesh)
	: Mesi(mesh)
{

}

ScriptComponent::ScriptComponent()
	: NativeScript(nullptr)
	, bInitialized(false)
{

}

ScriptComponent::ScriptComponent(Script* s)
	: NativeScript(s)
	, bInitialized(false)
{

}

ScriptComponent::~ScriptComponent()
{
	if (NativeScript)
	{
		delete NativeScript;
		NativeScript = nullptr;
	}
}
