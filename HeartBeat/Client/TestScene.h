#pragma once

#include "Scene.h"

class TestScene :
    public Scene
{
public:
    TestScene(Client* owner);
    
    virtual void Enter() override;
    virtual void Exit() override;
    virtual void ProcessInput() override;
    virtual void Update(float deltaTime) override;
    virtual void Render(unique_ptr<Renderer>& renderer) override;

private:
    //////////////////////////////////////////////////////////////////////////
    Entity mEnemy;
    Entity mCell;
    Entity mPlayer;
    Entity mMainCamera;
    Entity m2dCamera;

    Entity mSprite;
    Entity mSprite2;

    Entity mText;
    //////////////////////////////////////////////////////////////////////////
};

