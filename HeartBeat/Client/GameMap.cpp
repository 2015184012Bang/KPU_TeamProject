#include "ClientPCH.h"
#include "GameMap.h"

#include "rapidcsv.h"
#include "Define.h"

GameMap gGameMap;

/*
* Map �ε��� ����
* 6 ��  ��  ��  ��  ��  ��  ��  ��  ��  �� 
* 5 ��  ��  ��  ��  ��  ��  ��  ��  ��  ��
* 4 ��  ��  ��  ��  ��  ��  ��  ��  ��  ��
* 3 ��  ��  ��  ��  ��  ��  ��  ��  ��  ��
* 2 ��  ��  ��  ��  ��  ��  ��  ��  ��  ��
* 1 ��  ��  ��  ��  ��  ��  ��  ��  ��  �� 
* 0 ��  ��  ��  ��  ��  ��  ��  ��  ��  �� 
*   0  1  2  3  4  5  6  7  8  9 
*/ 

void GameMap::LoadMap(string_view mapFile)
{
	rapidcsv::Document doc(mapFile.data(), rapidcsv::LabelParams(-1, -1));

	mMaxRow = static_cast<uint32>(doc.GetRowCount());
	mMaxCol = static_cast<uint32>(doc.GetColumnCount());

	for (uint32 row = 0; row < mMaxRow; ++row)
	{
		// ������ �Ʒ��������� �Ľ��Ѵ�. ���ϴ��� ����.
		vector<int> tileTypes = doc.GetRow<int>(mMaxRow - row - 1);

		for (uint32 col = 0; col < mMaxCol; ++col)
		{
			mTiles.emplace_back(static_cast<TileType>(tileTypes[col]),
				col * Values::TileSide,
				row * Values::TileSide);
		}
	}

}

void GameMap::Unload()
{
	mTiles.clear();
}
