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

AIComponent::AIComponent()
	: AIScript(nullptr)
	, bInitialized(false)
{

}

AIComponent::AIComponent(shared_ptr<Script>&& ai)
	: AIScript(std::move(ai))
	, bInitialized(false)
{

}
