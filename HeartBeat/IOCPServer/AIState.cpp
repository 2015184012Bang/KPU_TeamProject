#include "pch.h"
#include "AIState.h"

#include "Box.h"
#include "Enemy.h"
#include "Timer.h"
#include "Components.h"
#include "Random.h"
#include "Protocol.h"
#include "RedCell.h"
#include "Entity.h"


AIState::AIState(string_view stateName)
	: mStateName{ stateName.data() }
{

}

/************************************************************************/
/* EnemyTankChaseState                                                  */
/************************************************************************/

EnemyTankChaseState::EnemyTankChaseState(shared_ptr<Enemy> owner)
	: AIState{ "EnemyTankChaseState" }
	, mOwner{ owner }
{

}

void EnemyTankChaseState::Enter()
{
	auto owner = mOwner.lock();

	if (!owner)
	{
		return;
	}

	owner->SetTargetTank();

	auto& pathfind = owner->GetComponent<PathFindComponent>();
	pathfind.bContinue = true;
	pathfind.MyPosition = owner->GetComponent<TransformComponent>().Position;
	auto target = owner->GetTarget();
	pathfind.TargetPosition = owner->GetRegistry().get<TransformComponent>(target).Position;
}

void EnemyTankChaseState::Update()
{
	auto owner = mOwner.lock();

	if (!owner)
	{
		return;
	}

	auto tank = owner->GetTarget();
	const auto& myBox = owner->GetComponent<BoxComponent>();
	const auto& tankBox = owner->GetRegistry().get<BoxComponent>(tank);

	// 탱크와 충돌 검사
	// True: 공격 상태로 전환
	if (Intersects(myBox.WorldBox, tankBox.WorldBox))
	{
		owner->ChangeState("EnemyAttackState");
		return;
	}

	entt::entity player = entt::null;
	if (owner->HasNearPlayer(player))
	{
		owner->SetTargetPlayer(player);
		owner->ChangeState("EnemyPlayerChaseState");
		return;
	}

	auto& pathfind = owner->GetComponent<PathFindComponent>();
	pathfind.MyPosition = owner->GetComponent<TransformComponent>().Position;
	pathfind.TargetPosition = owner->GetRegistry().get<TransformComponent>(tank).Position;
}

void EnemyTankChaseState::Exit()
{

}

/************************************************************************/
/* EnemyPlayerChaseState                                                */
/************************************************************************/

EnemyPlayerChaseState::EnemyPlayerChaseState(shared_ptr<Enemy> owner)
	: AIState{ "EnemyPlayerChaseState" }
	, mOwner{ owner }
{

}

void EnemyPlayerChaseState::Enter()
{
	auto owner = mOwner.lock();

	if (!owner)
	{
		return;
	}

	auto& pathfind = owner->GetComponent<PathFindComponent>();
	pathfind.bContinue = true;
}

void EnemyPlayerChaseState::Update()
{
	auto owner = mOwner.lock();

	if (!owner)
	{
		return;
	}

	if (!owner->IsTargetValid())
	{
		owner->ChangeState("EnemyTankChaseState");
		return;
	}

	auto player = owner->GetTarget();
	const auto& myPosition = owner->GetComponent<TransformComponent>().Position;
	const auto& playerPosition = owner->GetRegistry().get<TransformComponent>(player).Position;

	float dist = Vector3::DistanceSquared(myPosition, playerPosition);

	if (dist < ATTACK_DIST_SQ)
	{
		owner->ChangeState("EnemyAttackState");
		return;
	}

	if (dist > DEAGGRO_DIST_SQ)
	{
		owner->ChangeState("EnemyTankChaseState");
		return;
	}

	auto& pathfind = owner->GetComponent<PathFindComponent>();
	pathfind.MyPosition = owner->GetComponent<TransformComponent>().Position;
	pathfind.TargetPosition = owner->GetRegistry().get<TransformComponent>(player).Position;
}

void EnemyPlayerChaseState::Exit()
{

}

/************************************************************************/
/* EnemyNPCChaseState                                                   */
/************************************************************************/

EnemyNPCChaseState::EnemyNPCChaseState(shared_ptr<Enemy> owner)
	: AIState{ "EnemyNPCChaseState" }
	, mOwner{ owner }
{

}

