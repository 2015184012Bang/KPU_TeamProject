#include "ClientPCH.h"
#include "LobbyScene.h"

#include "HeartBeat/PacketType.h"

#include "Application.h"
#include "Client.h"
#include "ClientComponents.h"
#include "ClientSystems.h"
#include "Input.h"
#include "ResourceManager.h"
#include "Text.h"
#include "GameScene.h"

LobbyScene::LobbyScene(Client* owner)
	: Scene(owner)
{

}

void LobbyScene::Enter()
{
	mSocket = mOwner->GetMySocket();
	int myClientID = mOwner->GetClientID();

	createNicknameText(myClientID);
	createCharacterMesh(myClientID);

	{
		Entity readyButton = mOwner->CreateSpriteEntity(200, 100, TEXTURE(L"Ready_Button.png"));
		auto& transform = readyButton.GetComponent<RectTransformComponent>();
		transform.Position.x = Application::GetScreenWidth() / 2.0f;
		transform.Position.y = Application::GetScreenHeight() - 150.0f;

		readyButton.AddComponent<ButtonComponent>([this, myClientID]() {
			MemoryStream packet;
			packet.WriteUByte(static_cast<uint8>(CSPacket::eImReady));
			packet.WriteInt(myClientID);
			this->mSocket->Send(&packet, sizeof(MemoryStream));
			});
	}

	{
		mReadyText = mOwner->CreateTextEntity(FONT(L"fontdata.txt"));
		auto& text = mReadyText.GetComponent<TextComponent>();
		text.Txt->SetSentence("0 / 3");
		auto& transform = mReadyText.GetComponent<RectTransformComponent>();
		transform.Position.x = Application::GetScreenWidth() / 2.0f + 210.0f;
		transform.Position.y = Application::GetScreenHeight() - 200.0f;
	}

	auto& camera = mOwner->GetMainCamera();
	auto& cc = camera.GetComponent<CameraComponent>();
	cc.Position.z = -1000.0f;
}

void LobbyScene::Exit()
{
	mOwner->DestroyAll();

	HB_LOG("Alive entities: {0}", mOwner->GetRegistry().alive());
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
		uint8 packetType;
		packet->ReadUByte(&packetType);

		switch (static_cast<SCPacket>(packetType))
		{
		case SCPacket::eUserConnected:
			processUserConnected(packet);
			break;

		case SCPacket::eReadyPressed:
			processReadyPressed(packet);
			break;

		case SCPacket::eGameStart:
			processGameStart(packet);
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
	createNicknameText(clientID);
	createCharacterMesh(clientID);
}

void LobbyScene::createNicknameText(int clientID)
{
	Entity nickname = mOwner->CreateTextEntity(FONT(L"fontdata.txt"));
	auto& text = nickname.GetComponent<TextComponent>();
	text.Txt->SetSentence(mOwner->GetNickname());
	auto& transform = nickname.GetComponent<RectTransformComponent>();
	transform.Position.x = 10.0f;
	transform.Position.y = (clientID * SPACE_BETWEEN_LINES) + SPACE_BETWEEN_LINES;
}

void LobbyScene::createCharacterMesh(int clientID)
{
	wstring meshFile;
	wstring texFile;
	wstring skelFile;

	GetCharacterFiles(clientID, &meshFile, &texFile, &skelFile);

	Entity character = mOwner->CreateSkeletalMeshEntity(meshFile, texFile, skelFile);

	auto& transform = character.GetComponent<TransformComponent>();
	transform.Position.x = (clientID * WIDTH_BETWEEN_CHARACTERS) - WIDTH_BETWEEN_CHARACTERS;
	transform.Rotation.y = 180.0f;

	auto& animator = character.GetComponent<AnimatorComponent>();

	wstring idleAnimFile = GetCharacterAnimation(clientID, CharacterAnimationType::eIdle);
	Animation* idleAnim = ResourceManager::GetAnimation(idleAnimFile);
	ClientSystems::PlayAnimation(&animator, idleAnim, 1.0f);
}

void LobbyScene::processGameStart(MemoryStream* packet)
{
	mOwner->ChangeScene(new GameScene(mOwner));
}

void LobbyScene::processReadyPressed(MemoryStream* packet)
{
	int readyCount = 0;
	packet->ReadInt(&readyCount);

	auto& text = mReadyText.GetComponent<TextComponent>();
	text.Txt->SetSentence(std::to_string(readyCount) + " / 3");
}
