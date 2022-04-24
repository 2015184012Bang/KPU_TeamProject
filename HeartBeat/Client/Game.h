#pragma once

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

	void DestroyAll();
	void DestroyEntityByID(const uint32 id);
	void DestroyEntity(const entt::entity entity);

	template<typename T>
	void DestroyByComponent()
	{
		auto view = mRegistry.view<T>();
		for (auto entity : view)
		{
			mRegistry.destroy(entity);
		}
	}

	template<typename T>
	vector<entt::entity> FindObjectsWithTag()
	{
		auto view = mRegistry.view<T>();

		HB_ASSERT(!view.empty(), "0 entities with the tag!");

		vector<entt::entity> entts;
		entts.reserve(view.size());
		for (auto [entity, comp] : view.each())
		{
			entts.push_back(entity);
		}

		return entts;
	}

	entt::entity GetEntityByID(const uint32 id);
	entt::registry& GetRegistry() { return mRegistry; }
	entt::entity GetNewEntity();

private:
	bool mbRunning;
	entt::registry mRegistry;
};
