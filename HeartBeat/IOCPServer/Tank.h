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
		LOG("Tank Start!");
	}

	virtual void Update() override
	{
		LOG("Tank Update...");
	}

private:

};