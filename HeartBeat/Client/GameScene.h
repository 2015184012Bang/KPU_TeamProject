#pragma once

#include "Scene.h"

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
    void processPacket(MemoryStream* packet);
    void processCreateCharacter(MemoryStream* packet);

private:
    TCPSocketPtr mSocket;

    Entity mMyCharacter;
};

