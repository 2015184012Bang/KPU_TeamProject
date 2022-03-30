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
	void DestroyEntity(const entt::entity handle);
	void DestroyEntityByID(const HBID& id);

	template<typename T>
	void DestroyByComponent()
	{
		auto view = mRegistry.view<T>();
		
		for (auto entity : view)
		{
			DestroyEntity(entity);
		}
	}

	entt::entity GetEntityByID(const HBID& id);
	entt::registry& GetRegistry() { return mRegistry; }

protected:
	entt::entity getNewEntt();

private:
	void removeEntity(const HBID& id);

private:
	unordered_map<HBID, entt::entity> mEntities;

	bool mbRunning;
	entt::registry mRegistry;
};
