#pragma once

#include "Game.h"

class Scene;
class Renderer;
class PacketManager;
class Mesh;
class Texture;
class Skeleton;
class Font;

class Client :
    public Game
{
public:
    Client();
    
    virtual bool Init() override;
    virtual void Shutdown() override;
    virtual void Run() override;

    void ChangeScene(Scene* scene);

    Entity CreateSkeletalMeshEntity(const Mesh* mesh, const Texture* texFile, const Skeleton* skelFile, const wstring& boxFile = L"");
    Entity CreateSkeletalMeshEntity(const Mesh* mesh, const Texture* texFile, const Skeleton* skelFile, const uint64 eid, const wstring& boxFile = L"");
    Entity CreateStaticMeshEntity(const Mesh* meshFile, const Texture* texFile, const wstring& boxFile = L"");
    Entity CreateStaticMeshEntity(const Mesh* meshFile, const Texture* texFile, const uint64 eid);
    Entity CreateSpriteEntity(int width, int height, const Texture* texFile, int drawOrder = 100);
    Entity CreateTextEntity(const Font* fontFile);

    Entity& GetMainCamera() { return mMainCamera; }

    int GetClientID() const { return mClientID; }
    void SetClientID(int id) { mClientID = id; }

    const string& GetClientName() const { return mClientName; }
    void SetClientName(const string& nickname) { mClientName = nickname; }

    unique_ptr<PacketManager>& GetPacketManager() { return mPacketManager; }

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
    unique_ptr<Scene> mActiveScene = nullptr;
    unique_ptr<Renderer> mRenderer = nullptr;
    unique_ptr<PacketManager> mPacketManager = nullptr;

    Entity mMainCamera;
    Entity m2dCamera;

    int mClientID = -1;
    string mClientName = "KimMyungKyu";
};

