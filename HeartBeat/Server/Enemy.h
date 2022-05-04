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
		int randomindx = Random::RandInt(0, MAX_PLAYER-1);
		mChasingPlayerPosition = playerPositon[randomindx];

		SetStartNode();
		FindPath();
		bool retVal = GetNextTarget(&mCurrentTarget);
	}

	virtual void Update(float deltaTime) override
	{
		if (!mbChase)
		{
			mbChase = true;
			openList.clear();
			closeList.clear();
			SetStartNode();
			mbIsSamePath = true;

			FindPath();
			bool retVal = GetNextTarget(&mCurrentTarget);
		}
		else if (mbChase)
		{
			CheckGoalNode();

			if (!mbIsSamePath)
			{
				openList.clear();
				closeList.clear();
				std::stack<Tile> mPath;
				SetStartNode();
				mbIsSamePath = true;

				FindPath();
				bool retVal = GetNextTarget(&mCurrentTarget);
			}

			Vector3 to = Vector3(mCurrentTarget.X, 0.0f, mCurrentTarget.Z);

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

	void GetGoalIndex();	// ��ǥ ��� ã��
	void FindPath();		
	void SetStartNode();	// enemy�� positon���� ���� ��� ����
	void CheckGoalNode();	// ��ǥ ��尡 ���� ����� �˻�
	int GetClosestNode(int );	// ��ġ �������� ���� ����� ��� ã��

	Node* GetChildNodes(int childIndexRow, int childIndexCol, Node* parentNode);
	Node* CreateNodeByIndex(int rowIndex, int colIndex, Node* parentNode);


	bool GetNextTarget(Tile* outTarget);

private:
	vector<Vector3*> playerPositon;
	Vector3* mChasingPlayerPosition;

	STransformComponent* transform = nullptr;
	STransformComponent* playerTransform = nullptr;

	std::stack<Tile> mPath;
	std::tuple<int, int> mGoalIndex;
	Tile mCurrentTarget;

	int maxRow, maxCol;
	int goalRow, goalCol;
	Tile** graph;

	list<Node*> openList;
	list<Node*> closeList;

	bool mbIsSamePath = true;
	bool mbChase = true;
};
