#include "ClientPCH.h"
#include "LoginScene.h"

LoginScene::LoginScene(Client* owner)
	: Scene(owner)
{

}

void LoginScene::Enter()
{
	HB_LOG("LoginScene::Enter");
}

void LoginScene::Exit()
{
	HB_LOG("LoginScene::Exit");
}

void LoginScene::ProcessInput()
{

}

void LoginScene::Update(float deltaTime)
{

}

void LoginScene::Render(unique_ptr<Renderer>& renderer)
{

}
