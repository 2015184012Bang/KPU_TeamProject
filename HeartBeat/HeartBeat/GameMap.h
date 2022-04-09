#pragma once

struct Tile
{
	Tile(int type, float x, float z)
		: Type(type)
		, X(x)
		, Z(z) {}

	int Type;
	float X;
	float Z;
};

class GameMap
{
public:
	GameMap();

	void LoadMap(const string& mapFile);
	void Unload();

	const vector<Tile>& GetTiles() const { return mTiles; }

private:
	vector<Tile> mTiles;
};

extern GameMap gGameMap;