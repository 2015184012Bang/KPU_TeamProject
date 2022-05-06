#pragma once

class Room;

class ScriptSystem
{
public:
	ScriptSystem(entt::registry& registry, shared_ptr<Room>&& room);

	void Update();

private:
	entt::registry& mRegistry;
	shared_ptr<Room> mOwner = nullptr;
};

