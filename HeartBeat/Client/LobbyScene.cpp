#include "ClientPCH.h"
#include "LobbyScene.h"

#include "Application.h"
#include "Client.h"
#include "PacketManager.h"
#include "ResourceManager.h"
#include "Components.h"
#include "RoomScene.h"
#include "Tags.h"
#include "SoundManager.h"
#include "Helpers.h"

LobbyScene::LobbyScene(Client* owner)
	: Scene(owner)
{

}

void LobbyScene::Enter()
{
	mOwner->SetBackgroundColor(Colors::CornflowerBlue);

	{
		auto cell = mOwner->CreateSkeletalMeshEntity(MESH("Cell.mesh"), TEXTURE("Cell_Yellow.png"),
			SKELETON("Cell.skel"));
		auto& transform = cell.GetComponent<TransformComponent>();
		transform.Position.x += 250.0f;
		transform.Position.y -= 700.0f;
		transform.Rotation.y = 90.0f;
		auto& animator = cell.GetComponent<AnimatorComponent>();
		Helpers::PlayAnimation(&animator, ANIM("Cell_Run.anim"));
		auto o2 = mOwner->CreateStaticMeshEntity(MESH("O2.mesh"), TEXTURE("O2.png"));
		Helpers::AttachBone(cell, o2, "Weapon");
	}

	{
		auto dog = mOwner->CreateSkeletalMeshEntity(MESH("Dog.mesh"), TEXTURE("Dog.png"),
			SKELETON("Dog.skel"));
		auto& transform = dog.GetComponent<TransformComponent>();
		transform.Position.x -= 250.0f;
		transform.Position.y -= 700.0f;
		transform.Rotation.y = 90.0f;
		auto& animator = dog.GetComponent<AnimatorComponent>();
		Helpers::PlayAnimation(&animator, ANIM("Dog_Run.anim"));
	}

	{
		auto virus = mOwner->CreateSkeletalMeshEntity(MESH("Virus.mesh"), TEXTURE("Virus.png"),
			SKELETON("Virus.skel"));
		auto& transform = virus.GetComponent<TransformComponent>();
		transform.Position.x -= 750.0f;
		transform.Position.y -= 700.0f;
		transform.Rotation.y = 90.0f;
		auto& animator = virus.GetComponent<AnimatorComponent>();
		Helpers::PlayAnimation(&animator, ANIM("Virus_Run.anim"));

		auto hammer = mOwner->CreateStaticMeshEntity(MESH("Hammer.mesh"), TEXTURE("Hammer.png"));
		Helpers::AttachBone(virus, hammer, "Weapon");
	}

	{
		auto character = mOwner->CreateSkeletalMeshEntity(MESH("Character_Red.mesh"), 
			TEXTURE("Character_Red.png"), SKELETON("Character_Red.skel"));
		auto& transform = character.GetComponent<TransformComponent>();
		transform.Position.x += 750.0f;
		transform.Position.y -= 700.0f;
		transform.Rotation.y = 90.0f;
		auto& animator = character.GetComponent<AnimatorComponent>();
		Helpers::PlayAnimation(&animator, ANIM("CR_Run.anim"));

		auto weapon = mOwner->CreateStaticMeshEntity(MESH("Syringe.mesh"), TEXTURE("Syringe.png"));
		Helpers::AttachBone(character, weapon, "Weapon");
	}

	// 메인 카메라 위치 조정
	auto& camera = mOwner->GetMainCamera();
	auto& cc = camera.GetComponent<CameraComponent>();
	cc.Position.y = 0.0f;
	cc.Position.z = -1000.0f;
}

void LobbyScene::Exit()
{
	DestroyExclude<Tag_DontDestroyOnLoad>();
}

void LobbyScene::ProcessInput()
{
	PACKET packet;
	while (mOwner->GetPacketManager()->GetPacket(packet))
	{
		switch (packet.PacketID)
		{
		case NOTIFY_ROOM:
			processNotifyRoom(packet);
			break;

		case ANSWER_ENTER_ROOM:
			processAnswerEnterRoom(packet);
			break;

		default:
			HB_LOG("Unknown packet id: {0}", packet.PacketID);
			break;
		}

		if (mbChangeScene)
		{
			mOwner->ChangeScene(new RoomScene{ mOwner });
			break;
		}
	}
}

void LobbyScene::RequestAvailableRoom()
{
	REQUEST_ROOM_PACKET packet = {};
	packet.PacketID = REQUEST_ROOM;
	packet.PacketSize = sizeof(packet);
	mOwner->GetPacketManager()->Send(reinterpret_cast<char*>(&packet), packet.PacketSize);
}

void LobbyScene::processNotifyRoom(const PACKET& packet)
{
	NOTIFY_ROOM_PACKET* nrPacket = reinterpret_cast<NOTIFY_ROOM_PACKET*>(packet.DataPtr);

	for (auto i = 0; i < MAX_ROOM_NUM; ++i)
	{
		if (AVAILABLE == nrPacket->Room[i])
		{
			createRoomSprite(i, true);
		}
		else if (CANNOT == nrPacket->Room[i])
		{
			createRoomSprite(i, false);
		}
		else
		{
			break;
		}
	}
}

void LobbyScene::processAnswerEnterRoom(const PACKET& packet)
{
	ANSWER_ENTER_ROOM_PACKET* aerPacket = reinterpret_cast<ANSWER_ENTER_ROOM_PACKET*>(packet.DataPtr);

	if (aerPacket->Result == RESULT_CODE::ROOM_ENTER_SUCCESS)
	{
		mbChangeScene = true;
		mOwner->SetClientID(aerPacket->ClientID);
	}
}

void LobbyScene::createRoomSprite(int index, bool canEnter /*= false*/)
{
	Texture* tex = TEXTURE("Bar.png");

	const int spriteWidth = 1000;
	const int spriteHeight = 100;

	int xPos = (Application::GetScreenWidth() - spriteWidth) / 2;
	int yPos = 100 + index * 125;

	Entity sprite = mOwner->CreateSpriteEntity(spriteWidth, spriteHeight, tex);
	auto& transform = sprite.GetComponent<RectTransformComponent>();
	transform.Position.x = static_cast<float>(xPos);
	transform.Position.y = static_cast<float>(yPos);

	if (canEnter)
	{
		sprite.AddComponent<ButtonComponent>([this, index, xPos, yPos]()
			{
				SoundManager::PlaySound("ButtonClick.mp3");
				REQUEST_ENTER_ROOM_PACKET packet = {};
				packet.PacketID = REQUEST_ENTER_ROOM;
				packet.PacketSize = sizeof(packet);
				packet.RoomNumber = index;
				mOwner->GetPacketManager()->Send(reinterpret_cast<char*>(&packet),
					sizeof(packet));
			});

		auto roomNumberText = Entity{ gRegistry.create() };
		auto& text = roomNumberText.AddComponent<TextComponent>();
		auto roomNumber = std::to_wstring(index + 1);
		text.Sentence = L"Room " + roomNumber;
		text.X = static_cast<float>(xPos + spriteWidth / 2 - 100);
		text.Y = static_cast<float>(yPos + 25);
	}
}
