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

	entt::entity CreateEntity();
	void DestroyEntity(const entt::entity handle);

	entt::registry& GetRegistry() { return mRegistry; }

private:
	bool mbRunning;
	entt::registry mRegistry;
};
