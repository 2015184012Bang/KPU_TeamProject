#pragma once

#include "pch.h"
#include "Script.h"
#include "AIState.h"

class RedCell
	: public AIScript
{
public:
	RedCell(entt::registry& registry, entt::entity owner)
		: AIScript{ registry, owner } {}


	virtual void Start() override
	{
		auto deliverState = make_shared<CellDeliverState>(static_pointer_cast<RedCell>(shared_from_this()));
		AddState(deliverState);

		auto restState = make_shared<CellRestState>(static_pointer_cast<RedCell>(shared_from_this()));
		AddState(restState);

		mCart = GetEntityByName(mRegistry, "Cart");
		ASSERT(mRegistry.valid(mCart), "Invalid entity!");

		SetTargetHouse();
		StartState("CellDeliverState");
	}

	virtual void Update() override
	{
		AIScript::Update();
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
	entt::entity mTarget = entt::null;
	entt::entity mCart = entt::null;
};