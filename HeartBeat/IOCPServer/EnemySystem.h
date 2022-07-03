#pragma once

#include "rapidcsv.h"
#include "Protocol.h"

class Room;

class EnemySystem
{
public:
	EnemySystem(entt::registry& registry, shared_ptr<Room>&& room);

	void Update();

	void Start(string_view fileName);

	void Reset();

	void GenerateEnemyRandomly(const Vector3& controlPoint);

	entt::entity GenerateBoss();

public:
	bool IsGenerate() const { return mbGenerate; }

private:
	void loadStageFile(string_view fileName);

	entt::entity createEnemy(const Vector3& position, EntityType eType, entt::entity entity = entt::null);

private:
	entt::registry& mRegistry;
	shared_ptr<Room> mOwner = nullptr;

	unordered_map<string, rapidcsv::Document> mStages;

	bool mbGenerate = false;

	float mDuration = 0.0f;
};

