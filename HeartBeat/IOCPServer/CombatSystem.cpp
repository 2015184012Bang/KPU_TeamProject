#include "pch.h"
#include "CombatSystem.h"

#include "Entity.h"
#include "GameManager.h"
#include "Timer.h"

void CombatSystem::Update()
{
	auto view = gRegistry.view<CombatComponent>();

	for (auto [entity, combat] : view.each())
	{
		combat.BaseAttackTracker += Timer::GetDeltaTime();
	}
}

void CombatSystem::SetPreset(const UINT32 eid, UpgradePreset preset)
{
	auto actor = GetEntity(eid);
	ASSERT(actor, "Invalid entity!");

	auto& combat = actor.GetComponent<CombatComponent>();

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

	combat.BaseAttackCooldown = gBaseAttackCooldown;
}

bool CombatSystem::CanBaseAttack(const UINT32 eid)
{
	auto actor = GetEntity(eid);
	ASSERT(actor, "Invalid entity!");
	auto& combat = actor.GetComponent<CombatComponent>();

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
