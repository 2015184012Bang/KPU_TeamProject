#pragma once

#include "pch.h"
#include "Script.h"
#include "Random.h"
#include "GameMap.h"
#include "UserManager.h"
#include "Timer.h"
#include "Values.h"
#include "AIState.h"

class Enemy
	: public Script
{
public:
	Enemy(entt::registry& registry, entt::entity owner)
		: Script{ registry, owner } {}


	virtual void Start() override
	{
		auto tankChaseState = make_shared<EnemyTankChaseState>(static_pointer_cast<Enemy>(shared_from_this()));
		mAIStates.emplace(tankChaseState->GetStateName(), move(tankChaseState));

		auto playerChaseState = make_shared<EnemyPlayerChaseState>(static_pointer_cast<Enemy>(shared_from_this()));
		mAIStates.emplace(playerChaseState->GetStateName(), move(playerChaseState));

		auto attackState = make_shared<EnemyAttackState>(static_pointer_cast<Enemy>(shared_from_this()));
		mAIStates.emplace(attackState->GetStateName(), move(attackState));

		mCurrentState = getAIState("EnemyTankChaseState");
		mPreviousState = mCurrentState;
		ASSERT(mCurrentState, "CurrentState is nullptr!");
		mCurrentState->Enter();
	}

	virtual void Update() override
	{
		if (mCurrentState)
		{
			mCurrentState->Update();
		}
	}

	void ChangeState(string_view stateName)
	{
		string key{ stateName.data() };

		if (auto iter = mAIStates.find(key); iter != mAIStates.end())
		{
			mPreviousState = mCurrentState;
			mCurrentState->Exit();
			mCurrentState = iter->second;
			mCurrentState->Enter();
		}
		else
		{
			LOG("There is no AI state named: {0}", stateName);
		}
	}

	void ChangeToPreviousState()
	{
		mCurrentState->Exit();
		mPreviousState.swap(mCurrentState);
		mCurrentState->Enter();
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
	shared_ptr<AIState> getAIState(string_view stateName)
	{
		string key{ stateName.data() };

		if (auto iter = mAIStates.find(key); iter != mAIStates.end())
		{
			return iter->second;
		}
		else
		{
			LOG("There is no AI state named: {0}", stateName);
			return nullptr;
		}
	}

private:
	unordered_map<string, shared_ptr<AIState>> mAIStates;
	shared_ptr<AIState> mCurrentState = nullptr;
	shared_ptr<AIState> mPreviousState = nullptr;

	entt::entity mTarget = entt::null;
};