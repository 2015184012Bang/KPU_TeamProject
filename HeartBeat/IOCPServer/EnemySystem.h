#pragma once

#include "rapidcsv.h"

class Room;

class EnemySystem
{
public:
	EnemySystem(entt::registry& registry, shared_ptr<Room>&& room);

	void Update();

	void Start(string_view fileName);

	void Reset();

private:
	void loadStageFile(string_view fileName);

private:
	entt::registry& mRegistry;
	shared_ptr<Room> mOwner = nullptr;

	unordered_map<string, rapidcsv::Document> mStages;

	bool mbGenerate = false;
};

