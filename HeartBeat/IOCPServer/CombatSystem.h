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
	void checkWhiteCellAttack();
	void checkBossSkill();

	// 게임오버 여부를 리턴
	bool doEntityDie(const UINT32 id, EntityType eType);

private:
	entt::registry& mRegistry;
	shared_ptr<Room> mOwner = nullptr;

	entt::entity mPlayState = entt::null;

	INT8 mNumDeadPlayers = 0;
};

