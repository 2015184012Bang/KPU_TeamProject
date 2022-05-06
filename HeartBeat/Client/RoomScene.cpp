#include "ClientPCH.h"
#include "RoomScene.h"

#include "Application.h"
#include "Client.h"
#include "PacketManager.h"
#include "ResourceManager.h"
#include "Components.h"
#include "LobbyScene.h"

RoomScene::RoomScene(Client* owner)
	: Scene(owner)
{

}

void RoomScene::Enter()
{
	HB_LOG("Entered RoomScene...");
}

void RoomScene::Exit()
{
	HB_LOG("Exited RoomScene...");
	DestroyAll();
}

void RoomScene::ProcessInput()
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
			processEnterRoom(packet);
			break;

		default:
			HB_LOG("Unknown packet id: {0}", packet.PacketID);
			break;
		}

		if (mbChangeScene)
		{
			mOwner->ChangeScene(new LobbyScene{ mOwner });
			break;
		}
	}
}

void RoomScene::processNotifyRoom(const PACKET& packet)
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

void RoomScene::processEnterRoom(const PACKET& packet)
{
	ANSWER_ENTER_ROOM_PACKET* aerPacket = reinterpret_cast<ANSWER_ENTER_ROOM_PACKET*>(packet.DataPtr);

	if (aerPacket->Result == RESULT_CODE::ROOM_ENTER_SUCCESS)
	{
		mbChangeScene = true;
	}
}

void RoomScene::createRoomSprite(int index, bool canEnter /*= false*/)
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
