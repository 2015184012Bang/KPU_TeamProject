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

		Entity sp2 = mOwner->CreateSpriteEntity(100, 100, L"Assets/Textures/21_HEnemy.png", 120);

		auto& rect = sp2.GetComponent<RectTransformComponent>();
		rect.Position.x = 50.0f;

		sp2.AddComponent<ButtonComponent>();
		sp2.AddComponent<ScriptComponent>(new UIButtonTest(sp2));
	}

	{
		mEnemy = mOwner->CreateSkeletalMeshEntity(L"Assets/Meshes/21_HEnemy.mesh", L"Assets/Textures/21_HEnemy.png",
			L"Assets/Skeletons/21_HEnemy.skel", L"Assets/Boxes/21_HEnemy.box");

		auto& animator = mEnemy.GetComponent<AnimatorComponent>();
		ClientSystems::PlayAnimation(&animator, ResourceManager::GetAnimation(L"Assets/Animations/924_Running.anim"), 1.0f);

		Animation* idleAnim = ResourceManager::GetAnimation(L"Assets/Animations/921_Idle.anim");
		idleAnim->AddTransition("Walking", ResourceManager::GetAnimation(L"Assets/Animations/924_Running.anim"));
		idleAnim->AddTransition("Attacking", ResourceManager::GetAnimation(L"Assets/Animations/922_Attacking.anim"));

		Animation* runningAnim = ResourceManager::GetAnimation(L"Assets/Animations/924_Running.anim");
		runningAnim->AddTransition("Idle", ResourceManager::GetAnimation(L"Assets/Animations/921_Idle.anim"));

		Animation* attackingAnim = ResourceManager::GetAnimation(L"Assets/Animations/922_Attacking.anim");
		attackingAnim->SetLoop(false);
		attackingAnim->AddTransition("WhenEnd", ResourceManager::GetAnimation(L"Assets/Animations/921_Idle.anim"));

		mEnemy.AddComponent<ScriptComponent>(new CharacterMovement(mEnemy));
	}

	{
		mPickAx = mOwner->CreateStaticMeshEntity(L"Assets/Meshes/Pickax.mesh", L"Assets/Textures/Pickax.png");
		ClientSystems::SetBoneAttachment(mEnemy, mPickAx, "Bip001 R Hand");
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
