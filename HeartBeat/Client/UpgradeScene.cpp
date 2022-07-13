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
#include "Timer.h"
#include "Utils.h"

using namespace std::string_view_literals;

UpgradeScene::UpgradeScene(Client* owner)
	: Scene(owner)
{

}

void UpgradeScene::Enter()
{
	SoundManager::PlaySound("ClockTick.mp3");

	mPlayerCharacter = GetEntityByID(mOwner->GetClientID());
	HB_ASSERT(mPlayerCharacter, "Invalid entity!");

	// 메인 카메라 위치 조정
	mOwner->SetFollowCameraTarget(mPlayerCharacter, Vector3{ 0.0f, 500.0f, -1000.0f });

	createUI();
}

void UpgradeScene::Exit()
{
	SoundManager::StopSound("ClockTick.mp3");
	DestroyExclude<Tag_DontDestroyOnLoad>();
}

void UpgradeScene::Update(float deltaTime)
{
	mElapsedTime += deltaTime;
	auto& text = mCountdownText.GetComponent<TextComponent>();
	text.Sentence = std::to_wstring(static_cast<int>(mElapsedTime));

	if (text.Sentence.size() > 1)
	{
		text.X = 610.0f;
	}
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
	Entity background = mOwner->CreateSpriteEntity(Application::GetScreenWidth(),
		Application::GetScreenHeight(),
		TEXTURE("Upgrade_Background.png"), 90);

	{
		// 타이머
		Entity timer = mOwner->CreateSpriteEntity(136, 84, TEXTURE("Timer.png"));
		auto& tRect = timer.GetComponent<RectTransformComponent>();
		tRect.Position = Vector2{ (Application::GetScreenWidth() - 136) / 2.0f,
			30.0f };

		mCountdownText = Entity{ gRegistry.create() };
		auto& text = mCountdownText.AddComponent<TextComponent>();
		text.Sentence = std::to_wstring(static_cast<int>(mElapsedTime));
		text.X = tRect.Position.x + 53.0f;
		text.Y = 45.0f;
	}

	{
		// 플레이어 닉네임
		Entity playerName = mOwner->CreateSpriteEntity(237, 84, TEXTURE("Player_Name.png"));
		auto& nameRect = playerName.GetComponent<RectTransformComponent>();
		nameRect.Position = Vector2{ 522.0f, Application::GetScreenHeight() - 357.0f };

		Entity nameText = Entity{ gRegistry.create() };
		auto& text = nameText.AddComponent<TextComponent>();
		text.Sentence = s2ws(mOwner->GetClientName());
		text.X = nameRect.Position.x + 30.0f;
		text.Y = Application::GetScreenHeight() - 334.0f;
		text.FontSize = 30;
	}

	createExplainUI(UpgradePreset::ATTACK);
	createButtons();
}

void UpgradeScene::createButtons()
{
	Vector2 startPos = Vector2{ 358.0f, 590.0f };
	{
		auto atkButton = mOwner->CreateSpriteEntity(162, 147, TEXTURE("Skill_1_Slash.png"));
		auto& rect = atkButton.GetComponent<RectTransformComponent>();
		rect.Position = startPos;

		atkButton.AddComponent<ButtonComponent>([this]() {
			SoundManager::StopSound("ChangePreset.mp3");
			SoundManager::PlaySound("ChangePreset.mp3");
			REQUEST_UPGRADE_PACKET packet = {};
			packet.PacketSize = sizeof(packet);
			packet.PacketID = REQUEST_UPGRADE;
			packet.UpgradePreset = static_cast<UINT8>(UpgradePreset::ATTACK);
			mOwner->GetPacketManager()->Send(reinterpret_cast<char*>(&packet), packet.PacketSize);

			const auto& myPos = mPlayerCharacter.GetComponent<TransformComponent>().Position;
			createChangeEffect(myPos);
			createExplainUI(UpgradePreset::ATTACK);
			});
	}

	{
		auto healButton = mOwner->CreateSpriteEntity(162, 147, TEXTURE("Skill_2_Heal.png"));
		auto& rect = healButton.GetComponent<RectTransformComponent>();
		rect.Position = Vector2{ startPos.x + 201.0f, startPos.y };

		healButton.AddComponent<ButtonComponent>([this]() {
			SoundManager::StopSound("ChangePreset.mp3");
			SoundManager::PlaySound("ChangePreset.mp3");
			REQUEST_UPGRADE_PACKET packet = {};
			packet.PacketSize = sizeof(packet);
			packet.PacketID = REQUEST_UPGRADE;
			packet.UpgradePreset = static_cast<UINT8>(UpgradePreset::HEAL);
			mOwner->GetPacketManager()->Send(reinterpret_cast<char*>(&packet), packet.PacketSize);

			const auto& myPos = mPlayerCharacter.GetComponent<TransformComponent>().Position;
			createChangeEffect(myPos);
			createExplainUI(UpgradePreset::HEAL);
			});
	}

	{
		auto supButton = mOwner->CreateSpriteEntity(162, 147, TEXTURE("Skill_3_Power.png"));
		auto& rect = supButton.GetComponent<RectTransformComponent>();
		rect.Position = Vector2{ startPos.x + 402.0f, startPos.y };

		supButton.AddComponent<ButtonComponent>([this]() {
			SoundManager::StopSound("ChangePreset.mp3");
			SoundManager::PlaySound("ChangePreset.mp3");
			REQUEST_UPGRADE_PACKET packet = {};
			packet.PacketSize = sizeof(packet);
			packet.PacketID = REQUEST_UPGRADE;
			packet.UpgradePreset = static_cast<UINT8>(UpgradePreset::SUPPORT);
			mOwner->GetPacketManager()->Send(reinterpret_cast<char*>(&packet), packet.PacketSize);

			const auto& myPos = mPlayerCharacter.GetComponent<TransformComponent>().Position;
			createChangeEffect(myPos);
			createExplainUI(UpgradePreset::SUPPORT);
			});
	}
}

