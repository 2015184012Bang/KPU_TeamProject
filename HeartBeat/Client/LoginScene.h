#pragma once

#include "Scene.h"

class LoginScene : public Scene
{
public:
	LoginScene(Client* owner);

	virtual void Enter() override;
	virtual void Exit() override;
	virtual void ProcessInput() override;
	virtual void Update(float deltaTime) override;

private:
	bool mbConnected = false;
	bool mbChangeScene = false;
};

