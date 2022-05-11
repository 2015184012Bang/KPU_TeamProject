#pragma once

#include "pch.h"
#include "Script.h"
#include "Node.h"
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
		auto chaseState = make_shared<EnemyChaseState>(static_pointer_cast<Enemy>(shared_from_this()));
		mAIStates.emplace(chaseState->GetStateName(), move(chaseState));

		auto attackState = make_shared<EnemyAttackState>(static_pointer_cast<Enemy>(shared_from_this()));
		mAIStates.emplace(attackState->GetStateName(), move(attackState));

		mCurrentState = getAIState("EnemyChaseState");
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
			mCurrentState->Exit();
			mCurrentState = iter->second;
			mCurrentState->Enter();
		}
		else
		{
			LOG("There is no AI state named: {0}", stateName);
		}
	}

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
};