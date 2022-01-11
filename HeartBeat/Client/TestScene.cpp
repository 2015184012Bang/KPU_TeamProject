#include "ClientPCH.h"
#include "TestScene.h"

#include "Input.h"
#include "Client.h"

#include "Renderer.h"
#include "ResourceManager.h"

TestScene* TestScene::sInstance;

TestScene::TestScene(Client* owner)
	: Scene(owner)
{

}

void TestScene::StaticCreate(Client* owner)
{
	static TestScene instance(owner);
	sInstance = &instance;
}

TestScene* TestScene::Get()
{
	return sInstance;
}

void TestScene::Enter()
{
	HB_LOG("TestScene::Enter");

	mTestTex = ResourceManager::GetTexture(L"Assets/Textures/cat.png");
	mTestMesh = ResourceManager::GetMesh(L"����");
}

void TestScene::Exit()
{
	HB_LOG("TestScene::Exit");
}

void TestScene::ProcessInput()
{

}

void TestScene::Update(float deltaTime)
{

}

void TestScene::Render(unique_ptr<Renderer>& renderer)
{
	renderer->Submit(mTestMesh, mTestTex);
}
