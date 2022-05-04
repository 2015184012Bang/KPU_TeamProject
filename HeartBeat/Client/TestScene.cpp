#include "ClientPCH.h"
#include "TestScene.h"

#include "Client.h"
#include "Helpers.h"
#include "ResourceManager.h"
#include "Components.h"
#include "Input.h"

TestScene::TestScene(Client* owner)
	: Scene(owner)
{

}

void TestScene::Enter()
{
	auto& camera = mOwner->GetMainCamera();
	camera.GetComponent<CameraComponent>().Position.z = -1000.0f;

	mCube = mOwner->CreateStaticMeshEntity(MESH("Cube.mesh"),
		TEXTURE("Red.png"));
	auto& transform = mCube.GetComponent<TransformComponent>();
	transform.Rotation.y = 45.0f;
}

void TestScene::Exit()
{

}

void TestScene::Update(float deltaTime)
{
	static float yaw = 0.0f;

	yaw += deltaTime* 60.0;

	auto& transform = mCube.GetComponent<TransformComponent>();
	Helpers::UpdateYRotation(&transform.Rotation.y, yaw, &transform.bDirty);
}
