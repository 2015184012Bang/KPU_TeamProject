#pragma once

#include "Script.h"
#include "ClientComponents.h"
#include "ClientSystems.h"
#include "Input.h"


class Character : public Script
{
public:
	Character(Entity owner)
		: Script(owner)
	{

	}

	virtual void Start() override
	{
		transform = &GetComponent<TransformComponent>();
		animator = &GetComponent<AnimatorComponent>();
	}

	virtual void Update(float deltaTime) override
	{

	}

private:
	TransformComponent* transform;
	AnimatorComponent* animator;
};