#include "ClientPCH.h"
#include "GameScene.h"

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

GameScene::GameScene(Client* owner)
	: Scene(owner)
{

}

void GameScene::Enter()
{
	SoundManager::PlaySound("SteampipeSonata.mp3", 0.3f);

	// �� ĳ���� �˾Ƶα�
	mPlayerCharacter = GetEntityByID(mOwner->GetClientID());
	HB_ASSERT(mPlayerCharacter, "Invalid entity!");

	// �þ� ����
	mOwner->SetFollowCameraTarget(mPlayerCharacter, Vector3{ 0.0f, 1500.0f, -1300.0f });

	// �� ����
	createMap("../Assets/Maps/Map01.csv");
}

void GameScene::Exit()
{
	DestroyExclude<Tag_DontDestroyOnLoad>();
}

void GameScene::ProcessInput()
{
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
			processGameOver(packet);
			break;

		default:
			HB_LOG("Unknown packet id: {0}", packet.PacketID);
			break;
		}

		if (mbChangeScene)
		{
			switch (mStageCode)
			{
			case StageCode::FAIL:
				doWhenFail();
				break;

			case StageCode::CLEAR:
				break;

			default:
				HB_ASSERT(false, "Unknown stage code!");
				break;
			}

			// ��Ŷ ó�� �ݺ��� Ż��
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

	// BLOCKED Ÿ���� ���Ʒ�, �� ���� �����Ѵ�.
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
	// �ı� ������ FAT Ÿ�� �Ʒ����� MOVABLE Ÿ���� �����Ѵ�.

	const Texture* fatTex = GetTileTexture(tile.TType);
	const Texture* movableTex = GetTileTexture(TileType::MOVABLE);

	{
		Entity fat = mOwner->CreateSkeletalMeshEntity(MESH("Fat.mesh"), fatTex,
			SKELETON("Fat.skel"));
		fat.AddTag<Tag_Tile>();
		fat.AddComponent<IDComponent>(Values::EntityID++); // FAT Ÿ���� ������ ����ȭ�� �ʿ��ϹǷ� ���̵� �ο�
		auto& transform = fat.GetComponent<TransformComponent>();
		transform.Position.x = tile.X;
		transform.Position.y = 0.0f;
		transform.Position.z = tile.Z;
	}

	// TODO : Fat Ÿ���� �ı��Ǹ� �Ʒ� movable Ÿ���� �����ϱ�
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
	// TANK_FAT Ÿ�� �Ʒ����� RAIL Ÿ���� �����Ѵ�.

	const Texture* tankFatTex = GetTileTexture(tile.TType);
	const Texture* railTex = GetTileTexture(TileType::RAIL);

	{
		Entity fat = mOwner->CreateSkeletalMeshEntity(MESH("Fat.mesh"), tankFatTex,
			SKELETON("Fat.skel"));
		fat.AddTag<Tag_Tile>();
		fat.AddComponent<IDComponent>(Values::EntityID++); // FAT Ÿ���� ������ ����ȭ�� �ʿ��ϹǷ� ���̵� �ο�
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
	obj.AddTag<Tag_Tile>();

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

	if (naPacket->Result == RESULT_CODE::ATTACK_SUCCESS)
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

	// ������ ���� �÷��̾ ����� ���� ���
	const auto& playerID = mPlayerCharacter.GetComponent<IDComponent>().ID;
	if (neaPacket->VictimID == playerID)
	{
		SoundManager::PlaySound("Ouch.mp3", 0.3f);
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
		// TODO : �� ���߽�Ű��
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
		auto& animator = dog.GetComponent<AnimatorComponent>();
		Helpers::PlayAnimation(&animator, ANIM("Dog_Idle.anim"));
	}
	break;

	case EntityType::RED_CELL:
	{
		//TODO : Script ����
		Entity cell = mOwner->CreateSkeletalMeshEntity(MESH("Cell.mesh"),
			TEXTURE("Cell_Red.png"), SKELETON("Cell.skel"), ncePacket->EntityID, "../Assets/Boxes/Cell.box");
		cell.GetComponent<TransformComponent>().Position = ncePacket->Position;
		cell.AddComponent<MovementComponent>(Values::CellSpeed);
		cell.AddTag<Tag_RedCell>();
		auto& animator = cell.GetComponent<AnimatorComponent>();
		Helpers::PlayAnimation(&animator, ANIM("Cell_Run.anim"));
		
		Entity o2 = mOwner->CreateStaticMeshEntity(MESH("O2.mesh"),
			TEXTURE("O2.png"));
		Helpers::AttachBone(cell, o2, "Weapon");
	}
	break;

	default:
		break;
	}
}

void GameScene::processGameOver(const PACKET& packet)
{
	NOTIFY_GAME_OVER_PACKET* ngoPacket = reinterpret_cast<NOTIFY_GAME_OVER_PACKET*>(packet.DataPtr);

	switch (ngoPacket->Result)
	{
	case RESULT_CODE::STAGE_CLEAR:
		HB_ASSERT(false, "Not implemented yet.");
		mbChangeScene = true;
		mStageCode = StageCode::CLEAR;
		break;

	case RESULT_CODE::STAGE_FAIL:
	{
		mbChangeScene = true;
		mStageCode = StageCode::FAIL;
	}
	break;
	}
}

void GameScene::doWhenFail()
{
	// ������� ���� ��ƼƼ ���̵� �ʱ�ȭ
	Values::EntityID = 3;

	auto view = gRegistry.view<Tag_Player>();
	for (auto entity : view)
	{
		Entity player{ entity };

		// �÷��̾���� ������ ����(��Ʈ ����).
		Helpers::DetachBone(player);

		// ���� �ִϸ��̼����� ��ȯ.
		auto& animator = player.GetComponent<AnimatorComponent>();
		const auto& id = player.GetComponent<IDComponent>();
		Helpers::PlayAnimation(&animator, GetCharacterAnimationFile(id.ID, CharacterAnimationType::IDLE_NONE));
	}

	// ���׷��̵� ������ ��ȯ.
	auto newScene = new UpgradeScene{ mOwner };
	newScene->SetDirection(mDirection);
	mOwner->ChangeScene(newScene);
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
		return TEXTURE("Fat.png");

	default:
		HB_ASSERT(false, "Unknown tile type!");
		return nullptr;
	}
}
