#pragma once
#include "GameMap.h"

struct Node
{
	Node() = default;
	
	Node(TileType type, int row, int col) :
		ROW(row), COL(col), TYPE(type),
		G(0), H(0), F(0), CON(nullptr)
	{
	}

	Node(int row, int col) :
		ROW(row), COL(col), TYPE(TileType::MOVABLE),
		G(0), H(0), F(0), CON(nullptr)
	{
	}
	int ROW, COL;
	int G, H, F;
	TileType TYPE;
	Node* CON;
};
