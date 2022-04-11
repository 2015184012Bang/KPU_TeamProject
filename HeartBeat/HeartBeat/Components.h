#pragma once

#include "HBID.h"
#include "AABB.h"

class Script;

struct IDComponent
{
	IDComponent();
	IDComponent(uint64 id);

	HBID ID;
};

struct AttachmentParentComponent
{
	AttachmentParentComponent();
	AttachmentParentComponent(const HBID& id);

	HBID ChildID;
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
	NameComponent(const string& name);

	string Name;
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