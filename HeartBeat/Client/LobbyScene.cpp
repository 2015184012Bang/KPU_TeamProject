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
		Entity background = mOwner->CreateSpriteEntity(Application::GetScreenWidth(),
			Application::GetScreenHeight(), TEXTURE("Lobby_Background.png"));
	}
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
			createRoomSprite(i, nrPacket->NumUsers[i], true);
		}
		else if (CANNOT == nrPacket->Room[i])
		{
			createRoomSprite(i, nrPacket->NumUsers[i], false);
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

void LobbyScene::createRoomSprite(int index, int numUsers, bool canEnter /*= false*/)
{
	const int buttonWidth = 800;
	const int buttonHeight = 150;

	float buttonXPos = (Application::GetScreenWidth() - buttonWidth) / 2.0f;
	float buttonYPos = 160.0f + index * 180.0f;

	Texture* buttonTex = TEXTURE("Lobby_Button.png");
	Entity sprite = mOwner->CreateSpriteEntity(buttonWidth, buttonHeight, buttonTex, 110);
	auto& transform = sprite.GetComponent<RectTransformComponent>();
	transform.Position.x = buttonXPos;
	transform.Position.y = buttonYPos;

	Texture* roomState = nullptr;

	if (canEnter)
	{
		roomState = numUsers == 0 ? TEXTURE("Empty.png") : TEXTURE("Waiting.png");

		sprite.AddComponent<ButtonComponent>([this, index, buttonXPos, buttonYPos]()
			{
				SoundManager::PlaySound("ButtonClick.mp3");
				REQUEST_ENTER_ROOM_PACKET packet = {};
				packet.PacketID = REQUEST_ENTER_ROOM;
				packet.PacketSize = sizeof(packet);
				packet.RoomNumber = index;
				mOwner->GetPacketManager()->Send(reinterpret_cast<char*>(&packet),
					sizeof(packet));
			});
	}
	else
	{
		roomState = TEXTURE("Playing.png");
	}

	{
		// 방 상태(Empty, Waiting, Playing)
		Entity stateSprite = mOwner->CreateSpriteEntity(250, 50, roomState, 120);
		auto& stateRect = stateSprite.GetComponent<RectTransformComponent>();
		stateRect.Position.x = buttonXPos + 200;
		stateRect.Position.y = buttonYPos + 80;
	}

	{
		// 방 번호
		auto roomNumberText = Entity{ gRegistry.create() };
		auto& text = roomNumberText.AddComponent<TextComponent>();
		auto roomNumber = std::to_wstring(index + 1);
		text.Sentence = roomNumber;
		text.X = buttonXPos + 200.0f;
		text.Y = buttonYPos + 25.0f;
	}

	{
		// 접속 유저 수 
		auto userNumberText = Entity{ gRegistry.create() };
		auto& text = userNumberText.AddComponent<TextComponent>();
		text.Sentence = std::to_wstring(numUsers) + L"/3";
		text.X = buttonXPos + 600;
		text.Y = buttonYPos + 100.0f;
	}
}
