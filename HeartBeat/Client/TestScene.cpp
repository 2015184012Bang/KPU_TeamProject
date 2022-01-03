#include "ClientPCH.h"
#include "TestScene.h"

#include "Input.h"
#include "Client.h"

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

	//////////////////////////////////////////////////////
	Entity e = Entity(mOwner->CreateEntity(), mOwner);
	e.AddComponent<TransformComponent>(3.0f);
	e.AddComponent<HelloComponent>();
	e.AddTag<Foo>();
	//////////////////////////////////////////////////////
}

void TestScene::Exit()
{
	HB_LOG("TestScene::Exit");
}

void TestScene::ProcessInput()
{
	//////////////////////////////////////////////////////
	if (Input::IsButtonPressed(KeyCode::A))
	{
		auto view = (mOwner->GetRegistry()).view<Foo>();
	
		for (auto entity : view)
		{
			Entity e = Entity(entity, mOwner);

			e.RemoveComponent<HelloComponent>();
		}
	}
	//////////////////////////////////////////////////////
}

void TestScene::Update(float deltaTime)
{
	//////////////////////////////////////////////////////
	auto view = (mOwner->GetRegistry()).view<Foo>();

	for (auto entity : view)
	{
		Entity e = Entity(entity, mOwner);
		
		if (e.HasComponent<HelloComponent>())
		{
			HB_LOG("asdad");
		}

		auto& transform = e.GetComponent<TransformComponent>();
		HB_LOG("x: {0}", transform.X);
	}
	//////////////////////////////////////////////////////
}
