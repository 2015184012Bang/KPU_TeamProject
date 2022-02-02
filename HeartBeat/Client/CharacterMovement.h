#pragma once

#include "Script.h"
#include "ClientComponents.h"
#include "ClientSystems.h"
#include "Input.h"


class CharacterMovement : public Script
{
public:
	CharacterMovement(Entity owner)
		: Script(owner)
	{

	}

	virtual void Start() override
	{
		transform = &GetComponent<TransformComponent>();
	}

	virtual void Update(float deltaTime) override
	{
		if (Input::IsButtonRepeat(eKeyCode::Right))
		{
			ClientSystems::MovePosition(&transform->Position, Vector3(300.0f, 0.0f, 0.0f), deltaTime, &transform->bDirty);
		}

		if (Input::IsButtonRepeat(eKeyCode::Left))
		{
			ClientSystems::MovePosition(&transform->Position, Vector3(-300.0f, 0.0f, 0.0f), deltaTime, &transform->bDirty);
		}

		if (Input::IsButtonRepeat(eKeyCode::Up))
		{
			ClientSystems::MovePosition(&transform->Position, Vector3(0.0f, 0.0f, 300.0f), deltaTime, &transform->bDirty);
		}

		if (Input::IsButtonRepeat(eKeyCode::Down))
		{
			ClientSystems::MovePosition(&transform->Position, Vector3(0.0f, 0.0f, -300.0f), deltaTime, &transform->bDirty);
		}
	}

private:
	TransformComponent* transform;
};