void EnemyNPCChaseState::Enter()
{
	auto owner = mOwner.lock();

	if (!owner)
	{
		return;
	}

	owner->SetTargetNPC();

	auto& pathfind = owner->GetComponent<PathFindComponent>();
	pathfind.bContinue = true;
	pathfind.MyPosition = owner->GetComponent<TransformComponent>().Position;
	auto target = owner->GetTarget();
	pathfind.TargetPosition = owner->GetRegistry().get<TransformComponent>(target).Position;
}

void EnemyNPCChaseState::Update()
{
	auto owner = mOwner.lock();

	if (!owner)
	{
		return;
	}

	if (!owner->IsTargetValid())
	{
		owner->SetTargetNPC();
	}

	auto target = owner->GetTarget();
	const auto& myBox = owner->GetComponent<BoxComponent>();
	const auto& targetBox = owner->GetRegistry().get<BoxComponent>(target);

	// NPC와 충돌 검사
	// True: 공격 상태로 전환
	if (Intersects(myBox.WorldBox, targetBox.WorldBox))
	{
		owner->ChangeState("EnemyAttackState");
		return;
	}

	auto& pathfind = owner->GetComponent<PathFindComponent>();
	pathfind.MyPosition = owner->GetComponent<TransformComponent>().Position;
	pathfind.TargetPosition = owner->GetRegistry().get<TransformComponent>(target).Position;
}

void EnemyNPCChaseState::Exit()
{

}

/************************************************************************/
/* EnemyAttackState                                                     */
/************************************************************************/

EnemyAttackState::EnemyAttackState(shared_ptr<Enemy> owner)
	: AIState{ "EnemyAttackState" }
	, mOwner{ owner }
{

}

void EnemyAttackState::Enter()
{
	auto owner = mOwner.lock();

	if (!owner)
	{
		return;
	}

	auto& pathfind = owner->GetComponent<PathFindComponent>();
	pathfind.bContinue = false;

	auto& movement = owner->GetComponent<MovementComponent>();
	movement.Direction = Vector3::Zero;
	
	if (owner->HasComponent<IHitYouComponent>())
	{
		owner->RemoveComponent<IHitYouComponent>();
	}

	auto myID = owner->GetComponent<IDComponent>().ID;
	auto targetID = owner->GetRegistry().get<IDComponent>(owner->GetTarget()).ID;
	owner->AddComponent<IHitYouComponent>(myID, targetID);

	elapsed = 0.0f;
}

void EnemyAttackState::Update()
{
	auto owner = mOwner.lock();

	if (!owner)
	{
		return;
	}

	elapsed += Timer::GetDeltaTime();

	if (elapsed > ENEMY_ATTACK_ANIM_DURATION)
	{
		owner->ChangeToPreivous();
	}
}

void EnemyAttackState::Exit()
{

}

/************************************************************************/
/* CellDeliverState                                                     */
/************************************************************************/

CellDeliverState::CellDeliverState(shared_ptr<RedCell> owner)
	: AIState{ "CellDeliverState" }
	, mOwner{ owner }
{

}

void CellDeliverState::Enter()
{
	auto owner = mOwner.lock();

	if (!owner)
	{
		return;
	}

	if (owner->IsTargetValid())
	{
		auto& pathfind = owner->GetComponent<PathFindComponent>();
		pathfind.bContinue = true;
		pathfind.MyPosition = owner->GetComponent<TransformComponent>().Position;
		auto target = owner->GetTarget();
		pathfind.TargetPosition = owner->GetRegistry().get<TransformComponent>(target).Position;
	}

	mPlayState = GetEntityByName(owner->GetRegistry(), "PlayState");
	if (entt::null == mPlayState)
	{
		LOG("PlayState is not valid entity");
	}
}

void CellDeliverState::Update()
{
	auto owner = mOwner.lock();

	if (!owner)
	{
		return;
	}

	auto target = owner->GetTarget();
	const auto& myBox = owner->GetComponent<BoxComponent>();
	const auto& targetBox = owner->GetRegistry().get<BoxComponent>(target);

	// 타겟과 충돌 검사
	// 만약 타겟이 산소 공급소라면, 다음 타겟을 카트로 설정
	// 타겟이 카트일 경우 다음 타겟을 가까운 공급소로 설정
	if (Intersects(myBox.WorldBox, targetBox.WorldBox))
	{
		IncreaseScore();

		if (owner->GetRegistry().any_of<Tag_HouseTile>(target))
		{
			owner->SetTargetCart();
			owner->ChangeState("CellRestState");
			return;
		}
		else
		{
			owner->SetTargetHouse();
			owner->ChangeState("CellRestState");
			return;
		}

		target = owner->GetTarget();
	}

	auto& pathfind = owner->GetComponent<PathFindComponent>();
	pathfind.MyPosition = owner->GetComponent<TransformComponent>().Position;
	pathfind.TargetPosition = owner->GetRegistry().get<TransformComponent>(target).Position;
}

