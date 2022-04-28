#include "ClientPCH.h"
#include "UpgradeScene.h"

#include "Client.h"
#include "Components.h"
#include "Character.h"
#include "GameScene.h"
#include "PacketManager.h"
#include "Input.h"
#include "Helpers.h"
#include "Tags.h"
#include "SoundManager.h"

using namespace std::string_view_literals;

UpgradeScene::UpgradeScene(Client* owner)
	: Scene(owner)
{

}

void UpgradeScene::Enter()
{
	// Bgm ���
	SoundManager::PlaySound("ClockTick.mp3");

	// �� ĳ���� �˾Ƶα�
	auto entity = mOwner->GetEntityByID(mOwner->GetClientID());
	mPlayerCharacter = Entity(entity, mOwner);

	mOwner->SetFollowCameraTarget(mPlayerCharacter, Vector3{0.0f, 750.0f, -750.0f});

	initPlayersPositionToZero();

	// �ٴ�, ����/��/����Ʈ �ٴ� ����
	createPlanes();

	// ������ �÷��̾ ���� �ٴϴ� �ð� ����
	createClock();
}

void UpgradeScene::Exit()
{
	// ���� �ȵ� �Ҹ��� ������ ��� ���̶�� �����.
	SoundManager::StopSound("ClockTick.mp3");

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

		case NOTIFY_UPGRADE:
			processNotifyUpgrade(packet);
			break;

		case NOTIFY_ENTER_GAME:
			processNotifyEnterGame(packet);
			break;

		default:
			HB_LOG("Unknown packet id: {0}", packet.PacketID);
			break;
		}

		if (mbChangeScene)
		{
			auto newScene = new GameScene(mOwner);
			newScene->SetDirection(mDirection);
			mOwner->ChangeScene(newScene);
			break;
		}
	}
}

void UpgradeScene::Update(float deltaTime)
{
	mElapsed += deltaTime;

	// ���� �� 5�ʰ� �Ǹ� ī��Ʈ�ٿ��� �����Ѵ�.
	if (mElapsed > FIVE_SECS_BEFORE_START && !mbCountdownPlayed)
	{
		mbCountdownPlayed = true;
		startCountdown();
	}

	// ������ �ð��� �Ǹ� ������ ����.
	// ������ ��ǥ�� ������ ��Ŷ �۽�
	if (mElapsed > SECS_TO_START)
	{
		REQUEST_ENTER_GAME_PACKET packet = {};
		packet.PacketID = REQUEST_ENTER_GAME;
		packet.PacketSize = sizeof(packet);
		mOwner->GetPacketManager()->Send(reinterpret_cast<char*>(&packet), sizeof(packet));
	}

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
		// ��ȣ�ۿ�Ű(SPACE BAR)�� ������ � ���׷��̵� Plane ���� �� �ִ��� Ȯ���Ѵ�.
		checkCollisionWithPlanes();
	}

	updateClockPosition();
}

