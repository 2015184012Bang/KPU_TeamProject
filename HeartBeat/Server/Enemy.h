#pragma once

#include "ServerPCH.h"

#include "HeartBeat/Script.h"
#include "HeartBeat/GameMap.h"
#include "HeartBeat/Log.h"
#include "HeartBeat/Tags.h"
#include "HeartBeat/Node.h"
#include "HeartBeat/Define.h"

#include "ServerSystems.h"

class Enemy : public Script
{
public:
	Enemy(Entity owner)
		: Script(owner)
	{}

	virtual void Start() override
	{

		graph = gGameMap.GetGraph();
		if (graph != nullptr)
		{
			HB_LOG("Success to make graph");
		}
		else
		{
			HB_ASSERT(graph, "Failed make graph");
		}
		maxRow = gGameMap.GetMaxRow();
		maxCol = gGameMap.GetMaxCol();

		transform = &GetComponent<STransformComponent>();

		
		auto players = FindObjectsWithTag<Tag_Player>();

		for (auto& p : players)
		{
			auto& playerTransform = p.GetComponent<STransformComponent>();
			playerPositon.push_back(playerTransform.Position);
		}
		setPlayerPosition = &playerPositon[0];


		/*int randomindx = Random::RandInt(0, 0);
		playerOne = &playerPositon[randomindx];*/

	}

	virtual void Update(float deltaTime) override
	{
		Vector3 to = Vector3(mCurrrentTarget.X, 0.0f, mCurrrentTarget.Z);

		if (!mbChase)
		{
			if (!NearZero(transform->Position, to))
			{
				mbChase = true;
				openList.clear();
				closeList.clear();
				SetStartNode();
			}
		}
		else if (mbChase)
		{

			openList.clear();
			closeList.clear();
			SetStartNode();

			FindPath();

			ServerSystems::MoveToward(transform, to, Timer::GetDeltaTime());
			AddTag<Tag_UpdateTransform>();

			if (NearZero(transform->Position, to))
			{
				bool retVal = GetNextTarget(&mCurrrentTarget);

				if (!retVal)
				{
					mbChase = false;
				}
			}

		}

	}

	tuple<int, int> GetGoalIndex();
	void FindPath();
	void SetStartNode();

	Node* GetChildNodes(int childIndexRow, int childIndexCol, Node* parentNode);
	Node* CreateNodeByIndex(int rowIndex, int colIndex, Node* parentNode);

	bool GetNextTarget(Tile* outTarget);


private:
	vector<Vector3> playerPositon;
	Vector3* setPlayerPosition;

	STransformComponent* transform = nullptr;
	STransformComponent* playerTransform = nullptr;

	std::queue<Tile> mPath;
	Tile mCurrrentTarget;

	int maxRow;
	int maxCol;
	int goalRow, goalCol;
	Tile** graph;

	list<Node*> openList;
	list<Node*> closeList;


	bool mbChase = true;
};
