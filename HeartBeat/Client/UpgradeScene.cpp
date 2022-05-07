#include "ClientPCH.h"
#include "UpgradeScene.h"

#include "Client.h"
#include "Components.h"
#include "Character.h"
#include "Define.h"
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
	// Bgm 재생
	SoundManager::PlaySound("ClockTick.mp3");

	// 내 캐릭터 알아두기
	mPlayerCharacter = GetEntityByID(mOwner->GetClientID());
	HB_ASSERT(mPlayerCharacter, "Invalid entity!");

	mOwner->SetFollowCameraTarget(mPlayerCharacter, Vector3{0.0f, 750.0f, -750.0f});

	initPlayersPositionToZero();

	// 바닥, 공격/힐/서포트 바닥 생성
	createPlanes();

	// 위에서 플레이어를 따라 다니는 시계 생성
	createClock();
}

void UpgradeScene::Exit()
{
	// 만일 똑딱 소리가 아직도 재생 중이라면 멈춘다.
	SoundManager::StopSound("ClockTick.mp3");

	DestroyExclude<Tag_DontDestroyOnLoad>();
}

void UpgradeScene::ProcessInput()
{
	PACKET packet;
	while (mOwner->GetPacketManager()->GetPacket(packet))
	{
		switch (packet.PacketID)
		{
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

	// 시작 전 5초가 되면 카운트다운을 시작한다.
	if (mElapsed > FIVE_SECS_BEFORE_START && !mbCountdownPlayed)
	{
		mbCountdownPlayed = true;
		startCountdown();
	}

	// 시작할 시간이 되면 서버에 통지.
	// 방장이 대표로 서버에 패킷 송신.
	// 패킷을 여러 번 보내는 걸 막기 위해 bool 변수를 하나 두었다.
	if (mOwner->GetClientID() == Values::HostID 
		&& mElapsed > SECS_TO_START 
		&& !mbSendEnterPacket)
	{
		mbSendEnterPacket = true;

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

	if (Input::IsButtonPressed(KeyCode::SPACE))
	{
		// 상호작용키(SPACE BAR)를 누르면 어떤 업그레이드 Plane 위에 서 있는지 확인한다.
		checkCollisionWithPlanes();
	}

	updateClockPosition();
}

void UpgradeScene::initPlayersPositionToZero()
{
	// 로비씬 때 달랐던 플레이어들의 위치를 (0, 0, 0)으로 통일해주기
	auto players = GetEntitiesWithTag<Tag_Player>();
	if (players.empty())
	{
		HB_LOG("No Player exists.");
		return;
	}

	for (auto player : players)
	{
		auto& transform = player.GetComponent<TransformComponent>();
		Helpers::UpdatePosition(&transform.Position, Vector3::Zero, &transform.bDirty);
	}
}

bool UpgradeScene::pollKeyboardPressed()
{
	bool bChanged = false;

	if (Input::IsButtonPressed(KeyCode::LEFT))
	{
		mDirection.x -= 1.0f;
		bChanged = true;
	}

	if (Input::IsButtonPressed(KeyCode::RIGHT))
	{
		mDirection.x += 1.0f;
		bChanged = true;
	}

	if (Input::IsButtonPressed(KeyCode::UP))
	{
		mDirection.z += 1.0f;
		bChanged = true;
	}

	if (Input::IsButtonPressed(KeyCode::DOWN))
	{
		mDirection.z -= 1.0f;
		bChanged = true;
	}

	return bChanged;
}

bool UpgradeScene::pollKeyboardReleased()
{
	bool bChanged = false;

	if (Input::IsButtonReleased(KeyCode::LEFT))
	{
		mDirection.x += 1.0f;
		bChanged = true;
	}

	if (Input::IsButtonReleased(KeyCode::RIGHT))
	{
		mDirection.x -= 1.0f;
		bChanged = true;
	}

	if (Input::IsButtonReleased(KeyCode::UP))
	{
		mDirection.z -= 1.0f;
		bChanged = true;
	}

	if (Input::IsButtonReleased(KeyCode::DOWN))
	{
		mDirection.z += 1.0f;
		bChanged = true;
	}

	return bChanged;
}

void UpgradeScene::checkCollisionWithPlanes()
{
	const auto& playerBox = mPlayerCharacter.GetComponent<BoxComponent>().World;

	auto planes = GetEntitiesWithTag<Tag_Plane>();

	for (auto plane : planes)
	{
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
	
	auto target = GetEntityByID(nmPacket->EntityID);
	HB_ASSERT(target, "Invalid entity!");

	auto& transform = target.GetComponent<TransformComponent>();
	transform.Position = nmPacket->Position;

	auto& movement = target.GetComponent<MovementComponent>();
	movement.Direction = nmPacket->Direction;
}

void UpgradeScene::processNotifyUpgrade(const PACKET& packet)
{
	NOTIFY_UPGRADE_PACKET* nuPacket = reinterpret_cast<NOTIFY_UPGRADE_PACKET*>(packet.DataPtr);

	auto target = GetEntityByID(nuPacket->EntityID);
	HB_ASSERT(target, "Invalid entity!");

	// None 계열 애니메이션(비무장 상태)에서 무장 후 애니메이션으로 바꿔준다.
	auto& animator = target.GetComponent<AnimatorComponent>();
	Helpers::PlayAnimation(&animator, GetCharacterAnimationFile(nuPacket->EntityID, CharacterAnimationType::IDLE));
	
	equipPresetToCharacter(target, static_cast<UpgradePreset>(nuPacket->UpgradePreset));
}


void UpgradeScene::processNotifyEnterGame(const PACKET& packet)
{
	NOTIFY_ENTER_GAME_PACKET* nmgPacket = reinterpret_cast<NOTIFY_ENTER_GAME_PACKET*>(packet.DataPtr);

	if (nmgPacket->Result != RESULT_CODE::SUCCESS)
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
	// 기존에 붙여놨던 엔티티들을 삭제한다.
	Helpers::DetachBone(target);

	switch (preset)
	{
	case UpgradePreset::ATTACK:
	{
		Entity weapon = mOwner->CreateStaticMeshEntity(MESH("Syringe.mesh"),
			TEXTURE("Syringe.png"));
		Entity bag = mOwner->CreateSkeletalMeshEntity(MESH("Bag.mesh"),
			TEXTURE("Bag01.png"), SKELETON("Bag.skel"));
		Entity sup = mOwner->CreateStaticMeshEntity(MESH("Pill.mesh"),
			TEXTURE("Pill.png"));

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
			TEXTURE("HealPack.png"), SKELETON("HealPack.skel"));
		Entity sup = mOwner->CreateStaticMeshEntity(MESH("Ringer.mesh"),
			TEXTURE("Ringer.png"));

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
			TEXTURE("Thermometer.png"));
		Entity bag = mOwner->CreateSkeletalMeshEntity(MESH("Bag.mesh"),
			TEXTURE("Bag02.png"), SKELETON("Bag.skel"));
		Entity sup = mOwner->CreateStaticMeshEntity(MESH("Pill_Pack.mesh"),
			TEXTURE("Pill_Pack.png"));

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
	// 공격 바닥 생성
	{
		Entity attackPlane = mOwner->CreateStaticMeshEntity(MESH("Plane.mesh"),
			TEXTURE("Attack.png"), "../Assets/Meshes/Plane.mesh");

		auto& transform = attackPlane.GetComponent<TransformComponent>();
		transform.Position.x = -DISTANCE_BETWEEN_PLANE;

		attackPlane.AddTag<Tag_Plane>();
		attackPlane.AddComponent<NameComponent>("AttackPlane"sv);
	}

	// 힐 바닥 생성
	{
		Entity healPlane = mOwner->CreateStaticMeshEntity(MESH("Plane.mesh"),
			TEXTURE("Heal.png"), "../Assets/Meshes/Plane.mesh");

		auto& transform = healPlane.GetComponent<TransformComponent>();

		healPlane.AddTag<Tag_Plane>();
		healPlane.AddComponent<NameComponent>("HealPlane"sv);
	}

	// 서포트 바닥 생성
	{
		Entity supportPlane = mOwner->CreateStaticMeshEntity(MESH("Plane.mesh"),
			TEXTURE("Support.png"), "../Assets/Meshes/Plane.mesh");

		auto& transform = supportPlane.GetComponent<TransformComponent>();
		transform.Position.x = DISTANCE_BETWEEN_PLANE;

		supportPlane.AddTag<Tag_Plane>();
		supportPlane.AddComponent<NameComponent>("SupportPlane"sv);
	}

	// 바닥
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
	// 시계 똑딱 소리를 멈추고 카운트다운 재생을 시작한다.
	SoundManager::StopSound("ClockTick.mp3");
	SoundManager::PlaySound("Countdown.mp3");
}
