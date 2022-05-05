#pragma once

#include "HBID.h"

class Game
{
	friend class Entity;

public:
	Game();
	virtual ~Game() = default;

	virtual bool Init() = 0;
	virtual void Shutdown() = 0;
	virtual void Run() = 0;

	bool ShouldClose() const { return !mbRunning; }
	void SetRunning(bool value) { mbRunning = value; }

	void RegisterEntity(const HBID& id, const entt::entity entity);

	void DestroyAll();
	void DestroyEntityByID(const HBID& id);
	void DestroyEntity(const entt::entity entity);

	template<typename T>
	void DestroyByComponent()
	{
		auto view = mRegistry.view<T>();
		for (auto entity : view)
		{
			DestroyEntity(entity);
		}
	}

	template<typename T>
	vector<entt::entity> FindObjectsWithTag()
	{
		auto view = mRegistry.view<T>();

		HB_ASSERT((!view.empty()), "0 entities with the tag!");

		vector<entt::entity> entts;
		entts.reserve(view.size());
		for (auto id : view)
		{
			entts.push_back(id);
		}

		return entts;
	}

	entt::entity GetEntityByID(const HBID& id);
	entt::registry& GetRegistry() { return mRegistry; }
	entt::entity GetNewEntity();

private:
	unordered_map<HBID, entt::entity> mEntities;

	bool mbRunning;
	entt::registry mRegistry;
};
