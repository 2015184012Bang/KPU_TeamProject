#include "ClientPCH.h"
#include "LoginScene.h"

#include "Define.h"

#include "Application.h"
#include "Client.h"
#include "Input.h"
#include "LobbyScene.h"
#include "PacketManager.h"
#include "Utils.h"
#include "ResourceManager.h"

LoginScene::LoginScene(Client* owner)
	: Scene(owner)
{

}

void LoginScene::Enter()
{
	// ���ȭ�� ����
	mOwner->CreateSpriteEntity(Application::GetScreenWidth(), Application::GetScreenHeight(),
		TEXTURE(L"Login_Background.png"), 10);
}

void LoginScene::Exit()
{
	// DontDestroyOnLoad �±װ� �پ��� ��ƼƼ�� ������ ��� ��ƼƼ ����
	mOwner->DestroyAll();
}

void LoginScene::ProcessInput()
{
	PACKET packet;
	if (mOwner->GetPacketManager()->GetPacket(packet))
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
	}
}

void LoginScene::Update(float deltaTime)
{
	// ���� Ű�� ������ ������ ���� ��û
	if (Input::IsButtonPressed(eKeyCode::Return) && !mbConnected)
	{
		bool retVal = mOwner->GetPacketManager()->Connect("127.0.0.1", SERVER_PORT);

		if (retVal)
		{
			// ���ӿ� �����ϸ� Ŭ���̾�Ʈ ������ ����ŷ ���� �ٲٰ�
			// REQUEST_LOGIN ��Ŷ�� ������.
			mbConnected = true;
			mOwner->GetPacketManager()->SetNonblocking(true);

			REQUEST_LOGIN_PACKET packet = {};
			packet.PacketID = REQUEST_LOGIN;
			packet.PacketSize = sizeof(REQUEST_LOGIN_PACKET);
			CopyMemory(packet.ID, mOwner->GetClientName().data(), MAX_ID_LEN);
			mOwner->GetPacketManager()->Send(reinterpret_cast<char*>(&packet), sizeof(REQUEST_LOGIN_PACKET));
		}
	}

	// �����κ��� ANSWER_LOGIN_PACKET ��Ŷ ������ �� ����.
	if (mbChangeScene)
	{
		mOwner->ChangeScene(new LobbyScene(mOwner));
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

	mOwner->SetClientID(loginPacket->ClientID);
	mbChangeScene = true;
}
