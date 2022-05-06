#pragma once

#include "Entity.h"

class Script
{
public:
	Script(entt::registry& registry, entt::entity owner);
	virtual ~Script() = default;

	virtual void Start() abstract;
	virtual void Update() abstract;

	template<typename T>
	T& GetComponent()
	{
		return mRegistry.get<T>(mOwner);
	}

	template<typename T, typename... Args>
	T& AddComponent(Args&&... args)
	{
		return mRegistry.emplace<T>(mOwner, forward<Args>(args)...);
	}

	template<typename T>
	void RemoveComponent()
	{
		mRegistry.remove<T>(mOwner);
	}

	template<typename T>
	bool HasComponent()
	{
		return mRegistry.any_of<T>(mOwner);
	}

	template<typename T>
	void AddTag()
	{
		mRegistry.emplace<T>(mOwner);
	}

	template<typename T>
	vector<entt::entity> FindObjectsWithTag()
	{
		vector<entt::entity> entities;

		auto view = mRegistry.view<T>();
		entities.reserve(view.size());

		for (auto entity : view)
		{
			entities.emplace_back(entity);
		}

		return entities;
	}

	entt::entity Find(string_view targetName);

protected:
	entt::registry& mRegistry;
	entt::entity mOwner = entt::null;
};

