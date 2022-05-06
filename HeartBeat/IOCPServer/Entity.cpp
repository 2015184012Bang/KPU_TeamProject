#include "pch.h"
#include "Entity.h"


entt::entity GetEntityByID(entt::registry& registry, const UINT32 id)
{
	auto view = registry.view<IDComponent>();

	for (auto [entity, idc] : view.each())
	{
		if (idc.ID == id)
		{
			return entity;
		}
	}

	return entt::null;
}

entt::entity GetEntityByName(entt::registry& registry, string_view targetName)
{
	auto view = registry.view<NameComponent>();

	for (auto [entity, name] : view.each())
	{
		if (name.Name == targetName)
		{
			return entity;
		}
	}

	return entt::null;
}

extern void DestroyEntityByID(entt::registry& registry, const UINT32 id)
{
	auto view = registry.view<IDComponent>();

	for (auto [entity, idc] : view.each())
	{
		if (idc.ID == id)
		{
			registry.destroy(entity);
			break;
		}
	}
}

extern void DestroyEntity(entt::registry& registry, entt::entity entity)
{
	registry.destroy(entity);
}
