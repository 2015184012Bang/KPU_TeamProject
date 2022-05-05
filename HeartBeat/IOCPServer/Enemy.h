#pragma once

#include "pch.h"
#include "Script.h"
#include "Node.h"
#include "Random.h"
#include "GameMap.h"
#include "UserManager.h"
#include "Timer.h"


class Enemy
	: public Script
{
public:
	Enemy(Entity owner)
		: Script(owner) {}

	virtual void Start() override
	{

		graph = gGameMap.GetGraph();
		if (graph != nullptr)
		{
			LOG("Success to make graph");
		}
		else
		{
			ASSERT(graph, "Failed make graph");
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

		UserManager* userManager;
		UINT32 curUserCount = userManager->GetCurrentUserCount();
		int randomindx = Random::RandInt(0, curUserCount- 1);
		mChasingPlayerPosition = playerPositon[randomindx];
		
		SetStartNode();
		FindPath();
		bool retVal = GetNextTarget(&mCurrentTarget);
	}

	virtual void Update() override
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

	void GetGoalIndex();	// 목표 노드 찾기
	void FindPath();		
	void SetStartNode();	// enemy의 positon으로 시작 노드 설정
	void CheckGoalNode();	// 목표 노드가 변경 됬는지 검사
	int GetClosestNode(int );	// 위치 정보에서 가장 가까운 노드 찾기

	Node* GetChildNodes(int childIndexRow, int childIndexCol, Node* parentNode);
	Node* CreateNodeByIndex(int rowIndex, int colIndex, Node* parentNode);


	bool GetNextTarget(Tile* outTarget);

private:
	vector<Vector3*> playerPositon;
	Vector3* mChasingPlayerPosition;

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