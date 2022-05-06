#include "ClientPCH.h"
#include "LoginScene.h"

#include "Define.h"

#include "Application.h"
#include "Client.h"
#include "Define.h"
#include "Input.h"
#include "PacketManager.h"
#include "Utils.h"
#include "ResourceManager.h"
#include "LobbyScene.h"
#include "Tags.h"

LoginScene::LoginScene(Client* owner)
	: Scene(owner)
{

}

void LoginScene::Enter()
{
	// 배경화면 생성
	mOwner->CreateSpriteEntity(Application::GetScreenWidth(), Application::GetScreenHeight(),
		TEXTURE("Login_Background.png"), 10);
}

void LoginScene::Exit()
{
	// DontDestroyOnLoad 태그가 붙어진 엔티티를 제외한 모든 엔티티 삭제
	DestroyExclude<Tag_DontDestroyOnLoad>();
}

void LoginScene::ProcessInput()
{
	PACKET packet;
	while (mOwner->GetPacketManager()->GetPacket(packet))
	{
		switch (packet.PacketID)
		{
		case ANSWER_LOGIN:
			processAnswerLogin(packet);
			break;

		default:
			HB_LOG("Unknown packet type: {0}", packet.PacketID);
			break;
		}

		// Login -> Room 씬으로 전환.
		if (mbChangeScene)
		{
			mOwner->ChangeScene(new LobbyScene{ mOwner });
			break;
		}
	}
}

void LoginScene::Update(float deltaTime)
{
	// 엔터 키를 누르면 서버에 접속 요청
	if (Input::IsButtonPressed(KeyCode::RETURN) && !mbConnected)
	{
		bool retVal = mOwner->GetPacketManager()->Connect(Values::ServerIP, Values::ServerPort);

		if (retVal)
		{
			// 접속에 성공하면 클라이언트 소켓을 논블로킹 모드로 바꾸고
			// REQUEST_LOGIN 패킷을 보낸다.
			mbConnected = true;
			mOwner->GetPacketManager()->SetNonblocking(true);

			REQUEST_LOGIN_PACKET packet = {};
			packet.PacketID = REQUEST_LOGIN;
			packet.PacketSize = sizeof(REQUEST_LOGIN_PACKET);
			CopyMemory(packet.ID, mOwner->GetClientName().data(), MAX_ID_LEN);
			mOwner->GetPacketManager()->Send(reinterpret_cast<char*>(&packet), sizeof(REQUEST_LOGIN_PACKET));
		}
	}
}

void LoginScene::processAnswerLogin(const PACKET& packet)
{
	ANSWER_LOGIN_PACKET* loginPacket = reinterpret_cast<ANSWER_LOGIN_PACKET*>(packet.DataPtr);

	if (loginPacket->Result != SUCCESS)
	{
		HB_LOG("Login declined!");
		return;
	}

	mbChangeScene = true;
}
