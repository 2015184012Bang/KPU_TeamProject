#pragma once

#include "Scene.h"

class TestScene :
    public Scene
{
public:
    static void StaticCreate(Client* owner);
    static TestScene* Get();

    virtual void Enter() override;
    virtual void Exit() override;
    virtual void ProcessInput() override;
    virtual void Update(float deltaTime) override;
    virtual void Render(unique_ptr<Renderer>& renderer) override;

private:
    TestScene(Client* owner);

private:
    static TestScene* sInstance;

    //////////////////////////////////////////////////////////////////////////
    Entity mTestEntity;
    Entity mCell;
    Entity mMainCamera;
    //////////////////////////////////////////////////////////////////////////

    TCPSocketPtr clientSocket;
};

