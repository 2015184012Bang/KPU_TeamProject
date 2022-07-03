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
#include "Utils.h"

GameScene::GameScene(Client* owner)
	: Scene(owner)
{

}

void GameScene::Enter()
{
	SoundManager::PlaySound("NormalTheme.mp3", 0.15f);

	// 내 캐릭터 알아두기
	mPlayerCharacter = GetEntityByID(mOwner->GetClientID());
	HB_ASSERT(mPlayerCharacter, "Invalid entity!");

	// 시야 설정
	mOwner->SetFollowCameraTarget(mPlayerCharacter, Vector3{ 0.0f, 1500.0f, -1300.0f });

	mOwner->SetBackgroundColor(Colors::CornflowerBlue);

	// 맵 생성
	createMap("../Assets/Maps/Map.csv");

	createUI();
}

void GameScene::Exit()
{
	Timer::Clear();

	SoundManager::StopSound("NormalTheme.mp3");
	SoundManager::StopSound("BattleTheme.mp3");

	DestroyExclude<Tag_DontDestroyOnLoad>();
}

void GameScene::ProcessInput()
{
	if (mbIsGameOver)
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

		case NOTIFY_EVENT_OCCUR:
			processNotifyEventOccur(packet);
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

	updateUI(deltaTime);
}

