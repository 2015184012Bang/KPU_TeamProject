#include "pch.h"
#include "Script.h"

Script::Script(Entity owner)
	: mOwner(owner)
{

}

Entity Script::Find(string_view targetName)
{
	auto view = gRegistry.view<NameComponent>();

	for (auto [entity, name] : view.each())
	{
		if (name.Name == targetName)
		{
			return Entity{ entity };
		}
	}

	return Entity{ };
}
