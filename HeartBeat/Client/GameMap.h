#pragma once

#include "Define.h"

struct Tile
{
	Tile() = default;

	Tile(TileType type, float x, float z)
		: TType(type)
		, X(x)
		, Z(z) {}

	TileType TType = TileType::BLOCKED;
	float X = 0.0f;
	float Z = 0.0f;
};

class GameMap
{
public:
	GameMap() = default;

	void LoadMap(string_view mapFile);
	void Unload();

	const vector<Tile>& GetTiles() const { return mTiles; }

	uint32 GetMaxRow() const { return mMaxRow; }
	uint32 GetMaxCol() const { return mMaxCol; }

private:
	vector<Tile> mTiles;

	uint32 mMaxRow = 0;
	uint32 mMaxCol = 0;
};

extern GameMap gGameMap;