#pragma once

class CombatSystem
{
	const float BASE_ATTACK_COOLDOWN = 3.0f;

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

