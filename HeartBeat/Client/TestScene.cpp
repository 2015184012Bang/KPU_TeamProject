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

	{
		mCharacter = mOwner->CreateSkeletalMeshEntity(MESH("Character_Red.mesh"),
			TEXTURE("Character_Red.png"), SKELETON("Character_Red.skel"));
		auto& animator = mCharacter.GetComponent<AnimatorComponent>();
		Helpers::PlayAnimation(&animator, GetCharacterAnimationFile(2, CharacterAnimationType::RUN_NONE));
		Entity swab = mOwner->CreateStaticMeshEntity(MESH("Cotton_Swab.mesh"),
			TEXTURE("Cotton_Swab.png"));
		Helpers::AttachBone(mCharacter, swab, "Weapon");

		Entity belt = mOwner->CreateStaticMeshEntity(MESH("Belt_Red.mesh"),
			TEXTURE("Belt_Red.png"));
		Helpers::AttachBone(mCharacter, belt, "Bip001 Spine");

		Entity pill = mOwner->CreateStaticMeshEntity(MESH("Pill.mesh"),
			TEXTURE("Temp.png"));
		Helpers::AttachBone(mCharacter, pill, "Support");

		Entity bag = mOwner->CreateSkeletalMeshEntity(MESH("HealPack.mesh"),
			TEXTURE("Temp.png"), SKELETON("HealPack.skel"));
		Helpers::AttachBone(mCharacter, bag, "Bag");

		auto& bagAnimator = bag.GetComponent<AnimatorComponent>();
		Helpers::PlayAnimation(&bagAnimator, ANIM("HealPack.anim"));
	}

	{
		Entity character = mOwner->CreateSkeletalMeshEntity(MESH("Character_Green.mesh"),
			TEXTURE("Character_Green.png"), SKELETON("Character_Green.skel"));
		auto& animator = character.GetComponent<AnimatorComponent>();
		Helpers::PlayAnimation(&animator, GetCharacterAnimationFile(0, CharacterAnimationType::IDLE_NONE));
		Entity swab = mOwner->CreateStaticMeshEntity(MESH("Cotton_Swab.mesh"),
			TEXTURE("Cotton_Swab.png"));
		Helpers::AttachBone(character, swab, "Weapon");

		Entity belt = mOwner->CreateStaticMeshEntity(MESH("Belt_Red.mesh"),
			TEXTURE("Belt_Red.png"));
		Helpers::AttachBone(character, belt, "Bip001 Spine");

		Entity pill = mOwner->CreateStaticMeshEntity(MESH("Pill.mesh"),
			TEXTURE("Temp.png"));
		Helpers::AttachBone(character, pill, "Support");

		auto& transform = character.GetComponent<TransformComponent>();
		transform.Position.x = -700.0f;
	}

	{
		mEnemy = mOwner->CreateSkeletalMeshEntity(MESH("Virus.mesh"),
			TEXTURE("Virus.png"), SKELETON("Virus.skel"));
		auto& animator = mEnemy.GetComponent<AnimatorComponent>();
		Helpers::PlayAnimation(&animator, GetEnemyAnimation(EnemyType::VIRUS, EnemyAnimationType::DEAD));
		Entity swab = mOwner->CreateStaticMeshEntity(MESH("Hammer.mesh"),
			TEXTURE("Temp.png"));
		Helpers::AttachBone(mEnemy, swab, "Weapon");

		auto& transform = mEnemy.GetComponent<TransformComponent>();
		transform.Position.x = 700.0f;
	}

	{
		Entity cell = mOwner->CreateSkeletalMeshEntity(MESH("Cell.mesh"),
			TEXTURE("Cell_White.png"), SKELETON("Cell.skel"));

		auto& animator = cell.GetComponent<AnimatorComponent>();
		Helpers::PlayAnimation(&animator, ANIM("Cell_Dead.anim"));
	}
}

void TestScene::Exit()
{

}

void TestScene::Update(float deltaTime)
{
	if (Input::IsButtonPressed(KeyCode::SPACE))
	{
		auto entities = Helpers::GetEntityToDetach(mCharacter, false, "Bag");
		for (auto entity : entities)
		{
			DestroyEntity(entity);
		}

		mOwner->RearrangeAttachment();
	}
}
