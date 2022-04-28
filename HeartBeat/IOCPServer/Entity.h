#pragma once

#include "GameManager.h"

class Entity
{
public:
	Entity() = default;

	Entity(entt::entity handle, GameManager* gameManager)
		: mHandle(handle)
		, mGameManager(gameManager) {}

	// �� ����ü�� ����� �±׸� ǥ���ϴµ�,
	// �� ����ü�� ������ �� ������ �߻��ϱ⿡ AddComponent()�� ����� �� ����.
	// ���� ���� Ÿ���� void�� AddTag()�� �̿��� �±׸� ���δ�.
	template<typename T>
	void AddTag()
	{
		ASSERT(!HasComponent<T>(), "Entity already has component.");
		mGameManager->mRegistry.emplace<T>(mHandle);
	}

	template<typename T, typename... Args>
	T& AddComponent(Args&&... args)
	{
		ASSERT(!HasComponent<T>(), "Entity already has component.");
		T& component = mGameManager->mRegistry.emplace<T>(mHandle, std::forward<Args>(args)...);
		return component;
	}

	template<typename T>
	T& GetComponent()
	{
		ASSERT(HasComponent<T>(), "Component does not exist.")
		T& component = mGameManager->mRegistry.get<T>(mHandle);
		return component;
	}

	template<typename T>
	bool HasComponent()
	{
		return mGameManager->mRegistry.any_of<T>(mHandle);
	}

	template<typename T>
	void RemoveComponent()
	{
		ASSERT(HasComponent<T>(), "Component does not exist.");
		mGameManager->mRegistry.remove<T>(mHandle);
	}

	bool operator==(const Entity& other)
	{
		return mHandle == other.mHandle;
	}

	bool operator!=(const Entity& other)
	{
		return !(*this == other);
	}

	operator bool() { return mHandle != entt::null; }
	operator bool() const { return mHandle != entt::null; }

	operator entt::entity() { return mHandle; }
	operator entt::entity() const { return mHandle; }

	GameManager* GetGameManager() { return mGameManager; }

private:
	entt::entity mHandle = entt::null;
	GameManager* mGameManager = nullptr;
};