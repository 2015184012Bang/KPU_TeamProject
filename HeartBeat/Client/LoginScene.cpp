#include "ClientPCH.h"
#include "LoginScene.h"

#include "Application.h"
#include "Client.h"
#include "Input.h"
#include "LobbyScene.h"

LoginScene::LoginScene(Client* owner)
	: Scene(owner)
{

}

void LoginScene::Enter()
{
	mBackground = mOwner->CreateSpriteEntity(Application::GetScreenWidth(), Application::GetScreenHeight(),
		L"Assets/Textures/Login_Background.png", 10);

	HB_LOG("LoginScene::Enter - Alive Entity: {0}", mOwner->GetRegistry().alive());
}

void LoginScene::Exit()
{
	mOwner->DestroyEntity(mBackground);

	HB_LOG("LoginScene::Exit - Alive Entity: {0}", mOwner->GetRegistry().alive());
}

void LoginScene::ProcessInput()
{
	if (Input::IsButtonPressed(eKeyCode::Return))
	{
		TCPSocketPtr sock = mOwner->GetMySocket();

		SocketAddress serveraddr("127.0.0.1", SERVER_PORT);

		int retVal = sock->Connect(serveraddr);

		if (retVal == SOCKET_ERROR)
		{
			SocketUtil::ReportError(L"LoginScene::ProcessInput()");
		}
		else
		{
			mOwner->ChangeScene(new LobbyScene(mOwner));
			sock->SetNonBlockingMode(true);
		}
	}
}

void LoginScene::Update(float deltaTime)
{

}

void LoginScene::Render(unique_ptr<Renderer>& renderer)
{

}
