#pragma once

extern entt::registry gRegistry;

class Entity
{
public:
	Entity()
		: mHandle(entt::null)
	{}

	explicit Entity(entt::entity handle)
		: mHandle(handle)
	{}

	template<typename T, typename... Args>
	T& AddComponent(Args&&... args)
	{
		HB_ASSERT(!HasComponent<T>(), "Entity already has component.");
		T& component = gRegistry.emplace<T>(mHandle, std::forward<Args>(args)...);
		return component;
	}

	template<typename T>
	void AddTag()
	{
		if (HasComponent<T>())
		{
			return;
		}

		gRegistry.emplace<T>(mHandle);
	}

	template<typename T>
	T& GetComponent()
	{
		HB_ASSERT(HasComponent<T>(), "Entity does not have component.");
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
		HB_ASSERT(HasComponent<T>(), "Entity does not have component to remove.");
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

	operator entt::entity() const { return mHandle; }
	operator entt::entity() { return mHandle; }

private:
	entt::entity mHandle;
};

void DestroyAll();
void DestroyEntityByID(const uint32 id);
void DestroyEntity(const entt::entity entity);

template<typename T>
void DestroyByComponent()
{
	auto view = gRegistry.view<T>();
	for (auto entity : view)
	{
		gRegistry.destroy(entity);
	}
}

template<typename T>
vector<Entity> GetEntitiesWithTag()
{
	auto view = gRegistry.view<T>();

	vector<Entity> entts;
	entts.reserve(view.size());
	for (auto entity : view)
	{
		entts.emplace_back(entity);
	}

	return entts;
}

Entity GetEntityByID(const uint32 id);
Entity GetEntityByName(string_view name);
