#include "ClientPCH.h"
#include "LobbyScene.h"

#include "Application.h"
#include "Client.h"
#include "PacketManager.h"
#include "ResourceManager.h"
#include "Components.h"
#include "RoomScene.h"
#include "Tags.h"

LobbyScene::LobbyScene(Client* owner)
	: Scene(owner)
{

}

void LobbyScene::Enter()
{
	HB_LOG("Entered LobbyScene...");
}

void LobbyScene::Exit()
{
	HB_LOG("Exited LobbyScene...");
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
	Texture* tex = canEnter ? TEXTURE("RoomEnter.png") : TEXTURE("RoomNoEnter.png");

	const int spriteWidth = 100;
	const int spriteHeight = 100;

	int xPos = 50 + 120 * index;
	int yPos = Application::GetScreenHeight() / 2 - spriteHeight / 2;

	Entity sprite = mOwner->CreateSpriteEntity(spriteWidth, spriteHeight, tex);
	auto& transform = sprite.GetComponent<RectTransformComponent>();
	transform.Position.x = static_cast<float>(xPos);
	transform.Position.y = static_cast<float>(yPos);

	if (canEnter)
	{
		sprite.AddComponent<ButtonComponent>([this, index]()
			{
				REQUEST_ENTER_ROOM_PACKET packet = {};
				packet.PacketID = REQUEST_ENTER_ROOM;
				packet.PacketSize = sizeof(packet);
				packet.RoomNumber = index;
				mOwner->GetPacketManager()->Send(reinterpret_cast<char*>(&packet),
					sizeof(packet));
			});
	}
}
