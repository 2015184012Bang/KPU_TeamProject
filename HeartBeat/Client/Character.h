#pragma once

#include "Script.h"
#include "Components.h"
#include "Helpers.h"
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
		movement = &AddComponent<MovementComponent>(300.0f);
	}

	virtual void Update(float deltaTime) override
	{

	}

private:
	TransformComponent* transform = nullptr;
	AnimatorComponent* animator = nullptr;
	MovementComponent* movement = nullptr;
};