#pragma once

enum class TileType
{
	BLOCKED = 0,
	MOVABLE,
	RAIL,
	FAT,
	TANK_FAT,
	SCAR,
	END
};

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

	/*
	* Map └╬╡ж╜║ ▒╕┴╢
	* 6 бр  бр  бр  бр  бр  бр  бр  бр  бр  бр
	* 5 бр  бр  бр  бр  бр  бр  бр  бр  бр  бр
	* 4 бр  бр  бр  бр  бр  бр  бр  бр  бр  бр
	* 3 бр  бр  бр  бр  бр  бр  бр  бр  бр  бр
	* 2 бр  бр  бр  бр  бр  бр  бр  бр  бр  бр
	* 1 бр  бр  бр  бр  бр  бр  бр  бр  бр  бр
	* 0 бр  бр  бр  бр  бр  бр  бр  бр  бр  бр
	*   0  1  2  3  4  5  6  7  8  9
	*/
	void LoadMap(string_view mapFile);

	void Unload();

	const vector<Tile>& GetTiles() const { return mTiles; }

	UINT32 GetMaxRow() const { return mMaxRow; }
	UINT32 GetMaxCol() const { return mMaxCol; }

private:
	vector<Tile> mTiles;

	UINT32 mMaxRow = 0;
	UINT32 mMaxCol = 0;
};