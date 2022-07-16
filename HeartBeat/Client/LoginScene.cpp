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
#include "SoundManager.h"

LoginScene::LoginScene(Client* owner)
	: Scene(owner)
{

}

void LoginScene::Enter()
{
	SoundManager::PlaySound("LoginTheme.mp3", 0.25f);

	// ���ȭ�� ����
	mOwner->CreateSpriteEntity(Application::GetScreenWidth(), Application::GetScreenHeight(),
		TEXTURE("Login_Background.png"), 10);

	// �α��� �� ����
	{
		Entity loginForm = mOwner->CreateSpriteEntity(250, 55, TEXTURE("Login_Form.png"), 20);
		auto& rect = loginForm.GetComponent<RectTransformComponent>();
		rect.Position = Vector2{ Application::GetScreenWidth() / 2.0f - 125.0f , Application::GetScreenHeight() - 185.0f };
	}

	// �α��� ��ư ����
	{
		Entity loginButton = mOwner->CreateSpriteEntity(240, 74, TEXTURE("Login_Button.png"));
		auto& rect = loginButton.GetComponent<RectTransformComponent>();
		rect.Position = Vector2{ Application::GetScreenWidth() / 2 - 120.0f, Application::GetScreenHeight() - 125.0f };
		loginButton.AddComponent<ButtonComponent>([this]()
			{
				if (mbConnected)
				{
					return;
				}

				SoundManager::PlaySound("ButtonClick.mp3");

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
			});
	}

	// �Է��� ���ڸ� ǥ���� �ؽ�Ʈ
	mLoginText = Entity{ gRegistry.create() };
	auto& txt = mLoginText.AddComponent<TextComponent>();
	txt.X = Application::GetScreenWidth() / 2 - 115.0f;
	txt.Y = Application::GetScreenHeight() - 185.0f;
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
