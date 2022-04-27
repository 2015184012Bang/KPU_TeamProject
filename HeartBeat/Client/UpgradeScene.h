#pragma once

#include "Scene.h"
#include "../IOCPServer/Protocol.h"

class UpgradeScene :
    public Scene
{
    const float DISTANCE_BETWEEN_PLANE = 700.0f;

public:
    UpgradeScene(Client* owner);

    virtual void Enter() override;
    virtual void Exit() override;
    virtual void ProcessInput() override;
    virtual void Update(float deltaTime) override;

private:
    void initPlayersPositionToZero();
    void createPlanes();

    bool pollKeyboardPressed();
    bool pollKeyboardReleased();
    void checkCollisionWithPlanes();

    void processNotifyMove(const PACKET& packet);
    void processAnswerMove(const PACKET& packet);

private:
    Entity mPlayerCharacter = {};

    Vector3 mDirection = Vector3::Zero;
};

