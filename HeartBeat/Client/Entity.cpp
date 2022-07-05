#include "ClientPCH.h"
#include "Entity.h"

#include "Components.h"
#include "Tags.h"

entt::registry gRegistry;

void DestroyAll()
{
	gRegistry.clear();
}

void DestroyEntityByID(const uint32 id)
{
	Entity obj = GetEntityByID(id);

	if (obj)
	{
		DestroyEntity(obj);
	}
}

void DestroyEntity(const entt::entity entity)
{
	if (gRegistry.valid(entity))
	{
		gRegistry.destroy(entity);
	}
}

Entity GetEntityByID(const uint32 id)
{
	auto view = gRegistry.view<IDComponent>();

	for (auto [entity, idComp] : view.each())
	{
		if (idComp.ID == id)
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