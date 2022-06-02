#include "pch.h"
#include "CombatSystem.h"

#include "Entity.h"
#include "Timer.h"
#include "Values.h"
#include "Room.h"

constexpr INT32 MAX_PLAYER_LIFE = 3;

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

	combat.Life = MAX_PLAYER_LIFE;
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
		if (entt::null == victim ||
			!mRegistry.valid(entity))
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

		INT8 enemyAttack = 1;
		if (mRegistry.any_of<Tag_Dog>(entity))
		{
			enemyAttack = 3;
		}

		auto& health = mRegistry.get<HealthComponent>(victim);
		health.Health -= enemyAttack;

		auto& playState = mRegistry.get<PlayStateComponent>(mPlayState);
		playState.bChanged = true;
		EntityType eType = EntityType::END;
		if (mRegistry.any_of<Tag_Tank>(victim))
		{
			--playState.TankHealth;
			eType = EntityType::TANK;
		}
		else if(mRegistry.any_of<Tag_Player>(victim))
		{
			updatePlayerHPState(health.Health, hit.VictimID);
			eType = EntityType::PLAYER;
		}
		else if (mRegistry.any_of<Tag_RedCell>(victim))
		{
			eType = EntityType::RED_CELL;
		}

		if (mRegistry.any_of<Tag_Dog>(entity))
		{
			mOwner->SendDeleteEntityPacket(hit.HitterID, EntityType::DOG);
			DestroyEntity(mRegistry, entity);
		}

		if (health.Health <= 0)
		{
			doEntityDie(hit.VictimID, eType);
		}
	}
}

void CombatSystem::updatePlayerHPState(const INT32 health, const UINT32 id)
{
	auto& playState = mRegistry.get<PlayStateComponent>(mPlayState);

	switch (id)
	{
	case 0:
		playState.P0HP = health;
		break;

	case 1:
		playState.P1HP = health;
		break;

	case 2:
		playState.P2HP = health;
		break;

	default:
		ASSERT(false, "Unknown player id!");
		break;
	}
}

void CombatSystem::doEntityDie(const UINT32 id, EntityType eType)
{
	switch(eType)
	{
	case EntityType::TANK:
		mOwner->DoGameOver();
		break;

	case EntityType::RED_CELL:
	{
		mOwner->SendDeleteEntityPacket(id, EntityType::RED_CELL);
		DestroyEntityByID(mRegistry, id);
	}
		break;

	case EntityType::PLAYER:
	{
		auto player = GetEntityByID(mRegistry, id);
		if (mRegistry.any_of<Tag_Dead>(player))
		{
			return;
		}

		auto& combat = mRegistry.get<CombatComponent>(player);
		combat.Life -= 1;
		mRegistry.emplace<Tag_Dead>(player);
		mRegistry.remove<BoxComponent>(player);
		auto& movement = mRegistry.get<MovementComponent>(player);
		movement.Direction = Vector3::Zero;

		if (combat.Life > 0)
		{
			mOwner->SendEventOccurPacket(id, EventType::PLAYER_DEAD);

			Timer::AddEvent(3.0f, [this, id]() {
				auto player = GetEntityByID(mRegistry, id);
				if (player != entt::null)
				{
					mRegistry.remove<Tag_Dead>(player);
					auto& health = mRegistry.get<HealthComponent>(player);
					health.Health = Values::PlayerHealth;
					const auto& transform = mRegistry.get<TransformComponent>(player);
					mRegistry.emplace<BoxComponent>(player, &Box::GetBox("../Assets/Boxes/Character.box"),
						transform.Position, transform.Yaw);

					auto& playState = mRegistry.get<PlayStateComponent>(mPlayState);
					playState.bChanged = true;

					switch (id)
					{
					case 0: playState.P0HP = health.Health; break;
					case 1: playState.P1HP = health.Health; break;
					case 2: playState.P2HP = health.Health; break;
					}
				}
				});
		}
		else
		{
			mOwner->SendDeleteEntityPacket(id, EntityType::PLAYER);

			// 모든 플레이어가 Life를 소진했다면 게임오버 처리
			auto deads = mRegistry.view<Tag_Dead>();
			if (deads.size() == mOwner->GetCurrentUsers())
			{
				mOwner->DoGameOver();
			}
		}
	}
		break;
	}
}
