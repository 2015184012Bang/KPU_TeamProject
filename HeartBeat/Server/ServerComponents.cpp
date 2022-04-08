#include "ServerPCH.h"
#include "ServerComponents.h"


STransformComponent::STransformComponent()
	: Position(Vector3::Zero)
	, Rotation(Vector3::Zero)
{

}

STransformComponent::STransformComponent(const Vector3& position, const Vector3& rotation /*= Vector3::Zero*/)
	: Position(position)
	, Rotation(rotation)
{

}

SBoxComponent::SBoxComponent()
	: LocalBox(nullptr)
	, MyBox()
{

}

SBoxComponent::SBoxComponent(const AABB* localBox, const Vector3& position, float yaw /*= 0.0f*/)
	: LocalBox(localBox)
	, MyBox()
{
	MyBox = *localBox;
	MyBox.UpdateWorldBox(position, yaw);
}
