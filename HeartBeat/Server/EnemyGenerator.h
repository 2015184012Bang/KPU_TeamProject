#pragma once

#include <queue>

class Server;

class EnemyGenerator
{
	struct SpawnInfo
	{
		string Type;
		float SpawnTime;
		float X;
		float Z;

		SpawnInfo(const string& type, float spawnTime, float x, float z)
			: Type(type)
			, SpawnTime(spawnTime)
			, X(x)
			, Z(z) {}

		bool operator>(const SpawnInfo& other) const
		{
			return SpawnTime > other.SpawnTime;
		}
	};

public:
	EnemyGenerator(Server* server);
	~EnemyGenerator();

	void Update();

	void SetStart(bool value) { mbStart = value; }

private:
	void readStageFile(const string& stageFile);

	uint8 getEnemyType(const string& name);

private:
	Server* mServer = nullptr;
	float mElapsed = 0.0f;
	bool mbStart = false;

	std::priority_queue<SpawnInfo, vector<SpawnInfo>, std::greater<SpawnInfo>> mSpawnInfos;
};