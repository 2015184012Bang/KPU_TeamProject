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
	checkWhiteCellAttack();
	checkEnemyAttack();
	checkBossSkill();
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
		combat.BaseAttackDmg = Values::BaseAttackPower + 1;
		combat.SkillCooldown = Values::SkillSlashCooldown;
		combat.SkillTracker = combat.SkillCooldown;
		break;

	case UpgradePreset::HEAL:
		combat.Preset = UpgradePreset::HEAL;
		combat.BaseAttackDmg = Values::BaseAttackPower;
		combat.SkillCooldown = Values::SkillHealCooldown;
		combat.SkillTracker = combat.SkillCooldown;
		break;

	case UpgradePreset::SUPPORT:
		combat.Preset = UpgradePreset::SUPPORT;
		combat.BaseAttackDmg = Values::BaseAttackPower;
		combat.SkillCooldown = Values::SkillBuffCooldown;
		combat.SkillTracker = combat.SkillCooldown;
		break;
	}

	combat.Life = Values::PlayerLife;
	combat.BaseAttackCooldown = Values::BaseAttackCooldown;
	combat.BaseAttackTracker = combat.BaseAttackCooldown;
	combat.bEatCaffeine = false;
	combat.BuffDuration = 0.0f;
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
	auto players = mRegistry.view<Tag_Player, HealthComponent, IDComponent>();

	for (auto [entity, health, id] : players.each())
	{
		health.Health += 5;
		if (health.Health > Values::PlayerHealth)
		{
			health.Health = Values::PlayerHealth;
		}
		mOwner->UpdatePlayerHpInState(health.Health, id.ID);
	}
}

void CombatSystem::DoBuff(const INT8 clientID)
{
	auto player = GetEntityByID(mRegistry, clientID);
	ASSERT(mRegistry.valid(player), "Invalid entity!");
	auto& combat = mRegistry.get<CombatComponent>(player);
	combat.BuffDuration = Values::SkillBuffDuration;
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
	auto view = mRegistry.view<Tag_Enemy, IHitYouComponent>();
	set<pair<UINT32, EntityType>> deads;
	for (auto [entity, hit] : view.each())
	{
		auto victim = GetEntityByID(mRegistry, hit.VictimID);
		if (entt::null == victim || !mRegistry.valid(entity))
		{
			mRegistry.remove<IHitYouComponent>(entity);
			continue;
		}

		NOTIFY_ENEMY_ATTACK_PACKET packet = {};
		packet.HitterID = hit.HitterID;
		packet.VictimID = hit.VictimID;
		packet.PacketSize = sizeof(packet);
		packet.PacketID = NOTIFY_ENEMY_ATTACK;
		mOwner->Broadcast(sizeof(packet), reinterpret_cast<char*>(&packet));

		bool bDog = false;
		INT8 enemyAttack = Values::VirusPower;
		if (mRegistry.any_of<Tag_Dog>(entity))
		{
			bDog = true;
			enemyAttack = Values::DogPower;
		}

		auto& health = mRegistry.get<HealthComponent>(victim);
		health.Health -= enemyAttack;

		auto& playState = mRegistry.get<PlayStateComponent>(mPlayState);
		playState.bChanged = true;
		EntityType eType = EntityType::END;
		if (mRegistry.any_of<Tag_Tank>(victim))
		{
			playState.TankHealth = health.Health;
			eType = EntityType::TANK;
		}
		else if (mRegistry.any_of<Tag_Player>(victim))
		{
			if (health.Health < 0)
			{
				health.Health = 0;
			}

			mOwner->UpdatePlayerHpInState(health.Health, hit.VictimID);
			eType = EntityType::PLAYER;
		}
		else if (mRegistry.any_of<Tag_RedCell>(victim))
		{
			eType = EntityType::RED_CELL;
		}
		else if (mRegistry.any_of<Tag_WhiteCell>(victim))
		{
			eType = EntityType::WHITE_CELL;
		}

		mRegistry.remove<IHitYouComponent>(entity);

		if (bDog)
		{
			DestroyEntity(mRegistry, entity);
		}

		if (health.Health <= 0)
		{
			deads.emplace(hit.VictimID, eType);
		}
	}

	for (auto& [deadID, eType] : deads)
	{
		doEntityDie(deadID, eType);
	}
}


void CombatSystem::checkWhiteCellAttack()
{
	auto view = mRegistry.view<Tag_WhiteCell, IHitYouComponent>();
	set<pair<UINT32, EntityType>> deads;
	for (auto [entity, hit] : view.each())
	{
		auto victim = GetEntityByID(mRegistry, hit.VictimID);
		if (entt::null == victim || !mRegistry.valid(victim))
		{
			mRegistry.remove<IHitYouComponent>(entity);
			continue;
		}

		NOTIFY_ATTACK_PACKET packet = {};
		packet.PacketID = NOTIFY_ATTACK;
		packet.PacketSize = sizeof(packet);
		packet.EntityID = hit.HitterID;
		packet.Result = CELL_ATTACK;
		mOwner->Broadcast(packet.PacketSize, reinterpret_cast<char*>(&packet));

		auto& health = mRegistry.get<HealthComponent>(victim);
		health.Health -= Values::CellPower;

		if (health.Health <= 0)
		{
			EntityType eType = mRegistry.any_of<Tag_Virus>(victim) ?
				EntityType::VIRUS : EntityType::DOG;

			deads.emplace(hit.VictimID, eType);
		}
		mRegistry.remove<IHitYouComponent>(entity);
	}

	for (auto& [deadID, eType] : deads)
	{
		doEntityDie(deadID, eType);
	}
}


