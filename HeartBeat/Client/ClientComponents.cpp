#include "ClientPCH.h"
#include "ClientComponents.h"

TransformComponent::TransformComponent() 
	: Position(Vector3::Zero)
	, Rotation(Vector3::Zero)
	, Scale(1.0f)
	, Buffer(gDevice.Get(), 1, true)
{

}

TransformComponent::TransformComponent(const Vector3& position, const Vector3& rotation /*= Vector3::Zero*/, float scale /*= 1.0f*/) 
	: Position(position)
	, Rotation(rotation)
	, Scale(scale)
	, Buffer(gDevice.Get(), 1, true)
{

}
