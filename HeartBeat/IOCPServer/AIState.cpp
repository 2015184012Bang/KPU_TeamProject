#include "pch.h"
#include "AIState.h"

#include "Enemy.h"
#include "Timer.h"


AIState::AIState(string_view stateName)
	: mStateName{ stateName.data() }
{

}

/************************************************************************/
/* EnemyChaseState                                                      */
/************************************************************************/

EnemyChaseState::EnemyChaseState(shared_ptr<Enemy>&& owner)
	: AIState{ "EnemyChaseState" }
	, mOwner{ move(owner) }
{

}

void EnemyChaseState::Enter()
{
	LOG("Enter chase state...");
}

void EnemyChaseState::Update()
{

}

void EnemyChaseState::Exit()
{
	LOG("Exit chase state...");
}

/************************************************************************/
/* EnemyAttackState                                                     */
/************************************************************************/

EnemyAttackState::EnemyAttackState(shared_ptr<Enemy>&& owner)
	: AIState{ "EnemyAttackState" }
	, mOwner{ move(owner) }
{

}

void EnemyAttackState::Enter()
{
	LOG("Enter attack state...");
}

void EnemyAttackState::Update()
{

}

void EnemyAttackState::Exit()
{
	LOG("Exit attack state...");
}
