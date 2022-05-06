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
		GameMap* gameMap;
		graph = gameMap->GetGraph();
		ASSERT(graph, "Failed to make graph");


		Map map = gameMap->GetMap("Map01.csv");
		
		maxRow = map.MaxRow;
		maxCol = map.MaxCol;

		auto& transform = GetComponent<TransformComponent>();
		//transform = &GetComponent<STransformComponent>();
		
		auto players = FindObjectsWithTag<Tag_Player>();
		for (auto& p : players)
		{
			auto& playerTransform = p.GetComponent<TransformComponent>();
			playerPositon.push_back(&playerTransform.Position);
		}

		UserManager* user;
		UINT32 curUserCount = user->GetCurrentUserCount();
		mChasingPlayerNum = Random::RandInt(0, curUserCount- 1);
		mChasingPlayerPosition = playerPositon[mChasingPlayerNum];
		
		SetStartNode();
		FindPath();
		bool retVal = GetNextTarget(&mCurrentTarget);
	}

	virtual void Update() override
	{
		const auto& position = GetComponent<TransformComponent>().Position;
		const float posToEnd = Vector3::DistanceSquared(position, *mChasingPlayerPosition);

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

			const float posToCur = Vector3::DistanceSquared(position, mCurrentTarget);

			Vector3 direction = XMVectorSubtract(mCurrentTarget, position);
			direction.Normalize();
			GetComponent<MovementComponent>().Direction = direction;

			if (posToCur < CLOSE_ENOUGH)
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
	int GetClosestNode(UINT32);	// 위치 정보에서 가장 가까운 노드 찾기

	Node* GetChildNodes(UINT32 childIndexRow, UINT32 childIndexCol, Node* parentNode);
	Node* CreateNodeByIndex(UINT32 rowIndex, UINT32 colIndex, Node* parentNode);


	bool GetNextTarget(Tile* outTarget);
	bool GetNextTarget(Vector3* outTarget);

private:
	vector<Vector3*> playerPositon;
	Vector3* mChasingPlayerPosition;
	UINT32 mChasingPlayerNum;

	std::stack<Tile> mPath;
	std::tuple<UINT32, UINT32> mGoalIndex;
	Vector3 mCurrentTarget = Vector3::Zero;

	UINT32 maxRow, maxCol;
	UINT32 goalRow, goalCol;
	Tile** graph;

	list<Node*> openList;
	list<Node*> closeList;

	bool mbIsSamePath = true;
	bool mbChase = true;

	const float CLOSE_ENOUGH = 10.0f * 10.0f;
	const float TILE_WIDTH = 500.0f;
};