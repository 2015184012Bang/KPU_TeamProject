#pragma once

#include "pch.h"
#include "Script.h"
#include "AIState.h"
#include "Random.h"

class Enemy
	: public AIScript
{
public:
	Enemy(entt::registry& registry, entt::entity owner)
		: AIScript{ registry, owner } {}


	virtual void Start() override
	{
		initByEnemyType();
	}

	virtual void Update() override
	{
		AIScript::Update();
	}

	void SetTargetTank()
	{
		auto tank = Find("Tank");
		ASSERT(tank != entt::null, "Invalid entity!");
		mTarget = tank;
	}

	void SetTargetPlayer(entt::entity player)
	{
		mTarget = player;
	}

	void SetTargetNPC()
	{
		auto redCells = FindObjectsWithTag<Tag_RedCell>();

		if (redCells.empty())
		{
			auto whiteCells = FindObjectsWithTag<Tag_WhiteCell>();

			if (whiteCells.empty())
			{
				SetTargetTank();
			}
			else
			{
				mTarget = whiteCells[Random::RandInt(0, static_cast<INT32>(whiteCells.size() - 1))];
			}
			return;
		}

		mTarget = redCells[Random::RandInt(0, static_cast<INT32>(redCells.size() - 1))];
	}

	bool HasNearPlayer(OUT entt::entity& target)
	{
		static float aggroDistSq = 800.0f * 800.0f;

		auto players = FindObjectsWithTag<Tag_Player>();
		if (players.empty())
		{
			return false;
		}

		const auto& myPosition = GetComponent<TransformComponent>().Position;
		for (auto player : players)
		{
			if (mRegistry.any_of<Tag_Dead>(player))
			{
				continue;
			}

			const auto& playerPosition = mRegistry.get<TransformComponent>(player).Position;

			float distSq = Vector3::DistanceSquared(playerPosition, myPosition);

			if (distSq < aggroDistSq)
			{
				target = player;
				return true;
			}
		}

		return false;
	}

	bool IsTargetValid()
	{
		return mRegistry.valid(mTarget) && !mRegistry.any_of<Tag_Dead>(mTarget);
	}

	entt::entity GetTarget() const { return mTarget; }

private:
	void initByEnemyType()
	{
		if (HasComponent<Tag_Virus>())
		{
			auto tankChaseState = make_shared<EnemyTankChaseState>(static_pointer_cast<Enemy>(shared_from_this()));
			AddState(tankChaseState);

			auto playerChaseState = make_shared<EnemyPlayerChaseState>(static_pointer_cast<Enemy>(shared_from_this()));
			AddState(playerChaseState);

			auto attackState = make_shared<EnemyAttackState>(static_pointer_cast<Enemy>(shared_from_this()));
			AddState(attackState);

			StartState("EnemyTankChaseState");
		}

		else if (HasComponent<Tag_Dog>())
		{
			auto npcChaseState = make_shared<EnemyNPCChaseState>(static_pointer_cast<Enemy>(shared_from_this()));
			AddState(npcChaseState);

			auto attackState = make_shared<EnemyAttackState>(static_pointer_cast<Enemy>(shared_from_this()));
			AddState(attackState);

			StartState("EnemyNPCChaseState");
		}

		else if (HasComponent<Tag_Boss>())
		{
			auto bossIdleState = make_shared<BossIdleState>(static_pointer_cast<Enemy>(shared_from_this()));
			AddState(bossIdleState);

			auto bossSpecialState = make_shared<BossSpecialAttackState>(static_pointer_cast<Enemy>(shared_from_this()));
			AddState(bossSpecialState);

			auto bossOneState = make_shared<BossAttackOneState>(static_pointer_cast<Enemy>(shared_from_this()));
			AddState(bossOneState);

			auto bossTwoState = make_shared<BossAttackTwoState>(static_pointer_cast<Enemy>(shared_from_this()));
			AddState(bossTwoState);

			StartState("BossIdleState");
		}

		else
		{
			ASSERT(false, "Unknown enemy type!");
		}
	}

private:
	entt::entity mTarget = entt::null;
};