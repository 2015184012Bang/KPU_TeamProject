#pragma once

#include "Defines.h"

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

private:
	void updateCooldown();
	void checkEnemyAttack();

private:
	entt::registry& mRegistry;
	shared_ptr<Room> mOwner = nullptr;
};

