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

	auto& pathfind = mOwner->GetComponent<PathFindComponent>();
	pathfind.bFind = true;

	auto players = mOwner->FindObjectsWithTag<Tag_Player>();
	if (players.empty())
	{
		LOG("player is empty!");
		return;
	}

	mTargetID = players[Random::RandInt(0, players.size() - 1)];

	pathfind.MyPosition = mOwner->GetComponent<TransformComponent>().Position;
	pathfind.TargetPosition = mOwner->GetRegistry().get<TransformComponent>(mTargetID).Position;
}

void EnemyChaseState::Update()
{
	auto& pathfind = mOwner->GetComponent<PathFindComponent>();
	pathfind.MyPosition = mOwner->GetComponent<TransformComponent>().Position;
	pathfind.TargetPosition = mOwner->GetRegistry().get<TransformComponent>(mTargetID).Position;

	// TODO : Ÿ�ٰ��� �Ÿ��� ����� �����ٸ� ���ý�����Ʈ�� ��ȯ�ϱ�
}

void EnemyChaseState::Exit()
{
	LOG("Exit chase state...");

	auto& pathfind = mOwner->GetComponent<PathFindComponent>();
	pathfind.bFind = false;
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
