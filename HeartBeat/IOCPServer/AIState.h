#pragma once

class Enemy;

class AIState
{
public:
	AIState(string_view stateName);
	virtual ~AIState() = default;
	
	virtual void Enter() abstract;
	virtual void Update() abstract;
	virtual void Exit() abstract;

	string_view GetStateName() const { return mStateName; }

private:
	string mStateName = {};
};

// Enemy ���� ��Ÿ�
constexpr float RANGESQ_TO_ATTACK = 350.0f * 350.0f;

class EnemyChaseState
	: public AIState
{
public:
	EnemyChaseState(shared_ptr<Enemy>&& owner);

	virtual void Enter() override;
	virtual void Update() override;
	virtual void Exit() override;

private:
	shared_ptr<Enemy> mOwner = nullptr;
};

constexpr float ENEMY_ATTACK_ANIM_DURATION = 1.1f;

class EnemyAttackState
	: public AIState
{
public:
	EnemyAttackState(shared_ptr<Enemy>&& owner);

	virtual void Enter() override;
	virtual void Update() override;
	virtual void Exit() override;

private:
	shared_ptr<Enemy> mOwner = nullptr;

	float elapsed = 0.0f;
};