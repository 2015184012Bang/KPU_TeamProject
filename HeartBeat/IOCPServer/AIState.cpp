#include "pch.h"
#include "AIState.h"

#include "Enemy.h"
#include "Timer.h"
#include "Components.h"
#include "Random.h"


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

	setNewTarget();

	auto& pathfind = mOwner->GetComponent<PathFindComponent>();
	pathfind.bFind = true;
	pathfind.MyPosition = mOwner->GetComponent<TransformComponent>().Position;
	pathfind.TargetPosition = mOwner->GetRegistry().get<TransformComponent>(mTargetID).Position;
}

void EnemyChaseState::Update()
{
	if (!mOwner->GetRegistry().valid(mTargetID))
	{
		setNewTarget();
	}

	auto& pathfind = mOwner->GetComponent<PathFindComponent>();
	pathfind.MyPosition = mOwner->GetComponent<TransformComponent>().Position;
	pathfind.TargetPosition = mOwner->GetRegistry().get<TransformComponent>(mTargetID).Position;

	// TODO : 타겟과의 거리가 충분히 가깝다면 어택스테이트로 전환하기
}

void EnemyChaseState::Exit()
{
	LOG("Exit chase state...");

	auto& pathfind = mOwner->GetComponent<PathFindComponent>();
	pathfind.bFind = false;
}

void EnemyChaseState::setNewTarget()
{
	auto players = mOwner->FindObjectsWithTag<Tag_Player>();
	ASSERT(!players.empty(), "There are no players!");
	mTargetID = players[Random::RandInt(0, players.size() - 1)];
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
