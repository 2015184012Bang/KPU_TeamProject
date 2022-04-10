#include "PCH.h"
#include "Components.h"

#include "Script.h"

IDComponent::IDComponent()
	: ID()
{
	HB_LOG("Entity ID: {0}", ID);
}

IDComponent::IDComponent(uint64 id)
	: ID(id)
{
	HB_LOG("Entity ID: {0}", ID);
}

AttachmentParentComponent::AttachmentParentComponent()
	: ChildID(-1)
{

}

AttachmentParentComponent::AttachmentParentComponent(const HBID& id)
	: ChildID(id)
{

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
	: Name("default")
{

}

NameComponent::NameComponent(const string& name)
	: Name(name)
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

ScriptComponent::ScriptComponent(Script* s)
	: NativeScript(s)
	, bInitialized(false)
{

}

ScriptComponent::ScriptComponent(ScriptComponent&& other) noexcept
{
	NativeScript = other.NativeScript;
	bInitialized = other.bInitialized;

	other.NativeScript = nullptr;
}

ScriptComponent& ScriptComponent::operator=(ScriptComponent&& other) noexcept
{
	if (this != &other)
	{
		NativeScript = other.NativeScript;
		bInitialized = other.bInitialized;

		other.NativeScript = nullptr;
	}

	return *this;
}

ScriptComponent::~ScriptComponent()
{
	if (NativeScript)
	{
		delete NativeScript;
		NativeScript = nullptr;
	}
}