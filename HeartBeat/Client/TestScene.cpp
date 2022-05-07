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
	camera.GetComponent<CameraComponent>().Position.z = -1000.0f;

	Entity player = mOwner->CreateSkeletalMeshEntity(MESH("Character_Pink.mesh"),
		TEXTURE("Character_Pink.png"), SKELETON("Character_Pink.skel"));
	auto& animator = player.GetComponent<AnimatorComponent>();

	auto& transform = player.GetComponent<TransformComponent>();
	transform.Rotation.y = 180.0f;

	Helpers::PlayAnimation(&animator, ANIM("CG_Skill2.anim"));

	auto bagAnim = ANIM("HealPack_Attack.anim");
	bagAnim->SetLoop(false);

	Entity healpack = mOwner->CreateSkeletalMeshEntity(MESH("HealPack.mesh"),
		TEXTURE("HealPack.png"), SKELETON("HealPack.skel"));
	auto& hpAnimator = healpack.GetComponent<AnimatorComponent>();
	Helpers::PlayAnimation(&hpAnimator, ANIM("HealPack_Attack.anim"));

	Helpers::AttachBone(player, healpack, "Bag");
}

void TestScene::Exit()
{

}

void TestScene::Update(float deltaTime)
{
}
