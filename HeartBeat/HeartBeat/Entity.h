#pragma once

#include "Game.h"

class Entity
{
public:
	Entity()
		: mHandle(entt::null)
		, mGame(nullptr)
	{}

	Entity(entt::entity handle, Game* game)
		: mHandle(handle)
		, mGame(game)
	{}

	template<typename T, typename... Args>
	T& AddComponent(Args&&... args)
	{
		HB_ASSERT(!HasComponent<T>(), "Entity already has component.");
		T& component = mGame->mRegistry.emplace<T>(mHandle, std::forward<Args>(args)...);
		return component;
	}

	template<typename T>
	void AddTag()
	{
		HB_ASSERT(!HasComponent<T>(), "Entity already has tag.");
		mGame->mRegistry.emplace<T>(mHandle);
	}

	template<typename T>
	T& GetComponent()
	{
		HB_ASSERT(HasComponent<T>(), "Entity does not have component.");
		T& component = mGame->mRegistry.get<T>(mHandle);
		return component;
	}

	template<typename T>
	bool HasComponent()
	{
		return mGame->mRegistry.any_of<T>(mHandle);
	}

	template<typename T>
	void RemoveComponent()
	{
		HB_ASSERT(HasComponent<T>(), "Entity does not have component to remove.");
		mGame->mRegistry.remove<T>(mHandle);
	}

	bool operator==(const Entity& other)
	{
		return mHandle == other.mHandle;
	}

	bool operator!=(const Entity& other)
	{
		return !(*this == other);
	}

private:
	entt::entity mHandle;
	Game* mGame;
};