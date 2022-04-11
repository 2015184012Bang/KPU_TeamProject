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

	const vector<Tile>& GetTiles() const { return mTiles; }

private:
	vector<Tile> mTiles;
};

extern GameMap gGameMap;