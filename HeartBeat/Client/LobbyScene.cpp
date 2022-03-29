#include "ClientPCH.h"
#include "LobbyScene.h"

#include "Client.h"
#include "ClientComponents.h"
#include "ClientSystems.h"
#include "Renderer.h"
#include "ResourceManager.h"
#include "Application.h"

LobbyScene::LobbyScene(Client* owner)
	: Scene(owner)
{

}

void LobbyScene::Enter()
{
	HB_LOG("LobbyScene Entered");

	mCell = mOwner->CreateSkeletalMeshEntity(L"Assets/Meshes/11_Cell.mesh", L"Assets/Textures/13_Cell_Yellow.png",
		L"Assets/Skeletons/11_Cell.skel", L"Assets/Boxes/11_Cell.box");

	auto& animator = mCell.GetComponent<AnimatorComponent>();

	ClientSystems::PlayAnimation(&animator, ResourceManager::GetAnimation(L"Assets/Animations/910_Idle.anim"), 1.0f);

	auto& transform = mCell.GetComponent<TransformComponent>();

	transform.Position.x = -300.0f;

	auto sprite = mOwner->CreateSpriteEntity(Application::GetScreenWidth(), Application::GetScreenHeight(), L"Assets/Textures/RedBox.png");

	auto& rectTransform = sprite.GetComponent<RectTransformComponent>();

	rectTransform.Position.x = 100.0f;
}

void LobbyScene::Exit()
{
	HB_LOG("LobbyScene Exited");
}

void LobbyScene::ProcessInput()
{

}

void LobbyScene::Update(float deltaTime)
{

}

void LobbyScene::Render(unique_ptr<Renderer>& renderer)
{

}
