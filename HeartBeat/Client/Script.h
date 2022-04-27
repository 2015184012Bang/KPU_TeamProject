#pragma once

#include "Entity.h"
#include "Components.h"

class Script
{
public:
	Script(Entity owner);
	virtual ~Script() = default;

	template<typename T>
	T& GetComponent()
	{
		return mOwner.GetComponent<T>();
	}

	template<typename T>
	void AddTag()
	{
		mOwner.AddTag<T>();
	}

	template<typename T, typename... Args>
	T& AddComponent(Args&&... args)
	{
		return mOwner.AddComponent<T>(std::forward<Args>(args)...);
	}

	template<typename T>
	vector<Entity> FindObjectsWithTag()
	{
		vector<entt::entity> entities = mOwner.GetGame()->FindObjectsWithTag<T>();
		vector<Entity> objs;
		objs.reserve(entities.size());

		for (auto entity : entities)
		{
			objs.emplace_back(entity, mOwner.GetGame());
		}

		return objs;
	}

	Entity Find(const string& targetName)
	{
		auto view = mOwner.GetGame()->GetRegistry().view<NameComponent>();

		for (auto [entity, name] : view.each())
		{
			if (name.Name == targetName)
			{
				return Entity(entity, mOwner.GetGame());
			}
		}

		HB_ASSERT(false, "There is no entity with name");
	}

	virtual void Start() = 0;
	virtual void Update(float deltaTime) = 0;

private:
	Entity mOwner;
};

