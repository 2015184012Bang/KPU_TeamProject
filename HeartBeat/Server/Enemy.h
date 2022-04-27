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
			playerPositon.push_back(&playerTransform.Position);
		}
		mChasingPlayerPosition = playerPositon[0];

		SetStartNode();
		FindPath();
		bool retVal = GetNextTarget(&mCurrentTarget);

		/*int randomindx = Random::RandInt(0, 0);
		playerOne = &playerPositon[randomindx];*/

	}

	virtual void Update(float deltaTime) override
	{
		//HB_LOG("[player]({0},{1}", mChasingPlayerPosition->x, mChasingPlayerPosition->z);
		//HB_LOG("[Enemy]({0},{1})", transform->Position.x, transform->Position.z);

		if (!mbChase)
		{
			mbChase = true;
			openList.clear();
			closeList.clear();
			SetStartNode();
			goalRow = -1;
			mbFind = false;

			FindPath();
			bool retVal = GetNextTarget(&mCurrentTarget);
		}
		else if (mbChase)
		{

			//openList.clear();
			//closeList.clear();
			//SetStartNode();
			FindPath();

			
			Vector3 to = Vector3(mCurrentTarget.X, 0.0f, mCurrentTarget.Z);
			//HB_LOG("MoveToward : ({0},{1})", mCurrentTarget.X, mCurrentTarget.Z);


			ServerSystems::MoveToward(transform, to, Timer::GetDeltaTime());
			AddTag<Tag_UpdateTransform>();


			if (NearZero(transform->Position, to))
			{
				
				bool retVal = GetNextTarget(&mCurrentTarget);

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
	bool CheckPath();	// true : 갱신 필요 X	false : 갱신 필요 O

	Node* GetChildNodes(int childIndexRow, int childIndexCol, Node* parentNode);
	Node* CreateNodeByIndex(int rowIndex, int colIndex, Node* parentNode);

	bool GetNextTarget(Tile* outTarget);
	

private:
	vector<Vector3*> playerPositon;
	Vector3* mChasingPlayerPosition;
	Vector3 mPrevPlayerPosition;

	STransformComponent* transform = nullptr;
	STransformComponent* playerTransform = nullptr;

	std::stack<Tile> mPath;
	Tile mCurrentTarget;

	int maxRow;
	int maxCol;
	int goalRow, goalCol;
	Tile** graph;

	list<Node*> openList;
	list<Node*> closeList;

	bool mbFind = false;
	bool mbChase = true;
};