void UpgradeScene::createExplainUI(UpgradePreset preset)
{
	DestroyByComponent<Tag_UI>();

	Texture* jobBoardTex = nullptr;
	Texture* jobExplainTex = nullptr;
	Texture* skillExplainTex = nullptr;
	float selectXPos = 0.0f;

	switch (preset)
	{
	case UpgradePreset::ATTACK:
	{
		jobBoardTex = TEXTURE("Attacker_Board.png");
		jobExplainTex = TEXTURE("Attacker_Explain.png");
		skillExplainTex = TEXTURE("Skill_1_Explain.png");
		selectXPos = 352.0f;
		break;
	}

	case UpgradePreset::HEAL:
	{
		jobBoardTex = TEXTURE("Healer_Board.png");
		jobExplainTex = TEXTURE("Healer_Explain.png");
		skillExplainTex = TEXTURE("Skill_2_Explain.png");
		selectXPos = 352.0f + 201.0f;
		break;
	}

	case UpgradePreset::SUPPORT:
	{
		jobBoardTex = TEXTURE("Supporter_Board.png");
		jobExplainTex = TEXTURE("Supporter_Explain.png");
		skillExplainTex = TEXTURE("Skill_3_Explain.png");
		selectXPos = 352.0f + 402.0f;
		break;
	}
	}

	Entity jobBoard = mOwner->CreateSpriteEntity(226, 75, jobBoardTex);
	jobBoard.AddTag<Tag_UI>();
	auto& jbRect = jobBoard.GetComponent<RectTransformComponent>();
	jbRect.Position = Vector2{ 64.0f, 180.0f };

	Entity jobExplain = mOwner->CreateSpriteEntity(226, 215, jobExplainTex);
	jobExplain.AddTag<Tag_UI>();
	auto& jeRect = jobExplain.GetComponent<RectTransformComponent>();
	jeRect.Position = Vector2{ 64.0f, 275.0f };

	Entity skillBoard = mOwner->CreateSpriteEntity(226, 75, TEXTURE("Skill_Board.png"));
	skillBoard.AddTag<Tag_UI>();
	auto& srect = skillBoard.GetComponent<RectTransformComponent>();
	srect.Position = Vector2{ 988.0f, 180.0f };

	Entity skillExplain = mOwner->CreateSpriteEntity(226, 215, skillExplainTex);
	skillExplain.AddTag<Tag_UI>();
	auto& seRect = skillExplain.GetComponent<RectTransformComponent>();
	seRect.Position = Vector2{ 988.0f, 275.0f };

	Entity sel = mOwner->CreateSpriteEntity(174, 159, TEXTURE("Select.png"), 110);
	sel.AddTag<Tag_UI>();
	auto& selRect = sel.GetComponent<RectTransformComponent>();
	selRect.Position = Vector2{ selectXPos, 584.0f };
}

void UpgradeScene::createChangeEffect(const Vector3& pos)
{
	auto effect = mOwner->CreateSkeletalMeshEntity(MESH("Change_Effect.mesh"), 
		TEXTURE("Change_Effect.png"),
		SKELETON("Change_Effect.skel"));
	auto& effectTransform = effect.GetComponent<TransformComponent>();
	effectTransform.Position = pos;

	auto& effectAnimator = effect.GetComponent<AnimatorComponent>();
	Helpers::PlayAnimation(&effectAnimator, ANIM("Change_Effect.anim"));

	Timer::AddEvent(0.5f, [this, effect]() {
		DestroyEntity(effect);
		});
}
