#pragma once

#include "pch.h"
#include "Script.h"
#include "Node.h"
#include "Random.h"
#include "GameMap.h"
#include "UserManager.h"
#include "Timer.h"
#include "Values.h"


class Enemy
	: public Script
{
public:
	Enemy(entt::registry& registry, entt::entity owner)
		: Script{ registry, owner } {}

	virtual void Start() override
	{
		auto& gameMap = GameMap::GetInstance().GetMap("../Assets/Maps/Map01.csv");

		maxRow = gameMap.MaxRow;
		maxCol = gameMap.MaxCol;

		graph = gameMap.Graph;

		auto& transform = GetComponent<TransformComponent>();
		
		auto players = FindObjectsWithTag<Tag_Player>();
		for (auto& p : players)
		{
			//auto& playerTransform = p.GetComponent<TransformComponent>();
			auto& playerTransform = mRegistry.get<TransformComponent>(p).Position;
			playerPositon.push_back(&playerTransform);
		}

		auto& userManager = UserManager::GetInstance();
		UINT32 curUserCount = userManager.GetCurrentUserCount();
		mChasingPlayerNum = Random::RandInt(0, 2);
		mChasingPlayerPosition = playerPositon[mChasingPlayerNum];
		LOG("{0}", mChasingPlayerNum);
		
		SetStartNode();
		FindPath();
		bool retVal = GetNextTarget(&mCurrentTarget);
	}

	virtual void Update() override
	{
		const auto& position = GetComponent<TransformComponent>().Position;
		const float posToEnd = Vector3::DistanceSquared(position, *mChasingPlayerPosition);

		auto players = FindObjectsWithTag<Tag_Player>();
		for (auto& p : players)
		{
			auto& playerTransform = p.GetComponent<TransformComponent>();
			playerPositon.push_back(&playerTransform.Position);
		}
		mChasingPlayerPosition = playerPositon[mChasingPlayerNum];


		if (mbChase == false)
		{
			const float posToPlayer = Vector3::DistanceSquared(position, *playerPositon[mChasingPlayerNum]);

			ResetPath();

			if (posToPlayer > FAR_ENOUGH)
			{
				mbChase = true;
			}

		}
		else if (mbChase == true)
		{
			CheckGoalNode();

			if (mbIsSamePath == false)
			{
				ResetPath();
			}

			const float posToCur = Vector3::DistanceSquared(position, mCurrentTarget);
			const float posToPlayer = Vector3::DistanceSquared(position, *playerPositon[mChasingPlayerNum]);

			if (posToPlayer < FAR_ENOUGH)
			{
				mbChase = false;
				Vector3 direction = XMVectorSubtract(position, *playerPositon[mChasingPlayerNum]);
				direction.Normalize();
				GetComponent<MovementComponent>().Direction = Vector3::Zero;

				LOG("Done");
			}
			else
			{

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

	}

	void GetGoalIndex();		// 목표 노드 찾기
	void FindPath();			// 길찾기 수행
	void SetStartNode();		// enemy의 positon으로 시작 노드 설정
	void CheckGoalNode();		// 목표 노드가 변경 됬는지 검사
	int GetClosestNode(UINT32);	// 위치 정보에서 가장 가까운 노드 찾기
	void ResetPath();			// 길찾기에 필요한 변수 초기화

	Node* GetChildNodes(UINT32 childIndexRow, UINT32 childIndexCol, Node* parentNode);
	Node* CreateNodeByIndex(UINT32 rowIndex, UINT32 colIndex, Node* parentNode);


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
	bool mbChase = false;

	const float TILE_WIDTH = Values::TileSide;
	const float CLOSE_ENOUGH = 5.0f * 5.0f;
	const float FAR_ENOUGH = TILE_WIDTH*TILE_WIDTH;
};