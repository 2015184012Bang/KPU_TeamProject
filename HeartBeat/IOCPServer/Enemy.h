#pragma once

#include "Script.h"

class Enemy
	: public Script
{
public:
	Enemy(entt::registry& registry, entt::entity owner)
		: Script{ registry, owner } {}

	virtual void Start() override
	{
		
	}

	virtual void Update() override
	{

	}

private:

};