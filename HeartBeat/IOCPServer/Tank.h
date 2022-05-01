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
		movement->Direction = Vector3::UnitZ;
	}

	virtual void Update() override
	{

	}

private:
	MovementComponent* movement = nullptr;
};