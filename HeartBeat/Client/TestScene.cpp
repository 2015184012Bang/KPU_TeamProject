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

	Entity e = Entity(mOwner->CreateEntity(), mOwner);
	e.AddComponent<MeshRendererComponent>(ResourceManager::GetMesh(L"¿¿æ÷"), ResourceManager::GetTexture(L"Assets/Textures/cat.png"));
	e.AddTag<StaticMesh>();
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
	auto view = (mOwner->GetRegistry()).view<StaticMesh>();

	for (auto entity : view)
	{
		Entity e = Entity(entity, mOwner);

		MeshRendererComponent& meshRenderer = e.GetComponent<MeshRendererComponent>();

		renderer->Submit(meshRenderer.Mesi, meshRenderer.Tex);
	}
}
