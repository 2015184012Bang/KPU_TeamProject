#include "ServerPCH.h"
#include "ServerComponents.h"


STransformComponent::STransformComponent()
	: Position(Vector3::Zero)
	, Rotation(Vector3::Zero)
	, Scale(1.0f)
	, bDirty(true)
{

}

STransformComponent::STransformComponent(const Vector3& position, const Vector3& rotation /*= Vector3::Zero*/, float scale /*= 1.0f*/)
	: Position(position)
	, Rotation(rotation)
	, Scale(scale)
	, bDirty(true)
{

}
