#pragma once
#include "Scene.h"
#include "Entity.h"
class TestScene :
	public Scene
{
public:
	TestScene(Client* owner);

	virtual void Enter() override;
	virtual void Exit() override;
	virtual void Update(float deltaTime) override;

private:
	Entity mCube = {};
};

