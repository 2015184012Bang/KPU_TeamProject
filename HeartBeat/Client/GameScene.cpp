#include "ClientPCH.h"
#include "GameScene.h"

#include "Application.h"
#include "Client.h"
#include "Components.h"
#include "Define.h"
#include "PacketManager.h"
#include "Input.h"
#include "Random.h"
#include "ResourceManager.h"
#include "SoundManager.h"
#include "Helpers.h"
#include "Tags.h"
#include "Enemy.h"
#include "UpgradeScene.h"
#include "RedCell.h"
#include "LobbyScene.h"
#include "Timer.h"

constexpr int MAX_TANK_HP = 3;

GameScene::GameScene(Client* owner)
	: Scene(owner)
{

}

void GameScene::Enter()
{
	SoundManager::PlaySound("SteampipeSonata.mp3", 0.3f);

	// 내 캐릭터 알아두기
	mPlayerCharacter = GetEntityByID(mOwner->GetClientID());
	HB_ASSERT(mPlayerCharacter, "Invalid entity!");

	// 시야 설정
	mOwner->SetFollowCameraTarget(mPlayerCharacter, Vector3{ 0.0f, 1500.0f, -1300.0f });

	// 맵 생성
	createMap("../Assets/Maps/Map01.csv");

	createUI();
}

void GameScene::Exit()
{
	SoundManager::StopSound("SteampipeSonata.mp3");

	DestroyExclude<Tag_DontDestroyOnLoad>();
}

