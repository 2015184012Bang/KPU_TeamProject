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

	// TODO: ������ 0���� ����� ��� �ӵ��� 0���� �����.
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

	// Ÿ�ٰ� �浹 �˻�
	// ���� Ÿ���� ��� ���޼Ҷ��, ���� Ÿ���� īƮ�� ����
	// Ÿ���� īƮ�� ��� ���� Ÿ���� ����� ���޼ҷ� ����
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
	auto& movement = mOwner->GetComponent<MovementComponent>();
	movement.Direction = Vector3::Zero;

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