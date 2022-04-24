#include "ClientPCH.h"
#include "UpgradeScene.h"

#include "Client.h"
#include "Components.h"
#include "Character.h"
#include "PacketManager.h"

UpgradeScene::UpgradeScene(Client* owner)
	: Scene(owner)
{

}

void UpgradeScene::Enter()
{
	auto entity = mOwner->GetEntityByID(mOwner->GetClientID());
	mPlayerCharacter = Entity(entity, mOwner);
	mPlayerCharacter.AddComponent<ScriptComponent>(std::make_shared<Character>(mPlayerCharacter));
}

void UpgradeScene::Exit()
{
	mOwner->DestroyAll();
}

void UpgradeScene::ProcessInput()
{
	PACKET packet;
	while (mOwner->GetPacketManager()->GetPacket(packet))
	{
		switch (packet.PacketID)
		{
		}
	}
}

void UpgradeScene::Update(float deltaTime)
{

}
