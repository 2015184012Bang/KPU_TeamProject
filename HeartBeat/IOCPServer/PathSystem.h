#pragma once

#include "GameMap.h"

class Room;

struct Node
{
	pair<INT32, INT32> Parent = { -1, -1 };
	INT32 Row = -1;
	INT32 Col = -1;

	bool operator==(const Node& other) const
	{
		return Row == other.Row &&
			Col == other.Col;
	}
};

class PathSystem
{
public:
	PathSystem(entt::registry& registry, shared_ptr<Room>&& room);

	void Update();

	void Reset();

	void ChangeTileToRoad(INT32 row, INT32 col);

private:
	bool isBlockedTile(const Tile& tile);

	void printGraph();

	INT32 getClosetNodeIndex(float position);

private:
	enum
	{
		WALL = 0,
		ROAD,
	};

	entt::registry& mRegistry;
	shared_ptr<Room> mOwner = nullptr;

	// WALL(0) 과 ROAD(1)로 이루어진 그래프
	vector<vector<INT32>> mGraph;

	// Visited 
	vector<vector<bool>> mVisited;

	UINT32 mMaxRow = 0;
	UINT32 mMaxCol = 0;
};
