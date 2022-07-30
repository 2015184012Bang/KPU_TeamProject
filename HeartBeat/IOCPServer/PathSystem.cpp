#include "pch.h"
#include "PathSystem.h"

#include "Room.h"
#include "Components.h"
#include "Values.h"

#include <iostream>


INT32 dx[] = {1, -1, 0, 0};
INT32 dy[] = {0, 0, 1, -1};

INT32 GetHeuristics(const pair<INT32, INT32>& from, const pair<INT32, INT32>& to)
{
	return abs((from.first - to.first)) + abs((from.second - to.second));
}

PathSystem::PathSystem(entt::registry& registry, shared_ptr<Room>&& room)
	: mRegistry{ registry }
	, mOwner{ move(room) }
{
	auto& gameMap = GameMap::GetInstance().GetMap("../Assets/Maps/Map.csv");

	mGraph.resize(gameMap.MaxRow);
	mVisited.resize(gameMap.MaxRow);

	for (UINT32 row = 0; row < gameMap.MaxRow; ++row)
	{
		for (UINT32 col = 0; col < gameMap.MaxCol; ++col)
		{
			INT32 index = row * gameMap.MaxCol + col;

			auto& tile = gameMap.Tiles[index];

			// WALL : 갈 수 없음
			// ROAD : 갈 수 있음
			INT32 val = isBlockedTile(tile) ? WALL : ROAD;
			mGraph[row].push_back(val);
			mVisited[row].push_back(false);
		}
	}

	mMaxRow = gameMap.MaxRow;
	mMaxCol = gameMap.MaxCol;
}

void PathSystem::Update()
{
	auto view = mRegistry.view<PathFindComponent>();

	for (auto [entity, pathFind] : view.each())
	{
		if (!pathFind.bContinue)
		{
			continue;
		}

		priority_queue<Node> q;

		Node start = { {-1, -1}, getClosetNodeIndex(pathFind.TargetPosition.z)
			, getClosetNodeIndex(pathFind.TargetPosition.x), 0, 0 };
		q.push(start);

		Node goal = { {-1, -1}, getClosetNodeIndex(pathFind.MyPosition.z)
			, getClosetNodeIndex(pathFind.MyPosition.x), 0, 0 };

		while (!q.empty())
		{
			Node current = q.top();
			q.pop();

			if (current == goal)
			{
				Vector3 direction = Vector3::Zero;
				float tileX = current.Parent.second * Values::TileSide;
				float tileZ = current.Parent.first * Values::TileSide;
				const auto& position = mRegistry.get<TransformComponent>(entity).Position;

				direction.x = tileX - position.x;
				direction.z = tileZ - position.z;
				direction.Normalize();

				mRegistry.get<MovementComponent>(entity).Direction = direction;
				break;
			}

			for (INT32 i = 0; i < 4; ++i)
			{
				INT32 nx = current.Row + dx[i];
				INT32 ny = current.Col + dy[i];

				if (nx >= 0 && nx < (INT32)mMaxRow && ny >= 0 && ny < (INT32)mMaxCol)
				{
					if (mGraph[nx][ny] == ROAD && mVisited[nx][ny] == false)
					{
						mVisited[nx][ny] = true;
						Node node = { {current.Row, current.Col}, nx, ny, 
							GetHeuristics({nx, ny}, {goal.Row, goal.Col}) + node.Depth, node.Depth + 1 };
						q.push(node);
					}
				}
			}
		}

		for (auto& row : mVisited)
		{
			fill(row.begin(), row.end(), false);
		}
	}
}

void PathSystem::Reset()
{
	auto& gameMap = GameMap::GetInstance().GetMap("../Assets/Maps/Map.csv");

	for (UINT32 row = 0; row < gameMap.MaxRow; ++row)
	{
		for (UINT32 col = 0; col < gameMap.MaxCol; ++col)
		{
			INT32 index = row * gameMap.MaxCol + col;

			auto& tile = gameMap.Tiles[index];

			INT32 val = isBlockedTile(tile) ? WALL : ROAD;
			mGraph[row][col] = val;
			mVisited[row][col] = false;
		}
	}
}

void PathSystem::ChangeTileToRoad(INT32 row, INT32 col)
{
	mGraph[row][col] = ROAD;
}

bool PathSystem::isBlockedTile(const Tile& tile)
{
	switch (tile.TType)
	{
	case TileType::BLOCKED:
	case TileType::FAT:
	case TileType::TANK_FAT:
		return true;

	default:
		return false;
	}
}

void PathSystem::printGraph()
{
	for (const auto& row : mGraph)
	{
		for (auto n : row)
		{
			cout << n << ' ';
		}
		cout << '\n';
	}
}

INT32 PathSystem::getClosetNodeIndex(float position)
{
	static const INT32 tileHalfSide = static_cast<INT32>(Values::TileSide / 2);

	if (position <= 0.0f)
	{
		return 0;
	}

	auto pos = static_cast<INT32>(position);
	auto index = static_cast<INT32>(pos / Values::TileSide);
	if (pos % static_cast<INT32>(Values::TileSide) < tileHalfSide - 1)
	{
		return index;
	}
	else
	{
		return index + 1;
	}
}
