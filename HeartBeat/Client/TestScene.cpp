#include "ClientPCH.h"
#include "TestScene.h"

#include "Animation.h"
#include "Client.h"
#include "ClientSystems.h"
#include "Input.h"
#include "Renderer.h"
#include "ResourceManager.h"
#include "Text.h"

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
		Entity enemy = mOwner->CreateSkeletalMeshEntity(L"Assets/Meshes/21_HEnemy.mesh", L"Assets/Textures/21_HEnemy.png",
			L"Assets/Skeletons/21_HEnemy.skel", L"Assets/Boxes/21_HEnemy.box");

		auto& animator = enemy.GetComponent<AnimatorComponent>();
		ClientSystems::PlayAnimation(&animator, ResourceManager::GetAnimation(L"Assets/Animations/924_Running.anim"), 1.0f);

		Animation* idleAnim = ResourceManager::GetAnimation(L"Assets/Animations/921_Idle.anim");
		idleAnim->AddTransition("Walking", ResourceManager::GetAnimation(L"Assets/Animations/924_Running.anim"));
		idleAnim->AddTransition("Attacking", ResourceManager::GetAnimation(L"Assets/Animations/922_Attacking.anim"));

		Animation* runningAnim = ResourceManager::GetAnimation(L"Assets/Animations/924_Running.anim");
		runningAnim->AddTransition("Idle", ResourceManager::GetAnimation(L"Assets/Animations/921_Idle.anim"));

		Animation* attackingAnim = ResourceManager::GetAnimation(L"Assets/Animations/922_Attacking.anim");
		attackingAnim->SetLoop(false);
		attackingAnim->AddTransition("WhenEnd", ResourceManager::GetAnimation(L"Assets/Animations/921_Idle.anim"));

		enemy.AddComponent<ScriptComponent>(new CharacterMovement(enemy));
	}

	{
		Entity cell = mOwner->CreateSkeletalMeshEntity(L"Assets/Meshes/11_Cell.mesh", L"Assets/Textures/11_Cell_Red.png",
			L"Assets/Skeletons/11_Cell.skel", L"Assets/Boxes/11_Cell.box");

		auto& animator = cell.GetComponent<AnimatorComponent>();
		ClientSystems::PlayAnimation(&animator, ResourceManager::GetAnimation(L"Assets/Animations/912_Running.anim"), 1.0f);

		auto& transform = cell.GetComponent<TransformComponent>();
		transform.Position.x = 300.0f;
		transform.Rotation.y = 180.0f;
	}

	{
		Entity	player = mOwner->CreateSkeletalMeshEntity(L"Assets/Meshes/03_Character_Pink.mesh", L"Assets/Textures/03_Character_Pink.png",
			L"Assets/Skeletons/03_Character_Pink.skel", L"Assets/Boxes/01_Character.box");

		auto& animator = player.GetComponent<AnimatorComponent>();
		ClientSystems::PlayAnimation(&animator, ResourceManager::GetAnimation(L"Assets/Animations/901_Idle_Pink.anim"), 1.0f);

		auto& transform = player.GetComponent<TransformComponent>();
		transform.Position.x = -300.0f;
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

}

void TestScene::Render(unique_ptr<Renderer>& renderer)
{

}
