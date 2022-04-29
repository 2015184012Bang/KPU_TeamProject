#pragma once

class CollisionSystem
{
public:
	void Update();

	void TestCollision();

private:
	void checkPlayerAndTile();
};

