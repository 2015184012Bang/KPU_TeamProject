#include "pch.h"
#include "AIState.h"

#include "Box.h"
#include "Enemy.h"
#include "Timer.h"
#include "Components.h"
#include "Random.h"
#include "Protocol.h"
#include "RedCell.h"


AIState::AIState(string_view stateName)
	: mStateName{ stateName.data() }
{

}

/************************************************************************/
/* EnemyTankChaseState                                                  */
/************************************************************************/

EnemyTankChaseState::EnemyTankChaseState(shared_ptr<Enemy>&& owner)
	: AIState{ "EnemyTankChaseState" }
	, mOwner{ move(owner) }
{

}

void EnemyTankChaseState::Enter()
{
	mOwner->SetTargetTank();

	auto& pathfind = mOwner->GetComponent<PathFindComponent>();
	pathfind.bContinue = true;
	pathfind.MyPosition = mOwner->GetComponent<TransformComponent>().Position;
	auto target = mOwner->GetTarget();
	pathfind.TargetPosition = mOwner->GetRegistry().get<TransformComponent>(target).Position;
}

void EnemyTankChaseState::Update()
{
	auto tank = mOwner->GetTarget();
	const auto& myBox = mOwner->GetComponent<BoxComponent>();
	const auto& tankBox = mOwner->GetRegistry().get<BoxComponent>(tank);

	// 탱크와 충돌 검사
	// True: 공격 상태로 전환
	if (Intersects(myBox.WorldBox, tankBox.WorldBox))
	{
		mOwner->ChangeState("EnemyAttackState");
		return;
	}

	entt::entity player = entt::null;
	if (mOwner->HasNearPlayer(player))
	{
		mOwner->SetTargetPlayer(player);
		mOwner->ChangeState("EnemyPlayerChaseState");
		return;
	}

	auto& pathfind = mOwner->GetComponent<PathFindComponent>();
	pathfind.MyPosition = mOwner->GetComponent<TransformComponent>().Position;
	pathfind.TargetPosition = mOwner->GetRegistry().get<TransformComponent>(tank).Position;
}

void EnemyTankChaseState::Exit()
{

}

/************************************************************************/
/* EnemyPlayerChaseState                                                */
/************************************************************************/

EnemyPlayerChaseState::EnemyPlayerChaseState(shared_ptr<Enemy>&& owner)
	: AIState{ "EnemyPlayerChaseState" }
	, mOwner{ move(owner) }
{

}

void EnemyPlayerChaseState::Enter()
{
	auto& pathfind = mOwner->GetComponent<PathFindComponent>();
	pathfind.bContinue = true;
}

void EnemyPlayerChaseState::Update()
{
	auto player = mOwner->GetTarget();

	if (!mOwner->GetRegistry().valid(player))
	{
		mOwner->ChangeState("EnemyTankChaseState");
		return;
	}

	const auto& myPosition = mOwner->GetComponent<TransformComponent>().Position;
	const auto& playerPosition = mOwner->GetRegistry().get<TransformComponent>(player).Position;

	float dist = Vector3::DistanceSquared(myPosition, playerPosition);

	if (dist < ATTACK_DIST_SQ)
	{
		mOwner->ChangeState("EnemyAttackState");
		return;
	}

	if (dist > DEAGGRO_DIST_SQ)
	{
		mOwner->ChangeState("EnemyTankChaseState");
		return;
	}

	auto& pathfind = mOwner->GetComponent<PathFindComponent>();
	pathfind.MyPosition = mOwner->GetComponent<TransformComponent>().Position;
	pathfind.TargetPosition = mOwner->GetRegistry().get<TransformComponent>(player).Position;
}

void EnemyPlayerChaseState::Exit()
{

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
	auto& pathfind = mOwner->GetComponent<PathFindComponent>();
	pathfind.bContinue = false;

	auto& movement = mOwner->GetComponent<MovementComponent>();
	movement.Direction = Vector3::Zero;

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
		mOwner->ChangeToPreviousState();
	}
}

void EnemyAttackState::Exit()
{
	
}

/************************************************************************/
/* CellDeliverState                                                     */
/************************************************************************/

CellDeliverState::CellDeliverState(shared_ptr<RedCell>&& owner)
	: AIState{ "CellDeliverState" }
	, mOwner{ move(owner) }
{

}

void CellDeliverState::Enter()
{
	if (mOwner->IsTargetValid())
	{
		auto& pathfind = mOwner->GetComponent<PathFindComponent>();
		pathfind.bContinue = true;
		pathfind.MyPosition = mOwner->GetComponent<TransformComponent>().Position;
		auto target = mOwner->GetTarget();
		pathfind.TargetPosition = mOwner->GetRegistry().get<TransformComponent>(target).Position;
	}
}

void CellDeliverState::Update()
{
	auto target = mOwner->GetTarget();
	const auto& myBox = mOwner->GetComponent<BoxComponent>();
	const auto& targetBox = mOwner->GetRegistry().get<BoxComponent>(target);

	// 타겟과 충돌 검사
	// 만약 타겟이 산소 공급소라면, 다음 타겟을 카트로 설정
	// 타겟이 카트일 경우 다음 타겟을 가까운 공급소로 설정
	if (Intersects(myBox.WorldBox, targetBox.WorldBox))
	{
		if (mOwner->GetRegistry().any_of<Tag_HouseTile>(target))
		{
			mOwner->SetTargetCart();
			mOwner->ChangeState("CellRestState");
			return;
		}
		else
		{
			mOwner->SetTargetHouse();
			mOwner->ChangeState("CellRestState");
			return;
		}

		target = mOwner->GetTarget();
	}

	auto& pathfind = mOwner->GetComponent<PathFindComponent>();
	pathfind.MyPosition = mOwner->GetComponent<TransformComponent>().Position;
	pathfind.TargetPosition = mOwner->GetRegistry().get<TransformComponent>(target).Position;
}

void CellDeliverState::Exit()
{
	auto& pathfind = mOwner->GetComponent<PathFindComponent>();
	pathfind.bContinue = false;

	auto& movement = mOwner->GetComponent<MovementComponent>();
	movement.Direction = Vector3::Zero;
}

/************************************************************************/
/* CellRestState                                                        */
/************************************************************************/

CellRestState::CellRestState(shared_ptr<RedCell>&& owner)
	: AIState{ "CellRestState" }
	, mOwner{ move(owner) }
{

}

void CellRestState::Enter()
{
	mWaitTime = Random::RandFloat(0.5f, MAX_CELL_WAIT_TIME);
}

void CellRestState::Update()
{
	mWaitTime -= Timer::GetDeltaTime();

	if (mWaitTime < 0.0f)
	{
		mOwner->ChangeState("CellDeliverState");
	}
}

void CellRestState::Exit()
{

}

