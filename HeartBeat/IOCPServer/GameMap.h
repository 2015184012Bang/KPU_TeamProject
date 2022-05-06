#pragma once

enum class TileType
{
	BLOCKED = 0,
	MOVABLE,
	RAIL,
	FAT,
	TANK_FAT,
	SCAR,
	START_POINT,
	END_POINT,
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

struct Map
{
	string FileName = ""s;
	UINT32 MaxRow = 0;
	UINT32 MaxCol = 0;
	vector<Tile> Tiles;
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
	void LoadMap(string_view path);

	void Unload(string_view fileName);

	void InitGraph(UINT32 maxRow, UINT32 maxCol);
	void DeleteGraph(UINT32 maxRow);

	const Map& GetMap(string_view fileName) const;
	Tile** GetGraph() const { return graph; }

private:
	vector<Tile> mTiles;
	
	Tile** graph;
	vector<Map> mMaps;
};
