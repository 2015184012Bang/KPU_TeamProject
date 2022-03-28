#include "ClientPCH.h"
#include "Client.h"

#include "ClientComponents.h"
#include "Input.h"
#include "Mesh.h"
#include "Renderer.h"
#include "ResourceManager.h"
#include "Timer.h"
#include "TestScene.h"
#include "Text.h"

Client::Client()
	: Game()
	, mActiveScene(nullptr)
{

}

bool Client::Init()
{
	HB_LOG("Client::Init");

	Input::Init();
	Timer::Init();
	SocketUtil::Init();

	mRenderer = std::make_unique<Renderer>();
	mRenderer->Init();

	//////////////////////////////////////////////////////////////////////////
	mActiveScene = std::make_unique<TestScene>(this);
	mActiveScene->Enter();
	//////////////////////////////////////////////////////////////////////////

	return true;
}

void Client::Shutdown()
{
	HB_LOG("Client::Shutdown");

	GetRegistry().clear();

	if (mActiveScene)
	{
		mActiveScene->Exit();
	}

	mRenderer->Shutdown();
	SocketUtil::Shutdown();
}

void Client::Run()
{
	processInput();
	update();
	render();
}

void Client::ChangeScene(Scene* scene)
{
	if (scene == nullptr)
	{
		return;
	}

	mActiveScene->Exit();
	mActiveScene = nullptr;
	mActiveScene = unique_ptr<Scene>(scene);
	mActiveScene->Enter();
}

Entity Client::CreateSkeletalMeshEntity(const wstring& meshFile, const wstring& texFile, const wstring& skelFile, const wstring& boxFile)
{
	Entity e = Entity(CreateEntity(), this);

	auto& transform = e.AddComponent<TransformComponent>();
	e.AddTag<Tag_SkeletalMesh>();
	e.AddComponent<IDComponent>();
	e.AddComponent<MeshRendererComponent>(ResourceManager::GetMesh(meshFile), ResourceManager::GetTexture(texFile));
	e.AddComponent<AnimatorComponent>(ResourceManager::GetSkeleton(skelFile));

	if (boxFile.size() != 0)
	{
		e.AddComponent<BoxComponent>(ResourceManager::GetAABB(boxFile), transform.Position, transform.Rotation.y);
		e.AddComponent<DebugDrawComponent>(ResourceManager::GetDebugMesh(boxFile));
	}

	return e;
}

Entity Client::CreateStaticMeshEntity(const wstring& meshFile, const wstring& texFile, const wstring& boxFile)
{
	Entity e = Entity(CreateEntity(), this);

	auto& transform = e.AddComponent<TransformComponent>();
	e.AddTag<Tag_StaticMesh>();
	e.AddComponent<IDComponent>();
	e.AddComponent<MeshRendererComponent>(ResourceManager::GetMesh(meshFile), ResourceManager::GetTexture(texFile));

	if (boxFile.size() != 0)
	{
		e.AddComponent<BoxComponent>(ResourceManager::GetAABB(boxFile), transform.Position, transform.Rotation.y);
		e.AddComponent<DebugDrawComponent>(ResourceManager::GetDebugMesh(boxFile));
	}

	return e;
}

Entity Client::CreateSpriteEntity(int width, int height, const wstring& texFile)
{
	Entity e = Entity(CreateEntity(), this);

	e.AddTag<Tag_Sprite>();
	e.AddComponent<IDComponent>();
	e.AddComponent<RectTransformComponent>(width, height);
	e.AddComponent<SpriteRendererComponent>(new SpriteMesh(width, height), ResourceManager::GetTexture(texFile));

	return e;
}

Entity Client::CreateTextEntity(const wstring& fontFile)
{
	Entity e = Entity(CreateEntity(), this);

	e.AddTag<Tag_Text>();
	e.AddComponent<IDComponent>();
	e.AddComponent<RectTransformComponent>(0, 0);
	e.AddComponent<TextComponent>(new Text(ResourceManager::GetFont(L"Assets/Fonts/fontdata.txt")));

	return e;
}

void Client::processInput()
{
	Input::Update();

	if (Input::IsButtonPressed(eKeyCode::Escape))
	{
		SetRunning(false);
	}

	if (mActiveScene)
	{
		mActiveScene->ProcessInput();
	}
}

void Client::update()
{
	Timer::Update();

	float deltaTime = Timer::GetDeltaTime();

	if (mActiveScene)
	{
		mActiveScene->Update(deltaTime);
	}
}

void Client::render()
{
	mRenderer->BeginRender();

	mActiveScene->Render(mRenderer);

	mRenderer->EndRender();
}
