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
};