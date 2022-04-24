#pragma once

#include "Scene.h"
#include "../IOCPServer/Protocol.h"

class UpgradeScene :
    public Scene
{
public:
    UpgradeScene(Client* owner);

    virtual void Enter() override;
    virtual void Exit() override;
    virtual void ProcessInput() override;
    virtual void Update(float deltaTime) override;

private:
    bool pollKeyboardPressed();
    bool pollKeyboardReleased();

    void processNotifyMove(const PACKET& packet);

private:
    Entity mPlayerCharacter = {};
};