void CellDeliverState::Exit()
{
	auto owner = mOwner.lock();

	if (!owner)
	{
		return;
	}

	auto& pathfind = owner->GetComponent<PathFindComponent>();
	pathfind.bContinue = false;

	auto& movement = owner->GetComponent<MovementComponent>();
	movement.Direction = Vector3::Zero;
}

void CellDeliverState::IncreaseScore()
{
	auto owner = mOwner.lock();

	if (!owner)
	{
		return;
	}

	auto& gameState = owner->GetRegistry().get<PlayStateComponent>(mPlayState);
	++gameState.Score;
	gameState.bChanged = true;
}

/************************************************************************/
/* CellRestState                                                        */
/************************************************************************/

CellRestState::CellRestState(shared_ptr<RedCell> owner)
	: AIState{ "CellRestState" }
	, mOwner{ owner }
{

}

void CellRestState::Enter()
{
	mWaitTime = Random::RandFloat(0.5f, MAX_CELL_WAIT_TIME);
}

void CellRestState::Update()
{
	auto owner = mOwner.lock();

	if (!owner)
	{
		return;
	}

	mWaitTime -= Timer::GetDeltaTime();

	if (mWaitTime < 0.0f)
	{
		owner->ChangeState("CellDeliverState");
	}
}

void CellRestState::Exit()
{

}

/************************************************************************/
/* BossIdleState                                                        */
/************************************************************************/

BossIdleState::BossIdleState(shared_ptr<Enemy> owner)
	: AIState{ "BossIdleState" }
	, mOwner{ owner }
{

}

void BossIdleState::Enter()
{
	mElapsed = 0.0f;
}

void BossIdleState::Update()
{
	auto owner = mOwner.lock();
	if (!owner)
	{
		return;
	}

	auto& health = owner->GetComponent<HealthComponent>();
	if (health.Health <= 70 && !bFirstSpecialAttackDone)
	{
		bFirstSpecialAttackDone = true;
		//owner->ChangeState("BossSpecialAttackState");
		return;
	}

	if (health.Health <= 30 && !bSecondSpecialAttackDone)
	{
		bSecondSpecialAttackDone = true;
		//owner->ChangeState("BossSpecialAttackState");
		return;
	}

	mElapsed += Timer::GetDeltaTime();
	if (mElapsed > 10.0f)
	{
		auto randInt = Random::RandInt(0, 1);

		if (0 == randInt)
		{
			//owner->ChangeState("BossAttackOneState");
			return;
		}
		else
		{
			//owner->ChangeState("BossAttackTwoState");
			return;
		}
	}
}

void BossIdleState::Exit()
{

}

/************************************************************************/
/* BossSpecialAttackState                                               */
/************************************************************************/

BossSpecialAttackState::BossSpecialAttackState(shared_ptr<Enemy> owner)
	: AIState{ "BossSpecialAttackState" }
	, mOwner{ owner }
{

}

void BossSpecialAttackState::Enter()
{

}

void BossSpecialAttackState::Update()
{

}

void BossSpecialAttackState::Exit()
{

}

/************************************************************************/
/* BossAttackOneState                                                   */
/************************************************************************/

BossAttackOneState::BossAttackOneState(shared_ptr<Enemy> owner)
	: AIState{ "BossAttackOneState" }
	, mOwner{ owner }
{

}

void BossAttackOneState::Enter()
{

}

void BossAttackOneState::Update()
{

}

void BossAttackOneState::Exit()
{

}

/************************************************************************/
/* BossAttackTwoState                                                   */
/************************************************************************/

BossAttackTwoState::BossAttackTwoState(shared_ptr<Enemy> owner)
	: AIState{ "BossAttackTwoState" }
	, mOwner{ owner }
{

}

void BossAttackTwoState::Enter()
{

}

void BossAttackTwoState::Update()
{

}

void BossAttackTwoState::Exit()
{

}
