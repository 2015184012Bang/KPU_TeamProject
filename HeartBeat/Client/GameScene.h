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
    void processUpdateTransform(MemoryStream* packet);
    void processCreateEnemy(MemoryStream* packet);
    void processDeleteEntity(MemoryStream* packet);

    void sendUserInput();
    void updateAnimTrigger();
    void updateChildParentAfterDelete();

private:
    TCPSocketPtr mSocket;

    Entity mMyCharacter;
    HBID mMyEID;
};

