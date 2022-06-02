#pragma once

#include "Entity.h"

class Scene;
class Renderer;
class PacketManager;
class Mesh;
class Texture;
class Skeleton;

using namespace std::string_literals;

extern bool gShouldClose;

class Client
{
public:
    Client();
    ~Client();
    
    bool Init();
    void Shutdown();
    void Run();

    void ChangeScene(Scene* scene);

    Entity CreateSkeletalMeshEntity(const Mesh* mesh, const Texture* texFile, const Skeleton* skelFile, string_view boxFile = ""sv);
    Entity CreateSkeletalMeshEntity(const Mesh* mesh, const Texture* texFile, const Skeleton* skelFile, const uint32 eid, string_view boxFile = ""sv);
    Entity CreateStaticMeshEntity(const Mesh* meshFile, const Texture* texFile, string_view boxFile = ""sv);
    Entity CreateStaticMeshEntity(const Mesh* meshFile, const Texture* texFile, const uint32 eid, string_view boxFile = ""sv);
    Entity CreateSpriteEntity(int width, int height, const Texture* texFile, int drawOrder = 100);

    void DestroyEntityAfter(const uint32 eid, float secs);
   
    // MainCamera가 target을 따라다니도록 한다.
    void SetFollowCameraTarget(const Entity& target, const Vector3& offset);

    void ResetCamera();

    // Parent에 붙은 Children들 삭제
    void DeleteChildren(entt::registry& regi, entt::entity entity);

    void SetBackgroundColor(const XMVECTORF32& color);

public:
	int GetClientID() const { return mClientID; }
	void SetClientID(int id) { mClientID = id; }
	const string& GetClientName() const { return mClientName; }
	void SetClientName(string_view nickname) { mClientName = nickname.data(); }

	unique_ptr<PacketManager>& GetPacketManager() { return mPacketManager; }

    Entity& GetMainCamera() { return mMainCamera; }

private:
    void processInput();
    void update();
    void render();

    void createCameraEntity();
    void createLightEntity();

    void processButton();
    void processPendingEntities(float deltaTime);
    void updateMovement(float deltaTime);
    void updateScript(float deltaTime);
    void updateAnimation(float deltaTime);
    void updateCollisionBox(float deltaTime);
    void updateMainCamera();

    void drawSkeletalMesh();
    void drawStaticMesh();
    void drawCollisionBox();
    void drawSprite();
    void drawFont();

private:
    unique_ptr<Scene> mActiveScene = nullptr;
    unique_ptr<Renderer> mRenderer = nullptr;
    unique_ptr<PacketManager> mPacketManager = nullptr;

    Entity mMainCamera = {};
    Entity m2dCamera = {};
    Entity mLight = {};

    int mClientID = -1;
    string mClientName = {};

    Entity mFollowCameraTarget = {}; // 메인 카메라가 따라다닐 대상
    Vector3 mTargetOffset = Vector3::Zero; // 타겟과의 오프셋

    deque<std::pair<uint32, float>> mPendingEntities;
};

