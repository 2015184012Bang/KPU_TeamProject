#pragma once

#include "pch.h"
#include "Script.h"

#include "Timer.h"

class WhiteCell 
	: public Script
{
public:
	WhiteCell(entt::registry& registry, entt::entity owner)
		: Script{ registry, owner } {}

	virtual void Start() override
	{
		
	}

	virtual void Update() override
	{
		mCooldownTracker += Timer::GetDeltaTime();

		if (mCooldownTracker < ATTACK_COOLDOWN)
		{
			return;
		}

		auto target = findNearestEnemy();
		if (entt::null == target)
		{
			return;
		}

		const auto myID = GetComponent<IDComponent>().ID;
		const auto targetID = mRegistry.get<IDComponent>(target).ID;
		AddComponent<IHitYouComponent>(myID, targetID);

		mCooldownTracker = 0.0f;
	}

private:
	entt::entity findNearestEnemy()
	{
		entt::entity target = entt::null;

		const auto& myPos = GetComponent<TransformComponent>().Position;
		auto enemies = mRegistry.view<Tag_Enemy, TransformComponent>();
		for (auto [enemy, transform] : enemies.each())
		{
			const auto& enemyPos = transform.Position;

			float distSq = Vector3::DistanceSquared(myPos, enemyPos);

			if (distSq < ATTACK_RANGE_SQ)
			{
				target = enemy;
				break;
			}
		}

		return target;
	}

private:
	static constexpr float ATTACK_COOLDOWN = 2.0f;
	static constexpr float ATTACK_RANGE_SQ = 800.0f * 800.0f;
	float mCooldownTracker = 0.0f;
};