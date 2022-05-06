#pragma once
#include "Scene.h"

#include "../IOCPServer/Protocol.h"

class LobbyScene :
    public Scene
{
public:
    LobbyScene(Client* owner);

    virtual void Enter() override;
    virtual void Exit() override;
    virtual void ProcessInput() override;

private:
    void processNotifyRoom(const PACKET& packet);
    void processEnterRoom(const PACKET& packet);

    void createRoomSprite(int index, bool canEnter = false);

private:
    enum
    {
        AVAILABLE = 0,
        CANNOT,
        END
    };

    bool mbChangeScene = false;
};

