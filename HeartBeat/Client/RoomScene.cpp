#include "ClientPCH.h"
#include "RoomScene.h"

#include "Client.h"
#include "PacketManager.h"

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
}

void RoomScene::ProcessInput()
{
	PACKET packet;
	while (mOwner->GetPacketManager()->GetPacket(packet))
	{
		switch (packet.PacketID)
		{
		default:
			HB_LOG("Unknown packet id: {0}", packet.PacketID);
			break;
		}

		if (mbChangeScene)
		{
			break;
		}
	}
}
