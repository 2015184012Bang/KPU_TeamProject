#pragma once

#include "Scene.h"
#include "../IOCPServer/Protocol.h"

class GameScene :
    public Scene
{
public:
    GameScene(Client* owner);

    virtual void Enter() override;
    virtual void Exit() override;
    virtual void ProcessInput() override;
    virtual void Update(float deltaTime) override;

private:
	bool pollKeyboardPressed();
	bool pollKeyboardReleased();

	void processAnswerMove(const PACKET& packet);
	void processNotifyMove(const PACKET& packet);

private:
    Entity mPlayerCharacter = {};
    Vector3 mDirection = Vector3::Zero;

    bool mbChangeScene = false;
};

