#pragma once

#include "Entity.h"

class Script
{
public:
	Script(Entity owner);
	virtual ~Script() = default;

	virtual void Start() abstract;
	virtual void Update() abstract;

	template<typename T>
	T& GetComponent()
	{
		return mOwner.GetComponent<T>();
	}

	template<typename T, typename... Args>
	T& AddComponent(Args&&... args)
	{
		return mOwner.AddComponent<T>(forward<Args>(args)...);
	}

	template<typename T>
	void RemoveComponent()
	{
		mOwner.RemoveComponent<T>();
	}

	template<typename T>
	bool HasComponent()
	{
		return mOwner.HasComponent<T>();
	}

	template<typename T>
	void AddTag()
	{
		mOwner.AddTag<T>();
	}

	template<typename T>
	vector<Entity> FindObjectsWithTag()
	{
		vector<Entity> entities;

		auto view = gRegistry.view<T>();
		entities.reserve(view.size());

		for (auto entity : view)
		{
			entities.emplace_back(entity);
		}

		return entities;
	}

	Entity Find(string_view targetName);

private:
	Entity mOwner = {};
};

