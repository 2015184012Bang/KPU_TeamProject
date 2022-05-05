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
	}

	virtual void Update(float deltaTime) override
	{
		auto& movement = GetComponent<MovementComponent>();
		auto& animator = GetComponent<AnimatorComponent>();

		if (movement.Direction != Vector3::Zero)
		{
			animator.SetTrigger("Run");
		}
		else
		{
			animator.SetTrigger("Idle");
		}
	}

private:
};