#pragma once

class Game
{
public:
	Game();
	virtual ~Game() = default;

	virtual bool Init() = 0;
	virtual void Shutdown() = 0;
	virtual void Run() = 0;

	bool ShouldClose() const { return !mIsRunning; }
	void SetRunning(bool value) { mIsRunning = value; }

	entt::entity CreateEntity();
	void DestroyEntity(const entt::entity handle);

	entt::registry& GetRegistry() { return mRegistry; }

private:
	bool mIsRunning;
	entt::registry mRegistry;

	friend class Entity;
};
