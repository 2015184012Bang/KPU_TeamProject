#pragma once

class Room;

class CombatSystem
{
public:
	enum class UpgradePreset : UINT8
	{
		ATTACK = 0,
		HEAL,
		SUPPORT,
	};

	CombatSystem(entt::registry& registry, shared_ptr<Room>&& room);

	void Update();

	void SetPreset(const INT8 clientID, UpgradePreset preset);

	bool CanBaseAttack(const INT8 clientID);

private:
	void updateCooldown();
	void checkEnemyHit();

private:
	entt::registry& mRegistry;
	shared_ptr<Room> mOwner = nullptr;
};

