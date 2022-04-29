#include "pch.h"
#include "GameMap.h"

#include "GameManager.h"
#include <rapidcsv.h>


void GameMap::LoadMap(string_view mapFile)
{
	rapidcsv::Document doc(mapFile.data(), rapidcsv::LabelParams(-1, -1));

	mMaxRow = static_cast<UINT32>(doc.GetRowCount());
	mMaxCol = static_cast<UINT32>(doc.GetColumnCount());

	for (UINT32 row = 0; row < mMaxRow; ++row)
	{
		// 파일의 아래에서부터 파싱한다. 좌하단이 원점.
		vector<int> tileTypes = doc.GetRow<int>(mMaxRow - row - 1);

		for (UINT32 col = 0; col < mMaxCol; ++col)
		{
			mTiles.emplace_back(static_cast<TileType>(tileTypes[col]),
				col * gTileSide,
				row * gTileSide);
		}
	}
}

void GameMap::Unload()
{
	mTiles.clear();
}
