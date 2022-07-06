#include "ClientPCH.h"
#include "RoomScene.h"

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
#include "Define.h"
#include "LobbyScene.h"
#include "SoundManager.h"

constexpr float WIDTH_BETWEEN_CHARACTERS = 700.0f;

RoomScene::RoomScene(Client* owner)
	: Scene(owner)
{

}

void RoomScene::Enter()
{
	mOwner->SetBackgroundColor(Colors::CornflowerBlue);

	// �� ������ ��ư, ���� ���� ��ư ����
	createButtons();

	// Ŭ���̾�Ʈ ���̵� ���� ĳ���� ��ƼƼ ����
	createCharacter(mOwner->GetClientID(), mOwner->GetClientName());

	// ���� ī�޶� ��ġ ����
	auto& camera = mOwner->GetMainCamera();
	auto& cc = camera.GetComponent<CameraComponent>();
	cc.Position.y = 500.0f;
	cc.Position.z = -1000.0f;
}

void RoomScene::Exit()
{
	DestroyExclude<Tag_DontDestroyOnLoad>();
}

void RoomScene::ProcessInput()
{
	PACKET packet;

	while (mOwner->GetPacketManager()->GetPacket(packet))
	{
		switch (packet.PacketID)
		{
		case NOTIFY_LEAVE_ROOM:
			processNotifyLeaveRoom(packet);
			break;

		case NOTIFY_ENTER_ROOM:
			processNotifyEnterRoom(packet);
			break;

		case NOTIFY_ENTER_UPGRADE:
			processNotifyEnterUpgrade(packet);
			break;

		default:
			HB_LOG("Unknown packet type: {0}", packet.PacketID);
			break;
		}

		if (mbToUpgrade)
		{
			mOwner->ChangeScene(new UpgradeScene{ mOwner });
			break;
		}

		if (mbToRoom)
		{
			doWhenChangeToLobbyScene();
			break;
		}
	}
}

void RoomScene::createCharacter(int clientID, string_view userName)
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
	character.AddComponent<MovementComponent>(Values::PlayerSpeed);
	character.AddComponent<ScriptComponent>(std::make_shared<Character>(character));
	character.AddComponent<NameComponent>(userName);

	auto& transform = character.GetComponent<TransformComponent>();
	transform.Position.x = getXPosition(clientID);
	transform.Rotation.y = 180.0f;

	auto& animator = character.GetComponent<AnimatorComponent>();

	// �� �������� �ƹ��͵� ��� ���� �����Ƿ� IdleNone �ִϸ��̼� ���.
	Animation* idleAnim = GetCharacterAnimationFile(clientID, CharacterAnimationType::IDLE_NONE);
	Helpers::PlayAnimation(&animator, idleAnim);

	// ĳ���Ϳ��� ��Ʈ ����
	auto [beltMesh, beltTex] = getCharacterBelt(clientID);
	HB_ASSERT(beltMesh && beltTex, "Belt resource is nullptr!");
	Entity belt = mOwner->CreateStaticMeshEntity(beltMesh, beltTex);

	// ���� �Ѿ�鼭 ��Ʈ�� �������� �ʵ��� �±׸� �ٿ��д�.
	belt.AddTag<Tag_DontDestroyOnLoad>();
	Helpers::AttachBone(character, belt, "Bip001 Spine");
}

