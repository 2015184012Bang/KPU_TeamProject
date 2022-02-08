#include "PCH.h"
#include "Components.h"

IDComponent::IDComponent()
	: ID()
{
	HB_LOG("Entity ID: {0}", ID);
}
