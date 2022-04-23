#pragma once

#include "Scene.h"
#include "../IOCPServer/Protocol.h"

class LoginScene : public Scene
{
public:
	LoginScene(Client* owner);

	virtual void Enter() override;
	virtual void Exit() override;
	virtual void ProcessInput() override;
	virtual void Update(float deltaTime) override;

private:
	void processAnswerLogin(const PACKET& packet);

private:
	bool mbConnected = false;
	bool mbChangeScene = false;
};