void GameScene::updateUI(float deltaTime)
{
	// 플레이타임 텍스트 갱신
	mPlayTime += deltaTime;
	auto& text = mPlaytimeText.GetComponent<TextComponent>();
	text.Sentence = std::to_wstring(static_cast<int>(mPlayTime));

	// 스킬 쿨타임이 돌고 있다면 시간 갱신
	if (gRegistry.valid(mCooldownText))
	{
		mCooldown -= deltaTime;
		auto& text = mCooldownText.GetComponent<TextComponent>();
		text.Sentence = std::to_wstring(static_cast<int>(mCooldown));
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
	case TileType::MID_POINT:
	case TileType::BATTLE_TRIGGER:
	case TileType::BOSS_TRIGGER:
		createRailTile(tile);
		break;

	case TileType::FAT:
		createFatTile(tile);
		break;

	case TileType::TANK_FAT:
		createTankFatTile(tile);
		break;

	case TileType::SCAR:
	case TileType::SCAR_DOG:
		createScarTile(tile);
		break;

	case TileType::HOUSE:
		createHouseTile(tile);
		break;

	case TileType::DOOR:
		createDoorTile(tile);
		break;

	case TileType::SCAR_WALL:
		createWallTile(tile);
		break;

	case TileType::SCAR_BOSS:
		createBossTile(tile);
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

void GameScene::createDoorTile(const Tile& tile)
{
	{
		const Texture* doorTex = GetTileTexture(tile.TType);
		Entity door = mOwner->CreateSkeletalMeshEntity(MESH("Door.mesh"),
			doorTex, SKELETON("Door.skel"), "../Assets/Boxes/Door.box");
		door.AddTag<Tag_Door>();
		auto& transform = door.GetComponent<TransformComponent>();
		transform.Position.x = tile.X;
		transform.Position.y = 0.0f;
		transform.Position.z = tile.Z;
		transform.Rotation.y = 270.0f;
	}

	{
		const Texture* railTex = GetTileTexture(TileType::RAIL);
		Entity rail = mOwner->CreateStaticMeshEntity(MESH("Cube.mesh"),
			railTex);
		rail.AddTag<Tag_Tile>();
		auto& transform = rail.GetComponent<TransformComponent>();
		transform.Position.x = tile.X;
		transform.Position.y = -Values::TileSide;
		transform.Position.z = tile.Z;
	}
}

void GameScene::createWallTile(const Tile& tile)
{
	{
		const Texture* wallTex = GetTileTexture(tile.TType);
		Entity wall = mOwner->CreateSkeletalMeshEntity(MESH("Wall.mesh"),
			wallTex, SKELETON("Wall.skel"), "../Assets/Boxes/Wall.box");
		wall.AddComponent<NameComponent>("Wall");
		auto& transform = wall.GetComponent<TransformComponent>();
		transform.Position.x = tile.X;
		transform.Position.y = 0.0f;
		transform.Position.z = tile.Z;
		transform.Rotation.y = 270.0f;
	}

	{
		const Texture* railTex = GetTileTexture(TileType::RAIL);
		Entity rail = mOwner->CreateStaticMeshEntity(MESH("Cube.mesh"),
			railTex);
		rail.AddTag<Tag_Tile>();
		auto& transform = rail.GetComponent<TransformComponent>();
		transform.Position.x = tile.X;
		transform.Position.y = -Values::TileSide;
		transform.Position.z = tile.Z;
	}
}

void GameScene::createBossTile(const Tile& tile)
{
	{
		Entity bossWall = mOwner->CreateSkeletalMeshEntity(MESH("BWall.mesh"),
			TEXTURE("Temp.png"), SKELETON("BWall.skel"));
		bossWall.AddComponent<NameComponent>("BossWall");
		auto& transform = bossWall.GetComponent<TransformComponent>();
		auto& animator = bossWall.GetComponent<AnimatorComponent>();
		transform.Position.x = tile.X;
		transform.Position.y = 0.0f;
		transform.Position.z = tile.Z;
		transform.Rotation.y = 270.0f;
	}

	{
		const Texture* railTex = GetTileTexture(TileType::RAIL);
		Entity rail = mOwner->CreateStaticMeshEntity(MESH("Cube.mesh"),
			railTex);
		rail.AddTag<Tag_Tile>();
		auto& transform = rail.GetComponent<TransformComponent>();
		transform.Position.x = tile.X;
		transform.Position.y = -Values::TileSide;
		transform.Position.z = tile.Z;
	}
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

	mDirection.x = std::clamp(mDirection.x, -1.0f, 1.0f);
	mDirection.z = std::clamp(mDirection.z, -1.0f, 1.0f);

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

	mDirection.x = std::clamp(mDirection.x, -1.0f, 1.0f);
	mDirection.z = std::clamp(mDirection.z, -1.0f, 1.0f);

	return bChanged;
}

void GameScene::processNotifyMove(const PACKET& packet)
{
	NOTIFY_MOVE_PACKET* nmPacket = reinterpret_cast<NOTIFY_MOVE_PACKET*>(packet.DataPtr);

	auto target = GetEntityByID(nmPacket->EntityID);
	if (!target)
	{
		HB_LOG("Invalid entity!");
		return;
	}

	auto& transform = target.GetComponent<TransformComponent>();
	transform.Position = nmPacket->Position;

	auto& movement = target.GetComponent<MovementComponent>();
	movement.Direction = nmPacket->Direction;
}

void GameScene::processNotifyAttack(const PACKET& packet)
{
	NOTIFY_ATTACK_PACKET* naPacket = reinterpret_cast<NOTIFY_ATTACK_PACKET*>(packet.DataPtr);

	auto e = GetEntityByID(naPacket->EntityID);
	if (!e)
	{
		HB_LOG("Invalid entity!");
		return;
	}

	if (naPacket->Result == CELL_ATTACK)
	{
		// 백혈구 공격 패킷
		auto& animator = e.GetComponent<AnimatorComponent>();
		animator.SetTrigger("Attack");
	}
	else
	{
		// 플레이어 공격 패킷
		auto& animator = e.GetComponent<AnimatorComponent>();
		animator.SetTrigger(GetAttackAnimTrigger(false));
		if (naPacket->Result == RESULT_CODE::ATTACK_SUCCESS &&
			naPacket->EntityID == mOwner->GetClientID())
		{
			SoundManager::PlaySound("Punch.mp3", 0.15f);
		}
	}
}

void GameScene::processNotifyEnemyAttack(const PACKET& packet)
{
	NOTIFY_ENEMY_ATTACK_PACKET* neaPacket = reinterpret_cast<NOTIFY_ENEMY_ATTACK_PACKET*>(packet.DataPtr);

	auto hitter = GetEntityByID(neaPacket->HitterID);
	if (!hitter)
	{
		HB_LOG("Invalid entity!");
		return;
	}

	auto& animator = hitter.GetComponent<AnimatorComponent>();
	animator.SetTrigger(GetAttackAnimTrigger(true));

	// 적에게 맞은 플레이어가 나라면 사운드 재생
	if (gRegistry.valid(mPlayerCharacter))
	{
		const auto& playerID = mPlayerCharacter.GetComponent<IDComponent>().ID;
		if (neaPacket->VictimID == playerID)
		{
			SoundManager::PlaySound("PlayerAttacked.mp3");
		}
	}

	// Dog인 경우 공격 이후 삭제되도록 한다.
	// 서버에서도 이 엔티티는 삭제됐음.
	if (hitter.HasComponent<Tag_Dog>())
	{
		SoundManager::PlaySound("DogBomb.mp3");

		mOwner->DestroyEntityAfter(neaPacket->HitterID, 1.1f);

		Entity bomb = mOwner->CreateSkeletalMeshEntity(MESH("Bomb.mesh"),
			TEXTURE("Bomb.png"), SKELETON("Bomb.skel"));
		auto& transform = bomb.GetComponent<TransformComponent>();
		transform.Position = hitter.GetComponent<TransformComponent>().Position;
		auto& animator = bomb.GetComponent<AnimatorComponent>();
		Helpers::PlayAnimation(&animator, ANIM("Bomb_Explode.anim"));

		Timer::AddEvent(1.0f, [this, bomb]() {
			if (gRegistry.valid(bomb))
			{
				DestroyEntity(bomb);
			}
			});
	}
}

void GameScene::processNotifyDeleteEntity(const PACKET& packet)
{
	NOTIFY_DELETE_ENTITY_PACKET* ndePacket = reinterpret_cast<NOTIFY_DELETE_ENTITY_PACKET*>(packet.DataPtr);

	auto target = GetEntityByID(ndePacket->EntityID);
	if (!target)
	{
		HB_LOG("Invalid entity!");
		return;
	}

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
		SoundManager::PlaySound("DogDead.mp3");
		auto& animator = target.GetComponent<AnimatorComponent>();
		Helpers::PlayAnimation(&animator, ANIM("Dog_Dead.anim"));
		auto& movement = target.GetComponent<MovementComponent>();
		movement.Direction = Vector3::Zero;
		mOwner->DestroyEntityAfter(ndePacket->EntityID, 3.0f);
	}
	break;

	case EntityType::VIRUS:
	{
		SoundManager::PlaySound("VirusDead.mp3");
		auto& animator = target.GetComponent<AnimatorComponent>();
		Helpers::PlayAnimation(&animator, ANIM("Virus_Dead.anim"));
		auto& movement = target.GetComponent<MovementComponent>();
		movement.Direction = Vector3::Zero;
		mOwner->DestroyEntityAfter(ndePacket->EntityID, 3.0f);
	}
	break;

	case EntityType::RED_CELL:
	{
		SoundManager::PlaySound("CellDead.mp3");
		auto& animator = target.GetComponent<AnimatorComponent>();
		Helpers::PlayAnimation(&animator, ANIM("Cell_Dead.anim"));
		auto& movement = target.GetComponent<MovementComponent>();
		movement.Direction = Vector3::Zero;
		mOwner->DestroyEntityAfter(ndePacket->EntityID, 3.0f);
	}
	break;

	case EntityType::WHITE_CELL:
	{
		auto& animator = target.GetComponent<AnimatorComponent>();
		Helpers::PlayAnimation(&animator, ANIM("Cell_Dead.anim"));
		mOwner->DestroyEntityAfter(ndePacket->EntityID, 3.0f);
	}
	break;

	case EntityType::PLAYER:
	{
		auto player = GetEntityByID(ndePacket->EntityID);
		if (!player)
		{
			HB_LOG("Invalid entity: {0}", __FUNCTION__);
			return;
		}

		auto& movement = player.GetComponent<MovementComponent>();
		movement.Direction = Vector3::Zero;

		auto& animator = player.GetComponent<AnimatorComponent>();
		Helpers::PlayAnimation(&animator, GetCharacterAnimationFile(ndePacket->EntityID, CharacterAnimationType::DEAD));

		auto entityID = ndePacket->EntityID;
		Timer::AddEvent(3.0f, [this, entityID]() {
			auto player = GetEntityByID(entityID);
			if (gRegistry.valid(player))
			{
				auto& parent = player.GetComponent<ParentComponent>();
				for (auto child : parent.Children)
				{
					DestroyEntity(child);
				}
				player.RemoveComponent<Tag_SkeletalMesh>();
				player.RemoveComponent<DebugDrawComponent>();
			}
			});

		if (mOwner->GetClientID() == ndePacket->EntityID)
		{
			auto tank = GetEntityByName("Tank");
			if (!tank)
			{
				HB_LOG("Invalid entity: {0}", __FUNCTION__);
				return;
			}

			mOwner->SetFollowCameraTarget(tank, Vector3{ 0.0f, 1000.0f, -1000.0f });
		}
	}
	break;

	case EntityType::VITAMIN:
	{
		SoundManager::PlaySound("GetVitamin.mp3");
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
		cart.AddComponent<NameComponent>("Cart");
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

	case EntityType::WHITE_CELL:
	{
		Entity cell = mOwner->CreateSkeletalMeshEntity(MESH("Cell.mesh"),
			TEXTURE("Cell_White.png"), SKELETON("Cell.skel"), ncePacket->EntityID, "../Assets/Boxes/Cell.box");
		auto& transform = cell.GetComponent<TransformComponent>();
		transform.Position = ncePacket->Position;;
		transform.Rotation.y = 180.0f;
		cell.AddTag<Tag_WhiteCell>();
		auto& animator = cell.GetComponent<AnimatorComponent>();
		Helpers::PlayAnimation(&animator, ANIM("Cell_Idle.anim"));

		Entity bat = mOwner->CreateStaticMeshEntity(MESH("Bat.mesh"), TEXTURE("Bat.png"));
		Helpers::AttachBone(cell, bat, "Weapon");
	}
	break;

	case EntityType::CAFFEINE:
	{
		Entity caffeine = mOwner->CreateStaticMeshEntity(MESH("Caffeine.mesh"),
			TEXTURE("Caffeine.png"), ncePacket->EntityID);
		auto& transform = caffeine.GetComponent<TransformComponent>();
		transform.Position = ncePacket->Position;
		transform.Rotation.y = 180.0f;
		caffeine.AddTag<Tag_Item>();
	}
	break;

	case EntityType::VITAMIN:
	{
		Entity vitamin = mOwner->CreateStaticMeshEntity(MESH("Vitamin.mesh"),
			TEXTURE("Vitamin.png"), ncePacket->EntityID);
		auto& transform = vitamin.GetComponent<TransformComponent>();
		transform.Position = ncePacket->Position;
		transform.Rotation.y = 180.0f;
		vitamin.AddTag<Tag_Item>();
	}
	break;

	case EntityType::BOSS:
	{
		Entity boss = mOwner->CreateSkeletalMeshEntity(MESH("Boss.mesh"),
			TEXTURE("Temp.png"), SKELETON("Boss.skel"), ncePacket->EntityID, "../Assets/Boxes/Boss.box");
		boss.AddComponent<NameComponent>("Boss");
		auto& transform = boss.GetComponent<TransformComponent>();
		transform.Position = ncePacket->Position;
		transform.Rotation.y = 270.0f;

		auto& animator = boss.GetComponent<AnimatorComponent>();
		Helpers::PlayAnimation(&animator, ANIM("Boss_Idle.anim"));
	}
	break;

	default:
		break;
	}
}

void GameScene::processNotifyGameOver(const PACKET& packet)
{
	SoundManager::StopSound("NormalTheme.mp3");
	SoundManager::StopSound("BattleTheme.mp3");
	SoundManager::PlaySound("GameOver.mp3");

	NOTIFY_GAME_OVER_PACKET* ngoPacket = reinterpret_cast<NOTIFY_GAME_OVER_PACKET*>(packet.DataPtr);

	for (auto& hp : mTankHps)
	{
		DestroyEntity(hp);
	}
	mTankHps.clear();

	// 게임 오버 UI 팝업
	Entity gameOverUI = Entity{ gRegistry.create() };
	auto& text = gameOverUI.AddComponent<TextComponent>();
	text.Sentence = L"Score: " + std::to_wstring(ngoPacket->Score) +
		L" PlayTime: " + std::to_wstring(ngoPacket->PlayTimeSec) + L"sec";
	text.X = 100.0f;
	text.Y = 100.0f;

	// 더이상 패킷 처리하지 않도록 flag 값 변경
	mbIsGameOver = true;

	auto tank = GetEntityByName("Tank");
	mOwner->SetFollowCameraTarget(tank, Vector3{ 0.0f, 1000.0f, -1000.f });

	// 모든 엔티티를 멈추게 한다.
	auto view = gRegistry.view<MovementComponent>();
	for (auto [entity, movement] : view.each())
	{
		movement.Direction = Vector3::Zero;
	}

	// '나가기' 버튼 생성
	Entity button = mOwner->CreateSpriteEntity(249, 78, TEXTURE("Next_Button.png"));
	auto& rect = button.GetComponent<RectTransformComponent>();
	rect.Position = Vector2{ Application::GetScreenWidth() / 2.0f - 124.0f,
		Application::GetScreenHeight() - 250.0f };

	button.AddComponent<ButtonComponent>([this]() {
		SoundManager::PlaySound("ButtonClick.mp3");
		SoundManager::StopSound("GameOver.mp3");
		doGameOver();
		});
}

void GameScene::processNotifySkill(const PACKET& packet)
{
	NOTIFY_SKILL_PACKET* nsPacket = reinterpret_cast<NOTIFY_SKILL_PACKET*>(packet.DataPtr);

	auto player = GetEntityByID(nsPacket->EntityID);
	if (!player)
	{
		HB_LOG("Invalid entity!");
		return;
	}

	auto& animator = player.GetComponent<AnimatorComponent>();
	animator.SetTrigger(GetSkillAnimTrigger(nsPacket->Preset));

	if (nsPacket->EntityID == mOwner->GetClientID())
	{
		SoundManager::PlaySound(GetSkillSound(nsPacket->Preset), 0.3f);

		mCooldown = GetSkillCooldown(static_cast<UpgradePreset>(nsPacket->Preset));

		mCooldownText = Entity{ gRegistry.create() };
		auto& text = mCooldownText.AddComponent<TextComponent>();
		text.Sentence = std::to_wstring(static_cast<int>(mCooldown));
		text.X = 39.0f + (404.0f * mOwner->GetClientID()) + 35.0f;
		text.Y = Application::GetScreenHeight() - 120.0f - 55.0f;

		Timer::AddEvent(mCooldown, [this]()
			{
				if (gRegistry.valid(mCooldownText))
				{
					DestroyEntity(mCooldownText);
				}
			});
	}
}

void GameScene::processNotifyStateChange(const PACKET& packet)
{
	NOTIFY_STATE_CHANGE_PACKET* nscPacket = reinterpret_cast<NOTIFY_STATE_CHANGE_PACKET*>(packet.DataPtr);

	{
		auto& text = mScoreText.GetComponent<TextComponent>();
		text.Sentence = std::to_wstring(nscPacket->Score);
	}

	{
		auto diff = static_cast<int>(mTankHps.size()) - nscPacket->TankHealth;

		for (int i = 0; i < diff; ++i)
		{
			DestroyEntity(mTankHps.back());
			mTankHps.pop_back();
		}
	}

	updateHpUI(nscPacket->P0Health, 0);
	updateHpUI(nscPacket->P1Health, 1);
	updateHpUI(nscPacket->P2Health, 2);
}

Entity GetCloestDoor()
{
	auto doors = gRegistry.view<Tag_Door, TransformComponent>();

	float minX = FLT_MAX;
	entt::entity target = {};
	for (auto [door, transform] : doors.each())
	{
		if (transform.Position.x < minX)
		{
			target = door;
			minX = transform.Position.x;
		}
	}

	return Entity{ target };
}

void GameScene::processNotifyEventOccur(const PACKET& packet)
{
	NOTIFY_EVENT_OCCUR_PACKET* neoPacket = reinterpret_cast<NOTIFY_EVENT_OCCUR_PACKET*>(packet.DataPtr);

	switch (static_cast<EventType>(neoPacket->EventType))
	{
	case EventType::DOOR_DOWN:
	{
		auto door = GetCloestDoor();
		auto& animator = door.GetComponent<AnimatorComponent>();
		Helpers::PlayAnimation(&animator, ANIM("Door_Open.anim"));
		SoundManager::PlaySound("DoorOpen.mp3");

		Timer::AddEvent(3.0f, [door]() {
			if (gRegistry.valid(door))
			{
				auto& movement = const_cast<Entity&>(door).AddComponent<MovementComponent>();
				movement.MaxSpeed = 1600.0f;
				movement.Direction = Vector3{ 0.0f, -1.0f, 0.0f };
			}
			});

		Timer::AddEvent(5.0f, [door]() {
			if (gRegistry.valid(door))
			{
				DestroyEntity(door);
			}
			});
	}
	break;

	case EventType::PLAYER_DEAD:
	{
		auto id = neoPacket->AdditionalData;
		auto player = GetEntityByID(id);

		if (player)
		{
			auto& movement = player.GetComponent<MovementComponent>();
			movement.Direction = Vector3::Zero;

			auto& animator = player.GetComponent<AnimatorComponent>();
			const auto deadAnim = GetCharacterAnimationFile(id, CharacterAnimationType::DEAD);
			Helpers::PlayAnimation(&animator, deadAnim);

			Timer::AddEvent(3.0f, [id]() {
				auto player = GetEntityByID(id);
				if (player)
				{
					auto& animator = player.GetComponent<AnimatorComponent>();
					const auto idleAnim = GetCharacterAnimationFile(id, CharacterAnimationType::IDLE);
					Helpers::PlayAnimation(&animator, idleAnim);
				}
				});
		}
		else
		{
			HB_LOG("Invalid entity!");
			return;
		}
	}
	break;

	case EventType::BATTLE:
	{
		doBattleOccur();
	}
	break;

	case EventType::BATTLE_END:
	{
		doBattleEnd();
	}
	break;

	case EventType::BOSS_BATTLE:
	{
		doBossBattleOccur();
	}
	break;

	case EventType::BOSS_BATTLE_END:
	{
		doBossBattleEnd();
	}
	break;

	default:
		HB_ASSERT(false, "Invalid event type!");
		break;
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

void GameScene::doBattleOccur()
{
	SoundManager::StopSound("NormalTheme.mp3");
	SoundManager::PlaySound("Warning.mp3", 0.3f);

	auto tank = GetEntityByName("Tank");
	tank.GetComponent<MovementComponent>().MaxSpeed = 0.0f;
	auto cart = GetEntityByName("Cart");
	cart.GetComponent<MovementComponent>().MaxSpeed = 0.0f;

	auto redCells = gRegistry.view<Tag_RedCell>();
	for (auto entity : redCells)
	{
		DestroyEntity(entity);
	}

	Entity ui = mOwner->CreateSpriteEntity(400, 400, TEXTURE("Warning.png"));
	auto& rect = ui.GetComponent<RectTransformComponent>();
	rect.Position = Vector2{ Application::GetScreenWidth() / 2.0f - 200.0f, Application::GetScreenHeight() / 2.0f - 200.0f };

	Timer::AddEvent(3.0f, [this, ui]() {
		DestroyEntity(ui);

		auto wall = GetEntityByName("Wall");
		if (!gRegistry.valid(wall))
		{
			HB_ASSERT(false, "Invalid entity: Wall");
		}
		mOwner->SetFollowCameraTarget(wall, Vector3{ 0.0f, 1500.0f, -2000.0f });
		});

	Timer::AddEvent(4.5f, [this]() {
		if (gRegistry.valid(mPlayerCharacter))
		{
			mOwner->SetFollowCameraTarget(mPlayerCharacter, Vector3{ 0.0f, 1500.0f, -1300.0f });
		}
		else
		{
			mOwner->ResetCamera();
		}
		});

	const INT32 dialogueWidth = 1000;
	Timer::AddEvent(4.6f, [this, dialogueWidth]() {
		Entity dia1 = mOwner->CreateSpriteEntity(dialogueWidth, 250, TEXTURE("Dialogue1.png"), 110);
		dia1.AddTag<Tag_Dialogue>();
		auto& rect = dia1.GetComponent<RectTransformComponent>();
		rect.Position = Vector2{ Application::GetScreenWidth() / 2.0f - dialogueWidth / 2.0f, 10.0f };
		});

	Timer::AddEvent(6.6f, [this, dialogueWidth]() {
		Entity dia2 = mOwner->CreateSpriteEntity(dialogueWidth, 250, TEXTURE("Dialogue2.png"), 120);
		dia2.AddTag<Tag_Dialogue>();
		auto& rect = dia2.GetComponent<RectTransformComponent>();
		rect.Position = Vector2{ Application::GetScreenWidth() / 2.0f - dialogueWidth / 2.0f, 10.0f };
		});

	Timer::AddEvent(8.6f, [this, dialogueWidth]() {
		Entity dia3 = mOwner->CreateSpriteEntity(dialogueWidth, 250, TEXTURE("Dialogue3.png"), 130);
		dia3.AddTag<Tag_Dialogue>();
		auto& rect = dia3.GetComponent<RectTransformComponent>();
		rect.Position = Vector2{ Application::GetScreenWidth() / 2.0f - dialogueWidth / 2.0f, 10.0f };
		});

	Timer::AddEvent(10.6f, []() {
		SoundManager::PlaySound("BattleTheme.mp3", 0.1f);
		DestroyByComponent<Tag_Dialogue>();
		});
}

void GameScene::doBattleEnd()
{
	const INT32 dialogueWidth = 1000;

	auto whiteCells = gRegistry.view<Tag_WhiteCell>();
	for (auto entity : whiteCells)
	{
		DestroyEntity(entity);
	}

	{
		Entity dia1 = mOwner->CreateSpriteEntity(dialogueWidth, 250, TEXTURE("Dialogue4.png"), 110);
		dia1.AddTag<Tag_Dialogue>();
		auto& rect = dia1.GetComponent<RectTransformComponent>();
		rect.Position = Vector2{ Application::GetScreenWidth() / 2.0f - dialogueWidth / 2.0f, 10.0f };
	}

	Timer::AddEvent(2.0f, [this, dialogueWidth]() {
		Entity dia2 = mOwner->CreateSpriteEntity(dialogueWidth, 250, TEXTURE("Dialogue5.png"), 120);
		dia2.AddTag<Tag_Dialogue>();
		auto& rect = dia2.GetComponent<RectTransformComponent>();
		rect.Position = Vector2{ Application::GetScreenWidth() / 2.0f - dialogueWidth / 2.0f, 10.0f };
		});

	Timer::AddEvent(5.0f, [this]() {
		DestroyByComponent<Tag_Dialogue>();
		auto tank = GetEntityByName("Tank");
		mOwner->SetFollowCameraTarget(tank, Vector3{ 0.0f, 1500.0f, -1300.0f });
		auto& animator = tank.GetComponent<AnimatorComponent>();
		animator.SetTrigger("Attack");

		auto missile = mOwner->CreateStaticMeshEntity(MESH("Missile.mesh"), TEXTURE("Missile.png"));
		missile.AddComponent<NameComponent>("Missile");
		auto& transform = missile.GetComponent<TransformComponent>();
		const auto& tankPos = tank.GetComponent<TransformComponent>().Position;
		transform.Position = tankPos + Vector3{ 0.0f, 512.0f, 0.0f };
		auto& movement = missile.AddComponent<MovementComponent>(3200.0f);
		movement.Direction = Vector3{ 1.0f, 0.0f, 0.0f };

		SoundManager::PlaySound("MissileShot.mp3");
		});

	Timer::AddEvent(6.0f, [this]() {
		auto missile = GetEntityByName("Missile");
		DestroyEntity(missile);

		auto wall = GetEntityByName("Wall");
		mOwner->SetFollowCameraTarget(wall, Vector3{ 0.0f, 1500.0f, -2000.0f });

		auto& animator = wall.GetComponent<AnimatorComponent>();
		Helpers::PlayAnimation(&animator, ANIM("Wall_Break.anim"));
		});

	Timer::AddEvent(11.0f, [this, dialogueWidth]() {
		auto wall = GetEntityByName("Wall");
		DestroyEntity(wall);
		mOwner->SetFollowCameraTarget(mPlayerCharacter, Vector3{ 0.0f, 1500.0f, -1300.0f });

		Entity dia3 = mOwner->CreateSpriteEntity(dialogueWidth, 250, TEXTURE("Dialogue6.png"), 110);
		dia3.AddTag<Tag_Dialogue>();
		auto& rect = dia3.GetComponent<RectTransformComponent>();
		rect.Position = Vector2{ Application::GetScreenWidth() / 2.0f - dialogueWidth / 2.0f, 10.0f };
		});

	Timer::AddEvent(13.0f, []() {
		auto tank = GetEntityByName("Tank");
		tank.GetComponent<MovementComponent>().MaxSpeed = Values::TankSpeed;
		auto cart = GetEntityByName("Cart");
		cart.GetComponent<MovementComponent>().MaxSpeed = Values::TankSpeed;
		DestroyByComponent<Tag_Dialogue>();
		SoundManager::StopSound("BattleTheme.mp3");
		SoundManager::PlaySound("NormalTheme.mp3", 0.15f);
		});
}

void GameScene::updateHpUI(const INT8 hp, int clientID)
{
	Entity player = GetEntityByID(clientID);
	if (!gRegistry.valid(player))
	{
		return;
	}

	auto curHealth = mHps[clientID].size();
	int diff = static_cast<int>(hp - curHealth);

	if (diff < 0)
	{
		for (int i = 0; i < abs(diff); ++i)
		{
			DestroyEntity(mHps[clientID].back());
			mHps[clientID].pop_back();
		}
	}
	else
	{
		const int hpWidth = 26;
		const int hpHeight = 68;

		const float startX = 158.0f + clientID * 404.0f;

		for (auto i = curHealth; i < curHealth + diff; ++i)
		{
			Entity hp = mOwner->CreateSpriteEntity(hpWidth, hpHeight, TEXTURE("Hp.png"));
			auto& hprect = hp.GetComponent<RectTransformComponent>();
			hprect.Position.x = startX + (26.0f * i); // HUD에서 119 오른쪽으로
			hprect.Position.y = Application::GetScreenHeight() - 96.0f; // HUD에서 24 아래로
			mHps[clientID].push_back(hp);
		}
	}
}

void GameScene::doBossBattleEnd()
{

}

void GameScene::doBossBattleOccur()
{
	SoundManager::StopSound("NormalTheme.mp3");
	SoundManager::PlaySound("Warning.mp3", 0.3f);

	auto tank = GetEntityByName("Tank");
	tank.GetComponent<MovementComponent>().MaxSpeed = 0.0f;
	auto cart = GetEntityByName("Cart");
	cart.GetComponent<MovementComponent>().MaxSpeed = 0.0f;

	auto redCells = gRegistry.view<Tag_RedCell>();
	for (auto entity : redCells)
	{
		DestroyEntity(entity);
	}

	Entity ui = mOwner->CreateSpriteEntity(400, 400, TEXTURE("Warning.png"));
	auto& rect = ui.GetComponent<RectTransformComponent>();
	rect.Position = Vector2{ Application::GetScreenWidth() / 2.0f - 200.0f, Application::GetScreenHeight() / 2.0f - 200.0f };

	Timer::AddEvent(3.0f, [this, ui]() {
		DestroyEntity(ui);

		auto bossWall = GetEntityByName("BossWall");
		const auto& bossWallTransform = bossWall.GetComponent<TransformComponent>();
		const auto& bossWallPosition = bossWallTransform.Position;

		mOwner->DisableFollowTarget();
		auto camera = mOwner->GetMainCamera();
		auto& cc = camera.GetComponent<CameraComponent>();
		cc.Position = Vector3{ bossWallPosition.x - 5200.0f, 2000.0f, bossWallPosition.z };
		cc.Target = Vector3{ bossWallPosition.x, 500.0f, bossWallPosition.z };

		auto& bossWallAnimator = bossWall.GetComponent<AnimatorComponent>();
		Helpers::PlayAnimation(&bossWallAnimator, ANIM("BWall_Break.anim"));

		auto boss = GetEntityByName("Boss");
		auto& bossAnimator = boss.GetComponent<AnimatorComponent>();
		Helpers::PlayAnimation(&bossAnimator, ANIM("Boss_Start.anim"));

		SoundManager::PlaySound("BossSpawn.mp3");
		});

	Timer::AddEvent(8.0f, [this]() {
		mOwner->SetFollowCameraTarget(mPlayerCharacter, Vector3{ 0.0f, 3500.0f, -1300.0f });
		auto bossWall = GetEntityByName("BossWall");
		DestroyEntity(bossWall);
		});
}

void GameScene::createUI()
{
	const int tropyWidth = 56;
	const int tropyHeight = 48;

	{
		auto score = mOwner->CreateSpriteEntity(tropyWidth, tropyHeight, TEXTURE("Trophy.png"));
		score.AddTag<Tag_UI>();
		auto& rect = score.GetComponent<RectTransformComponent>();
		rect.Position.x = 10.0f;
		rect.Position.y = 10.0f;
	}

	{
		mScoreText = Entity{ gRegistry.create() };
		mScoreText.AddTag<Tag_UI>();
		auto& text = mScoreText.AddComponent<TextComponent>();
		text.Sentence = L"0";
		text.X = 68.0f;
		text.Y = 10.0f;
	}

	{
		mPlaytimeText = Entity{ gRegistry.create() };
		mPlaytimeText.AddTag<Tag_UI>();
		auto& text = mPlaytimeText.AddComponent<TextComponent>();
		text.Sentence = L"0.0";
		text.X = 10.0f;
		text.Y = 55.0f;
	}

	{
		auto tankPortrait = mOwner->CreateSpriteEntity(30, 30, TEXTURE("Tank_Portrait.png"));
		auto& rect = tankPortrait.GetComponent<RectTransformComponent>();
		rect.Position = { 160.0f, 10.0f };
	}

	{
		for (int i = 0; i < 30; ++i)
		{
			auto tankHp = mOwner->CreateSpriteEntity(30, 30, TEXTURE("Heart.png"));
			auto& rect = tankHp.GetComponent<RectTransformComponent>();
			rect.Position = { 190.0f + 30 * i, 10.0f };
			mTankHps.push_back(tankHp);
		}
	}

	createHpbar();
}

void GameScene::createHpbar()
{
	/// 
	/// [39][394][10][394][10][394][39] = 1280
	///
	const int hpWidth = 26;
	const int hpHeight = 68;

	auto players = gRegistry.view<Tag_Player>();
	mHps.resize(3);

	for (auto entity : players)
	{
		Entity player = Entity{ entity };
		auto id = player.GetComponent<IDComponent>().ID;

		Entity hpbar = mOwner->CreateSpriteEntity(394, 111, GetHpbarTexture(id));
		auto& rect = hpbar.GetComponent<RectTransformComponent>();
		rect.Position.x = 39.0f + (404.0f * id);
		rect.Position.y = Application::GetScreenHeight() - 120.0f;

		for (int i = 0; i < 10; ++i)
		{
			Entity hp = mOwner->CreateSpriteEntity(hpWidth, hpHeight, TEXTURE("Hp.png"));
			auto& hprect = hp.GetComponent<RectTransformComponent>();
			hprect.Position.x = (rect.Position.x + 119.0f) + (26.0f * i); // HUD에서 119 오른쪽으로
			hprect.Position.y = Application::GetScreenHeight() - 96.0f; // HUD에서 24 아래로
			mHps[id].push_back(hp);
		}

		const auto& name = player.GetComponent<NameComponent>().Name;
		Entity nameText = Entity{ gRegistry.create() };
		auto& text = nameText.AddComponent<TextComponent>();
		text.Sentence = s2ws(name);
		text.X = rect.Position.x + 121.0f;
		text.Y = rect.Position.y - 40.0f;

		if (id == mOwner->GetClientID())
		{
			UpgradePreset preset = mOwner->GetPreset();
			auto skillTex = GetSkillTexture(preset);
			Entity skill = mOwner->CreateSpriteEntity(50, 50, skillTex);
			auto& cooldownRect = skill.GetComponent<RectTransformComponent>();
			cooldownRect.Position.x = 39.0f + (404.0f * id) + 25.0f;
			cooldownRect.Position.y = Application::GetScreenHeight() - 120.0f - 55.0f;
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

float GetSkillWaitTime(const uint8 preset)
{
	switch (preset)
	{
	case 0: return 0.7f;
	case 1: return 1.0f;
	case 2: return 2.0f;
	default: HB_LOG("Invalid preset number: {0}", preset);  return 0.0f;
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
	case TileType::SCAR_DOG:
	case TileType::SCAR_BOSS:
		return TEXTURE("Red.png");

	case TileType::HOUSE:
		return TEXTURE("House.png");

	default:
		return TEXTURE("Temp.png");
	}
}

Texture* GetHpbarTexture(const int clientID)
{
	switch (clientID)
	{
	case 0: return TEXTURE("Hpbar_Green.png");
	case 1: return TEXTURE("Hpbar_Pink.png");
	case 2: return TEXTURE("Hpbar_Red.png");
	default: HB_ASSERT(false, "Unknown client id: {0}", clientID); return nullptr;
	}
}

Texture* GetSkillTexture(UpgradePreset preset)
{
	switch (preset)
	{
	case UpgradePreset::ATTACK: return TEXTURE("Sword.png");
	case UpgradePreset::HEAL: return TEXTURE("Potion.png");
	case UpgradePreset::SUPPORT: return TEXTURE("Arm.png");
	default: HB_ASSERT(false, "Unknown preset: {0}", static_cast<int>(preset)); return nullptr;
	}
}

float GetSkillCooldown(UpgradePreset preset)
{
	switch (preset)
	{
	case UpgradePreset::ATTACK: return 4.0f;
	case UpgradePreset::HEAL: return 18.0f;
	case UpgradePreset::SUPPORT: return 20.0f;
	default: HB_ASSERT(false, "Unknown preset: {0}", static_cast<int>(preset)); return 0.0f;
	}
}
