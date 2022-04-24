#include "PCH.h"
#include "GameMap.h"

#include "rapidcsv.h"
#include "Define.h"

GameMap::GameMap()
{
	LoadMap("../Assets/Maps/Map1.csv");
	InitGraph();
}

void GameMap::LoadMap(const string& mapFile)
{
	rapidcsv::Document doc(mapFile, rapidcsv::LabelParams(-1, -1));

	for (int i = doc.GetRowCount() - 1; i >= 0; --i)
	{
		std::vector<int> parsed = doc.GetRow<int>(i);

		for (size_t j = 0; j < parsed.size(); ++j)
		{
			mTiles.emplace_back(parsed[j], j * TILE_WIDTH, (doc.GetRowCount() - 1 - i) * TILE_WIDTH);
		}
	}

	maxRow = doc.GetRowCount();
	maxCol = doc.GetColumnCount();
}

void GameMap::Unload()
{
	mTiles.clear();
}

void GameMap::InitGraph()
{
	graph = new Tile* [maxRow];
	for (int i = 0; i < maxRow; ++i)
	{
		graph[i] = new Tile[maxCol];
	}

	for (int i = maxRow - 1; i >= 0; --i)
	{
		for (int j = 0; j < maxCol; ++j)
		{
			graph[i][j].Type = mTiles[i * maxRow + j].Type;
			graph[i][j].X = mTiles[i * maxRow + j].X;
			graph[i][j].Z = mTiles[i * maxRow + j].Z;
		}
	}

	if (graph == nullptr)
	{
		HB_LOG("Graph Unloaded");
	}
}

void GameMap::DeleteGraph()
{
	for (int i = 0; i < maxRow; ++i)
	{
		delete[] graph[i];
	}

	delete[] graph;
}

GameMap gGameMap;