#pragma once

class Enemy;
class RedCell;

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

/************************************************************************/
/* EnemyTankChaseState                                                  */
/************************************************************************/

class EnemyTankChaseState
	: public AIState
{
public:
	EnemyTankChaseState(shared_ptr<Enemy> owner);

	virtual void Enter() override;
	virtual void Update() override;
	virtual void Exit() override;

private:
	weak_ptr<Enemy> mOwner;
};

/************************************************************************/
/* EnemyPlayerChaseState                                                */
/************************************************************************/

constexpr float DEAGGRO_DIST_SQ = 800.0f * 800.0f;
constexpr float ATTACK_DIST_SQ = 300.0f * 300.0f;

class EnemyPlayerChaseState
	: public AIState
{
public:
	EnemyPlayerChaseState(shared_ptr<Enemy> owner);

	virtual void Enter() override;
	virtual void Update() override;
	virtual void Exit() override;

private:
	weak_ptr<Enemy> mOwner;
};

/************************************************************************/
/* EnemyNPCChaseState                                                   */
/************************************************************************/

class EnemyNPCChaseState
	: public AIState
{
public:
	EnemyNPCChaseState(shared_ptr<Enemy> owner);

	virtual void Enter() override;
	virtual void Update() override;
	virtual void Exit() override;

private:
	weak_ptr<Enemy> mOwner;
};

/************************************************************************/
/* EnemyAttackState                                                     */
/************************************************************************/

constexpr float ENEMY_ATTACK_ANIM_DURATION = 1.1f;

class EnemyAttackState
	: public AIState
{
public:
	EnemyAttackState(shared_ptr<Enemy> owner);

	virtual void Enter() override;
	virtual void Update() override;
	virtual void Exit() override;

private:
	weak_ptr<Enemy> mOwner;

	float elapsed = 0.0f;
};

/************************************************************************/
/* BossIdleState                                                        */
/************************************************************************/

class BossIdleState
	: public AIState
{
public:
	BossIdleState(shared_ptr<Enemy> owner);

	virtual void Enter() override;
	virtual void Update() override;
	virtual void Exit() override {};

private:
	weak_ptr<Enemy> mOwner;

	float mElapsed = 0.0f;
	bool bFirstSpecialAttackDone = false;
	bool bSecondSpecialAttackDone = false;
};

/************************************************************************/
/* BossSkillSpecialState                                                */
/************************************************************************/

class BossSkillSpecialState
	: public AIState
{
public:
	BossSkillSpecialState(shared_ptr<Enemy> owner);

	virtual void Enter() override;
	virtual void Update() override;
	virtual void Exit() override;

private:
	weak_ptr<Enemy> mOwner;

	float mElapsed = 0.0f;
};

/************************************************************************/
/* BossSkillOneState                                                    */
/************************************************************************/

class BossSkillOneState
	: public AIState
{
public:
	BossSkillOneState(shared_ptr<Enemy> owner);

	virtual void Enter() override;
	virtual void Update() override;
	virtual void Exit() override {};

private:
	weak_ptr<Enemy> mOwner;

	float mElapsed = 0.0f;
};

/************************************************************************/
/* BossSkillTwoState                                                    */
/************************************************************************/

class BossSkillTwoState
	: public AIState
{
public:
	BossSkillTwoState(shared_ptr<Enemy> owner);

	virtual void Enter() override;
	virtual void Update() override;
	virtual void Exit() override {};

private:
	weak_ptr<Enemy> mOwner;

	float mElapsed = 0.0f;
};

/************************************************************************/
/* CellDeliverState                                                     */
/************************************************************************/

class CellDeliverState
	: public AIState
{
public:
	CellDeliverState(shared_ptr<RedCell> owner);

	virtual void Enter() override;
	virtual void Update() override;
	virtual void Exit() override;

private:
	void IncreaseScore();

private:
	weak_ptr<RedCell> mOwner;

	entt::entity mPlayState = entt::null;
};

/************************************************************************/
/* CellRestState                                                        */
/************************************************************************/

constexpr float MAX_CELL_WAIT_TIME = 2.0f;

class CellRestState
	: public AIState
{
public:
	CellRestState(shared_ptr<RedCell> owner);

	virtual void Enter() override;
	virtual void Update() override;
	virtual void Exit() override;

private:
	weak_ptr<RedCell> mOwner;

	float mWaitTime = 0.0f;
};
