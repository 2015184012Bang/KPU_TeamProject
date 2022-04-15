#include "ClientPCH.h"
#include "TestScene.h"

#include "HeartBeat/PacketType.h"
#include "HeartBeat/Tags.h"
#include "HeartBeat/Define.h"
#include "HeartBeat/GameMap.h"

#include "Animation.h"
#include "Client.h"
#include "ClientSystems.h"
#include "Character.h"
#include "Enemy.h"
#include "Input.h"
#include "Renderer.h"
#include "ResourceManager.h"
#include "Text.h"
#include "Skeleton.h"


TestScene::TestScene(Client* owner)
	: Scene(owner)
{

}

void TestScene::Enter()
{
	auto& camera = mOwner->GetMainCamera();
	camera.GetComponent<CameraComponent>().Position.z = -2000.0f;

	{
		Entity cell = mOwner->CreateSkeletalMeshEntity(MESH(L"Cell.mesh"), TEXTURE(L"Cell_White.png"), SKELETON(L"Cell.skel"));
		cell.GetComponent<TransformComponent>().Rotation.y = 180.0f;
		auto& animator = cell.GetComponent<AnimatorComponent>();
		ClientSystems::PlayAnimation(&animator, ResourceManager::GetAnimation(ANIM(L"Cell_Run.anim")));

		Entity pickax = mOwner->CreateStaticMeshEntity(MESH(L"Pickax.mesh"), TEXTURE(L"Pickax.png"));
		ClientSystems::SetBoneAttachment(cell, pickax, "Weapon");
	}
	
	{
		Entity dog = mOwner->CreateSkeletalMeshEntity(MESH(L"Character_Red.mesh"), TEXTURE(L"Character_Red.png"), SKELETON(L"Character_Red.skel"));
		dog.GetComponent<TransformComponent>().Position.x = 500.0f;
		auto& dAnimator = dog.GetComponent<AnimatorComponent>();
		ClientSystems::PlayAnimation(&dAnimator, ResourceManager::GetAnimation(ANIM(L"CR_Run.anim")));

		Entity bag = mOwner->CreateSkeletalMeshEntity(MESH(L"Bag.mesh"), TEXTURE(L"Temp.png"), SKELETON(L"Bag.skel"));

		auto& bAnimator = bag.GetComponent<AnimatorComponent>();
		ClientSystems::PlayAnimation(&bAnimator, ResourceManager::GetAnimation(ANIM(L"Bag.anim")));

		ClientSystems::SetBoneAttachment(dog, bag, "Bag");
	}

	{
		Entity dog = mOwner->CreateSkeletalMeshEntity(MESH(L"Dog.mesh"), TEXTURE(L"Dog.png"), SKELETON(L"Dog.skel"), BOX(L"Dog.box"));
		dog.GetComponent<TransformComponent>().Position.x = -500.0f;
		dog.GetComponent<TransformComponent>().Rotation.y = 90.0f;
		auto& dAnimator = dog.GetComponent<AnimatorComponent>();
		ClientSystems::PlayAnimation(&dAnimator, ResourceManager::GetAnimation(ANIM(L"Dog_Idle.anim")));
	}

	{
		Entity dog = mOwner->CreateSkeletalMeshEntity(MESH(L"Cart.mesh"), TEXTURE(L"Tank.png"), SKELETON(L"Cart.skel"), BOX(L"Cart.box"));
		dog.GetComponent<TransformComponent>().Position.x = -1000.0f;
		auto& dAnimator = dog.GetComponent<AnimatorComponent>();
		ClientSystems::PlayAnimation(&dAnimator, ResourceManager::GetAnimation(ANIM(L"Cart_Run.anim")));
	}
}

void TestScene::Exit()
{

}

void TestScene::ProcessInput()
{

}

void TestScene::Update(float deltaTime)
{

}
