#pragma once

struct Node
{
	Node() = default;
	
	Node(int type, int row, int col) :
		ROW(row), COL(col), TYPE(type),
		G(0), H(0), F(0), conn(nullptr)
	{
	}

	Node(int row, int col) :
		ROW(row), COL(col), TYPE(0),
		G(0), H(0), F(0), conn(nullptr)
	{
	}
	int ROW, COL;
	int G, H, F;
	int TYPE;
	Node* conn;
};
