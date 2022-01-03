#include "ClientPCH.h"
#include "Client.h"

#include "Input.h"
#include "Timer.h"

#include "Renderer.h"
#include "TestScene.h"

Client::Client()
	: Game()
	, mActiveScene(nullptr)
{

}

bool Client::Init()
{
	HB_LOG("Client::Init");

	Input::StaticInit();
	Timer::StaticInit();

	mRenderer = std::make_unique<Renderer>();
	mRenderer->Init();

	//////////////////////////////////////////////////////////////////////////
	TestScene::StaticCreate(this);
	mActiveScene = TestScene::Get();
	mActiveScene->Enter();
	//////////////////////////////////////////////////////////////////////////

	return true;
}

void Client::Shutdown()
{
	HB_LOG("Client::Shutdown");

	if (mActiveScene)
	{
		mActiveScene->Exit();
	}

	mRenderer->Shutdown();
}

void Client::Run()
{
	ProcessInput();
	Update();
	Render();
}

void Client::ChangeScene(Scene* scene)
{
	if (scene == nullptr)
	{
		return;
	}

	mActiveScene->Exit();
	mActiveScene = scene;
	mActiveScene->Enter();
}

void Client::ProcessInput()
{
	Input::Update();

	if (Input::IsButtonPressed(KeyCode::Escape))
	{
		SetRunning(false);
	}

	if (mActiveScene)
	{
		mActiveScene->ProcessInput();
	}
}

void Client::Update()
{
	Timer::Update();

	float deltaTime = Timer::GetDeltaTime();

	if (mActiveScene)
	{
		mActiveScene->Update(deltaTime);
	}
}

void Client::Render()
{
	mRenderer->BeginRender();

	mActiveScene->Render();

	mRenderer->EndRender();
}
