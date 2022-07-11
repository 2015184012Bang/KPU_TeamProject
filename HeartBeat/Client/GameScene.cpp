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
	DestroyByComponent<Tag_StaticMesh>();
	DestroyByComponent<Tag_SkeletalMesh>();
	DestroyByComponent<Tag_Player>();
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

	updateLightPosition();
	updateUI(deltaTime);
}

void GameScene::updateLightPosition()
{
	auto& light = mOwner->GetLight();
	auto& lc = light.GetComponent<LightComponent>();
	const auto& playerPos = mPlayerCharacter.GetComponent<TransformComponent>().Position;
	lc.Light.LightPosition = Vector3{ playerPos.x , 1000.0f, playerPos.z };
}

void GameScene::updateUI(float deltaTime)
{
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
			TEXTURE("BWall.png"), SKELETON("BWall.skel"));
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
			PlayHitSound(naPacket->EntityType);
			createAttackEffect(naPacket->EntityID);
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
		mOwner->DestroyEntityAfter(neaPacket->HitterID, 1.1f);

		auto victimID = neaPacket->VictimID;
		Timer::AddEvent(1.0f, [this, victimID]() {
			SoundManager::PlaySound("DogBomb.mp3");

			Entity bomb = mOwner->CreateSkeletalMeshEntity(MESH("Bomb.mesh"),
				TEXTURE("Bomb.png"), SKELETON("Bomb.skel"));

			auto victim = GetEntityByID(victimID);

			auto& transform = bomb.GetComponent<TransformComponent>();
			transform.Position = victim.GetComponent<TransformComponent>().Position;
			auto& animator = bomb.GetComponent<AnimatorComponent>();
			Helpers::PlayAnimation(&animator, ANIM("Bomb_Explode.anim"));

			Timer::AddEvent(1.0f, [this, bomb]() {
				if (gRegistry.valid(bomb))
				{
					DestroyEntity(bomb);
				}
				});
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

	case EntityType::BOSS:
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
		auto id = ncePacket->EntityID;
		auto position = ncePacket->Position;

		Timer::AddEvent(3.0f, [this, id, position]() {
			Entity boss = mOwner->CreateSkeletalMeshEntity(MESH("Boss.mesh"),
				TEXTURE("Boss_1.png"), SKELETON("Boss.skel"), id, "../Assets/Boxes/Boss.box");
			boss.AddComponent<NameComponent>("Boss");
			boss.AddComponent<MovementComponent>(0.0f);
			boss.AddTag<Tag_Boss>();
			auto& transform = boss.GetComponent<TransformComponent>();
			transform.Position = position;
			transform.Rotation.y = 270.0f;

			auto& animator = boss.GetComponent<AnimatorComponent>();
			Helpers::PlayAnimation(&animator, ANIM("Boss_Start.anim"));

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

			SoundManager::PlaySound("BossSpawn.mp3");
			});
	}
	break;

	case EntityType::ATTACK_POINT:
	{
		auto attackPoint = mOwner->CreateStaticMeshEntity(MESH("Attack_Point.mesh"),
			TEXTURE("Red.png"));
		auto& pointTransform = attackPoint.GetComponent<TransformComponent>();
		pointTransform.Position = ncePacket->Position;

		Timer::AddEvent(2.0f, [this, attackPoint]() {
			Entity apoint = Entity{ attackPoint };

			Texture* tailTex = nullptr;
			switch (mBossSpecialSkillUsageCount)
			{
			case 0: tailTex = TEXTURE("Tail_1.png"); break;
			case 1: tailTex = TEXTURE("Tail_2.png"); break;
			case 2: tailTex = TEXTURE("Tail_3.png"); break;
			default: tailTex = TEXTURE("Tail_3.png"); break;
			}

			auto tail = mOwner->CreateSkeletalMeshEntity(MESH("Tail.mesh"), 
				tailTex,
				SKELETON("Tail.skel"), "../Assets/Boxes/Tail.box");
			auto& tailTransform = tail.GetComponent<TransformComponent>();
			tailTransform.Position = apoint.GetComponent<TransformComponent>().Position;
			tailTransform.Rotation.y = 90.0f;

			auto& tailAnimator = tail.GetComponent<AnimatorComponent>();
			Helpers::PlayAnimation(&tailAnimator, ANIM("Tail_Attack.anim"));

			Timer::AddEvent(2.0f, [tail]() {
				DestroyEntity(tail);
				});

			DestroyEntity(attackPoint);
			});
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
	SoundManager::StopSound("BossTheme.mp3");

	NOTIFY_GAME_OVER_PACKET* ngoPacket = reinterpret_cast<NOTIFY_GAME_OVER_PACKET*>(packet.DataPtr);

	// 게임오버
	if (!ngoPacket->IsWin)
	{
		SoundManager::PlaySound("GameOver.mp3");

		// 모든 플레이어의 체력을 0으로 표시한다.
		auto players = gRegistry.view<Tag_Player, IDComponent>();
		for (auto [player, id] : players.each())
		{
			updateHpUI(0, id.ID);
		}

		// 탱크 체력 0으로 표시한다.
		auto& text = mTankHpText.GetComponent<TextComponent>();
		text.Sentence = std::to_wstring(0);
	}

	{
		Entity gameOverUI = Entity{ gRegistry.create() };
		auto& text = gameOverUI.AddComponent<TextComponent>();
		text.Sentence = L"Score: " + std::to_wstring(ngoPacket->Score) +
			L" PlayTime: " + std::to_wstring(ngoPacket->PlayTimeSec) + L"sec";
		text.X = 100.0f;
		text.Y = 100.0f;
	}

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

	auto entity = GetEntityByID(nsPacket->EntityID);
	if (!entity)
	{
		HB_LOG("Invalid entity!");
		return;
	}

	// 보스가 스킬을 사용한 경우
	if (entity.HasComponent<Tag_Boss>())
	{
		doBossSkill(nsPacket->Preset);
	}

	// 플레이어가 스킬을 사용한 경우
	else
	{
		auto& animator = entity.GetComponent<AnimatorComponent>();
		animator.SetTrigger(GetSkillAnimTrigger(nsPacket->Preset));

		createSkillEffect(nsPacket->EntityID, nsPacket->Preset);

		if (nsPacket->EntityID == mOwner->GetClientID())
		{
			SoundManager::PlaySound(GetSkillSound(nsPacket->Preset), 0.3f);

			mCooldown = GetSkillCooldown(static_cast<UpgradePreset>(nsPacket->Preset));

			if (gRegistry.valid(mCooldownText))
			{
				DestroyEntity(mCooldownText);
			}

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
}

void GameScene::processNotifyStateChange(const PACKET& packet)
{
	NOTIFY_STATE_CHANGE_PACKET* nscPacket = reinterpret_cast<NOTIFY_STATE_CHANGE_PACKET*>(packet.DataPtr);

	{
		auto& text = mScoreText.GetComponent<TextComponent>();
		text.Sentence = std::to_wstring(nscPacket->Score);
	}

	{
		auto& text = mTankHpText.GetComponent<TextComponent>();
		text.Sentence = std::to_wstring(nscPacket->TankHealth);
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

	Entity ui = mOwner->CreateSpriteEntity(Application::GetScreenWidth(), 400, TEXTURE("Warning.png"));
	auto& rect = ui.GetComponent<RectTransformComponent>();
	rect.Position = Vector2{ 0.0f, Application::GetScreenHeight() / 2.0f - 200.0f };

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
		createDialogue(TEXTURE("Dialogue1.png"), 110);
		});

	Timer::AddEvent(6.6f, [this, dialogueWidth]() {
		createDialogue(TEXTURE("Dialogue2.png"), 120);
		});

	Timer::AddEvent(8.6f, [this, dialogueWidth]() {
		createDialogue(TEXTURE("Dialogue3.png"), 130);
		});

	Timer::AddEvent(10.6f, [this]() {
		SoundManager::PlaySound("BattleTheme.mp3", 0.1f);
		clearDialogue();
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

	createDialogue(TEXTURE("Dialogue4.png"), 110);

	Timer::AddEvent(2.0f, [this, dialogueWidth]() {
		createDialogue(TEXTURE("Dialogue5.png"), 120);
		});

	Timer::AddEvent(5.0f, [this]() {
		clearDialogue();
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
		
		SoundManager::PlaySound("WallDead.mp3");
		});

	Timer::AddEvent(11.0f, [this, dialogueWidth]() {
		auto wall = GetEntityByName("Wall");
		DestroyEntity(wall);
		mOwner->SetFollowCameraTarget(mPlayerCharacter, Vector3{ 0.0f, 1500.0f, -1300.0f });

		createDialogue(TEXTURE("Dialogue6.png"), 110);
		});

	Timer::AddEvent(13.0f, [this]() {
		auto tank = GetEntityByName("Tank");
		tank.GetComponent<MovementComponent>().MaxSpeed = Values::TankSpeed;
		auto cart = GetEntityByName("Cart");
		cart.GetComponent<MovementComponent>().MaxSpeed = Values::TankSpeed;
		clearDialogue();
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
	SoundManager::StopSound("BossTheme.mp3");

	auto boss = GetEntityByName("Boss");
	const auto& bossPosition = boss.GetComponent<TransformComponent>().Position;

	mOwner->DisableFollowTarget();
	auto camera = mOwner->GetMainCamera();
	auto& cc = camera.GetComponent<CameraComponent>();
	cc.Position = Vector3{ bossPosition.x - 5200.0f, 2000.0f, bossPosition.z };
	cc.Target = Vector3{ bossPosition.x, 500.0f, bossPosition.z };

	SoundManager::PlaySound("BossDead.mp3", 0.5f);

	auto& animator = boss.GetComponent<AnimatorComponent>();
	Helpers::PlayAnimation(&animator, ANIM("Boss_Dead.anim"));

	const auto id = boss.GetComponent<IDComponent>().ID;
	mOwner->DestroyEntityAfter(id, 4.0f);

	DestroyByComponent<Tag_Enemy>();

	Timer::AddEvent(4.0f, [this]() {
		mOwner->SetFollowCameraTarget(mPlayerCharacter, Vector3{ 0.0f, 1500.0f, -1300.0f });
		createDialogue(TEXTURE("Dialogue10.png"), 110);
		SoundManager::PlaySound("GameWin.mp3");
		});

	Timer::AddEvent(9.0f, [this]() {
		clearDialogue();
		});
}

void GameScene::doBossSkill(const UINT8 skillType)
{
	switch (skillType)
	{
	case 0:
	{
		auto boss = GetEntityByName("Boss");
		auto& animator = boss.GetComponent<AnimatorComponent>();
		Helpers::PlayAnimation(&animator, ANIM("Boss_Attack1.anim"));

		break;
	}
	case 1:
	{
		auto boss = GetEntityByName("Boss");
		auto& animator = boss.GetComponent<AnimatorComponent>();
		Helpers::PlayAnimation(&animator, ANIM("Boss_Attack2.anim"));
	
		auto sweep = mOwner->CreateStaticMeshEntity(MESH("Sweep_Area.mesh"), TEXTURE("Red.png"));
		auto& sweepTransform = sweep.GetComponent<TransformComponent>();
		const auto& bossPos = boss.GetComponent<TransformComponent>().Position;
		sweepTransform.Position = Vector3{ bossPos.x - 1400.0f, 10.0f, bossPos.z };
		sweepTransform.Rotation.y = 90.0f;

		Timer::AddEvent(2.0f, [sweep]() {
			DestroyEntity(sweep);
			});
		break;
	}
	case 2: // 보스 필살기
	{
		auto boss = GetEntityByName("Boss");
		auto& animator = boss.GetComponent<AnimatorComponent>();
		Helpers::PlayAnimation(&animator, ANIM("Boss_Skill.anim"));

		Timer::AddEvent(1.0f, []() {
			SoundManager::PlaySound("BossSpecial.mp3");
			});

		Timer::AddEvent(4.0f, [this]() {
			auto boss = GetEntityByName("Boss");
			auto& meshRenderer = boss.GetComponent<MeshRendererComponent>();
			meshRenderer.Tex = mBossSpecialSkillUsageCount == 0 ? TEXTURE("Boss_2.png") 
				: TEXTURE("Boss_3.png");
			mBossSpecialSkillUsageCount++;
			});
		break;
	}
	default:
		HB_LOG("Unknown boss skill type: {0}", skillType);
		break;
	}
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

	Entity ui = mOwner->CreateSpriteEntity(Application::GetScreenWidth(), 400, TEXTURE("Warning.png"));
	auto& rect = ui.GetComponent<RectTransformComponent>();
	rect.Position = Vector2{ 0.0f, Application::GetScreenHeight() / 2.0f - 200.0f };

	Timer::AddEvent(3.0f, [this, ui]() {
		DestroyEntity(ui);
		});

	Timer::AddEvent(8.0f, [this]() {
		mOwner->SetFollowCameraTarget(mPlayerCharacter, Vector3{ 0.0f, 3500.0f, -1300.0f });
		auto bossWall = GetEntityByName("BossWall");
		DestroyEntity(bossWall);
		createDialogue(TEXTURE("Dialogue7.png"), 110);
		});

	Timer::AddEvent(10.0f, [this]() {
		createDialogue(TEXTURE("Dialogue8.png"), 120);
		});

	Timer::AddEvent(12.0f, [this]() {
		createDialogue(TEXTURE("Dialogue9.png"), 130);
		});

	Timer::AddEvent(14.0f, [this]() {
		clearDialogue();
		SoundManager::PlaySound("BossTheme.mp3", 0.1f);
		});
}

void GameScene::createUI()
{
	{
		auto score = mOwner->CreateSpriteEntity(120, 48, TEXTURE("Trophy.png"));
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
		text.X = 50.0f;
		text.Y = 10.0f;
	}

	{
		auto tankPortrait = mOwner->CreateSpriteEntity(56, 48, TEXTURE("Tank_Portrait.png"));
		auto& rect = tankPortrait.GetComponent<RectTransformComponent>();
		rect.Position = { 10.0f, 60.0f };
	}

	{
		mTankHpText = Entity{ gRegistry.create() };
		auto& text = mTankHpText.AddComponent<TextComponent>();
		text.Sentence = L"50";
		text.X = 68.0f;
		text.Y = 60.0f;
	}

	{
		auto heart = mOwner->CreateSpriteEntity(56, 48, TEXTURE("Heart.png"));
		auto& rect = heart.GetComponent<RectTransformComponent>();
		rect.Position = { 68.0f, 60.0f };
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

void GameScene::createAttackEffect(const UINT32 entityID)
{
	auto effect = mOwner->CreateSkeletalMeshEntity(MESH("Attack_Effect.mesh"), TEXTURE("Attack_Effect.png"),
		SKELETON("Attack_Effect.skel"));
	auto& effectTransform = effect.GetComponent<TransformComponent>();

	auto player = GetEntityByID(entityID);
	const auto& playerTransform = player.GetComponent<TransformComponent>();

	Quaternion q = Quaternion::CreateFromYawPitchRoll(XMConvertToRadians(playerTransform.Rotation.y),
		0.0f, 0.0f);
	Vector3 forward = Vector3::Transform(Vector3::UnitZ, q);

	effectTransform.Position = playerTransform.Position + forward * 200.0f;
	effectTransform.Position.y = 200.0f;

	auto& effectAnimator = effect.GetComponent<AnimatorComponent>();
	Helpers::PlayAnimation(&effectAnimator, ANIM("Attack_Effect.anim"));

	Timer::AddEvent(0.7f, [effect]() {
		DestroyEntity(effect);
		});
}

void GameScene::createSkillEffect(const UINT32 entityID, const UINT8 preset)
{
	auto player = GetEntityByID(entityID);
	const auto& playerPos = player.GetComponent<TransformComponent>().Position;

	switch (static_cast<UpgradePreset>(preset))
	{
	case UpgradePreset::ATTACK:
	{
		float yaw = 0.0f;

		for (auto i = 0; i < 4; ++i)
		{
			auto effect = mOwner->CreateSkeletalMeshEntity(MESH("Skill_Effect_Atk.mesh"), 
				TEXTURE("Slash_Effect.png"),
				SKELETON("Skill_Effect_Atk.skel"));
			auto& effectTransform = effect.GetComponent<TransformComponent>();
			effectTransform.Position = Vector3{ playerPos.x, 200.f, playerPos.z };
			effectTransform.Rotation.y = yaw;

			auto& effectAnimator = effect.GetComponent<AnimatorComponent>();
			Helpers::PlayAnimation(&effectAnimator, ANIM("Skill_Effect_Atk.anim"));

			yaw += 90.0f;

			Timer::AddEvent(0.4f, [effect]() {
				DestroyEntity(effect);
				});
		}
		break;
	}

	case UpgradePreset::HEAL:
	{
		// 힐 이펙트는 모든 유저에게 생성
		auto players = gRegistry.view<Tag_Player, TransformComponent, IDComponent>();
		for (auto [player, transform, id] : players.each())
		{
			auto effect = mOwner->CreateSkeletalMeshEntity(MESH("Skill_Effect_Heal.mesh"), 
				TEXTURE("Heal_Effect.png"),
				SKELETON("Skill_Effect_Heal.skel"));
			effect.AddComponent<FollowComponent>(id.ID);
			auto& effectTransform = effect.GetComponent<TransformComponent>();
			effectTransform.Position = transform.Position;

			auto& effectAnimator = effect.GetComponent<AnimatorComponent>();
			Helpers::PlayAnimation(&effectAnimator, ANIM("Skill_Effect_Heal.anim"));

			Timer::AddEvent(1.0f, [effect]() {
				DestroyEntity(effect);
				});
		}
		break;
	}

	case UpgradePreset::SUPPORT:
	{
		auto effect = mOwner->CreateSkeletalMeshEntity(MESH("Skill_Effect_Sup.mesh"), 
			TEXTURE("Power_Effect.png"),
			SKELETON("Skill_Effect_Sup.skel"));
		effect.AddComponent<FollowComponent>(entityID);
		auto& effectTransform = effect.GetComponent<TransformComponent>();
		effectTransform.Position = playerPos;

		auto& effectAnimator = effect.GetComponent<AnimatorComponent>();
		Helpers::PlayAnimation(&effectAnimator, ANIM("Skill_Effect_Sup.anim"));

		Timer::AddEvent(1.0f, [effect]() {
			DestroyEntity(effect);
			});
		break;
	}

	default:
		HB_LOG("Unknown preset: {0}", preset);
		break;
	}
}

void GameScene::createDialogue(Texture* dia, int drawOrder)
{
	static constexpr INT32 dialogueWidth = 1000;
	Entity diag = mOwner->CreateSpriteEntity(dialogueWidth, 250, dia, drawOrder);
	diag.AddTag<Tag_Dialogue>();
	auto& rect = diag.GetComponent<RectTransformComponent>();
	rect.Position = Vector2{ Application::GetScreenWidth() / 2.0f - dialogueWidth / 2.0f, 10.0f };
}

void GameScene::clearDialogue()
{
	DestroyByComponent<Tag_Dialogue>();
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
	case TileType::SCAR_BOSS:
		return TEXTURE("Rail.png");

	case TileType::SCAR_WALL:
		return TEXTURE("Wall.png");

	case TileType::FAT:
		return TEXTURE("Fat.png");

	case TileType::TANK_FAT:
		return TEXTURE("Fat.png");

	case TileType::SCAR:
	case TileType::SCAR_DOG:
		return TEXTURE("Road.png");

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
	case UpgradePreset::ATTACK: return TEXTURE("Skill_1_Slash.png");
	case UpgradePreset::HEAL: return TEXTURE("Skill_2_Heal.png");
	case UpgradePreset::SUPPORT: return TEXTURE("Skill_3_Power.png");
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

void PlayHitSound(const uint8 entityType)
{
	switch (static_cast<EntityType>(entityType))
	{
	case EntityType::VIRUS: 
	{
		SoundManager::StopSound("VirusHit.mp3");
		SoundManager::PlaySound("VirusHit.mp3", 1.0f); 
		break;
	}
	case EntityType::DOG: 
	{
		SoundManager::StopSound("DogHit.mp3");
		SoundManager::PlaySound("DogHit.mp3", 1.0f); 
		break;
	}
	case EntityType::BOSS:
	{
		SoundManager::StopSound("BossHit.mp3");
		SoundManager::PlaySound("BossHit.mp3", 0.15f); 
		break;
	}
	case EntityType::FAT:
	{
		SoundManager::StopSound("FatHit.mp3");
		SoundManager::PlaySound("FatHit.mp3", 1.0f); 
		break;
	}
	default: 
	{
		SoundManager::StopSound("BossHit.mp3");
		SoundManager::PlaySound("BossHit.mp3", 0.15f);
		break;
	}
	}
}
