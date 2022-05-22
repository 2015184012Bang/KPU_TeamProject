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

	//Entity boss = mOwner->CreateSkeletalMeshEntity(MESH("Boss.mesh"),
	//	TEXTURE("Temp.png"), SKELETON("Boss.skel"));
	//auto& animator = boss.GetComponent<AnimatorComponent>();

	//auto& transform = boss.GetComponent<TransformComponent>();
	//transform.Rotation.y = 180.0f;

	//Helpers::PlayAnimation(&animator, ANIM("Boss_Idle.anim"));

	//Entity door = mOwner->CreateSkeletalMeshEntity(MESH("Door.mesh"),
	//	TEXTURE("Temp.png"), SKELETON("Door.skel"));
	//auto& animator = door.GetComponent<AnimatorComponent>();

	//auto openAnim = ANIM("Door_Open.anim");
	//openAnim->SetLoop(false);
	//Helpers::PlayAnimation(&animator, openAnim);
}

void TestScene::Exit()
{

}

void TestScene::Update(float deltaTime)
{
}
