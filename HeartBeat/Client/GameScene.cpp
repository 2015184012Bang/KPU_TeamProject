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

GameScene::GameScene(Client* owner)
	: Scene(owner)
{

}

void GameScene::Enter()
{
	// 내 캐릭터 알아두기
	mPlayerCharacter = GetEntityByID(mOwner->GetClientID());
	HB_ASSERT(mPlayerCharacter, "Invalid entity!");

	// 맵 생성
	createMap("../Assets/Maps/Map01.csv");
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
		case NOTIFY_MOVE:
			processNotifyMove(packet);
			break;

		case NOTIFY_ATTACK:
			processNotifyAttack(packet);
			break;

		case NOTIFY_DELETE_ENTITY:
			processNotifyDeleteEntity(packet);
			break;

		case NOTIFY_CREATE_ENTITY:
			processNotifyCreateEntity(packet);
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
	// 파괴 가능한 FAT 타일 아래에는 MOVABLE 타일을 생성한다.

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

	// TODO : Fat 타일이 파괴되면 아래 movable 타일을 생성하기
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
	animator.SetTrigger(GetRandomAttackAnimFile());

	if (naPacket->Result == ERROR_CODE::ATTACK_SUCCESS)
	{
		SoundManager::PlaySound("Punch.mp3");
	}
}

void GameScene::processNotifyDeleteEntity(const PACKET& packet)
{
	NOTIFY_DELETE_ENTITY_PACKET* ndePacket = reinterpret_cast<NOTIFY_DELETE_ENTITY_PACKET*>(packet.DataPtr);

	auto e = GetEntityByID(ndePacket->EntityID);
	HB_ASSERT(e, "Invalid entity!");

	switch (static_cast<EntityType>(ndePacket->EntityType))
	{
	case EntityType::FAT:
	{
		auto& animator = e.GetComponent<AnimatorComponent>();
		Helpers::PlayAnimation(&animator, ANIM("Fat_Break.anim"));
		mOwner->DestroyEntityAfter(ndePacket->EntityID, 2.0f);
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
			TEXTURE("Tank.png"), SKELETON("Tank.skel"), ncePacket->EntityID);
		tank.GetComponent<TransformComponent>().Position = ncePacket->Position;
		tank.AddComponent<MovementComponent>(Values::TankSpeed);
		auto& animator = tank.GetComponent<AnimatorComponent>();
		Helpers::PlayAnimation(&animator, ANIM("Tank_Run.anim"));
	}
	break;

	case EntityType::CART:
		break;

	default:
		break;
	}
}

string GetRandomAttackAnimFile(bool isEnemy /*= false*/)
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
		return TEXTURE("Black.png");

	case TileType::MOVABLE:
		return TEXTURE("LightGreen.png");

	case TileType::RAIL:
	case TileType::START_POINT:
	case TileType::END_POINT:
		return TEXTURE("Brown.png");

	case TileType::FAT:
		return TEXTURE("Pink.png");

	case TileType::TANK_FAT:
		return TEXTURE("Orange.png");

	case TileType::SCAR:
		return TEXTURE("Red.png");

	default:
		HB_ASSERT(false, "Unknown tile type!");
		return nullptr;
	}
}
