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
    Entity CreateSpriteEntity(int width, int height, const wstring& texFile, int drawOrder = 100);
    Entity CreateTextEntity(const wstring& fontFile);

    Entity& GetMainCamera() { return mMainCamera; }

private:
    void processInput();
    void update();
    void render();

    void createCameraEntity();

    void processButton();
    void updateScript(float deltaTime);
    void updateAnimation(float deltaTime);
    void updateCollisionBox(float deltaTime);

    void drawSkeletalMesh();
    void drawStaticMesh();
    void drawCollisionBox();
    void drawSpriteAndText();

private:
    unique_ptr<Scene> mActiveScene;
    
    unique_ptr<Renderer> mRenderer;

    Entity mMainCamera;
    Entity m2dCamera;
};

