#include "ClientPCH.h"
#include "TestScene.h"

#include "Input.h"
#include "Client.h"

#include "Renderer.h"
#include "ResourceManager.h"
#include "ClientSystems.h"

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

	mTestEntity = Entity(mOwner->CreateEntity(), mOwner);
	mTestEntity.AddComponent<MeshRendererComponent>(ResourceManager::GetMesh(L"¿¿æ÷"), ResourceManager::GetTexture(L"Assets/Textures/cat.png"));
	mTestEntity.AddComponent<TransformComponent>();
	mTestEntity.AddTag<StaticMesh>();
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
	if (Input::IsButtonRepeat(eKeyCode::Right))
	{
		TransformComponent& transform = mTestEntity.GetComponent<TransformComponent>();
		ClientSystems::Move(&transform.Position, Vector3(1.0f, 0.0f, 0.0f), deltaTime);
	}
}

void TestScene::Render(unique_ptr<Renderer>& renderer)
{
	auto view = (mOwner->GetRegistry()).view<StaticMesh>();

	for (auto entity : view)
	{
		Entity e = Entity(entity, mOwner);

		TransformComponent& transform = e.GetComponent<TransformComponent>();
		ClientSystems::SetWorldMatrix(transform.Position, transform.Rotation, transform.Scale, transform.Buffer);

		MeshRendererComponent& meshRenderer = e.GetComponent<MeshRendererComponent>();
		renderer->Submit(meshRenderer.Mesi, meshRenderer.Tex);
	}
}
