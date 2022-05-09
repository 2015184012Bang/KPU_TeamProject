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
		graph = GameMap::GetInstance().GetGraph(gameMap);
		
		auto& transform = GetComponent<TransformComponent>();
		
		auto players = FindObjectsWithTag<Tag_Player>();
		for (auto& p : players)
		{
			auto& playerTransform = mRegistry.get<TransformComponent>(p).Position;
			playerPositon.push_back(&playerTransform);
		}
		
		// �i�� �÷��̾� ��ȣ�� �޾� ������ �÷��̾ �i��
		auto& userManager = UserManager::GetInstance();
		UINT32 curUserCount = userManager.GetCurrentUserCount();
		mChasingPlayerNum = Random::RandInt(0, curUserCount);
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
			auto& playerTransform = mRegistry.get<TransformComponent>(p).Position;
			playerPositon.push_back(&playerTransform);
		}
		mChasingPlayerPosition = playerPositon[mChasingPlayerNum];


		if (mbChase == false)	// �������¿��� �÷��̾���� �Ÿ��� �ٽ� �־����� ��ã�� �����
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

			if (posToPlayer < FAR_ENOUGH) // �÷��̾���� �Ÿ��� ��������� ����
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

	void SetStartNode();		// enemy�� positon���� ���� ��� ����
	void GetGoalIndex();		// ��ǥ(Player Pos) ��� ã��
	void FindPath();			// ��ã�� ����
	void CheckGoalNode();		// ��ǥ ��尡 ���� ����� �˻�
	int GetClosestNode(UINT32);	// ��ġ �������� ���� ����� ��� ã��
	void ResetPath();			// ��ã�⿡ �ʿ��� ���� �ʱ�ȭ
	bool GetNextTarget(Vector3* outTarget);

	Node* GetChildNodes(UINT32 childIndexRow, UINT32 childIndexCol, Node* parentNode);
	Node* CreateNodeByIndex(UINT32 rowIndex, UINT32 colIndex, Node* parentNode);

private:
	vector<Vector3*> playerPositon;
	UINT32 mChasingPlayerNum;			// ���ʹ̰� �i�� �÷��̾� ��ȣ
	Vector3* mChasingPlayerPosition;	// ���ʹ̰� �i�� �÷��̾� ������
	Vector3 mCurrentTarget = Vector3::Zero;

	Tile** graph;

	UINT32 maxRow, maxCol;
	UINT32 goalRow, goalCol;

	stack<Tile> mPath;
	list<Node*> openList;
	list<Node*> closeList;

	bool mbIsSamePath = true;
	bool mbChase = false;

	const float TILE_WIDTH = Values::TileSide;
	const float CLOSE_ENOUGH = 5.0f * 5.0f;
	const float FAR_ENOUGH = TILE_WIDTH*TILE_WIDTH;	// ���ʹ��� �ൿ������ �ٲ�� �Ÿ�
};