void GameScene::ProcessInput()
{
	if (bIsGameOver)
	{
		return;
	}

	PACKET packet;
	while (mOwner->GetPacketManager()->GetPacket(packet))
	{
		switch (packet.PacketID)
		{
		case NOTIFY_MOVE:
			processNotifyMove(packet);
			break;

		case NOTIFY_ATTACK:
			processNotifyAttack(packet);
			break;

		case NOTIFY_ENEMY_ATTACK:
			processNotifyEnemyAttack(packet);
			break;

		case NOTIFY_DELETE_ENTITY:
			processNotifyDeleteEntity(packet);
			break;

		case NOTIFY_CREATE_ENTITY:
			processNotifyCreateEntity(packet);
			break;

		case NOTIFY_GAME_OVER:
			processNotifyGameOver(packet);
			break;

		case NOTIFY_SKILL:
			processNotifySkill(packet);
			break;

		case NOTIFY_STATE_CHANGE:
			processNotifyStateChange(packet);
			break;

		default:
			HB_LOG("Unknown packet id: {0}", packet.PacketID);
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

	if (Input::IsButtonPressed(KeyCode::A))
	{
		REQUEST_ATTACK_PACKET packet = {};
		packet.PacketID = REQUEST_ATTACK;
		packet.PacketSize = sizeof(packet);

		mOwner->GetPacketManager()->Send(reinterpret_cast<char*>(&packet), sizeof(packet));
	}

	if (Input::IsButtonPressed(KeyCode::S))
	{
		REQUEST_SKILL_PACKET packet = {};
		packet.PacketID = REQUEST_SKILL;
		packet.PacketSize = sizeof(packet);

		mOwner->GetPacketManager()->Send(reinterpret_cast<char*>(&packet), sizeof(packet));
	}
}


void GameScene::createMap(string_view mapFile)
{
	const auto& gameMap = gGameMap.GetMap(mapFile);

	for (const auto& tile : gameMap.Tiles)
	{
		createTile(tile);
	}
}

void GameScene::createTile(const Tile& tile)
{
	switch (tile.TType)
	{
	case TileType::BLOCKED:
		createBlockedTile(tile);
		break;

	case TileType::MOVABLE:
		createMovableTile(tile);
		break;

	case TileType::RAIL:
	case TileType::START_POINT:
	case TileType::END_POINT:
		createRailTile(tile);
		break;

	case TileType::FAT:
		createFatTile(tile);
		break;

	case TileType::TANK_FAT:
		createTankFatTile(tile);
		break;

	case TileType::SCAR:
		createScarTile(tile);
		break;

	case TileType::HOUSE:
		createHouseTile(tile);
		break;

	default:
		HB_ASSERT(false, "Unknown tile type!");
		break;
	}
}


void GameScene::createBlockedTile(const Tile& tile)
{
	const Texture* tileTex = GetTileTexture(tile.TType);

	// BLOCKED 타일은 위아래, 두 개를 생성한다.
	{
		Entity top = mOwner->CreateStaticMeshEntity(MESH("Cube.mesh"),
			tileTex);
		top.AddTag<Tag_Tile>();
		auto& transform = top.GetComponent<TransformComponent>();
		transform.Position.x = tile.X;
		transform.Position.y = 0.0f;
		transform.Position.z = tile.Z;
	}

	{
		Entity down = mOwner->CreateStaticMeshEntity(MESH("Cube.mesh"),
			tileTex);
		down.AddTag<Tag_Tile>();
		auto& transform = down.GetComponent<TransformComponent>();
		transform.Position.x = tile.X;
		transform.Position.y = -Values::TileSide;
		transform.Position.z = tile.Z;
	}
}

void GameScene::createMovableTile(const Tile& tile)
{
	const Texture* tileTex = GetTileTexture(tile.TType);

	Entity obj = mOwner->CreateStaticMeshEntity(MESH("Cube.mesh"),
		tileTex);
	obj.AddTag<Tag_Tile>();

	auto& transform = obj.GetComponent<TransformComponent>();
	transform.Position.x = tile.X;
	transform.Position.y = -Values::TileSide;
	transform.Position.z = tile.Z;
}

void GameScene::createRailTile(const Tile& tile)
{
	const Texture* tileTex = GetTileTexture(tile.TType);

	Entity obj = mOwner->CreateStaticMeshEntity(MESH("Cube.mesh"),
		tileTex);
	obj.AddTag<Tag_Tile>();

	auto& transform = obj.GetComponent<TransformComponent>();
	transform.Position.x = tile.X;
	transform.Position.y = -Values::TileSide;
	transform.Position.z = tile.Z;
}

void GameScene::createFatTile(const Tile& tile)
{
	const Texture* fatTex = GetTileTexture(tile.TType);
	const Texture* movableTex = GetTileTexture(TileType::MOVABLE);

	{
		Entity fat = mOwner->CreateSkeletalMeshEntity(MESH("Fat.mesh"), fatTex,
			SKELETON("Fat.skel"));
		fat.AddTag<Tag_Tile>();
		fat.AddComponent<IDComponent>(Values::EntityID++); // FAT 타일은 서버와 동기화가 필요하므로 아이디 부여
		auto& transform = fat.GetComponent<TransformComponent>();
		transform.Position.x = tile.X;
		transform.Position.y = 0.0f;
		transform.Position.z = tile.Z;
	}

	{
		Entity movable = mOwner->CreateStaticMeshEntity(MESH("Cube.mesh"),
			movableTex);
		movable.AddTag<Tag_Tile>();
		auto& transform = movable.GetComponent<TransformComponent>();
		transform.Position.x = tile.X;
		transform.Position.y = -Values::TileSide;
		transform.Position.z = tile.Z;
	}
}

void GameScene::createTankFatTile(const Tile& tile)
{
	// TANK_FAT 타일 아래에는 RAIL 타일을 생성한다.

	const Texture* tankFatTex = GetTileTexture(tile.TType);
	const Texture* railTex = GetTileTexture(TileType::RAIL);

	{
		Entity fat = mOwner->CreateSkeletalMeshEntity(MESH("Fat.mesh"), tankFatTex,
			SKELETON("Fat.skel"));
		fat.AddTag<Tag_Tile>();
		fat.AddComponent<IDComponent>(Values::EntityID++); // FAT 타일은 서버와 동기화가 필요하므로 아이디 부여
		auto& transform = fat.GetComponent<TransformComponent>();
		transform.Position.x = tile.X;
		transform.Position.y = 0.0f;
		transform.Position.z = tile.Z;
	}

	{
		Entity rail = mOwner->CreateStaticMeshEntity(MESH("Cube.mesh"),
			railTex);
		rail.AddTag<Tag_Tile>();
		auto& transform = rail.GetComponent<TransformComponent>();
		transform.Position.x = tile.X;
		transform.Position.y = -Values::TileSide;
		transform.Position.z = tile.Z;
	}
}

void GameScene::createScarTile(const Tile& tile)
{
	const Texture* tileTex = GetTileTexture(tile.TType);

	Entity obj = mOwner->CreateStaticMeshEntity(MESH("Cube.mesh"),
		tileTex);
	obj.AddTag<Tag_Tile>();

	auto& transform = obj.GetComponent<TransformComponent>();
	transform.Position.x = tile.X;
	transform.Position.y = -Values::TileSide;
	transform.Position.z = tile.Z;
}

void GameScene::createHouseTile(const Tile& tile)
{
	const Texture* tileTex = GetTileTexture(tile.TType);

	Entity obj = mOwner->CreateStaticMeshEntity(MESH("House.mesh"),
		tileTex, "../Assets/Boxes/House.box");

	// House 타일은 조명 적용해야하므로 Tile 태그 붙이지 않음.
	//obj.AddTag<Tag_Tile>();

	auto& transform = obj.GetComponent<TransformComponent>();
	transform.Position.x = tile.X;
	transform.Position.y = 0.0f;
	transform.Position.z = tile.Z;
	transform.Rotation.y = 180.0f;
}

bool GameScene::pollKeyboardPressed()
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

bool GameScene::pollKeyboardReleased()
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

void GameScene::processNotifyMove(const PACKET& packet)
{
	NOTIFY_MOVE_PACKET* nmPacket = reinterpret_cast<NOTIFY_MOVE_PACKET*>(packet.DataPtr);

	auto target = GetEntityByID(nmPacket->EntityID);
	HB_ASSERT(target, "Invalid entity!");

	auto& transform = target.GetComponent<TransformComponent>();
	transform.Position = nmPacket->Position;

	auto& movement = target.GetComponent<MovementComponent>();
	movement.Direction = nmPacket->Direction;
}

void GameScene::processNotifyAttack(const PACKET& packet)
{
	NOTIFY_ATTACK_PACKET* naPacket = reinterpret_cast<NOTIFY_ATTACK_PACKET*>(packet.DataPtr);

	auto e = GetEntityByID(naPacket->EntityID);
	HB_ASSERT(e, "Invalid entity!");

	auto& animator = e.GetComponent<AnimatorComponent>();
	animator.SetTrigger(GetAttackAnimTrigger(false));

	if (naPacket->Result == RESULT_CODE::ATTACK_SUCCESS &&
		naPacket->EntityID == mOwner->GetClientID())
	{
		SoundManager::PlaySound("Punch.mp3", 0.15f);
	}
}

void GameScene::processNotifyEnemyAttack(const PACKET& packet)
{
	NOTIFY_ENEMY_ATTACK_PACKET* neaPacket = reinterpret_cast<NOTIFY_ENEMY_ATTACK_PACKET*>(packet.DataPtr);

	auto hitter = GetEntityByID(neaPacket->HitterID);
	HB_ASSERT(hitter, "Invalid entity!");

	auto& animator = hitter.GetComponent<AnimatorComponent>();
	animator.SetTrigger(GetAttackAnimTrigger(true));

	// 적에게 맞은 플레이어가 나라면 사운드 재생
	const auto& playerID = mPlayerCharacter.GetComponent<IDComponent>().ID;
	if (neaPacket->VictimID == playerID)
	{
		SoundManager::PlaySound("Ouch.mp3", 0.3f);
	}

	// Dog인 경우 공격 이후 삭제되도록 한다.
	// 서버에서도 이 엔티티는 삭제됐음.
	if (hitter.HasComponent<Tag_Dog>())
	{
		mOwner->DestroyEntityAfter(neaPacket->HitterID, 1.1f);
	}
}

void GameScene::processNotifyDeleteEntity(const PACKET& packet)
{
	NOTIFY_DELETE_ENTITY_PACKET* ndePacket = reinterpret_cast<NOTIFY_DELETE_ENTITY_PACKET*>(packet.DataPtr);

	auto target = GetEntityByID(ndePacket->EntityID);
	HB_ASSERT(target, "Invalid entity!");

	switch (static_cast<EntityType>(ndePacket->EntityType))
	{
	case EntityType::FAT:
	{
		auto& animator = target.GetComponent<AnimatorComponent>();
		Helpers::PlayAnimation(&animator, ANIM("Fat_Break.anim"));
		mOwner->DestroyEntityAfter(ndePacket->EntityID, 2.0f);
	}
	break;

	case EntityType::DOG:
	{
		// TODO : dog die 애니메이션
		auto& movement = target.GetComponent<MovementComponent>();
		movement.Direction = Vector3::Zero;
		mOwner->DestroyEntityAfter(ndePacket->EntityID, 1.0f);
	}
	break;

	case EntityType::VIRUS:
	{
		auto& animator = target.GetComponent<AnimatorComponent>();
		Helpers::PlayAnimation(&animator, ANIM("Virus_Dead.anim"));
		auto& movement = target.GetComponent<MovementComponent>();
		movement.Direction = Vector3::Zero;
		mOwner->DestroyEntityAfter(ndePacket->EntityID, 3.0f);
	}
	break;

	case EntityType::PLAYER:
	{
		mOwner->DestroyEntityAfter(ndePacket->EntityID, 1.0f);
	}
	break;

	case EntityType::VITAMIN:
	{
		DestroyEntityByID(ndePacket->EntityID);
	}
	break;

	case EntityType::CAFFEINE:
	{
		DestroyEntityByID(ndePacket->EntityID);
	}
	break;

	default:
		break;
	}
}

void GameScene::processNotifyCreateEntity(const PACKET& packet)
{
	NOTIFY_CREATE_ENTITY_PACKET* ncePacket = reinterpret_cast<NOTIFY_CREATE_ENTITY_PACKET*>(packet.DataPtr);

	switch (static_cast<EntityType>(ncePacket->EntityType))
	{
	case EntityType::TANK:
	{
		Entity tank = mOwner->CreateSkeletalMeshEntity(MESH("Tank.mesh"),
			TEXTURE("Tank.png"), SKELETON("Tank.skel"), ncePacket->EntityID, "../Assets/Boxes/Tank.box");
		tank.GetComponent<TransformComponent>().Position = ncePacket->Position;
		tank.AddComponent<MovementComponent>(Values::TankSpeed);
		tank.AddComponent<NameComponent>("Tank");
		auto& animator = tank.GetComponent<AnimatorComponent>();
		Helpers::PlayAnimation(&animator, ANIM("Tank_Run.anim"));
	}
	break;

	case EntityType::CART:
	{
		Entity cart = mOwner->CreateSkeletalMeshEntity(MESH("Cart.mesh"),
			TEXTURE("Cart.png"), SKELETON("Cart.skel"), ncePacket->EntityID, "../Assets/Boxes/Cart.box");
		cart.GetComponent<TransformComponent>().Position = ncePacket->Position;
		cart.AddComponent<MovementComponent>(Values::TankSpeed);
		auto& animator = cart.GetComponent<AnimatorComponent>();
		Helpers::PlayAnimation(&animator, ANIM("Cart_Run.anim"));
	}
	break;

	case EntityType::VIRUS:
	{
		Entity virus = mOwner->CreateSkeletalMeshEntity(MESH("Virus.mesh"),
			TEXTURE("Virus.png"), SKELETON("Virus.skel"), ncePacket->EntityID, "../Assets/Boxes/Virus.box");
		virus.GetComponent<TransformComponent>().Position = ncePacket->Position;
		virus.AddComponent<MovementComponent>(Values::EnemySpeed);
		virus.AddComponent<ScriptComponent>(std::make_shared<Enemy>(virus));
		virus.AddTag<Tag_Enemy>();
		auto& animator = virus.GetComponent<AnimatorComponent>();
		Helpers::PlayAnimation(&animator, ANIM("Virus_Idle.anim"));

		Entity hammer = mOwner->CreateStaticMeshEntity(MESH("Hammer.mesh"),
			TEXTURE("Hammer.png"));
		Helpers::AttachBone(virus, hammer, "Weapon");
	}
	break;

	case EntityType::DOG:
	{
		Entity dog = mOwner->CreateSkeletalMeshEntity(MESH("Dog.mesh"),
			TEXTURE("Dog.png"), SKELETON("Dog.skel"), ncePacket->EntityID, "../Assets/Boxes/Dog.box");
		dog.GetComponent<TransformComponent>().Position = ncePacket->Position;
		dog.AddComponent<MovementComponent>(Values::EnemySpeed);
		dog.AddComponent<ScriptComponent>(std::make_shared<Enemy>(dog));
		dog.AddTag<Tag_Enemy>();
		dog.AddTag<Tag_Dog>();
		auto& animator = dog.GetComponent<AnimatorComponent>();
		Helpers::PlayAnimation(&animator, ANIM("Dog_Idle.anim"));
	}
	break;

	case EntityType::RED_CELL:
	{
		Entity cell = mOwner->CreateSkeletalMeshEntity(MESH("Cell.mesh"),
			TEXTURE("Cell_Red.png"), SKELETON("Cell.skel"), ncePacket->EntityID, "../Assets/Boxes/Cell.box");
		cell.GetComponent<TransformComponent>().Position = ncePacket->Position;
		cell.AddComponent<MovementComponent>(Values::CellSpeed);
		cell.AddTag<Tag_RedCell>();
		cell.AddComponent<ScriptComponent>(std::make_shared<RedCell>(cell));
		auto& animator = cell.GetComponent<AnimatorComponent>();
		Helpers::PlayAnimation(&animator, ANIM("Cell_Run.anim"));

		Entity o2 = mOwner->CreateStaticMeshEntity(MESH("O2.mesh"),
			TEXTURE("O2.png"));
		Helpers::AttachBone(cell, o2, "Weapon");
	}
	break;

	case EntityType::CAFFEINE:
	{
		Entity caffeine = mOwner->CreateStaticMeshEntity(MESH("Sphere.mesh"),
			TEXTURE("Brown.png"), ncePacket->EntityID);
		caffeine.GetComponent<TransformComponent>().Position = ncePacket->Position;
		caffeine.AddTag<Tag_Item>();
	}
	break;

	case EntityType::VITAMIN:
	{
		Entity vitamin = mOwner->CreateStaticMeshEntity(MESH("Sphere.mesh"),
			TEXTURE("Yellow.png"), ncePacket->EntityID);
		vitamin.GetComponent<TransformComponent>().Position = ncePacket->Position;
		vitamin.AddTag<Tag_Item>();
	}
	break;

	default:
		break;
	}
}

void GameScene::processNotifyGameOver(const PACKET& packet)
{
	NOTIFY_GAME_OVER_PACKET* ngoPacket = reinterpret_cast<NOTIFY_GAME_OVER_PACKET*>(packet.DataPtr);

	// 탱크 Hp UI 삭제
	for (auto ui : mTankHpUI)
	{
		DestroyEntity(ui);
	}
	mTankHpUI.clear();

	// 게임 오버 UI 팝업
	Entity gameOverUI = Entity{ gRegistry.create() };
	auto& text = gameOverUI.AddComponent<TextComponent>();
	text.Sentence = L"CO2: " + std::to_wstring(ngoPacket->CO2) +
		L" O2: " + std::to_wstring(ngoPacket->O2) +
		L" PlayTime: " + std::to_wstring(ngoPacket->PlayTimeSec) + L"sec";
	text.X = 100.0f;
	text.Y = 100.0f;

	// 더이상 패킷 처리하지 않도록 flag 값 변경
	bIsGameOver = true;

	auto tank = GetEntityByName("Tank");
	mOwner->SetFollowCameraTarget(tank, Vector3{ 0.0f, 1000.0f, -1000.f });

	// 모든 엔티티를 멈추게 한다.
	auto view = gRegistry.view<MovementComponent>();
	for (auto [entity, movement] : view.each())
	{
		movement.Direction = Vector3::Zero;
	}

	// '나가기' 버튼 생성
	Entity button = mOwner->CreateSpriteEntity(200, 100, TEXTURE("OutButton.png"));
	auto& rect = button.GetComponent<RectTransformComponent>();
	rect.Position = Vector2{ Application::GetScreenWidth() / 2.0f - 100.0f,
		Application::GetScreenHeight() - 125.0f };

	button.AddComponent<ButtonComponent>([this]() {
		doGameOver();
		});
}

void GameScene::processNotifySkill(const PACKET& packet)
{
	NOTIFY_SKILL_PACKET* nsPacket = reinterpret_cast<NOTIFY_SKILL_PACKET*>(packet.DataPtr);

	auto player = GetEntityByID(nsPacket->EntityID);
	auto& animator = player.GetComponent<AnimatorComponent>();
	animator.SetTrigger(GetSkillAnimTrigger(nsPacket->Preset));

	if (nsPacket->EntityID == mOwner->GetClientID())
	{
		SoundManager::PlaySound(GetSkillSound(nsPacket->Preset), 0.8f);
	}
}

void GameScene::processNotifyStateChange(const PACKET& packet)
{
	NOTIFY_STATE_CHANGE_PACKET* nscPacket = reinterpret_cast<NOTIFY_STATE_CHANGE_PACKET*>(packet.DataPtr);

	auto& o2 = mO2Text.GetComponent<TextComponent>();
	o2.Sentence = std::to_wstring(nscPacket->O2);

	auto& co2 = mCO2Text.GetComponent<TextComponent>();
	co2.Sentence = std::to_wstring(nscPacket->CO2);

	if (mTankHP > nscPacket->TankHealth)
	{
		mTankHP = nscPacket->TankHealth;
		DestroyEntity(mTankHpUI.back());
		mTankHpUI.pop_back();
	}
}

void GameScene::doGameOver()
{
	mOwner->ResetCamera();

	// Renderable things 삭제
	DestroyByComponent<Tag_StaticMesh>();
	DestroyByComponent<Tag_SkeletalMesh>();

	// 재시작을 위해 엔티티 아이디 초기화
	Values::EntityID = 3;

	// 로비씬으로 전환.
	auto lobbyScene = new LobbyScene{ mOwner };
	lobbyScene->RequestAvailableRoom();
	mOwner->ChangeScene(lobbyScene);
}

void GameScene::createUI()
{
	{
		auto o2 = mOwner->CreateSpriteEntity(100, 100, TEXTURE("UI_O2.png"));
		o2.AddTag<Tag_UI>();
		auto& rect = o2.GetComponent<RectTransformComponent>();
		rect.Position = Vector2::Zero;
	}

	{
		auto co2 = mOwner->CreateSpriteEntity(100, 100, TEXTURE("UI_CO2.png"));
		co2.AddTag<Tag_UI>();
		auto& rect = co2.GetComponent<RectTransformComponent>();
		rect.Position = Vector2{ 0.0f, 100.0f };
	}

	{
		mO2Text = Entity{ gRegistry.create() };
		mO2Text.AddTag<Tag_UI>();
		auto& text = mO2Text.AddComponent<TextComponent>();
		text.Sentence = L"0";
		text.X = 100.0f;
		text.Y = 0.0f;
	}

	{
		mCO2Text = Entity{ gRegistry.create() };
		mCO2Text.AddTag<Tag_UI>();
		auto& text = mCO2Text.AddComponent<TextComponent>();
		text.Sentence = L"0";
		text.X = 100.0f;
		text.Y = 100.0f;
	}

	{
		mTankHP = MAX_TANK_HP;
		for (int i = 0; i < MAX_TANK_HP; ++i)
		{
			auto ui = mOwner->CreateSpriteEntity(40, 20, TEXTURE("Red.png"));
			ui.AddTag<Tag_UI>();
			auto& rect = ui.GetComponent<RectTransformComponent>();
			rect.Position = Vector2{ 600.0f, 150.0f - i * 25.0f };

			mTankHpUI.push_back(ui);
		}
	}
}

string GetAttackAnimTrigger(bool isEnemy /*= false*/)
{
	if (isEnemy)
	{
		return "Attack";
	}
	else
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
}

string GetSkillAnimTrigger(const uint8 preset)
{
	switch (preset)
	{
	case 0:
		return "Skill1";

	case 1:
		return "Skill2";

	case 2:
		return "Skill3";

	default:
		HB_ASSERT(false, "Unknown preset!");
		return "";
	}
}

string GetSkillSound(const uint8 preset)
{
	switch (preset)
	{
	case 0:
		return "Skill1.mp3";

	case 1:
		return "Skill2.mp3";

	case 2:
		return "Skill3.mp3";

	default:
		return "Skill1.mp3";
	}
}
Texture* GetTileTexture(TileType ttype)

{
	switch (ttype)
	{
	case TileType::BLOCKED:
		return TEXTURE("Blocked.png");

	case TileType::MOVABLE:
		return TEXTURE("Road.png");

	case TileType::RAIL:
	case TileType::START_POINT:
	case TileType::END_POINT:
		return TEXTURE("Rail.png");

	case TileType::FAT:
		return TEXTURE("Fat.png");

	case TileType::TANK_FAT:
		return TEXTURE("Fat.png");

	case TileType::SCAR:
		return TEXTURE("Red.png");

	case TileType::HOUSE:
		return TEXTURE("House.png");

	default:
		HB_ASSERT(false, "Unknown tile type!");
		return nullptr;
	}
}
