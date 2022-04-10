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
    void processCreateTank(MemoryStream* packet);
    void processUpdateCollision(MemoryStream* packet);

    void sendUserInput();
    void updateAnimTrigger();
    void updateChildParentAfterDelete();
    void updateMainCamera();

private:
    TCPSocketPtr mSocket;

    Entity mMyCharacter;
    HBID mMyCharacterID;
};

wstring GetTileTex(int type);

