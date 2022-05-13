#include "pch.h"
#include "Script.h"

#include "AIState.h"

Script::Script(entt::registry& registry, entt::entity owner)
	: mRegistry{ registry }
	, mOwner{ owner }
{

}

entt::entity Script::Find(string_view targetName)
{
	auto view = mRegistry.view<NameComponent>();

	for (auto [entity, name] : view.each())
	{
		if (name.Name == targetName)
		{
			return entity;
		}
	}

	return entt::null;
}

void AIScript::Update()
{
	if (mCurrentState)
	{
		mCurrentState->Update();
	}
}

void AIScript::AddState(shared_ptr<AIState>&& newState)
{
	mAIStates.emplace(newState->GetStateName(), move(newState));
}

void AIScript::StartState(string_view stateName)
{
	string key{ stateName.data() };

	if (auto iter = mAIStates.find(key); iter != mAIStates.end())
	{
		mCurrentState = iter->second;
		mPreviousState = mCurrentState;
		mCurrentState->Enter();
	}
	else
	{
		LOG("There is no state to start: {0}", stateName);
	}
}

void AIScript::ChangeState(string_view stateName)
{
	string key{ stateName.data() };

	if (auto iter = mAIStates.find(key); iter != mAIStates.end())
	{
		mPreviousState = mCurrentState;
		mCurrentState->Exit();
		mCurrentState = iter->second;
		mCurrentState->Enter();
	}
	else
	{
		LOG("There is no state to change: {0}", stateName);
	}
}

void AIScript::ChangeToPreivous()
{
	mCurrentState->Exit();
	mCurrentState.swap(mPreviousState);
	mCurrentState->Enter();
}
