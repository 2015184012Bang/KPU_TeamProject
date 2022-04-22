#include "ClientPCH.h"
#include "LoginScene.h"

#include "HeartBeat/PacketType.h"
#include "HeartBeat/Define.h"

#include "../IOCPServer/Protocol.h"

#include "Application.h"
#include "Client.h"
#include "Input.h"
#include "LobbyScene.h"
#include "PacketManager.h"

LoginScene::LoginScene(Client* owner)
	: Scene(owner)
{

}

void LoginScene::Enter()
{
	// 배경화면 생성
	mOwner->CreateSpriteEntity(Application::GetScreenWidth(), Application::GetScreenHeight(),
		TEXTURE(L"Login_Background.png"), 10);
}

void LoginScene::Exit()
{
	// DontDestroyOnLoad 태그가 붙어진 엔티티를 제외한 모든 엔티티 삭제
	mOwner->DestroyAll();
}

void LoginScene::ProcessInput()
{
	PACKET packet;
	while (mOwner->GetPacketManager()->GetPacket(packet))
	{
		switch (packet.PacketID)
		{
		case ANSWER_LOGIN:
		{
			ANSWER_LOGIN_PACKET* loginPacket = reinterpret_cast<ANSWER_LOGIN_PACKET*>(packet.DataPtr);
			mOwner->SetClientID(loginPacket->ClientID);
			mbChangeScene = true;
			delete[] packet.DataPtr;
		}
			break;

		default:
			HB_LOG("Unknown packet type: {0}", packet.PacketID);
			break;
		}
	}
}

void LoginScene::Update(float deltaTime)
{
	// 엔터 키를 누르면 서버에 접속 요청
	if (Input::IsButtonPressed(eKeyCode::Return) && !mbConnected)
	{
		bool retVal = mOwner->GetPacketManager()->Connect("127.0.0.1", SERVER_PORT);

		if (retVal)
		{
			// 접속에 성공하면 클라이언트 소켓을 논블로킹 모드로 바꾸고
			// REQUEST_LOGIN 패킷을 보낸다.
			mbConnected = true;
			mOwner->GetPacketManager()->SetNonblocking(true);

			REQUEST_LOGIN_PACKET packet;
			packet.PacketID = REQUEST_LOGIN;
			packet.PacketSize = sizeof(REQUEST_LOGIN_PACKET);
			CopyMemory(packet.ID, mOwner->GetClientName().data(), MAX_ID_LEN);
			mOwner->GetPacketManager()->Send(reinterpret_cast<char*>(&packet), sizeof(REQUEST_LOGIN_PACKET));
		}
	}

	// 서버로부터 ANSWER_LOGIN_PACKET 패킷 받으면 씬 변경.
	if (mbChangeScene)
	{
		mOwner->ChangeScene(new LobbyScene(mOwner));
	}
}
