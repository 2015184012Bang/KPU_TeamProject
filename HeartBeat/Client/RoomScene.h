#pragma once
#include "Scene.h"

#include "../IOCPServer/Protocol.h"

class RoomScene :
    public Scene
{
public:
    RoomScene(Client* owner);

    virtual void Enter() override;
    virtual void Exit() override;
    virtual void ProcessInput() override;

private:
    bool mbChangeScene = false;
};

