#include "ClientPCH.h"
#include "UpgradeScene.h"

#include "Client.h"
#include "Components.h"
#include "Character.h"
#include "PacketManager.h"
#include "Input.h"
#include "Helpers.h"
#include "Tags.h"

using namespace std::string_view_literals;

UpgradeScene::UpgradeScene(Client* owner)
	: Scene(owner)
{

}

void UpgradeScene::Enter()
{
	// 내 캐릭터 알아두기
	auto entity = mOwner->GetEntityByID(mOwner->GetClientID());
	mPlayerCharacter = Entity(entity, mOwner);

	initPlayersPositionToZero();
	createPlanes();
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

	if (Input::IsButtonPressed(eKeyCode::Space))
	{
		// 상호작용키(SPACE BAR)를 누르면 어떤 업그레이드 Plane 위에 서 있는지 확인한다.
		checkCollisionWithPlanes();
	}
}

void UpgradeScene::initPlayersPositionToZero()
{
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

bool UpgradeScene::pollKeyboardPressed()
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

bool UpgradeScene::pollKeyboardReleased()
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

void UpgradeScene::checkCollisionWithPlanes()
{
	const auto& playerBox = mPlayerCharacter.GetComponent<BoxComponent>().World;

	auto entities = mOwner->FindObjectsWithTag<Tag_Plane>();

	for (auto entity : entities)
	{
		Entity plane = { entity, mOwner };
		const auto& planeBox = plane.GetComponent<BoxComponent>().World;

		if (Helpers::Intersects(playerBox, planeBox))
		{
			auto& name = plane.GetComponent<NameComponent>().Name;
			HB_LOG(L"Collision with plane: {0}", name);
			break;
		}
	}
}

void UpgradeScene::processNotifyMove(const PACKET& packet)
{
	NOTIFY_MOVE_PACKET* nmPacket = reinterpret_cast<NOTIFY_MOVE_PACKET*>(packet.DataPtr);
	
	auto entity = mOwner->GetEntityByID(nmPacket->EntityID);

	Entity target = { entity, mOwner };

	auto& transform = target.GetComponent<TransformComponent>();
	transform.Position = nmPacket->Position;

	auto& movement = target.GetComponent<MovementComponent>();
	movement.Direction = nmPacket->Direction;
}

void UpgradeScene::processAnswerMove(const PACKET& packet)
{
	ANSWER_MOVE_PACKET* amPacket = reinterpret_cast<ANSWER_MOVE_PACKET*>(packet.DataPtr);

	auto& transform = mPlayerCharacter.GetComponent<TransformComponent>();
	transform.Position = amPacket->Position;

	auto& movement = mPlayerCharacter.GetComponent<MovementComponent>();
	movement.Direction = amPacket->Direction;
}

void UpgradeScene::createPlanes()
{
	// 공격 바닥 생성
	{
		Entity attackPlane = mOwner->CreateStaticMeshEntity(MESH(L"Plane.mesh"),
			TEXTURE(L"Attack.png"), L"../Assets/Meshes/Plane.mesh");

		auto& transform = attackPlane.GetComponent<TransformComponent>();
		transform.Position.x = -DISTANCE_BETWEEN_PLANE;

		attackPlane.AddTag<Tag_Plane>();
		attackPlane.AddComponent<NameComponent>(L"AttackPlane"sv);
	}

	// 힐 바닥 생성
	{
		Entity healPlane = mOwner->CreateStaticMeshEntity(MESH(L"Plane.mesh"),
			TEXTURE(L"Heal.png"), L"../Assets/Meshes/Plane.mesh");

		auto& transform = healPlane.GetComponent<TransformComponent>();

		healPlane.AddTag<Tag_Plane>();
		healPlane.AddComponent<NameComponent>(L"HealPlane"sv);
	}

	// 서포트 바닥 생성
	{
		Entity supportPlane = mOwner->CreateStaticMeshEntity(MESH(L"Plane.mesh"),
			TEXTURE(L"Support.png"), L"../Assets/Meshes/Plane.mesh");

		auto& transform = supportPlane.GetComponent<TransformComponent>();
		transform.Position.x = DISTANCE_BETWEEN_PLANE;

		supportPlane.AddTag<Tag_Plane>();
		supportPlane.AddComponent<NameComponent>(L"SupportPlane"sv);
	}

	// 바닥
	{
		Entity plane = mOwner->CreateStaticMeshEntity(MESH(L"Big_Plane.mesh"),
			TEXTURE(L"Brick.jpg"));

		auto& transform = plane.GetComponent<TransformComponent>();
		transform.Position.y -= 45.0f;
	}
}
