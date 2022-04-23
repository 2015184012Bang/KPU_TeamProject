#include "ClientPCH.h"
#include "GameMap.h"

#include "rapidcsv.h"
#include "Define.h"

GameMap::GameMap()
{
	LoadMap("../Assets/Maps/Map1.csv");
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
}

void GameMap::Unload()
{
	mTiles.clear();
}

GameMap gGameMap;