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
#include "Components.h"

LoginScene::LoginScene(Client* owner)
	: Scene(owner)
{

}

void LoginScene::Enter()
{
	// ���ȭ�� ����
	mOwner->CreateSpriteEntity(Application::GetScreenWidth(), Application::GetScreenHeight(),
		TEXTURE("Login_Background.png"), 10);

	// �α��� �� ����
	Entity loginForm = mOwner->CreateSpriteEntity(200, 50, TEXTURE("White.png"), 20);
	auto& rect = loginForm.GetComponent<RectTransformComponent>();
	rect.Position = Vector2{ Application::GetScreenWidth() / 2.0f - 100.0f , Application::GetScreenHeight() - 200.0f };

	// �Է��� ���ڸ� ǥ���� �ؽ�Ʈ
	mLoginText = Entity{ gRegistry.create() };
	auto& txt = mLoginText.AddComponent<TextComponent>();
	txt.X = Application::GetScreenWidth() / 2 - 75.0f;
	txt.Y = Application::GetScreenHeight() - 200.0f;
}

void LoginScene::Exit()
{
	// DontDestroyOnLoad �±װ� �پ��� ��ƼƼ�� ������ ��� ��ƼƼ ����
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

		// Login -> Room ������ ��ȯ.
		if (mbChangeScene)
		{
			mOwner->ChangeScene(new LobbyScene{ mOwner });
			break;
		}
	}
}

void LoginScene::Update(float deltaTime)
{
	// �ؽ�Ʈ ������Ʈ
	auto& txt = mLoginText.GetComponent<TextComponent>();
	txt.Sentence = s2ws(gKeyInput);

	// ���� Ű�� ������ ������ ���� ��û
	if (Input::IsButtonPressed(KeyCode::RETURN) && !mbConnected)
	{
		mOwner->SetClientName(gKeyInput);

		bool retVal = mOwner->GetPacketManager()->Connect(Values::ServerIP, Values::ServerPort);

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
