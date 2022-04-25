#include "ServerPCH.h"
#include "Enemy.h"
#include "HeartBeat/Define.h"

tuple<int, int> Enemy::GetGoalIndex()
{
	int tileWidth = TILE_WIDTH;
	goalRow = static_cast<int>(setPlayerPosition->x) / tileWidth;
	goalCol = static_cast<int>(setPlayerPosition->z) / tileWidth;

	return std::make_tuple(goalRow, goalCol);
}

void Enemy::FindPath()
{
	if (openList.size() == 0)
	{
		//HB_LOG("No Path");
	}

	Node* openNode = nullptr;
	int minF = 1000;

	for (auto& op : openList)
	{
		if (op->F < minF)
		{
			minF = op->F;
			openNode = op;
		}
	}
	
	if (openNode != nullptr)
	{
		if (openNode->TYPE == -1 || (openNode->ROW == goalRow && openNode->COL == goalCol))
		{
			while (openNode != nullptr)
			{
				int row = openNode->ROW;
				int col = openNode->COL;

				HB_LOG("({0}, {1})", row, col);
				mPath.push(Tile(openNode->TYPE, row * TILE_WIDTH, col * TILE_WIDTH));

				if(openNode->conn != nullptr)
					openNode = openNode->conn;
				
			}
		}
		else
		{
			int row = openNode->ROW;
			int col = openNode->COL;

			Node* childNode;

			if (openNode->ROW - 1 >= 0)
			{
				int childIndexRow = openNode->ROW - 1;
				int childIndexCol = openNode->COL;
				childNode = GetChildNodes(childIndexRow, childIndexCol, openNode);
			}

			if (openNode->ROW +1 < maxRow)
			{
				int childIndexRow = openNode->ROW + 1;
				int childIndexCol = openNode->COL;
				childNode = GetChildNodes(childIndexRow, childIndexCol, openNode);
			}

			if (openNode->COL + 1 < maxCol)
			{
				int childIndexRow = openNode->ROW;
				int childIndexCol = openNode->COL + 1;
				childNode = GetChildNodes(childIndexRow, childIndexCol, openNode);
			}

			if (openNode->COL -1 >= 0)
			{
				int childIndexRow = openNode->ROW;
				int childIndexCol = openNode->COL - 1;
				childNode = GetChildNodes(childIndexRow, childIndexCol, openNode);
			}

			openList.remove_if([&](Node* node)
				{
					if (node->ROW == row && node->COL == col)
					{
						return true;
					}
					else
					{
						return false;
					}
				});

			closeList.push_back(openNode);

			FindPath();

		}
	}
	
}

void Enemy::SetStartNode()
{
	int myRow = static_cast<int>(transform->Position.x) / static_cast<int>(TILE_WIDTH);
	int myCol = static_cast<int>(transform->Position.z) / static_cast<int>(TILE_WIDTH);
	Node* startNode = new Node(myRow, myCol);

	openList.push_back(startNode);
}

Node* Enemy::GetChildNodes(int childIndexRow, int childIndexCol, Node* parentNode)
{
	auto it_open = find_if(openList.begin(), openList.end(), [&](Node* node)
		{
			if (node->ROW == childIndexRow && node->COL == childIndexCol)
			{
				return true;
			}
			else
			{
				return false;
			}}
	);

	auto it_close = find_if(closeList.begin(), closeList.end(), [&](Node* node)
		{
			if (node->ROW == childIndexRow && node->COL == childIndexCol)
			{
				return true;
			}
			else
			{
				return false;
			}}
	);

	if (it_open != openList.end())
	{
		if ((*it_open)->G < parentNode->G + 1)
		{
			(*it_open)->G = parentNode->G + 1;
			parentNode->conn = (*it_open);
			(*it_open)->F = (*it_open)->G + (*it_open)->H;

		}
		return *it_open;
	}
	else if (it_close != closeList.end())
	{
		if((*it_close)->G < parentNode->G + 1)
		{
			(*it_close)->G = parentNode->G + 1;
			parentNode->conn = (*it_close);
			(*it_close)->F = (*it_close)->G + (*it_close)->H;
		}
		return *it_close;
	}
	else
	{
		Node* newNode = CreateNodeByIndex(childIndexRow, childIndexCol, parentNode);
		if (newNode != nullptr)
		{
			openList.push_back(newNode);
		}
		return newNode;
	}
	return nullptr;
}

Node* Enemy::CreateNodeByIndex(int rowIndex, int colIndex, Node* parentNode)
{
	int val = graph[rowIndex][colIndex].Type;

	if (val == 2) // Àå¾Ö¹°
	{
		return nullptr;
	}

	auto inds = GetGoalIndex();
	Node* node = nullptr;
	
	if (val == -1)
	{
		node = new Node(-1, rowIndex, colIndex);
		node->G = parentNode->G + 1;
		node->H = 0;
		node->F = node->G;
		node->conn = parentNode;
	}
	else if (val == 0 || val == 1)
	{
		node = new Node(val, rowIndex, colIndex);
		node->G = parentNode->G + 1;

		auto indexs = GetGoalIndex();
		int goalRowIndex = std::get<0>(indexs);
		int goalColIndex = std::get<1>(indexs);

		int h = abs(goalRowIndex - rowIndex) + abs(goalColIndex - colIndex);
		node->H = h;
		node->F = node->G + h;
		node->conn = parentNode;
	}

	return node;

}

bool Enemy::GetNextTarget(Tile* outTarget)
{
	if (mPath.empty())
	{
		return false;
	}
	
	*outTarget = mPath.front();
	mPath.pop();
	return true;
}
