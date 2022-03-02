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

    Entity CreateSkeletalMeshEntity(const wstring& meshFile, const wstring& texFile, const wstring& skelFile, const wstring& boxFile = L"");
    Entity CreateStaticMeshEntity(const wstring& meshFile, const wstring& texFile, const wstring& boxFile = L"");

private:
    void processInput();
    void update();
    void render();

private:
    Scene* mActiveScene;
    
    unique_ptr<Renderer> mRenderer;
};

