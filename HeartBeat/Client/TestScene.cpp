#include "ClientPCH.h"
#include "TestScene.h"

#include "Client.h"
#include "Helpers.h"
#include "ResourceManager.h"
#include "Components.h"
#include "Input.h"
#include "Animation.h"

TestScene::TestScene(Client* owner)
	: Scene(owner)
{

}

void TestScene::Enter()
{
	auto& camera = mOwner->GetMainCamera();
	camera.GetComponent<CameraComponent>().Position.z = -5000.0f;

	Entity boss = mOwner->CreateSkeletalMeshEntity(MESH("Wall.mesh"),
		TEXTURE("Temp.png"), SKELETON("Wall.skel"));
	auto& animator = boss.GetComponent<AnimatorComponent>();

	auto& transform = boss.GetComponent<TransformComponent>();
	transform.Rotation.y = 180.0f;

	//Helpers::PlayAnimation(&animator, ANIM("Tail_Attack.anim"));
}

void TestScene::Exit()
{

}

void TestScene::Update(float deltaTime)
{
}