void RoomScene::createButtons()
{
	// �� ������ ��ư ����
	Entity leave = mOwner->CreateSpriteEntity(239, 78, TEXTURE("Back_Button.png"));
	auto& transform = leave.GetComponent<RectTransformComponent>();
	transform.Position = Vector2{ 20.0f, Application::GetScreenHeight() - 120.0f };

	leave.AddComponent<ButtonComponent>([this]()
		{
			SoundManager::PlaySound("ButtonClick.mp3");
			REQUEST_LEAVE_ROOM_PACKET packet = {};
			packet.PacketID = REQUEST_LEAVE_ROOM;
			packet.PacketSize = sizeof(packet);
			mOwner->GetPacketManager()->Send(reinterpret_cast<char*>(&packet),
				sizeof(packet));
		});

	// Ŭ���̾�Ʈ�� ���� ȣ��Ʈ���
	// ���� ��ư�� ���� �� �ֵ��� ��ư�� �����Ѵ�.
	if (Values::HostID == mOwner->GetClientID())
	{
		Entity gameStartButton = mOwner->CreateSpriteEntity(302, 85, TEXTURE("Start_Button.png"));
		auto& transform = gameStartButton.GetComponent<RectTransformComponent>();
		transform.Position.x = (Application::GetScreenWidth() / 2.0f) - (transform.Width / 2.0f);
		transform.Position.y = Application::GetScreenHeight() - 150.f;

		gameStartButton.AddComponent<ButtonComponent>([this]() {
			SoundManager::PlaySound("ButtonClick.mp3");
			REQUEST_ENTER_UPGRADE_PACKET packet = {};
			packet.PacketID = REQUEST_ENTER_UPGRADE;
			packet.PacketSize = sizeof(packet);
			mOwner->GetPacketManager()->Send(reinterpret_cast<char*>(&packet), sizeof(packet));
			});
	}
}

float RoomScene::getXPosition(int clientID)
{
	//		  Ŭ���̾�Ʈ ID�� ���� x ��ġ
	//     1(-WIDTH)     0(0)      2(WIDTH)
	switch (clientID)
	{
	case 0:
		return 0.0f;
		break;

	case 1:
		return -WIDTH_BETWEEN_CHARACTERS;
		break;

	case 2:
		return WIDTH_BETWEEN_CHARACTERS;
		break;

	default:
		HB_LOG("Invalid client id!");
		return 0.0f;
		break;
	}
}

void RoomScene::processNotifyEnterUpgrade(const PACKET& packet)
{
	NOTIFY_ENTER_UPGRADE_PACKET* neuPacket = reinterpret_cast<NOTIFY_ENTER_UPGRADE_PACKET*>(packet.DataPtr);

	if (neuPacket->Result != RESULT_CODE::SUCCESS)
	{
		HB_LOG("Unable to enter upgrade scene.");
		return;
	}

	mbToUpgrade = true;
}

std::tuple<Mesh*, Texture*> RoomScene::getCharacterBelt(int clientID)
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

void RoomScene::doWhenChangeToLobbyScene()
{
	// �÷��̾� ��ƼƼ�� DontDestroyOnLoad �±װ� �پ� �־
	// Scene Exit�� �� �������� �ʱ� ������ ���� �������ش�.
	DestroyByComponent<Tag_Player>();
	mOwner->ChangeScene(new LobbyScene{ mOwner });
}

void RoomScene::processNotifyLeaveRoom(const PACKET& packet)
{
	NOTIFY_LEAVE_ROOM_PACKET* nlrPacket = reinterpret_cast<NOTIFY_LEAVE_ROOM_PACKET*>(packet.DataPtr);

	auto clientID = nlrPacket->ClientID;

	// �� LEAVE ��û�� ���� ������ �����̶��
	// RoomScene���� ��ȯ�Ѵ�.
	if (mOwner->GetClientID() == clientID)
	{
		mbToRoom = true;
	}
	else
	{
		// �÷��̾� ĳ���ʹ� Ŭ���̾�Ʈ ���̵� �� ��ƼƼ ���̵��̴�.
		DestroyEntityByID(clientID);
	}
}

void RoomScene::processNotifyEnterRoom(const PACKET& packet)
{
	NOTIFY_ENTER_ROOM_PACKET* nerPacket = reinterpret_cast<NOTIFY_ENTER_ROOM_PACKET*>(packet.DataPtr);
	createCharacter(nerPacket->ClientID, nerPacket->UserName);
}
