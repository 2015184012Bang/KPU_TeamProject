#pragma once

#include "HeartBeat/Entity.h"

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
		vector<entt::entity> ids = mOwner.GetGame()->FindObjectsWithTag<T>();
		vector<Entity> entts;
		entts.reserve(ids.size());
		
		for (auto id : ids)
		{
			entts.emplace_back(id, mOwner.GetGame());
		}

		return entts;
	}

	virtual void Start() = 0;
	virtual void Update(float deltaTime) = 0;

private:
	Entity mOwner;
};

