#include "ClientPCH.h"
#include "LoginScene.h"

#include "Application.h"
#include "Client.h"
#include "Input.h"
#include "TestScene.h"

LoginScene::LoginScene(Client* owner)
	: Scene(owner)
{

}

void LoginScene::Enter()
{
	HB_LOG("LoginScene::Enter");

	mBackground = mOwner->CreateSpriteEntity(Application::GetScreenWidth(), Application::GetScreenHeight(),
		L"Assets/Textures/Login_Background.png", 10);
}

void LoginScene::Exit()
{
	HB_LOG("LoginScene::Exit");

	mOwner->DestroyEntity(mBackground);
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
			mOwner->ChangeScene(new TestScene(mOwner));
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
