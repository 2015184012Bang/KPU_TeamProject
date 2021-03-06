#pragma once

#include "Entity.h"

class AIState;

class Script
	: public enable_shared_from_this<Script>
{
public:
	Script(entt::registry& registry, entt::entity owner);
	virtual ~Script() = default;

	virtual void Start() abstract;
	virtual void Update() abstract;

	template<typename T>
	T& GetComponent()
	{
		return mRegistry.get<T>(mOwner);
	}

	template<typename T, typename... Args>
	T& AddComponent(Args&&... args)
	{
		return mRegistry.emplace<T>(mOwner, forward<Args>(args)...);
	}

	template<typename T>
	void RemoveComponent()
	{
		mRegistry.remove<T>(mOwner);
	}

	template<typename T>
	bool HasComponent()
	{
		return mRegistry.any_of<T>(mOwner);
	}

	template<typename T>
	void AddTag()
	{
		mRegistry.emplace<T>(mOwner);
	}

	template<typename T>
	vector<entt::entity> FindObjectsWithTag()
	{
		vector<entt::entity> entities;

		auto view = mRegistry.view<T>();
		entities.reserve(view.size());

		for (auto entity : view)
		{
			entities.emplace_back(entity);
		}

		return entities;
	}

	entt::entity Find(string_view targetName);

	entt::registry& GetRegistry() { return mRegistry; }

protected:
	entt::registry& mRegistry;
	entt::entity mOwner = entt::null;
};

class AIScript
	: public Script
{
public:
	AIScript(entt::registry& registry, entt::entity owner)
		: Script{ registry, owner }
	{

	}

	virtual void Start() override {}

	virtual void Update() override;

	void AddState(shared_ptr<AIState>&& newState);

	void StartState(string_view stateName);
	
	void ChangeState(string_view stateName);

	void ChangeToPreivous();

private:
	unordered_map<string, shared_ptr<AIState>> mAIStates;
	shared_ptr<AIState> mCurrentState = nullptr;
	shared_ptr<AIState> mPreviousState = nullptr;
};