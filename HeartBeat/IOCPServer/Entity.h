#pragma once

#include <entt/entt.hpp>

entt::registry gRegistry;

class Entity
{
public:
	Entity() = default;

	explicit Entity(entt::entity handle)
		: mHandle(handle) {}

	// 빈 구조체를 사용해 태그를 표현하는데,
	// 빈 구조체를 리턴할 때 오류가 발생하기에 AddComponent()를 사용할 수 없다.
	// 따라서 리턴 타입이 void인 AddTag()를 이용해 태그를 붙인다.
	template<typename T>
	void AddTag()
	{
		ASSERT(!HasComponent<T>(), "Entity already has component.");
		gRegistry.emplace<T>(mHandle);
	}

	template<typename T, typename... Args>
	T& AddComponent(Args&&... args)
	{
		ASSERT(!HasComponent<T>(), "Entity already has component.");
		T& component = gRegistry.emplace<T>(mHandle, std::forward<Args>(args)...);
		return component;
	}

	template<typename T>
	T& GetComponent()
	{
		ASSERT(HasComponent<T>(), "Component does not exist.")
		T& component = gRegistry.get<T>(mHandle);
		return component;
	}

	template<typename T>
	bool HasComponent()
	{
		return gRegistry.any_of<T>(mHandle);
	}

	template<typename T>
	void RemoveComponent()
	{
		ASSERT(HasComponent<T>(), "Component does not exist.");
		gRegistry.remove<T>(mHandle);
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

private:
	entt::entity mHandle = entt::null;
};