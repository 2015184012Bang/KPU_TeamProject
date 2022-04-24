#include "ClientPCH.h"
#include "UpgradeScene.h"

#include "Client.h"
#include "Components.h"
#include "Character.h"
#include "PacketManager.h"
#include "Input.h"
#include "Helpers.h"
#include "Tags.h"

UpgradeScene::UpgradeScene(Client* owner)
	: Scene(owner)
{

}

void UpgradeScene::Enter()
{
	// 내 캐릭터 알아두기
	auto entity = mOwner->GetEntityByID(mOwner->GetClientID());
	mPlayerCharacter = Entity(entity, mOwner);

	// 로비씬 때 달랐던 플레이어들의 위치를 (0, 0, 0)으로 통일해주기
	auto entities = mOwner->FindObjectsWithTag<Tag_Player>();
	if (entities.empty())
	{
		HB_LOG("No Player exists.");
		return;
	}

	for (auto eid : entities)
	{
		Entity character = { eid, mOwner };
		auto& transform = character.GetComponent<TransformComponent>();
		Helpers::UpdatePosition(&transform.Position, Vector3::Zero, &transform.bDirty);
	}
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
		case ANSWER_MOVE:
			processAnswerMove(packet);
			break;

		case NOTIFY_MOVE:
			processNotifyMove(packet);
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

void UpgradeScene::processNotifyMove(const PACKET& packet)
{
	NOTIFY_MOVE_PACKET* nmPacket = reinterpret_cast<NOTIFY_MOVE_PACKET*>(packet.DataPtr);
	
	auto entity = mOwner->GetEntityByID(nmPacket->EntityID);

	Entity target = { entity, mOwner };

	auto& movement = target.GetComponent<MovementComponent>();
	movement.Direction = nmPacket->Direction;
}

void UpgradeScene::processAnswerMove(const PACKET& packet)
{
	ANSWER_MOVE_PACKET* amPacket = reinterpret_cast<ANSWER_MOVE_PACKET*>(packet.DataPtr);

	auto& transform = mPlayerCharacter.GetComponent<TransformComponent>();
	transform.Position = amPacket->Position;
}
