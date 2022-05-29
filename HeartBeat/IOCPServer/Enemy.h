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
		auto npcs = FindObjectsWithTag<Tag_RedCell>();

		if (npcs.empty())
		{
			SetTargetTank();
			return;
		}

		mTarget = npcs[Random::RandInt(0, static_cast<INT32>(npcs.size() - 1))];
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
		return mRegistry.valid(mTarget);
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

		else
		{
			ASSERT(false, "Unknown enemy type!");
		}
	}

private:
	entt::entity mTarget = entt::null;
};