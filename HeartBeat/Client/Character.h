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
		bool bMove = false;

		if (Input::IsButtonRepeat(eKeyCode::Right))
		{
			bMove = true;
		}

		if (Input::IsButtonRepeat(eKeyCode::Left))
		{
			bMove = true;
		}

		if (Input::IsButtonRepeat(eKeyCode::Up))
		{
			bMove = true;
		}

		if (Input::IsButtonRepeat(eKeyCode::Down))
		{
			bMove = true;
		}

		if (bMove)
		{
			animator->SetTrigger("Run");
		}
		else
		{
			animator->SetTrigger("Idle");
		}
	}

private:
	TransformComponent* transform;
	AnimatorComponent* animator;
};