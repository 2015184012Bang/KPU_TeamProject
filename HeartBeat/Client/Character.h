#pragma once

#include "Script.h"
#include "Components.h"
#include "Helpers.h"
#include "Input.h"
#include "ResourceManager.h"

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
		movement = &GetComponent<MovementComponent>();
	}

	virtual void Update(float deltaTime) override
	{
		if (movement->Direction != Vector3::Zero)
		{
			animator->SetTrigger("Run");
		}
		else
		{
			animator->SetTrigger("Idle");
		}
	}

private:
	TransformComponent* transform = nullptr;
	AnimatorComponent* animator = nullptr;
	MovementComponent* movement = nullptr;
};