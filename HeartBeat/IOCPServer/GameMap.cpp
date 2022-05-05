#include "pch.h"
#include "GameMap.h"

#include "GameManager.h"
#include "Values.h"
#include <rapidcsv.h>


void GameMap::LoadMap(string_view path)
{
	rapidcsv::Document doc(path.data(), rapidcsv::LabelParams(-1, -1));

	Map gameMap;
	gameMap.FileName = path.data();
	gameMap.MaxRow = static_cast<UINT32>(doc.GetRowCount());
	gameMap.MaxCol = static_cast<UINT32>(doc.GetColumnCount());

	for (UINT32 row = 0; row < gameMap.MaxRow; ++row)
	{
		// 파일의 아래에서부터 파싱한다. 좌하단이 원점.
		vector<int> tileTypes = doc.GetRow<int>(gameMap.MaxRow - row - 1);

		for (UINT32 col = 0; col < gameMap.MaxCol; ++col)
		{
			gameMap.Tiles.emplace_back(static_cast<TileType>(tileTypes[col]),
				col * Values::TileSide,
				row * Values::TileSide);
		}
	}

	mMaps.push_back(move(gameMap));
}

void GameMap::Unload(string_view fileName)
{
	auto iter = find_if(mMaps.begin(), mMaps.end(), [fileName](const Map& m)
		{
			return m.FileName == fileName;
		});
	ASSERT(iter != mMaps.end(), "No map to unload: {0}", fileName.data());
	iter_swap(iter, mMaps.end() - 1);
	mMaps.pop_back();
}

const Map& GameMap::GetMap(string_view fileName) const
{
	auto iter = find_if(mMaps.begin(), mMaps.end(), [fileName](const Map& m)
		{
			return m.FileName == fileName;
		});
	ASSERT(iter != mMaps.end(), "Could not find map: {0}", fileName.data());
	return *iter;
}
void GameMap::InitGraph()
{
	graph = new Tile * [maxRow];
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
