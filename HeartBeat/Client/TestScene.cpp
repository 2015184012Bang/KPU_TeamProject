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

	mMainCamera = Entity(mOwner->CreateEntity(), mOwner);
	mMainCamera.AddComponent<CameraComponent>(Vector3(0.0f, 0.0f, -1.0f), Vector3(0.0f, 0.0f, 1.0f));
	mMainCamera.AddTag<Camera>();
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
		ClientSystems::MovePosition(&transform.Position, Vector3(1.0f, 0.0f, 0.0f), deltaTime, &transform.bDirty);
	}
}

void TestScene::Render(unique_ptr<Renderer>& renderer)
{
	auto& camera = mMainCamera.GetComponent<CameraComponent>();

	ClientSystems::BindViewProjectionMatrix(camera.Position, 
		camera.Target, camera.Up, camera.FOV, camera.Buffer);

	auto view = (mOwner->GetRegistry()).view<StaticMesh>();
	for (auto entity : view)
	{
		Entity e = Entity(entity, mOwner);

		TransformComponent& transform = e.GetComponent<TransformComponent>();
		ClientSystems::BindWorldMatrix(transform.Position, transform.Rotation, transform.Scale, transform.Buffer, &transform.bDirty);

		MeshRendererComponent& meshRenderer = e.GetComponent<MeshRendererComponent>();
		renderer->Submit(meshRenderer.Mesi, meshRenderer.Tex);
	}
}
