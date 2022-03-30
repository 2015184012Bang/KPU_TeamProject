#include "ClientPCH.h"
#include "TestScene.h"

#include "Animation.h"
#include "Client.h"
#include "ClientSystems.h"
#include "Input.h"
#include "Renderer.h"
#include "ResourceManager.h"
#include "Text.h"
#include "Skeleton.h"

#include "CharacterMovement.h"
#include "UIButtonTest.h"
#include "UIButtonTest2.h"

TestScene::TestScene(Client* owner)
	: Scene(owner)
{

}

void TestScene::Enter()
{
	HB_LOG("TestScene::Enter");

	{
		mTestText = mOwner->CreateTextEntity(L"Assets/Fonts/fontdata.txt");

		auto& t = mTestText.GetComponent<TextComponent>();
		t.Txt->SetSentence("Hello, world!");

		auto& rect = mTestText.GetComponent<RectTransformComponent>();
		rect.Position = Vector2(400.0f, 300.0f);
	}

	{
		Entity sp1 = mOwner->CreateSpriteEntity(100, 100, L"Assets/Textures/Smile.png");
		sp1.AddComponent<ButtonComponent>();
		sp1.AddComponent<ScriptComponent>(new UIButtonTest2(sp1));

		Entity sp2 = mOwner->CreateSpriteEntity(100, 100, L"Assets/Textures/Virus.png", 120);

		auto& rect = sp2.GetComponent<RectTransformComponent>();
		rect.Position.x = 50.0f;

		sp2.AddComponent<ButtonComponent>();
		sp2.AddComponent<ScriptComponent>(new UIButtonTest(sp2));
	}

	{
		mEnemy = mOwner->CreateSkeletalMeshEntity(L"Assets/Meshes/Virus.mesh", L"Assets/Textures/Virus.png",
			L"Assets/Skeletons/Virus.skel", L"Assets/Boxes/Virus.box");

		auto& animator = mEnemy.GetComponent<AnimatorComponent>();
		ClientSystems::PlayAnimation(&animator, ResourceManager::GetAnimation(L"Assets/Animations/Virus_Idle.anim"), 1.0f);

		Animation* idleAnim = ResourceManager::GetAnimation(L"Assets/Animations/Virus_Idle.anim");
		idleAnim->AddTransition("Run", ResourceManager::GetAnimation(L"Assets/Animations/Virus_Run.anim"));
		idleAnim->AddTransition("Attack", ResourceManager::GetAnimation(L"Assets/Animations/Virus_Attack.anim"));

		Animation* runningAnim = ResourceManager::GetAnimation(L"Assets/Animations/Virus_Run.anim");
		runningAnim->AddTransition("Idle", ResourceManager::GetAnimation(L"Assets/Animations/Virus_Idle.anim"));

		Animation* attackingAnim = ResourceManager::GetAnimation(L"Assets/Animations/Virus_Attack.anim");
		attackingAnim->SetLoop(false);
		attackingAnim->AddTransition("WhenEnd", ResourceManager::GetAnimation(L"Assets/Animations/Virus_Idle.anim"));

		mEnemy.AddComponent<ScriptComponent>(new CharacterMovement(mEnemy));
	}

	{
		mPickAx = mOwner->CreateStaticMeshEntity(L"Assets/Meshes/Pickax.mesh", L"Assets/Textures/Pickax.png");
		ClientSystems::SetBoneAttachment(mEnemy, mPickAx, "Weapon");
	}
}

void TestScene::Exit()
{
	HB_LOG("TestScene::Exit");
}

void TestScene::ProcessInput()
{
	if (Input::IsButtonPressed(eKeyCode::Return))
	{
		auto& text = mTestText.GetComponent<TextComponent>();
		text.Txt->SetSentence("Myung kyu god");
	}
}

void TestScene::Update(float deltaTime)
{
	auto& transform = mEnemy.GetComponent<TransformComponent>();
	ClientSystems::RotateY(&transform.Rotation, 30.0f, deltaTime, &transform.bDirty);
}

void TestScene::Render(unique_ptr<Renderer>& renderer)
{

}
