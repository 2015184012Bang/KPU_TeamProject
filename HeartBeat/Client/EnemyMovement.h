#pragma once

#include "Script.h"
#include "ClientComponents.h"
#include "ClientSystems.h"
#include "Input.h"


class EnemyMovement : public Script
{
public:
	EnemyMovement(Entity owner)
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
		if (Input::IsButtonPressed(eKeyCode::MouseLButton))
		{
			animator->SetTrigger("Attack");
		}

		bool bMove = false;

		if (Input::IsButtonRepeat(eKeyCode::Right))
		{
			ClientSystems::MovePosition(&transform->Position, Vector3(300.0f, 0.0f, 0.0f), deltaTime, &transform->bDirty);
			bMove = true;
		}

		if (Input::IsButtonRepeat(eKeyCode::Left))
		{
			ClientSystems::MovePosition(&transform->Position, Vector3(-300.0f, 0.0f, 0.0f), deltaTime, &transform->bDirty);
			bMove = true;
		}

		if (Input::IsButtonRepeat(eKeyCode::Up))
		{
			ClientSystems::MovePosition(&transform->Position, Vector3(0.0f, 0.0f, 300.0f), deltaTime, &transform->bDirty);
			bMove = true;
		}

		if (Input::IsButtonRepeat(eKeyCode::Down))
		{
			ClientSystems::MovePosition(&transform->Position, Vector3(0.0f, 0.0f, -300.0f), deltaTime, &transform->bDirty);
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