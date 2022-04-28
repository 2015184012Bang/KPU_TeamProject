#include "ClientPCH.h"
#include "GameScene.h"

#include "Client.h"
#include "Components.h"
#include "PacketManager.h"
#include "Input.h"
#include "Random.h"
#include "ResourceManager.h"

GameScene::GameScene(Client* owner)
	: Scene(owner)
{

}

void GameScene::Enter()
{
	// 내 캐릭터 알아두기
	auto entity = mOwner->GetEntityByID(mOwner->GetClientID());
	mPlayerCharacter = Entity(entity, mOwner);

	// 임시 바닥
	{
		Entity plane = mOwner->CreateStaticMeshEntity(MESH("Plane_Big.mesh"),
			TEXTURE("Brick.jpg"));

		auto& transform = plane.GetComponent<TransformComponent>();
		transform.Position.y -= 45.0f;
	}
}

void GameScene::Exit()
{

}

void GameScene::ProcessInput()
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
			HB_LOG("Unknown packet id: {0}", packet.PacketID);
			break;
		}

		if (mbChangeScene)
		{
			// TODO: 게임 종료 씬으로 전환
			break;
		}
	}
}

void GameScene::Update(float deltaTime)
{
	bool isKeyPressed = pollKeyboardPressed();
	bool isKeyReleased = pollKeyboardReleased();

	if (isKeyPressed || isKeyReleased)
	{
		REQUEST_MOVE_PACKET packet = {};
		packet.PacketID = REQUEST_MOVE;
		packet.PacketSize = sizeof(packet);
		packet.Direction = mDirection;
		mOwner->GetPacketManager()->Send(reinterpret_cast<char*>(&packet), sizeof(packet));
	}

	if (Input::IsButtonPressed(eKeyCode::A))
	{
		// TODO: 서버에 공격 요청 패킷을 보낸다
		auto& animator = mPlayerCharacter.GetComponent<AnimatorComponent>();
		animator.SetTrigger(GetRandomAttackAnimFile());
	}
}


bool GameScene::pollKeyboardPressed()
{
	bool bChanged = false;

	if (Input::IsButtonPressed(eKeyCode::Left))
	{
		mDirection.x -= 1.0f;
		bChanged = true;
	}

	if (Input::IsButtonPressed(eKeyCode::Right))
	{
		mDirection.x += 1.0f;
		bChanged = true;
	}

	if (Input::IsButtonPressed(eKeyCode::Up))
	{
		mDirection.z += 1.0f;
		bChanged = true;
	}

	if (Input::IsButtonPressed(eKeyCode::Down))
	{
		mDirection.z -= 1.0f;
		bChanged = true;
	}

	return bChanged;
}

bool GameScene::pollKeyboardReleased()
{
	bool bChanged = false;

	if (Input::IsButtonReleased(eKeyCode::Left))
	{
		mDirection.x += 1.0f;
		bChanged = true;
	}

	if (Input::IsButtonReleased(eKeyCode::Right))
	{
		mDirection.x -= 1.0f;
		bChanged = true;
	}

	if (Input::IsButtonReleased(eKeyCode::Up))
	{
		mDirection.z -= 1.0f;
		bChanged = true;
	}

	if (Input::IsButtonReleased(eKeyCode::Down))
	{
		mDirection.z += 1.0f;
		bChanged = true;
	}

	return bChanged;
}

void GameScene::processAnswerMove(const PACKET& packet)
{
	ANSWER_MOVE_PACKET* amPacket = reinterpret_cast<ANSWER_MOVE_PACKET*>(packet.DataPtr);

	auto& transform = mPlayerCharacter.GetComponent<TransformComponent>();
	transform.Position = amPacket->Position;

	auto& movement = mPlayerCharacter.GetComponent<MovementComponent>();
	movement.Direction = amPacket->Direction;
}

void GameScene::processNotifyMove(const PACKET& packet)
{
	NOTIFY_MOVE_PACKET* nmPacket = reinterpret_cast<NOTIFY_MOVE_PACKET*>(packet.DataPtr);

	auto entity = mOwner->GetEntityByID(nmPacket->EntityID);

	Entity target = { entity, mOwner };

	auto& transform = target.GetComponent<TransformComponent>();
	transform.Position = nmPacket->Position;

	auto& movement = target.GetComponent<MovementComponent>();
	movement.Direction = nmPacket->Direction;
}

string GetRandomAttackAnimFile()
{
	switch (Random::RandInt(1, 3))
	{
	case 1:
		return "Attack1";
		break;

	case 2:
		return "Attack2";
		break;

	case 3:
		return "Attack3";
		break;

	default:
		return "";
		break;
	}
}
