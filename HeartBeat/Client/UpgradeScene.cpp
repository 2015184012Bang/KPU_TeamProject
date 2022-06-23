#include "ClientPCH.h"
#include "UpgradeScene.h"

#include "Application.h"
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
	SoundManager::PlaySound("ClockTick.mp3", 0.2f);

	mPlayerCharacter = GetEntityByID(mOwner->GetClientID());
	HB_ASSERT(mPlayerCharacter, "Invalid entity!");

	mOwner->SetFollowCameraTarget(mPlayerCharacter, Vector3{ 0.0f, 750.0f, -750.0f });
	mOwner->SetBackgroundColor(Colors::Black);

	createUI();
}

void UpgradeScene::Exit()
{
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
			mOwner->ChangeScene(newScene);
			break;
		}
	}
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

	if (nuPacket->EntityID == mOwner->GetClientID())
	{
		switch (static_cast<UpgradePreset>(nuPacket->UpgradePreset))
		{
		case UpgradePreset::ATTACK: mOwner->SetPreset(UpgradePreset::ATTACK); break;
		case UpgradePreset::HEAL: mOwner->SetPreset(UpgradePreset::HEAL); break;
		case UpgradePreset::SUPPORT: mOwner->SetPreset(UpgradePreset::SUPPORT); break;
		default: HB_ASSERT(false, "Unknown preset: {0}", nuPacket->UpgradePreset); break;
		}
	}
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

void UpgradeScene::createUI()
{
	Vector2 startPos = Vector2{ Application::GetScreenWidth() / 2.0f - 150.0f, Application::GetScreenHeight() - 150.0f };

	{
		auto atkButton = mOwner->CreateSpriteEntity(100, 100, TEXTURE("Sword.png"));
		auto& rect = atkButton.GetComponent<RectTransformComponent>();
		rect.Position = startPos;

		atkButton.AddComponent<ButtonComponent>([this]() {
			SoundManager::StopSound("ButtonClick.mp3");
			SoundManager::PlaySound("ButtonClick.mp3");
			REQUEST_UPGRADE_PACKET packet = {};
			packet.PacketSize = sizeof(packet);
			packet.PacketID = REQUEST_UPGRADE;
			packet.UpgradePreset = static_cast<UINT8>(UpgradePreset::ATTACK);
			mOwner->GetPacketManager()->Send(reinterpret_cast<char*>(&packet), packet.PacketSize);
			});
	}

	{
		auto healButton = mOwner->CreateSpriteEntity(100, 100, TEXTURE("Potion.png"));
		auto& rect = healButton.GetComponent<RectTransformComponent>();
		rect.Position = Vector2{ startPos.x + 100.0f, startPos.y };

		healButton.AddComponent<ButtonComponent>([this]() {
			SoundManager::StopSound("ButtonClick.mp3");
			SoundManager::PlaySound("ButtonClick.mp3");
			REQUEST_UPGRADE_PACKET packet = {};
			packet.PacketSize = sizeof(packet);
			packet.PacketID = REQUEST_UPGRADE;
			packet.UpgradePreset = static_cast<UINT8>(UpgradePreset::HEAL);
			mOwner->GetPacketManager()->Send(reinterpret_cast<char*>(&packet), packet.PacketSize);
			});
	}

	{
		auto supButton = mOwner->CreateSpriteEntity(100, 100, TEXTURE("Arm.png"));
		auto& rect = supButton.GetComponent<RectTransformComponent>();
		rect.Position = Vector2{ startPos.x + 200.0f, startPos.y };

		supButton.AddComponent<ButtonComponent>([this]() {
			SoundManager::StopSound("ButtonClick.mp3");
			SoundManager::PlaySound("ButtonClick.mp3");
			REQUEST_UPGRADE_PACKET packet = {};
			packet.PacketSize = sizeof(packet);
			packet.PacketID = REQUEST_UPGRADE;
			packet.UpgradePreset = static_cast<UINT8>(UpgradePreset::SUPPORT);
			mOwner->GetPacketManager()->Send(reinterpret_cast<char*>(&packet), packet.PacketSize);
			});
	}
}
