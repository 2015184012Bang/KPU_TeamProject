#pragma once

#include "Entity.h"
#include "Components.h"

class Script
{
public:
	Script(Entity owner);
	virtual ~Script() = default;

	template<typename T>
	T& GetComponent()
	{
		return mOwner.GetComponent<T>();
	}

	template<typename T>
	void AddTag()
	{
		mOwner.AddTag<T>();
	}

	template<typename T, typename... Args>
	T& AddComponent(Args&&... args)
	{
		return mOwner.AddComponent<T>(std::forward<Args>(args)...);
	}

	virtual void Start() = 0;
	virtual void Update(float deltaTime) = 0;

private:
	Entity mOwner;
};

