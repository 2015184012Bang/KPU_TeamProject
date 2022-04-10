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

ScriptComponent::ScriptComponent(shared_ptr<Script>&& script)
	: NativeScript(std::move(script))
	, bInitialized(false)
{

}
