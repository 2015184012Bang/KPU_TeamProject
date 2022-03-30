#pragma once

#include "HBID.h"

struct IDComponent
{
	IDComponent();
	IDComponent(uint64 id);

	HBID ID;
};