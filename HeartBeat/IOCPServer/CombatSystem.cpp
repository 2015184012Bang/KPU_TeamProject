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
	checkEnemyAttack();
}

void CombatSystem::SetPreset(const INT8 clientID, UpgradePreset preset)
{
	auto actor = GetEntityByID(mRegistry, clientID);
	ASSERT(mRegistry.valid(actor), "Invalid entity!");

	auto& combat = mRegistry.get<CombatComponent>(actor);

	switch (preset)
	{
	case UpgradePreset::ATTACK:
		combat.Preset = UpgradePreset::ATTACK;
		combat.BaseAttackDmg = 3;
		combat.Armor = 1;
		combat.Regeneration = 2;
		break;

	case UpgradePreset::HEAL:
		combat.Preset = UpgradePreset::HEAL;
		combat.BaseAttackDmg = 1;
		combat.Armor = 2;
		combat.Regeneration = 3;
		break;

	case UpgradePreset::SUPPORT:
		combat.Preset = UpgradePreset::SUPPORT;
		combat.BaseAttackDmg = 2;
		combat.Armor = 3;
		combat.Regeneration = 2;
		break;
	}

	combat.BaseAttackCooldown = Values::BaseAttackCooldown;
	combat.BaseAttackTracker = combat.BaseAttackCooldown;
	combat.SkillCooldown = Values::SkillCooldown;
	combat.SkillTracker = combat.SkillCooldown;
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

bool CombatSystem::CanUseSkill(const INT8 clientID)
{
	auto actor = GetEntityByID(mRegistry, clientID);
	ASSERT(mRegistry.valid(actor), "Invalid entity!");
	auto& combat = mRegistry.get<CombatComponent>(actor);

	if (combat.SkillTracker > combat.SkillCooldown)
	{
		combat.SkillTracker = 0.0f;
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
		combat.SkillTracker += Timer::GetDeltaTime();
	}
}

void CombatSystem::checkEnemyAttack()
{
	auto view = mRegistry.view<IHitYouComponent>();
	for (auto [entity, hit] : view.each())
	{
		auto victim = GetEntityByID(mRegistry, hit.VictimID);
		if (entt::null == victim)
		{
			continue;
		}

		auto& health = mRegistry.get<HealthComponent>(victim);
		--health.Health;

		if (health.Health <= 0)
		{
			// TODO: �÷��̾�/NPC ��� ó��
		}

		NOTIFY_ENEMY_ATTACK_PACKET packet = {};
		packet.HitterID = hit.HitterID;
		packet.VictimID = hit.VictimID;
		packet.PacketSize = sizeof(packet);
		packet.PacketID = NOTIFY_ENEMY_ATTACK;
		mOwner->Broadcast(sizeof(packet), reinterpret_cast<char*>(&packet));

		mRegistry.remove<IHitYouComponent>(entity);

		// Dog�� ��� ���� �� �� �ϰ� ����.
		// Ŭ���̾�Ʈ���� Dog�� ������ ��� �ִϸ��̼� ��� �� ����.
		if (mRegistry.any_of<Tag_Dog>(entity))
		{
			DestroyEntity(mRegistry, entity);
		}
	}
}
