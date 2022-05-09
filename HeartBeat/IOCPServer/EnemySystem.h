#pragma once

class Room;

class EnemySystem
{
public:
	EnemySystem(entt::registry& registry, shared_ptr<Room>&& room);

	void Update();

	void LoadStageFile(string_view fileName);

	void SetGenerate(bool value) { mbGenerate = value; }

private:
	void readStageFile(string_view fileName);

	void testDeletion();

private:
	entt::registry& mRegistry;
	shared_ptr<Room> mOwner = nullptr;

	bool mbGenerate = false;
};

