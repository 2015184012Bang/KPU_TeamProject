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

	// 방 나가기 버튼, 게임 시작 버튼 생성
	createButtons();

	// 클라이언트 아이디에 따른 캐릭터 엔티티 생성
	createCharacter(mOwner->GetClientID(), mOwner->GetClientName());

	// 메인 카메라 위치 조정
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

	// 씬 변경에도 삭제되지 않도록 DontDestroyOnLoad 태그를 붙여둔다.
	character.AddTag<Tag_DontDestroyOnLoad>();

	character.AddTag<Tag_Player>();

	// 서버와 동기화가 필요한 객체들은 ID를 부여한다.
	// ex. 플레이어 캐릭터, 적, NPC... 등
	// 플레이어 캐릭터의 아이디는 본인의 클라이언트 아이디를 대입한다.
	character.AddComponent<IDComponent>(clientID);
	character.AddComponent<MovementComponent>(Values::PlayerSpeed);
	character.AddComponent<ScriptComponent>(std::make_shared<Character>(character));
	character.AddComponent<NameComponent>(userName);

	auto& transform = character.GetComponent<TransformComponent>();
	transform.Position.x = getXPosition(clientID);
	transform.Rotation.y = 180.0f;

	auto& animator = character.GetComponent<AnimatorComponent>();

	// 룸 씬에서는 아무것도 들고 있지 않으므로 IdleNone 애니메이션 재생.
	Animation* idleAnim = GetCharacterAnimationFile(clientID, CharacterAnimationType::IDLE_NONE);
	Helpers::PlayAnimation(&animator, idleAnim);

	// 캐릭터에게 벨트 장착
	auto [beltMesh, beltTex] = getCharacterBelt(clientID);
	HB_ASSERT(beltMesh && beltTex, "Belt resource is nullptr!");
	Entity belt = mOwner->CreateStaticMeshEntity(beltMesh, beltTex);

	// 씬을 넘어가면서 벨트가 삭제되지 않도록 태그를 붙여둔다.
	belt.AddTag<Tag_DontDestroyOnLoad>();
	Helpers::AttachBone(character, belt, "Bip001 Spine");
}

void RoomScene::createButtons()
{
	// 방 나가기 버튼 생성
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

	// 클라이언트가 만일 호스트라면
	// 시작 버튼을 누를 수 있도록 버튼을 생성한다.
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
	//		  클라이언트 ID에 따른 x 위치
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
	// 플레이어 엔티티는 DontDestroyOnLoad 태그가 붙어 있어서
	// Scene Exit할 때 삭제되지 않기 때문에 직접 삭제해준다.
	DestroyByComponent<Tag_Player>();
	mOwner->ChangeScene(new LobbyScene{ mOwner });
}

void RoomScene::processNotifyLeaveRoom(const PACKET& packet)
{
	NOTIFY_LEAVE_ROOM_PACKET* nlrPacket = reinterpret_cast<NOTIFY_LEAVE_ROOM_PACKET*>(packet.DataPtr);

	auto clientID = nlrPacket->ClientID;

	// 내 LEAVE 요청에 대한 서버의 응답이라면
	// RoomScene으로 전환한다.
	if (mOwner->GetClientID() == clientID)
	{
		mbToRoom = true;
	}
	else
	{
		// 플레이어 캐릭터는 클라이언트 아이디가 곧 엔티티 아이디이다.
		DestroyEntityByID(clientID);
	}
}

void RoomScene::processNotifyEnterRoom(const PACKET& packet)
{
	NOTIFY_ENTER_ROOM_PACKET* nerPacket = reinterpret_cast<NOTIFY_ENTER_ROOM_PACKET*>(packet.DataPtr);
	createCharacter(nerPacket->ClientID, nerPacket->UserName);
}
