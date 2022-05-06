#include "pch.h"
#include "Enemy.h"

void Enemy::GetGoalIndex()
{	
	goalRow = GetClosestNode(static_cast<int>(mChasingPlayerPosition->z));
	goalCol = GetClosestNode(static_cast<int>(mChasingPlayerPosition->x));

	mGoalIndex = std::make_tuple(goalRow, goalCol);
	//HB_LOG("[Player]({0},{1})", mChasingPlayerPosition->x, mChasingPlayerPosition->z);
	LOG("Get GoalNode : ({0},{1})", goalRow, goalCol);
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
		if (openNode->ROW == goalRow && openNode->COL == goalCol)
		{
			while (openNode != nullptr)
			{
				int row = openNode->ROW;
				int col = openNode->COL;

				LOG("({0}, {1})", row, col);
				if(openNode->CON != nullptr)
					mPath.push(Tile(openNode->TYPE, col * TILE_WIDTH, row * TILE_WIDTH));

				openNode = openNode->CON;
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
			//HB_LOG("CloseNode({0},{1})", openNode->ROW, openNode->COL);

			FindPath();

		}
	}
	
}

void Enemy::SetStartNode()
{
	auto& transform = GetComponent<TransformComponent>();
	int myRow = GetClosestNode(static_cast<int>(transform.Position.z));
	int myCol = GetClosestNode(static_cast<int>(transform.Position.x));

	Node* startNode = new Node(myRow, myCol);

	//HB_LOG("[Enemy]({0},{1})", transform->Position.x, transform->Position.z);
	LOG("Set StartNode : ({0},{1})", myRow, myCol);

	openList.push_back(startNode);

	GetGoalIndex();
}

void Enemy::CheckGoalNode()
{
	int curGoalRow = GetClosestNode(static_cast<int>(mChasingPlayerPosition->z));
	int curGoalCol = GetClosestNode(static_cast<int>(mChasingPlayerPosition->x));

	if (curGoalRow == goalRow && curGoalCol == goalCol)
	{
		mbIsSamePath = true;
	}
	else
	{
		mbIsSamePath = false;
	}
}

Node* Enemy::GetChildNodes(UINT32 childIndexRow, UINT32 childIndexCol, Node* parentNode)
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
			parentNode->CON = (*it_open);
			(*it_open)->F = (*it_open)->G + (*it_open)->H;

		}
		return *it_open;
	}
	else if (it_close != closeList.end())
	{
		if((*it_close)->G < parentNode->G + 1)
		{
			(*it_close)->G = parentNode->G + 1;
			parentNode->CON = (*it_close);
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

Node* Enemy::CreateNodeByIndex(UINT32 rowIndex, UINT32 colIndex, Node* parentNode)
{
	TileType val = graph[rowIndex][colIndex].TType;
	//HB_LOG("({0},{1}):Type({2})", rowIndex, colIndex, val);

	if (val == TileType::BLOCKED) // Àå¾Ö¹°
	{
		return nullptr;
	}

	Node* node = nullptr;
	
	if (val == TileType::MOVABLE)
	{
		node = new Node(val, rowIndex, colIndex);
		node->G = parentNode->G + 1;

		UINT32 goalRowIndex = std::get<0>(mGoalIndex);
		UINT32 goalColIndex = std::get<1>(mGoalIndex);
		
		UINT32 h = abs((int)goalRowIndex - (int)rowIndex) + abs((int)goalColIndex - (int)colIndex);
		node->H = h;
		node->F = node->G + h;
		node->CON = parentNode;
	}

	return node;
}

int Enemy::GetClosestNode(UINT32 pos)
{
	int nodePos = pos / 500;
	if ((pos % static_cast<int>(TILE_WIDTH)) < ((static_cast<int>(TILE_WIDTH) / 2 ) - 1))
		return nodePos;
	else
		return nodePos + 1;

}

bool Enemy::GetNextTarget(Tile* outTarget)
{
	if (mPath.empty())
	{
		return false;
	}
	
	*outTarget = mPath.top();
	mPath.pop();

	return true;
}

bool Enemy::GetNextTarget(Vector3* outTarget)
{
	if (mPath.empty())
	{
		return false;
	}

	Vector3* position = nullptr;
	position->x = mPath.top().X;
	position->y = 0.0;
	position->z = mPath.top().Z;
	*outTarget = *position;
	
	mPath.pop();

	return true;
}