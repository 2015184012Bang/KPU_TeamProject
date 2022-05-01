#pragma once

#include "pch.h"
#include "Script.h"

class Tank : public Script
{
public:
	Tank(Entity owner)
		: Script(owner) {}

	virtual void Start() override
	{
		movement = &GetComponent<MovementComponent>();
		movement->Direction = Vector3::UnitX;

		transform = &GetComponent<TransformComponent>();

		Entity startTile = GetEntityByName("StartPoint");
		const auto& tilePosition = startTile.GetComponent<TransformComponent>().Position;

		transform->Position.x = tilePosition.x;
		transform->Position.z = tilePosition.z;
	}

	virtual void Update() override
	{

	}

private:
	MovementComponent* movement = nullptr;
	TransformComponent* transform = nullptr;
};