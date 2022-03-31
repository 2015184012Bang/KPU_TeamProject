#pragma once

#include "HeartBeat/Entity.h"

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

	virtual void Start() = 0;
	virtual void Update(float deltaTime) = 0;

private:
	Entity mOwner;
};

