#pragma once

struct Tile
{
	Tile() = default;

	Tile(int type, float x, float z)
		: Type(type)
		, X(x)
		, Z(z) {}

	int Type = 0;
	float X = 0.0f;
	float Z = 0.0f;
};

class GameMap
{
public:
	GameMap();

	void LoadMap(const string& mapFile);
	void Unload();
	void InitGraph();
	void DeleteGraph();

	const vector<Tile>& GetTiles() const { return mTiles; }
	Tile** GetGraph() const { return graph; }
	const int& GetMaxRow() const { return maxRow; }
	const int& GetMaxCol() const { return maxCol; }

private:
	vector<Tile> mTiles;
	
	int maxRow;
	int maxCol;
	Tile** graph;
};

extern GameMap gGameMap;