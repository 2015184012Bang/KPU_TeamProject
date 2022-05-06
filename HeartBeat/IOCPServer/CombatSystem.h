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

	void SetPreset(const UINT32 eid, UpgradePreset preset);

	bool CanBaseAttack(const UINT32 eid);

private:
	entt::registry& mRegistry;
	shared_ptr<Room> mOwner = nullptr;
};

