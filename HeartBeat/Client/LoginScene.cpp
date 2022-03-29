#include "ClientPCH.h"
#include "LoginScene.h"

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

	mClientSocket = mOwner->GetClientSocket();
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
	if (Input::IsButtonPressed(eKeyCode::Return))
	{
		SocketAddress serveraddr("127.0.0.1", 9000);

		int retVal = 0;
		do 
		{
			retVal = mClientSocket->Connect(serveraddr);
		} while (retVal == SOCKET_ERROR);

		HB_LOG("Connection Success!");

		mClientSocket->SetNonBlockingMode(true);

		mOwner->ChangeScene(new TestScene(mOwner));
	}
}

void LoginScene::Render(unique_ptr<Renderer>& renderer)
{

}
