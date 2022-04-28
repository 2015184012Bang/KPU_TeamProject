#pragma once

#include <entt/entt.hpp>

entt::registry gRegistry;

class Entity
{
public:
	Entity() = default;

	explicit Entity(entt::entity handle)
		: mHandle(handle) {}

	// �� ����ü�� ����� �±׸� ǥ���ϴµ�,
	// �� ����ü�� ������ �� ������ �߻��ϱ⿡ AddComponent()�� ����� �� ����.
	// ���� ���� Ÿ���� void�� AddTag()�� �̿��� �±׸� ���δ�.
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