#pragma once

class GameManager;

class CombatSystem
{
public:
	enum class UpgradePreset : UINT8
	{
		ATTACK = 0,
		HEAL,
		SUPPORT,
	};

	CombatSystem(shared_ptr<GameManager>&& gm);

	void Update();

	void SetPreset(const UINT32 eid, UpgradePreset preset);

	bool CanBaseAttack(const UINT32 eid);

private:
	shared_ptr<GameManager> mGameManager = nullptr;
};

