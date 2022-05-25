#pragma once

#include "Defines.h"
#include "Protocol.h"

class Room;

class CombatSystem
{
public:
	CombatSystem(entt::registry& registry, shared_ptr<Room>&& room);

	void Update();

	void SetPreset(const INT8 clientID, UpgradePreset preset);

	bool CanBaseAttack(const INT8 clientID);

	bool CanUseSkill(const INT8 clientID);

	void DoHeal(const INT8 clientID);

	void DoBuff(const INT8 clientID);

	void Start();

private:
	void updateCooldown();
	void checkEnemyAttack();

	void updatePlayerHPState(const INT32 health, const UINT32 id);
	void doEntityDie(const entt::entity eid, EntityType eType);

private:
	entt::registry& mRegistry;
	shared_ptr<Room> mOwner = nullptr;

	entt::entity mPlayState = entt::null;
};

