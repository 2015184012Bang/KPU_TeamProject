#pragma once

#include "pch.h"
#include "Script.h"
#include "Node.h"
#include "Random.h"
#include "GameMap.h"
#include "UserManager.h"
#include "Timer.h"
#include "Values.h"

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