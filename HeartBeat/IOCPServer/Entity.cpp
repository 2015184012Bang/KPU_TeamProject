#include "pch.h"
#include "Entity.h"

entt::registry gRegistry;

Entity GetEntity(const UINT32 eid)
{
	auto view = gRegistry.view<IDComponent>();

	for (auto [entity, id] : view.each())
	{
		if (id.ID == eid)
		{
			return Entity{ entity };
		}
	}

	return Entity{};
}

Entity GetEntityByName(string_view name)
{
	auto view = gRegistry.view<NameComponent>();

	for (auto [entity, nameComp] : view.each())
	{
		if (nameComp.Name == name)
		{
			return Entity{ entity };
		}
	}

	return Entity{};
}

