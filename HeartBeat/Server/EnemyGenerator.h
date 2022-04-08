#pragma once

class Server;

class EnemyGenerator
{
public:
	EnemyGenerator(Server* server);
	~EnemyGenerator();

	void Update();

	void SetStart(bool value) { mbStart = value; }
	void SetSpawnInterval(float interval) { mSpawnInterval = interval; }

private:
	Server* mServer = nullptr;
	float mElapsed = 0.0f;
	float mSpawnInterval = 5.0f;
	bool mbStart = false;
};