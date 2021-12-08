#pragma once

#include "Scene.h"

class TestScene :
    public Scene
{
public:
    TestScene(Client* owner);

    static void StaticCreate(Client* owner);
    static TestScene* Get();

    virtual void Enter() override;
    virtual void Exit() override;
    virtual void ProcessInput() override;

private:
    static TestScene* sInstance;
};

