#pragma once

#include "HBID.h"

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