#pragma once
#include "Scene.h"


class TestScene :
    public Scene
{
public:
	TestScene(Client* owner);

	virtual void Enter() override;
	virtual void Exit() override;
	virtual void ProcessInput() override;
	virtual void Update(float deltaTime) override;
};

