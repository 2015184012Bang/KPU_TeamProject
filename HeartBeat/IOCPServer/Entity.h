#pragma once

#include <entt/entt.hpp>

#include "Components.h"
#include "Tags.h"

extern entt::entity GetEntityByID(entt::registry& registry, const UINT32 id);

extern entt::entity GetEntityByName(entt::registry& registry, string_view targetName);

template<typename T>
void DestroyByComponent(entt::registry& registry)
{
	auto view = registry.view<T>();

	for (auto entity : view)
	{
		if (registry.valid(entity))
		{
			registry.destroy(entity);
		}
	}
}

extern void DestroyEntityByID(entt::registry& registry, const UINT32 id);

extern void DestroyEntity(entt::registry& registry, entt::entity entity);
