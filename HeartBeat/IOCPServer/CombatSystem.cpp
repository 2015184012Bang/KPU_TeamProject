#include "pch.h"
#include "CombatSystem.h"

#include "Entity.h"
#include "Timer.h"
#include "Values.h"
#include "Room.h"

CombatSystem::CombatSystem(entt::registry& registry, shared_ptr<Room>&& room)
	: mRegistry{ registry }
	, mOwner{ move(room) }
{

}

void CombatSystem::Update()
{
	updateCooldown();
	checkEnemyHit();
}

void CombatSystem::SetPreset(const INT8 clientID, UpgradePreset preset)
{
	auto actor = GetEntityByID(mRegistry, clientID);
	ASSERT(mRegistry.valid(actor), "Invalid entity!");

	auto& combat = mRegistry.get<CombatComponent>(actor);

	switch (preset)
	{
	case UpgradePreset::ATTACK:
		combat.BaseAttackDmg = 3;
		combat.Armor = 1;
		combat.Regeneration = 2;
		break;

	case UpgradePreset::HEAL:
		combat.BaseAttackDmg = 1;
		combat.Armor = 2;
		combat.Regeneration = 3;
		break;

	case UpgradePreset::SUPPORT:
		combat.BaseAttackDmg = 2;
		combat.Armor = 3;
		combat.Regeneration = 2;
		break;
	}

	combat.BaseAttackCooldown = Values::BaseAttackCooldown;
}

bool CombatSystem::CanBaseAttack(const INT8 clientID)
{
	auto actor = GetEntityByID(mRegistry, clientID);
	ASSERT(mRegistry.valid(actor), "Invalid entity!");
	auto& combat = mRegistry.get<CombatComponent>(actor);

	if (combat.BaseAttackTracker > combat.BaseAttackCooldown)
	{
		combat.BaseAttackTracker = 0.0f;
		return true;
	}
	else
	{
		return false;
	}
}

void CombatSystem::updateCooldown()
{
	auto view = mRegistry.view<CombatComponent>();
	for (auto [entity, combat] : view.each())
	{
		combat.BaseAttackTracker += Timer::GetDeltaTime();
	}
}

void CombatSystem::checkEnemyHit()
{
	auto view = mRegistry.view<IHitYouComponent>();
	for (auto [entity, hit] : view.each())
	{
		auto victim = GetEntityByID(mRegistry, hit.VictimID);
		auto& health = mRegistry.get<HealthComponent>(victim);
		--health.Health;

		if (health.Health <= 0)
		{
			// TODO: 플레이어 사망 처리
			LOG("Player dead...");
		}

		NOTIFY_ATTACK_PACKET packet = {};
		packet.EntityID = hit.HitterID;
		packet.PacketID = NOTIFY_ATTACK;
		packet.PacketSize = sizeof(packet);
		packet.Result = RESULT_CODE::ATTACK_SUCCESS;
		mOwner->Broadcast(sizeof(packet), reinterpret_cast<char*>(&packet));

		mRegistry.remove<IHitYouComponent>(entity);
	}
}
