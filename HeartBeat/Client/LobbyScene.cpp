#include "ClientPCH.h"
#include "LobbyScene.h"

#include "HeartBeat/PacketType.h"

#include "Application.h"
#include "Client.h"
#include "ClientComponents.h"
#include "ClientSystems.h"
#include "Input.h"
#include "Text.h"
#include "TestScene.h"

LobbyScene::LobbyScene(Client* owner)
	: Scene(owner)
{

}

void LobbyScene::Enter()
{
	mSocket = mOwner->GetMySocket();

	{
		Entity nickname = mOwner->CreateTextEntity(L"Assets/Fonts/fontdata.txt");
		auto& text = nickname.GetComponent<TextComponent>();
		text.Txt->SetSentence(mOwner->GetNickname());
		auto& transform = nickname.GetComponent<RectTransformComponent>();
		transform.Position.x = (mOwner->GetClientID() * 200.0f) + 200.0f;
		transform.Position.y = Application::GetScreenHeight() / 2.0f;
	}

	{
		mReadyButton = mOwner->CreateSpriteEntity(200, 100, L"Assets/Textures/Ready_Button.png");
		auto& transform = mReadyButton.GetComponent<RectTransformComponent>();
		transform.Position.x = Application::GetScreenWidth() / 2.0f;
		transform.Position.y = Application::GetScreenHeight() - 150.0f;

		mReadyButton.AddComponent<ButtonComponent>([]() {
			HB_LOG("Ready Button Pressed!"); });
	}
}

void LobbyScene::Exit()
{
	mOwner->DestroyAll();
}

void LobbyScene::ProcessInput()
{
	MemoryStream packet;
	int retVal = mSocket->Recv(&packet, sizeof(MemoryStream));

	if (retVal == SOCKET_ERROR)
	{
		int error = WSAGetLastError();

		if (error == WSAEWOULDBLOCK)
		{
			return;
		}
		else
		{
			SocketUtil::ReportError(L"LobbyScene::ProcessInput", error);
			mOwner->SetRunning(false);
		}
	}
	else
	{
		processPacket(&packet);
	}
}

void LobbyScene::processPacket(MemoryStream* packet)
{
	int totalLen = packet->GetLength();
	packet->SetLength(0);

	while (packet->GetLength() < totalLen)
	{
		SCPacket packetType;
		packet->ReadInt(reinterpret_cast<int*>(&packetType));

		switch (packetType)
		{
		case SCPacket::eUserConnected:
			processUserConnected(packet);
			break;

		default:
			HB_LOG("Unknown packet type: {0}", static_cast<int>(packetType));
			packet->SetLength(totalLen);
			break;
		}
	}
}

void LobbyScene::processUserConnected(MemoryStream* packet)
{
	int clientID = -1;
	packet->ReadInt(&clientID);

	int nickLen = -1;
	packet->ReadInt(&nickLen);

	string nickname;
	packet->ReadString(&nickname, nickLen);

	if (clientID == mOwner->GetClientID())
	{
		return;
	}

	bool exists = std::any_of(mConnectedID.begin(), mConnectedID.end(), [clientID](int id) {
		return id == clientID;
		});

	if (exists)
	{
		return;
	}
	
	mConnectedID.push_back(clientID);

	Entity e = mOwner->CreateTextEntity(L"Assets/Fonts/fontdata.txt");
	auto& text = e.GetComponent<TextComponent>();
	text.Txt->SetSentence(nickname);
	auto& transform = e.GetComponent<RectTransformComponent>();
	transform.Position.x = (clientID * 200.0f) + 200.0f;
	transform.Position.y = Application::GetScreenHeight() / 2.0f;
}
