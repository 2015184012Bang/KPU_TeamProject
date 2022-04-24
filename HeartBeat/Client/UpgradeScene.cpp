#include "ClientPCH.h"
#include "UpgradeScene.h"

#include "Client.h"
#include "Components.h"
#include "Character.h"
#include "PacketManager.h"
#include "Input.h"

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
		case ANSWER_NOTIFY_MOVE:
			processAnswerNotifyMove(packet);
			break;

		default:
			break;
		}
	}
}

void UpgradeScene::Update(float deltaTime)
{
	if (pollKeyboardPressed() || pollKeyboardReleased())
	{
		REQUEST_MOVE_PACKET packet = {};
		packet.PacketID = REQUEST_MOVE;
		packet.PacketSize = sizeof(packet);
		packet.Direction = mPlayerCharacter.GetComponent<MovementComponent>().Direction;
		mOwner->GetPacketManager()->Send(reinterpret_cast<char*>(&packet), sizeof(packet));
	}
}

bool UpgradeScene::pollKeyboardPressed()
{
	Vector3& direction = mPlayerCharacter.GetComponent<MovementComponent>().Direction;
	bool bChanged = false;

	if (Input::IsButtonPressed(eKeyCode::Left))
	{
		direction.x = -1.0f;
		bChanged = true;
	}

	if (Input::IsButtonPressed(eKeyCode::Right))
	{
		direction.x = 1.0f;
		bChanged = true;
	}

	if (Input::IsButtonPressed(eKeyCode::Up))
	{
		direction.z = 1.0f;
		bChanged = true;
	}

	if (Input::IsButtonPressed(eKeyCode::Down))
	{
		direction.z = -1.0f;
		bChanged = true;
	}

	return bChanged;
}

bool UpgradeScene::pollKeyboardReleased()
{
	Vector3& direction = mPlayerCharacter.GetComponent<MovementComponent>().Direction;
	bool bChanged = false;

	if (Input::IsButtonReleased(eKeyCode::Left))
	{
		direction.x += 1.0f;
		bChanged = true;
	}

	if (Input::IsButtonReleased(eKeyCode::Right))
	{
		direction.x -= 1.0f;
		bChanged = true;
	}

	if (Input::IsButtonReleased(eKeyCode::Up))
	{
		direction.z -= 1.0f;
		bChanged = true;
	}

	if (Input::IsButtonReleased(eKeyCode::Down))
	{
		direction.z += 1.0f;
		bChanged = true;
	}

	return bChanged;
}

void UpgradeScene::processAnswerNotifyMove(const PACKET& packet)
{
	ANSWER_NOTIFY_MOVE_PACKET* nmPacket = reinterpret_cast<ANSWER_NOTIFY_MOVE_PACKET*>(packet.DataPtr);
	
	auto entity = mOwner->GetEntityByID(nmPacket->EntityID);
	Entity target = { entity, mOwner };

	auto& movement = target.GetComponent<MovementComponent>();
	movement.Direction = nmPacket->Direction;
}
