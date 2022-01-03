#pragma once

#include "Game.h"

class Scene;
class Renderer;

class Client :
    public Game
{
public:
    Client();
    
    virtual bool Init() override;
    virtual void Shutdown() override;
    virtual void Run() override;

    void ChangeScene(Scene* scene);

private:
    void ProcessInput();
    void Update();
    void Render();

private:
    Scene* mActiveScene;
    
    unique_ptr<Renderer> mRenderer;
};