void CombatSystem::checkBossSkill()
{
	auto view = mRegistry.view<BossSkillComponent>();
	for (auto [entity, skill] : view.each())
	{
		NOTIFY_SKILL_PACKET packet = {};
		packet.PacketSize = sizeof(packet);
		packet.PacketID = NOTIFY_SKILL;
		packet.EntityID = mRegistry.get<IDComponent>(entity).ID;
		packet.Preset = static_cast<UINT8>(skill.SkillNumber);
		mOwner->Broadcast(packet.PacketSize, reinterpret_cast<char*>(&packet));

		switch (skill.SkillNumber)
		{
		case BossSkill::SKILL_1:
		{
			auto players = mRegistry.view<Tag_Player, TransformComponent>();
			for (auto [player, transform] : players.each())
			{
				const auto& playerPos = transform.Position;

				auto tail = mRegistry.create();
				mRegistry.emplace<Tag_Tail>(tail);
				auto& tailTransform = mRegistry.emplace<TransformComponent>(tail);
				tailTransform.Position = playerPos;
				mRegistry.emplace<BoxComponent>(tail, &Box::GetBox("../Assets/Boxes/Tail.box"), 
					tailTransform.Position, tailTransform.Yaw);
			}

			Timer::AddEvent(2.0f, [this]() {
				auto tails = mRegistry.view<Tag_Tail, BoxComponent>();
				auto players = mRegistry.view<Tag_Player, BoxComponent>();
				for (auto [tail, tailBox] : tails.each())
				{
					for (auto [player, playerBox] : players.each())
					{
						if (Intersects(tailBox.WorldBox, playerBox.WorldBox))
						{
							auto& playerHealth = mRegistry.get<HealthComponent>(player);
							playerHealth.Health -= Values::BossPower;
							playerHealth.Health = clamp(playerHealth.Health, static_cast<INT8>(0), static_cast<INT8>(Values::PlayerHealth));

							auto id = mRegistry.get<IDComponent>(player).ID;
							mOwner->UpdatePlayerHpInState(playerHealth.Health, id);

							if (playerHealth.Health <= 0)
							{
								doEntityDie(id, EntityType::PLAYER);
							}
						}
					}

					DestroyEntity(mRegistry, tail);
				}
				});

			break;
		}

		case BossSkill::SKILL_2:
		{
			auto boss = entity;
			Timer::AddEvent(1.0f, [this, boss]() {
				const auto& bossPos = mRegistry.get<TransformComponent>(boss).Position;
				auto players = mRegistry.view<Tag_Player, TransformComponent>();
				for (auto [player, transform] : players.each())
				{
					if (bossPos.x - transform.Position.x <= 1800.0f)
					{
						auto& playerHealth = mRegistry.get<HealthComponent>(player);
						playerHealth.Health -= Values::BossPower;
						playerHealth.Health = clamp(playerHealth.Health, static_cast<INT8>(0), static_cast<INT8>(Values::PlayerHealth));

						auto id = mRegistry.get<IDComponent>(player).ID;
						mOwner->UpdatePlayerHpInState(playerHealth.Health, id);

						if (playerHealth.Health <= 0)
						{
							doEntityDie(id, EntityType::PLAYER);
						}
					}
				}
				});
			break;
		}

		case BossSkill::SKILL_SPECIAL:
		{
			Timer::AddEvent(2.0f, [this]() {
				auto players = mRegistry.view<Tag_Player, HealthComponent>();
				for (auto [player, health] : players.each())
				{
					auto& playerHealth = mRegistry.get<HealthComponent>(player);
					playerHealth.Health -= Values::BossPower * 2;
					playerHealth.Health = clamp(playerHealth.Health, static_cast<INT8>(0), static_cast<INT8>(Values::PlayerHealth));

					auto id = mRegistry.get<IDComponent>(player).ID;
					mOwner->UpdatePlayerHpInState(playerHealth.Health, id);

					if (playerHealth.Health <= 0)
					{
						doEntityDie(id, EntityType::PLAYER);
					}
				}
				});
			break;
		}

		default:
			break;
		}

		mRegistry.remove<BossSkillComponent>(entity);
	}
}

void CombatSystem::doEntityDie(const UINT32 id, EntityType eType)
{
	switch (eType)
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

	case EntityType::WHITE_CELL:
	{
		mOwner->SendDeleteEntityPacket(id, EntityType::WHITE_CELL);
		DestroyEntityByID(mRegistry, id);
	}
	break;

	case EntityType::VIRUS:
	{
		mOwner->SendDeleteEntityPacket(id, EntityType::VIRUS);
		DestroyEntityByID(mRegistry, id);
	}
	break;
	case EntityType::DOG:
	{
		mOwner->SendDeleteEntityPacket(id, EntityType::DOG);
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
					mOwner->UpdatePlayerHpInState(health.Health, id);
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