void UpgradeScene::initPlayersPositionToZero()
{
	// �κ�� �� �޶��� �÷��̾���� ��ġ�� (0, 0, 0)���� �������ֱ�
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
			
			REQUEST_UPGRADE_PACKET packet = {};
			packet.PacketID = REQUEST_UPGRADE;
			packet.PacketSize = sizeof(packet);
			packet.UpgradePreset = getPresetNumber(name);

			mOwner->GetPacketManager()->Send(reinterpret_cast<char*>(&packet),
				sizeof(packet));

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

void UpgradeScene::processNotifyUpgrade(const PACKET& packet)
{
	NOTIFY_UPGRADE_PACKET* nuPacket = reinterpret_cast<NOTIFY_UPGRADE_PACKET*>(packet.DataPtr);

	auto entity = mOwner->GetEntityByID(nuPacket->EntityID);
	Entity target = { entity, mOwner };
	
	equipPresetToCharacter(target, static_cast<UpgradePreset>(nuPacket->UpgradePreset));
}


void UpgradeScene::processNotifyEnterGame(const PACKET& packet)
{
	NOTIFY_ENTER_GAME_PACKET* nmgPacket = reinterpret_cast<NOTIFY_ENTER_GAME_PACKET*>(packet.DataPtr);

	if (nmgPacket->Result != ERROR_CODE::SUCCESS)
	{
		HB_LOG("Unable to enter game scene.");
		return;
	}

	mbChangeScene = true;
}

uint8 UpgradeScene::getPresetNumber(string_view planeName)
{
	if (planeName == "AttackPlane")
	{
		return static_cast<uint8>(UpgradePreset::ATTACK);
	}
	else if (planeName == "HealPlane")
	{
		return static_cast<uint8>(UpgradePreset::HEAL);
	}
	else
	{
		return static_cast<uint8>(UpgradePreset::SUPPORT);
	}
}

void UpgradeScene::equipPresetToCharacter(Entity& target, UpgradePreset preset)
{
	// ������ �ٿ����� ��ƼƼ���� �����Ѵ�.
	auto entities = Helpers::GetEntityToDetach(target);
	for (auto entity : entities)
	{
		mOwner->DestroyEntity(entity);
	}

	switch (preset)
	{
	case UpgradePreset::ATTACK:
	{
		Entity weapon = mOwner->CreateStaticMeshEntity(MESH("Syringe.mesh"),
			TEXTURE("Temp.png"));
		Entity bag = mOwner->CreateSkeletalMeshEntity(MESH("Bag.mesh"),
			TEXTURE("Temp.png"), SKELETON("Bag.skel"));
		Entity sup = mOwner->CreateStaticMeshEntity(MESH("Pill.mesh"),
			TEXTURE("Temp.png"));

		weapon.AddTag<Tag_DontDestroyOnLoad>();
		bag.AddTag<Tag_DontDestroyOnLoad>();
		sup.AddTag<Tag_DontDestroyOnLoad>();

		Helpers::AttachBone(target, weapon, "Weapon");
		Helpers::AttachBone(target, bag, "Bag");
		Helpers::AttachBone(target, sup, "Support");
	}
		break;

	case UpgradePreset::HEAL:
	{
		Entity weapon = mOwner->CreateStaticMeshEntity(MESH("Cotton_Swab.mesh"),
			TEXTURE("Cotton_Swab.png"));
		Entity bag = mOwner->CreateSkeletalMeshEntity(MESH("HealPack.mesh"),
			TEXTURE("Temp.png"), SKELETON("HealPack.skel"));
		Entity sup = mOwner->CreateStaticMeshEntity(MESH("Ringer.mesh"),
			TEXTURE("Temp.png"));

		weapon.AddTag<Tag_DontDestroyOnLoad>();
		bag.AddTag<Tag_DontDestroyOnLoad>();
		sup.AddTag<Tag_DontDestroyOnLoad>();

		Helpers::AttachBone(target, weapon, "Weapon");
		Helpers::AttachBone(target, bag, "Bag");
		Helpers::AttachBone(target, sup, "Support");
	}
		break;

	case UpgradePreset::SUPPORT:
	{
		Entity weapon = mOwner->CreateStaticMeshEntity(MESH("Thermometer.mesh"),
			TEXTURE("Temp.png"));
		Entity bag = mOwner->CreateSkeletalMeshEntity(MESH("Bag.mesh"),
			TEXTURE("Temp.png"), SKELETON("Bag.skel"));
		Entity sup = mOwner->CreateStaticMeshEntity(MESH("Pill_Pack.mesh"),
			TEXTURE("Temp.png"));

		weapon.AddTag<Tag_DontDestroyOnLoad>();
		bag.AddTag<Tag_DontDestroyOnLoad>();
		sup.AddTag<Tag_DontDestroyOnLoad>();

		Helpers::AttachBone(target, weapon, "Weapon");
		Helpers::AttachBone(target, bag, "Bag");
		Helpers::AttachBone(target, sup, "Support");
	}
		break;
	}
}

void UpgradeScene::updateClockPosition()
{
	const auto& playerPosition = mPlayerCharacter.GetComponent<TransformComponent>().Position;

	auto& clockTransform = mClock.GetComponent<TransformComponent>();
	Vector3 to = { playerPosition.x, clockTransform.Position.y, playerPosition.z };
	Helpers::UpdatePosition(&clockTransform.Position, to, &clockTransform.bDirty);
}

void UpgradeScene::createPlanes()
{
	// ���� �ٴ� ����
	{
		Entity attackPlane = mOwner->CreateStaticMeshEntity(MESH("Plane.mesh"),
			TEXTURE("Attack.png"), "../Assets/Meshes/Plane.mesh");

		auto& transform = attackPlane.GetComponent<TransformComponent>();
		transform.Position.x = -DISTANCE_BETWEEN_PLANE;

		attackPlane.AddTag<Tag_Plane>();
		attackPlane.AddComponent<NameComponent>("AttackPlane"sv);
	}

	// �� �ٴ� ����
	{
		Entity healPlane = mOwner->CreateStaticMeshEntity(MESH("Plane.mesh"),
			TEXTURE("Heal.png"), "../Assets/Meshes/Plane.mesh");

		auto& transform = healPlane.GetComponent<TransformComponent>();

		healPlane.AddTag<Tag_Plane>();
		healPlane.AddComponent<NameComponent>("HealPlane"sv);
	}

	// ����Ʈ �ٴ� ����
	{
		Entity supportPlane = mOwner->CreateStaticMeshEntity(MESH("Plane.mesh"),
			TEXTURE("Support.png"), "../Assets/Meshes/Plane.mesh");

		auto& transform = supportPlane.GetComponent<TransformComponent>();
		transform.Position.x = DISTANCE_BETWEEN_PLANE;

		supportPlane.AddTag<Tag_Plane>();
		supportPlane.AddComponent<NameComponent>("SupportPlane"sv);
	}

	// �ٴ�
	{
		Entity plane = mOwner->CreateStaticMeshEntity(MESH("Plane_Big.mesh"),
			TEXTURE("Brick.jpg"));

		auto& transform = plane.GetComponent<TransformComponent>();
		transform.Position.y -= 45.0f;
	}
}

void UpgradeScene::createClock()
{
	mClock = mOwner->CreateSkeletalMeshEntity(MESH("Clock.mesh"),
		TEXTURE("Clock.png"), SKELETON("Clock.skel"));

	auto& transform = mClock.GetComponent<TransformComponent>();
	transform.Position.y = 600.0f;
	transform.Rotation.y = 180.0f;

	auto& animator = mClock.GetComponent<AnimatorComponent>();
	Helpers::PlayAnimation(&animator, ANIM("Clock.anim"));
}

void UpgradeScene::startCountdown()
{
	// �ð� �ȵ� �Ҹ��� ���߰� ī��Ʈ�ٿ� ����� �����Ѵ�.
	SoundManager::StopSound("ClockTick.mp3");
	SoundManager::PlaySound("Countdown.mp3");
}
