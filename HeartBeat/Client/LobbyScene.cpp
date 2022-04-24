#include "ClientPCH.h"
#include "LobbyScene.h"

#include "Application.h"
#include "Client.h"
#include "Components.h"
#include "Helpers.h"
#include "Input.h"
#include "ResourceManager.h"
#include "PacketManager.h"
#include "Utils.h"


LobbyScene::LobbyScene(Client* owner)
	: Scene(owner)
{

}

void LobbyScene::Enter()
{
	int myClientID = mOwner->GetClientID();

	// 클라이언트가 만일 호스트라면
	// 시작 버튼을 누를 수 있도록 버튼을 생성한다.
	if (HOST_ID == myClientID)
	{
		Entity gameStartButton = mOwner->CreateSpriteEntity(START_BUTTON_WIDTH, START_BUTTON_HEIGHT, TEXTURE(L"Start_Button.png"));
		auto& transform = gameStartButton.GetComponent<RectTransformComponent>();
		transform.Position.x = (Application::GetScreenWidth() / 2.0f) - (transform.Width / 2.0f);
		transform.Position.y = Application::GetScreenHeight() - START_BUTTON_DIST_FROM_BOTTOM;

		gameStartButton.AddComponent<ButtonComponent>([this]() {
			REQUEST_GAME_START_PACKET packet = {};
			packet.PacketID = REQUEST_GAME_START;
			packet.PacketSize = sizeof(REQUEST_GAME_START_PACKET);
			mOwner->GetPacketManager()->Send(reinterpret_cast<char*>(&packet), sizeof(packet));
			});
	}

	createCharacterMesh(myClientID);

	// 메인 카메라 위치 조정
	auto& camera = mOwner->GetMainCamera();
	auto& cc = camera.GetComponent<CameraComponent>();
	cc.Position.z = -1000.0f;
}

void LobbyScene::Exit()
{
	mOwner->DestroyAll();
}

void LobbyScene::ProcessInput()
{
	PACKET packet;
	if (mOwner->GetPacketManager()->GetPacket(packet))
	{
		switch (packet.PacketID)
		{
		case ANSWER_NOTIFY_LOGIN:
			processAnswerNofifyLogin(packet);
			break;

		case ANSWER_GAME_START:
			processAnswerGameStart(packet);
			break;
			
		default:
			HB_LOG("Unknown packet type: {0}", packet.PacketID);
			break;
		}
	}
}

void LobbyScene::createCharacterMesh(int clientID)
{
	auto [mesh, tex, skel] = GetCharacterFiles(clientID);

	Entity character = mOwner->CreateSkeletalMeshEntity(mesh, tex, skel);

	auto& transform = character.GetComponent<TransformComponent>();
	transform.Position.x = getXPosition(clientID);
	transform.Rotation.y = 180.0f;

	auto& animator = character.GetComponent<AnimatorComponent>();
	Animation* idleAnim = GetCharacterAnimationFile(clientID, CharacterAnimationType::eIdle);
	Helpers::PlayAnimation(&animator, idleAnim);
}

float LobbyScene::getXPosition(int clientID)
{
	switch (clientID)
	{
	case 0:
		return WIDTH_BETWEEN_CHARACTERS;
		break;

	case 1:
		return -WIDTH_BETWEEN_CHARACTERS;
		break;

	case 2:
		return 0.0f;
		break;

	default:
		HB_LOG("Invalid client id!");
		return 0.0f;
		break;
	}
}

void LobbyScene::processAnswerNofifyLogin(const PACKET& packet)
{
	ANSWER_NOTIFY_LOGIN_PACKET* nfyPacket = reinterpret_cast<ANSWER_NOTIFY_LOGIN_PACKET*>(packet.DataPtr);
	createCharacterMesh(nfyPacket->ClientID);
}

void LobbyScene::processAnswerGameStart(const PACKET& packet)
{
	ANSWER_GAME_START_PACKET* gsPacket = reinterpret_cast<ANSWER_GAME_START_PACKET*>(packet.DataPtr);

	if (gsPacket->Result != ERROR_CODE::START_GAME)
	{
		HB_LOG("Unable to start game.");
		return;
	}

	mbChangeScene = true;
}
