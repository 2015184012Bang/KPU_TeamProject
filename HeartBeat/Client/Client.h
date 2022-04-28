#pragma once

#include "Game.h"

class Scene;
class Renderer;
class PacketManager;
class Mesh;
class Texture;
class Skeleton;
class Font;

using namespace std::string_literals;

class Client :
    public Game
{
public:
    Client();
    
    virtual bool Init() override;
    virtual void Shutdown() override;
    virtual void Run() override;

    void ChangeScene(Scene* scene);

    Entity CreateSkeletalMeshEntity(const Mesh* mesh, const Texture* texFile, const Skeleton* skelFile, string_view boxFile = ""sv);
    Entity CreateSkeletalMeshEntity(const Mesh* mesh, const Texture* texFile, const Skeleton* skelFile, const uint32 eid, string_view boxFile = ""sv);
    Entity CreateStaticMeshEntity(const Mesh* meshFile, const Texture* texFile, string_view boxFile = ""sv);
    Entity CreateStaticMeshEntity(const Mesh* meshFile, const Texture* texFile, const uint32 eid, string_view boxFile = ""sv);
    Entity CreateSpriteEntity(int width, int height, const Texture* texFile, int drawOrder = 100);
    Entity CreateTextEntity(const Font* fontFile);
   
    // Child entity를 삭제하고 난 후 호출해줄 것.
    void RearrangeAttachment();

    // MainCamera가 target을 따라다니도록 한다.
    void SetFollowCameraTarget(const Entity& target, const Vector3& offset);

    UINT16 ServerPort = 0;
    string ServerIP = {};

public:
	int GetClientID() const { return mClientID; }
	void SetClientID(int id) { mClientID = id; }
	const string& GetClientName() const { return mClientName; }
	void SetClientName(string_view nickname) { mClientName = nickname.data(); }

	unique_ptr<PacketManager>& GetPacketManager() { return mPacketManager; }

    Entity& GetMainCamera() { return mMainCamera; }

private:
    void loadServerSettingsFromXML(string_view fileName);

    void processInput();
    void update();
    void render();

    void createCameraEntity();

    void processButton();
    void updateMovement(float deltaTime);
    void updateScript(float deltaTime);
    void updateAnimation(float deltaTime);
    void updateCollisionBox(float deltaTime);
    void updateMainCamera();

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

    Entity mFollowCameraTarget = {}; // 메인 카메라가 따라다닐 대상
    Vector3 mTargetOffset = Vector3::Zero; // 타겟과의 오프셋
};

