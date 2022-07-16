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
	mOwner->SetBackgroundColor(Colors::Black);

	createUI();

	// Ŭ���̾�Ʈ ���̵� ���� ĳ���� ��ƼƼ ����
	createCharacter(mOwner->GetClientID(), mOwner->GetClientName());

	// ���� ī�޶� ��ġ ����
	auto& camera = mOwner->GetMainCamera();
	auto& cc = camera.GetComponent<CameraComponent>();
	cc.Position = Vector3{ 0.0f, 500.0f, -1000.0f };
}

void RoomScene::Exit()
{
	SoundManager::StopSound("LoginTheme.mp3");

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

void RoomScene::createUI()
{
	{
		// �� �׶��� �̹���
		Entity background = mOwner->CreateSpriteEntity(Application::GetScreenWidth(),
			227, TEXTURE("Room_Background.png"));
		auto& rect = background.GetComponent<RectTransformComponent>();
		rect.Position = Vector2{ 0.0f, Application::GetScreenHeight() - 227.0f };
	}

	{
		// �޽���
		Entity message = mOwner->CreateSpriteEntity(680, 46, TEXTURE("Room_Message.png"));
		auto& rect = message.GetComponent<RectTransformComponent>();
		rect.Position = Vector2{ 300.0f, 23.0f };
	}

	createPlayerUI(mOwner->GetClientID(), mOwner->GetClientName());
	createButtons();
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

void RoomScene::createPlayerUI(const int clientID, string_view name)
{
	float xPos = 0.0f;
	float xOffset = 0.0f;
	Texture* tagTex = nullptr;

	switch (clientID)
	{
	case 0: xPos = 590.0f; xOffset = 68.0f; tagTex = TEXTURE("P1.png"); break;
	case 1: xPos = 290.0f; xOffset = 38.0f; tagTex = TEXTURE("P2.png"); break;
	case 2: xPos = 890.0f; xOffset = 98.0f; tagTex = TEXTURE("P3.png"); break;
	default: HB_LOG("Unknown client id: {0}", clientID); break;
	}

	Entity playerTag = mOwner->CreateSpriteEntity(101, 73, tagTex);
	playerTag.AddTag<Tag_UI>();
	playerTag.AddComponent<NameComponent>(name);
	auto& tagRect = playerTag.GetComponent<RectTransformComponent>();
	tagRect.Position = Vector2{ xPos, 89.0f };

	Entity playerName = mOwner->CreateSpriteEntity(237, 84, TEXTURE("Player_Name.png"));
	playerName.AddTag<Tag_UI>();
	playerName.AddComponent<NameComponent>(name);
	auto& nameRect = playerName.GetComponent<RectTransformComponent>();
	nameRect.Position = Vector2{ xPos - xOffset, Application::GetScreenHeight() - 357.0f };

	Entity nameText = Entity{ gRegistry.create() };
	nameText.AddTag<Tag_UI>();
	nameText.AddComponent<NameComponent>(name);
	auto& text = nameText.AddComponent<TextComponent>();
	text.Sentence = s2ws(name.data());
	text.X = nameRect.Position.x + 30.0f;
	text.Y = Application::GetScreenHeight() - 334.0f;
	text.FontSize = 30;
}

void RoomScene::createButtons()
{
	// �� ������ ��ư ����
	Entity leave = mOwner->CreateSpriteEntity(173, 65, TEXTURE("Back_Button.png"), 110);
	auto& transform = leave.GetComponent<RectTransformComponent>();
	transform.Position = Vector2{ 76.0f, Application::GetScreenHeight() - 99.5f };

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
		Entity gameStartButton = mOwner->CreateSpriteEntity(416, 119, 
			TEXTURE("Start_Button.png"), 110);
		auto& transform = gameStartButton.GetComponent<RectTransformComponent>();
		transform.Position.x = 435.0f;
		transform.Position.y = Application::GetScreenHeight() - 162.0f;

		gameStartButton.AddComponent<ButtonComponent>([this]() {
			SoundManager::PlaySound("ButtonClick.mp3");
			REQUEST_ENTER_UPGRADE_PACKET packet = {};
			packet.PacketID = REQUEST_ENTER_UPGRADE;
			packet.PacketSize = sizeof(packet);
			mOwner->GetPacketManager()->Send(reinterpret_cast<char*>(&packet), sizeof(packet));
			});
	}
	else
	{
		Entity readyButton = mOwner->CreateSpriteEntity(416, 119,
			TEXTURE("Ready_Button.png"), 110);
		auto& transform = readyButton.GetComponent<RectTransformComponent>();
		transform.Position.x = 435.0f;
		transform.Position.y = Application::GetScreenHeight() - 162.0f;

		readyButton.AddComponent<ButtonComponent>([this]() {
			SoundManager::PlaySound("ButtonClick.mp3");
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
		auto player = GetEntityByID(clientID);
		auto playerName = player.GetComponent<NameComponent>().Name;
		DestroyEntity(player);

		auto uis = gRegistry.view<Tag_UI, NameComponent>();

		for (auto [ui, name] : uis.each())
		{
			if (name.Name == playerName)
			{
				DestroyEntity(ui);
			}
		}
	}
}

void RoomScene::processNotifyEnterRoom(const PACKET& packet)
{
	NOTIFY_ENTER_ROOM_PACKET* nerPacket = reinterpret_cast<NOTIFY_ENTER_ROOM_PACKET*>(packet.DataPtr);
	createCharacter(nerPacket->ClientID, nerPacket->UserName);
	createPlayerUI(nerPacket->ClientID, nerPacket->UserName);
}
