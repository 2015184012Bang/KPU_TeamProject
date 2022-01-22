#include "ClientPCH.h"
#include "ClientComponents.h"

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
	, Anim(nullptr)
	, AnimPlayRate(0.0f)
	, AnimTime(0.0f)
	, bLoop(false)
	, Buffer(gDevice.Get(), 1, true)
{

}

AnimatorComponent::AnimatorComponent(Skeleton* skel)
	: Skel(skel)
	, Anim(nullptr)
	, AnimPlayRate(0.0f)
	, AnimTime(0.0f)
	, bLoop(false)
	, Buffer(gDevice.Get(), 1, true)
{

}
