#pragma once

#include "HeartBeat/Game.h"

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
    Entity CreateSkeletalMeshEntity(const wstring& meshFile, const wstring& texFile, const wstring& skelFile, const uint64 eid);

    Entity CreateStaticMeshEntity(const wstring& meshFile, const wstring& texFile, const wstring& boxFile = L"");
    Entity CreateSpriteEntity(int width, int height, const wstring& texFile, int drawOrder = 100);
    Entity CreateTextEntity(const wstring& fontFile);

    Entity& GetMainCamera() { return mMainCamera; }

    TCPSocketPtr GetMySocket() { return mMySocket; }

    int GetClientID() const { return mClientID; }
    void SetClientID(int id) { mClientID = id; }

    const string& GetNickname() const { return mNickname; }
    void SetNickname(const string& nickname) { mNickname = nickname; }

private:
    void processInput();
    void update();
    void render();

    void createCameraEntity();
    void createAnimationTransitions();

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

    TCPSocketPtr mMySocket;
    
    int mClientID;
    string mNickname;
};

