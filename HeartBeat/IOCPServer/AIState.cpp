#include "pch.h"
#include "AIState.h"

#include "Enemy.h"
#include "Timer.h"
#include "Components.h"
#include "Random.h"
#include "Protocol.h"


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
	if (!mOwner->IsTargetValid())
	{
		mOwner->SetNewTarget();
	}

	auto& pathfind = mOwner->GetComponent<PathFindComponent>();
	pathfind.bContinue = true;
	pathfind.MyPosition = mOwner->GetComponent<TransformComponent>().Position;
	auto target = mOwner->GetTarget();
	pathfind.TargetPosition = mOwner->GetRegistry().get<TransformComponent>(target).Position;
}

void EnemyChaseState::Update()
{
	if (!mOwner->IsTargetValid())
	{
		mOwner->SetNewTarget();
	}

	auto& pathfind = mOwner->GetComponent<PathFindComponent>();
	pathfind.MyPosition = mOwner->GetComponent<TransformComponent>().Position;
	auto target = mOwner->GetTarget();
	pathfind.TargetPosition = mOwner->GetRegistry().get<TransformComponent>(target).Position;

	if (Vector3::DistanceSquared(pathfind.MyPosition, pathfind.TargetPosition) < RANGESQ_TO_ATTACK)
	{
		mOwner->ChangeState("EnemyAttackState");
	}
}

void EnemyChaseState::Exit()
{
	auto& pathfind = mOwner->GetComponent<PathFindComponent>();
	pathfind.bContinue = false;

	auto& movement = mOwner->GetComponent<MovementComponent>();
	movement.Direction = Vector3::Zero;
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
	auto myID = mOwner->GetComponent<IDComponent>().ID;
	auto targetID = mOwner->GetRegistry().get<IDComponent>(mOwner->GetTarget()).ID;
	mOwner->AddComponent<IHitYouComponent>(myID, targetID);

	elapsed = 0.0f;
}

void EnemyAttackState::Update()
{
	elapsed += Timer::GetDeltaTime();

	if (elapsed > ENEMY_ATTACK_ANIM_DURATION)
	{
		mOwner->ChangeState("EnemyChaseState");
	}
}

void EnemyAttackState::Exit()
{

}
