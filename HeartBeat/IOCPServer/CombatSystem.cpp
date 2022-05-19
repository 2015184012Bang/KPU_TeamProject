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

void CombatSystem::DoHeal(const INT8 clientID)
{
	auto players = mRegistry.view<Tag_Player, HealthComponent>();

	for (auto [entity, health] : players.each())
	{
		health.Health += 5;
		if (health.Health > Values::PlayerHealth)
		{
			health.Health = Values::PlayerHealth;
		}
	}

	// TODO: 체력 회복을 알리는 패킷 보내기
}

void CombatSystem::DoBuff(const INT8 clientID)
{
	auto player = GetEntityByID(mRegistry, clientID);
	ASSERT(mRegistry.valid(player), "Invalid entity!");
	auto& combat = mRegistry.get<CombatComponent>(player);
	combat.BuffDuration = 10.0f;
}

void CombatSystem::Start()
{
	// PlayState id 캐싱
	mPlayState = GetEntityByName(mRegistry, "PlayState");
	ASSERT(mRegistry.valid(mPlayState), "Invalid entity!");
}

void CombatSystem::updateCooldown()
{
	auto view = mRegistry.view<CombatComponent>();
	for (auto [entity, combat] : view.each())
	{
		combat.BaseAttackTracker += Timer::GetDeltaTime();
		combat.SkillTracker += Timer::GetDeltaTime();
		combat.BuffDuration -= Timer::GetDeltaTime();
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

		NOTIFY_ENEMY_ATTACK_PACKET packet = {};
		packet.HitterID = hit.HitterID;
		packet.VictimID = hit.VictimID;
		packet.PacketSize = sizeof(packet);
		packet.PacketID = NOTIFY_ENEMY_ATTACK;
		mOwner->Broadcast(sizeof(packet), reinterpret_cast<char*>(&packet));
		mRegistry.remove<IHitYouComponent>(entity);

		// Dog인 경우 공격 한 번 하고 삭제.
		// 클라이언트에선 Dog가 공격한 경우 애니메이션 재생 후 삭제.
		if (mRegistry.any_of<Tag_Dog>(entity))
		{
			DestroyEntity(mRegistry, entity);
		}

		auto& health = mRegistry.get<HealthComponent>(victim);
		--health.Health;

		auto& playState = mRegistry.get<PlayStateComponent>(mPlayState);
		playState.bChanged = true;
		EntityType eType = EntityType::END;
		if (mRegistry.any_of<Tag_Tank>(victim))
		{
			--playState.TankHealth;
			eType = EntityType::TANK;
		}
		else if(mRegistry.any_of<Tag_RedCell>(victim))
		{
			eType = EntityType::RED_CELL;
		}
		else // Player
		{
			auto& id = mRegistry.get<IDComponent>(victim);
			updatePlayerHPState(id.ID);
			eType = EntityType::PLAYER;
		}

		if (health.Health <= 0)
		{
			doEntityDie(victim, eType);
		}
	}
}

void CombatSystem::updatePlayerHPState(const UINT32 id)
{
	auto& playState = mRegistry.get<PlayStateComponent>(mPlayState);

	switch (id)
	{
	case 0:
		--playState.P0HP;
		break;

	case 1:
		--playState.P1HP;
		break;

	case 2:
		--playState.P2HP;
		break;

	default:
		ASSERT(false, "Unknown player id!");
		break;
	}
}

void CombatSystem::doEntityDie(const entt::entity eid, EntityType eType)
{
	if (EntityType::TANK == eType)
	{
		mOwner->DoGameOver();
	}
}
