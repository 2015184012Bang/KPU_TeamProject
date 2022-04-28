#include "ClientPCH.h"
#include "LobbyScene.h"

#include "Application.h"
#include "Client.h"
#include "Character.h"
#include "Components.h"
#include "Helpers.h"
#include "Input.h"
#include "ResourceManager.h"
#include "PacketManager.h"
#include "Utils.h"
#include "UpgradeScene.h"
#include "Tags.h"


LobbyScene::LobbyScene(Client* owner)
	: Scene(owner)
{

}

void LobbyScene::Enter()
{
	int myClientID = mOwner->GetClientID();

	// Ŭ���̾�Ʈ�� ���� ȣ��Ʈ���
	// ���� ��ư�� ���� �� �ֵ��� ��ư�� �����Ѵ�.
	if (HOST_ID == myClientID)
	{
		Entity gameStartButton = mOwner->CreateSpriteEntity(START_BUTTON_WIDTH, START_BUTTON_HEIGHT, TEXTURE("Start_Button.png"));
		auto& transform = gameStartButton.GetComponent<RectTransformComponent>();
		transform.Position.x = (Application::GetScreenWidth() / 2.0f) - (transform.Width / 2.0f);
		transform.Position.y = Application::GetScreenHeight() - START_BUTTON_DIST_FROM_BOTTOM;

		gameStartButton.AddComponent<ButtonComponent>([this]() {
			REQUEST_ENTER_UPGRADE_PACKET packet = {};
			packet.PacketID = REQUEST_ENTER_UPGRADE;
			packet.PacketSize = sizeof(packet);
			mOwner->GetPacketManager()->Send(reinterpret_cast<char*>(&packet), sizeof(packet));
			});
	}

	createCharacterMesh(myClientID);

	// ���� ī�޶� ��ġ ����
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

	while (mOwner->GetPacketManager()->GetPacket(packet))
	{
		switch (packet.PacketID)
		{
		case NOTIFY_LOGIN:
			processNofifyLogin(packet);
			break;

		case NOTIFY_ENTER_UPGRADE:
			processNotifyEnterUpgrade(packet);
			break;
			
		default:
			HB_LOG("Unknown packet type: {0}", packet.PacketID);
			break;
		}

		if (mbChangeScene)
		{
			mOwner->ChangeScene(new UpgradeScene(mOwner));
			break;
		}
	}
}

void LobbyScene::createCharacterMesh(int clientID)
{
	auto [mesh, tex, skel] = GetCharacterFiles(clientID);

	Entity character = mOwner->CreateSkeletalMeshEntity(mesh, tex, skel, "../Assets/Boxes/Character.box");

	// �� ���濡�� �������� �ʵ��� DontDestroyOnLoad �±׸� �ٿ��д�.
	character.AddTag<Tag_DontDestroyOnLoad>();

	character.AddTag<Tag_Player>();

	// ������ ����ȭ�� �ʿ��� ��ü���� ID�� �ο��Ѵ�.
	// ex. �÷��̾� ĳ����, ��, NPC... ��
	// �÷��̾� ĳ������ ���̵�� ������ Ŭ���̾�Ʈ ���̵� �����Ѵ�.
	character.AddComponent<IDComponent>(clientID);
	character.AddComponent<MovementComponent>(PLAYER_MAX_SPEED);

	// [����] Character ��ũ��Ʈ�� MovementComponent ���� �ְ� �����ؾ� �Ѵ�.
	character.AddComponent<ScriptComponent>(std::make_shared<Character>(character));

	auto& transform = character.GetComponent<TransformComponent>();
	transform.Position.x = getXPosition(clientID);
	transform.Rotation.y = 180.0f;

	auto& animator = character.GetComponent<AnimatorComponent>();

	// �κ� �������� �ƹ��͵� ��� ���� �����Ƿ� IdleNone �ִϸ��̼� ���.
	Animation* idleAnim = GetCharacterAnimationFile(clientID, CharacterAnimationType::IDLE_NONE);
	Helpers::PlayAnimation(&animator, idleAnim);

	// ĳ���Ϳ��� ��Ʈ ����
	auto [beltMesh, beltTex] = getCharacterBelt(clientID);
	Entity belt = mOwner->CreateStaticMeshEntity(beltMesh, beltTex);

	// ���� �Ѿ�鼭 ��Ʈ�� �������� �ʵ��� �±׸� �ٿ��д�.
	belt.AddTag<Tag_DontDestroyOnLoad>();
	Helpers::AttachBone(character, belt, "Bip001 Spine");
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

void LobbyScene::processNofifyLogin(const PACKET& packet)
{
	NOTIFY_LOGIN_PACKET* nfyPacket = reinterpret_cast<NOTIFY_LOGIN_PACKET*>(packet.DataPtr);
	createCharacterMesh(nfyPacket->ClientID);
}

void LobbyScene::processNotifyEnterUpgrade(const PACKET& packet)
{
	NOTIFY_ENTER_UPGRADE_PACKET* neuPacket = reinterpret_cast<NOTIFY_ENTER_UPGRADE_PACKET*>(packet.DataPtr);

	if (neuPacket->Result != ERROR_CODE::SUCCESS)
	{
		HB_LOG("Unable to enter upgrade scene.");
		return;
	}

	mbChangeScene = true;
}

std::tuple<Mesh*, Texture*> LobbyScene::getCharacterBelt(int clientID)
{
	switch (clientID)
	{
	case 0:
		return { MESH("Belt_Green.mesh"), TEXTURE("Belt_Green.png") };
		break;

	case 1:
		return { MESH("Belt_Pink.mesh"), TEXTURE("Belt_Pink.png") };
		break;

	case 2:
		return { MESH("Belt_Red.mesh"), TEXTURE("Belt_Red.png") };
		break;
	}

	return { nullptr, nullptr };
}
