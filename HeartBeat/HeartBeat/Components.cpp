#include "PCH.h"
#include "Components.h"

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
