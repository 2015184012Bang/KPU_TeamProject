#pragma once

#include "pch.h"
#include "Script.h"
#include "AIState.h"

class RedCell
	: public Script
{
public:
	RedCell(entt::registry& registry, entt::entity owner)
		: Script{ registry, owner } {}


	virtual void Start() override
	{
		auto deliverState = make_shared<CellDeliverState>(static_pointer_cast<RedCell>(shared_from_this()));
		mAIStates.emplace(deliverState->GetStateName(), move(deliverState));

		mCart = GetEntityByName(mRegistry, "Cart");
		ASSERT(mRegistry.valid(mCart), "Invalid entity!");

		SetTargetHouse();

		mCurrentState = getAIState("CellDeliverState");
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

	void SetTargetHouse()
	{
		auto houses = mRegistry.view<Tag_HouseTile, TransformComponent>();

		float distSq = FLT_MAX;
		entt::entity targetEntity = entt::null;

		const auto& myPosition = GetComponent<TransformComponent>().Position;
		for (auto [entity, transform] : houses.each())
		{
			float distTargetSq = Vector3::DistanceSquared(transform.Position, myPosition);

			if (distSq > distTargetSq)
			{
				distSq = distTargetSq;
				targetEntity = entity;
			}
		}

		mTarget = targetEntity;
	}

	void SetTargetCart()
	{
		mTarget = mCart;
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

	entt::entity mTarget = entt::null;
	entt::entity mCart = entt::null;
};