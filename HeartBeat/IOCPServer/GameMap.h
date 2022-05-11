#pragma once

enum class TileType
{
	BLOCKED = 0,
	MOVABLE = 1,
	RAIL = 2,
	FAT = 3,
	TANK_FAT = 4,
	SCAR = 5,
	START_POINT = 6,
	END_POINT = 7,
	HOUSE = 8,
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
	Tile** Graph;
};

class GameMap
{
public:
	GameMap(const GameMap&) = delete;
	GameMap& operator=(const GameMap&) = delete;

	static GameMap& GetInstance()
	{
		static GameMap instance;
		return instance;
	}

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

	void LoadMap(string_view path);
	void Unload(string_view fileName);

	Tile** GetGraph(const Map& map);

	const Map& GetMap(string_view fileName) const;

private:
	GameMap();

	vector<Tile> mTiles;
	vector<Map> mMaps;
};
