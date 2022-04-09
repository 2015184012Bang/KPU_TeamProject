#pragma once

#include "HeartBeat/Entity.h"
#include "HeartBeat/Components.h"

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

		HB_ASSERT(false, "There is no entity named: {0}", targetName);
	}

	virtual void Start() = 0;
	virtual void Update(float deltaTime) = 0;

private:
	Entity mOwner;
};

