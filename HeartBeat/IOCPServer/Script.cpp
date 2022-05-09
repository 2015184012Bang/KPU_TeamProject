#include "pch.h"
#include "Script.h"


Script::Script(entt::registry& registry, entt::entity owner)
	: mRegistry{ registry }
	, mOwner{ owner }
{

}

entt::entity Script::Find(string_view targetName)
{
	auto view = mRegistry.view<NameComponent>();

	for (auto [entity, name] : view.each())
	{
		if (name.Name == targetName)
		{
			return entity;
		}
	}

	return entt::null;
}
