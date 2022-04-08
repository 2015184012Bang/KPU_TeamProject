#pragma once

#include "HBID.h"
#include "AABB.h"

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