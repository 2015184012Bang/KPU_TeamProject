#pragma once

class CombatSystem
{
public:
	enum class UpgradePreset : UINT8
	{
		ATTACK = 0,
		HEAL,
		SUPPORT,
	};

	void Update();

	void SetPreset(const UINT32 eid, UpgradePreset preset);

	bool CanBaseAttack(const UINT32 eid);
};

