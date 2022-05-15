#include "pch.h"
#include "Room.h"

#include "Cart.h"
#include "User.h"
#include "Values.h"
#include "Tags.h"
#include "Tank.h"
#include "Random.h"
#include "RedCell.h"

void Room::Init(const INT32 index, function<void(INT32, UINT32, char*)> sendFunc)
{
	mRoomIndex = index;
	SendPacketFunction = sendFunc;

	for (auto i = 0; i < ROOM_MAX_USER; ++i)
	{
		mClientIDs.push_back(i);
	}

	createSystems();
}

bool Room::existsFreeSlot()
{
	if (mUsers.size() < ROOM_MAX_USER)
	{
		return true;
	}

	return false;
}

bool Room::CanEnter()
{
	if (mRoomState == RoomState::Waiting && existsFreeSlot())
	{
		return true;
	}
	else
	{
		return false;
	}
}

void Room::AddUser(User* user)
{
	// ���� Ŭ���̾�Ʈ ���̵� ����(0~2)
	// HOST_ID(2)�� ���� �ο��ǵ��� �ϱ� ���� �����Ѵ�.
	mClientIDs.sort();
	auto id = mClientIDs.back();
	mClientIDs.pop_back();

	user->SetClientID(id);

	// ���� �� �ε��� ����
	user->SetRoomIndex(mRoomIndex);

	// �� ���·� ����
	user->SetUserState(User::UserState::IN_ROOM);

	mUsers.push_back(user);

	if (mUsers.size() == ROOM_MAX_USER)
	{
		mRoomState = RoomState::Waiting_Full;
	}
}

void Room::RemoveUser(User* user)
{
	if (auto iter = find(mUsers.begin(), mUsers.end(), user); iter != mUsers.end())
	{
		auto erasedClientID = (*iter)->GetClientID();

		// Ŭ���̾�Ʈ ���̵� ��ȯ
		mClientIDs.push_back((*iter)->GetClientID());

		// ��ȯ �� �ʱ�ȭ
		(*iter)->SetClientID(-1);
		(*iter)->SetRoomIndex(-1);
		(*iter)->SetUserState(User::UserState::IN_LOBBY);
		mUsers.erase(iter);

		if (mRoomState == RoomState::Playing)
		{
			// ������ �����鿡�� ��ƼƼ ���� ��Ŷ �۽�
			NOTIFY_DELETE_ENTITY_PACKET packet = {};
			packet.EntityID = erasedClientID;
			packet.EntityType = static_cast<UINT8>(EntityType::PLAYER);
			packet.PacketID = NOTIFY_DELETE_ENTITY;
			packet.PacketSize = sizeof(packet);
			Broadcast(sizeof(packet), reinterpret_cast<char*>(&packet));
		}

		if (mRoomState == RoomState::Waiting_Full)
		{
			mRoomState = RoomState::Waiting;
		}

		// �� ���� ��� ������ ������ ���� ���¸� �ʱ�ȭ�Ѵ�.
		if (mUsers.size() == 0 && mRoomState == RoomState::Playing)
		{
			mRoomState = RoomState::Waiting;
			clearGame();
		}
	}
	else
	{
		LOG("There is no user named: {0}", user->GetUserName());
	}
}

void Room::Broadcast(const UINT32 packetSize, char* packet)
{
	for (auto user : mUsers)
	{
		SendPacketFunction(user->GetIndex(), packetSize, packet);
	}
}

void Room::DoEnterUpgrade()
{
	if (mRoomState == RoomState::Playing)
	{
		LOG("This room is now playing!");
		return;
	}

	SetState(RoomState::Playing);

	// �÷��̾� ��ƼƼ ����
	for (auto user : mUsers)
	{
		user->SetRegistry(&mRegistry);
		user->CreatePlayerEntity();
	}

	NOTIFY_ENTER_UPGRADE_PACKET ansPacket = {};
	ansPacket.PacketID = NOTIFY_ENTER_UPGRADE;
	ansPacket.PacketSize = sizeof(NOTIFY_ENTER_UPGRADE_PACKET);
	ansPacket.Result = RESULT_CODE::SUCCESS;
	Broadcast(sizeof(ansPacket), reinterpret_cast<char*>(&ansPacket));
}

void Room::DoEnterGame()
{
	createTiles("../Assets/Maps/Map01.csv");

	createTankAndCart();

	createCells();

	// �÷��̾���� ���� ��ġ�� �����ϰ� ����
	mMovementSystem->SetPlayersStartPos();

	// �������� ���� �о �� ���� ����
	mEnemySystem->LoadStageFile("../Assets/Stages/Stage1.csv");
	mEnemySystem->SetGenerate(true);

	// �浹 üũ ����
	mCollisionSystem->SetStart(true);
}

void Room::NotifyNewbie(User* newbie)
{
	NOTIFY_ENTER_ROOM_PACKET nerPacket = {};
	nerPacket.ClientID = newbie->GetClientID();
	nerPacket.PacketID = NOTIFY_ENTER_ROOM;
	nerPacket.PacketSize = sizeof(nerPacket);

	// ���� �����鿡�� �� ������ ������ �˸�
	for (auto user : mUsers)
	{
		if (newbie == user)
		{
			continue;
		}

		SendPacketFunction(user->GetIndex(), sizeof(nerPacket), reinterpret_cast<char*>(&nerPacket));
	}

	// �� �������� ���� �������� �˸�
	for (auto user : mUsers)
	{
		if (newbie == user)
		{
			continue;
		}

		nerPacket.ClientID = user->GetClientID();
		SendPacketFunction(newbie->GetIndex(), sizeof(nerPacket), reinterpret_cast<char*>(&nerPacket));
	}
}

void Room::SetDirection(const INT8 clientID, const Vector3& direction)
{
	mMovementSystem->SetDirection(clientID, direction);
}

void Room::Update()
{
	mScriptSystem->Update();
	mCombatSystem->Update();
	mMovementSystem->Update();
	mCollisionSystem->Update();
	mEnemySystem->Update();
	mPathSystem->Update();
}

void Room::SetPreset(const INT8 clientID, UpgradePreset preset)
{
	mCombatSystem->SetPreset(clientID, preset);
}

bool Room::CanBaseAttack(const INT8 clientID)
{
	return mCombatSystem->CanBaseAttack(clientID);
}

bool Room::DoAttack(const INT8 clientID)
{
	return mCollisionSystem->DoAttack(clientID);
}

void Room::DoSkill(const INT8 clientID)
{
	if (!mCombatSystem->CanUseSkill(clientID))
	{
		return;
	}

	auto player = GetEntityByID(mRegistry, clientID);
	const auto& combat = mRegistry.get<CombatComponent>(player);

	NOTIFY_SKILL_PACKET packet = {};
	packet.EntityID = clientID;
	packet.PacketID = NOTIFY_SKILL;
	packet.PacketSize = sizeof(packet);
	packet.Preset = static_cast<UINT8>(combat.Preset);
	Broadcast(sizeof(packet), reinterpret_cast<char*>(&packet));

	switch (combat.Preset)
	{
	case UpgradePreset::ATTACK:
		mCollisionSystem->DoWhirlwind(clientID);
		break;

	case UpgradePreset::HEAL:
		//mCombatSystem->DoHeal();
		break;

	case UpgradePreset::SUPPORT:
		//mCombatSystem->DoBuff();
		break;
		
	default:
		ASSERT(false, "Unknown upgrade preset!");
	}
}

void Room::ChangeTileToRoad(INT32 row, INT32 col)
{
	mPathSystem->ChangeTileToRoad(row, col);
}

void Room::createSystems()
{
	mMovementSystem = make_unique<MovementSystem>(mRegistry, shared_from_this());
	mScriptSystem = make_unique<ScriptSystem>(mRegistry, shared_from_this());
	mEnemySystem = make_unique<EnemySystem>(mRegistry, shared_from_this());
	mCombatSystem = make_unique<CombatSystem>(mRegistry, shared_from_this());
	mCollisionSystem = make_unique<CollisionSystem>(mRegistry, shared_from_this());
	mPathSystem = make_unique<PathSystem>(mRegistry, shared_from_this());
}

void Room::createTiles(string_view fileName)
{
	const auto& gameMap = GameMap::GetInstance().GetMap(fileName);

	mCollisionSystem->SetBorder(Vector3{ (gameMap.MaxCol - 1) * Values::TileSide, 0.0f, (gameMap.MaxRow - 1) * Values::TileSide });

	for (const auto& tile : gameMap.Tiles)
	{
		auto obj = mRegistry.create();
		addTagToTile(obj, tile.TType);

		auto& transform = mRegistry.emplace<TransformComponent>(obj);
		transform.Position = Vector3{ tile.X, GetTileYPos(tile.TType), tile.Z };

		// �浹 ó���� �ʿ��� Ÿ���� ��쿡�� BoxComponent�� ����
		if (tile.TType == TileType::BLOCKED ||
			tile.TType == TileType::FAT ||
			tile.TType == TileType::TANK_FAT)
		{
			mRegistry.emplace<BoxComponent>(obj, &Box::GetBox("../Assets/Boxes/Cube.box"), transform.Position, transform.Yaw);
		}
		else if (tile.TType == TileType::HOUSE)
		{
			// ��� ���޼��� ���, ����Ʈ ������ +z���� ���ϰ� �ִ�.
			// Ŭ�󿡼� 180�� �����Ƿ� ���������� �����ش�.
			transform.Yaw = 180.0f;
			mRegistry.emplace<BoxComponent>(obj, &Box::GetBox("../Assets/Boxes/House.box"), transform.Position, transform.Yaw);
		}

		// TANK_FAT Ÿ���� ��쿡�� �Ʒ� �ʿ� RAIL_TILE�� ��ƾ� 
		// ��ũ�� ��� �ν��� �����ϴ�.
		if (tile.TType == TileType::TANK_FAT)
		{
			auto rail = mRegistry.create();
			addTagToTile(rail, TileType::RAIL);

			auto& railTransform = mRegistry.emplace<TransformComponent>(rail);
			railTransform.Position = { tile.X, GetTileYPos(TileType::RAIL), tile.Z };
		}
	}
}

void Room::createTankAndCart()
{
	NOTIFY_CREATE_ENTITY_PACKET packet = {};
	packet.PacketSize = sizeof(packet);
	packet.PacketID = NOTIFY_CREATE_ENTITY;

	// ��ũ ����
	{
		auto tank = mRegistry.create();

		auto& id = mRegistry.emplace<IDComponent>(tank, GetEntityID());
		mRegistry.emplace<NameComponent>(tank, "Tank");
		auto& transform = mRegistry.emplace<TransformComponent>(tank);
		mRegistry.emplace<MovementComponent>(tank, Vector3::Zero, Values::TankSpeed);
		mRegistry.emplace<BoxComponent>(tank, &Box::GetBox("../Assets/Boxes/Tank.box"),
			transform.Position, transform.Yaw);
		mRegistry.emplace<HealthComponent>(tank, Values::TankHealth);
		mRegistry.emplace<ScriptComponent>(tank, make_shared<Tank>(mRegistry, tank));
		mRegistry.emplace<Tag_Tank>(tank);

		packet.EntityID = id.ID;
		packet.EntityType = static_cast<UINT8>(EntityType::TANK);
		packet.Position = transform.Position;

		Broadcast(sizeof(packet), reinterpret_cast<char*>(&packet));
	}

	// īƮ ����
	{
		auto cart = mRegistry.create();

		auto& id = mRegistry.emplace<IDComponent>(cart, GetEntityID());
		mRegistry.emplace<NameComponent>(cart, "Cart");
		auto& transform = mRegistry.emplace<TransformComponent>(cart);
		mRegistry.emplace<MovementComponent>(cart, Vector3::Zero, Values::TankSpeed);
		mRegistry.emplace<BoxComponent>(cart, &Box::GetBox("../Assets/Boxes/Cart.box"),
			transform.Position, transform.Yaw);
		mRegistry.emplace<ScriptComponent>(cart, make_shared<Cart>(mRegistry, cart));
		mRegistry.emplace<Tag_Cart>(cart);

		packet.EntityID = id.ID;
		packet.EntityType = static_cast<UINT8>(EntityType::CART);
		packet.Position = transform.Position;

		Broadcast(sizeof(packet), reinterpret_cast<char*>(&packet));
	}
}

void Room::createCells()
{
	NOTIFY_CREATE_ENTITY_PACKET packet = {};
	packet.PacketSize = sizeof(packet);
	packet.PacketID = NOTIFY_CREATE_ENTITY;

	for (INT32 i = 0; i < MAX_CELL_COUNT; ++i)
	{
		auto cell = mRegistry.create();

		auto& id = mRegistry.emplace<IDComponent>(cell, GetEntityID());
		auto& transform = mRegistry.emplace<TransformComponent>(cell);
		transform.Position = getCellStartPosition(i);
		mRegistry.emplace<MovementComponent>(cell, Vector3::Zero, Values::CellSpeed);
		mRegistry.emplace<BoxComponent>(cell, &Box::GetBox("../Assets/Boxes/Cell.box"),
			transform.Position, transform.Yaw);
		mRegistry.emplace<Tag_RedCell>(cell);
		mRegistry.emplace<PathFindComponent>(cell);
		mRegistry.emplace<ScriptComponent>(cell, make_shared<RedCell>(mRegistry, cell));
		mRegistry.emplace<HealthComponent>(cell, Values::CellHealth);

		packet.EntityID = id.ID;
		packet.EntityType = static_cast<UINT8>(EntityType::RED_CELL);
		packet.Position = transform.Position;

		Broadcast(sizeof(packet), reinterpret_cast<char*>(&packet));
	}
}

void Room::addTagToTile(entt::entity tile, TileType ttype)
{
	switch (ttype)
	{
	case TileType::BLOCKED:
		mRegistry.emplace<Tag_Tile>(tile);
		mRegistry.emplace<Tag_BlockingTile>(tile);
		break;

	case TileType::FAT:
	case TileType::TANK_FAT:
		mRegistry.emplace<Tag_Tile>(tile);
		mRegistry.emplace<Tag_BlockingTile>(tile);
		mRegistry.emplace<Tag_BreakableTile>(tile);
		mRegistry.emplace<HealthComponent>(tile, Random::RandInt(1, 3));
		mRegistry.emplace<IDComponent>(tile, GetEntityID());
		break;

	case TileType::MOVABLE:
	case TileType::SCAR:
		mRegistry.emplace<Tag_Tile>(tile);
		break;

	case TileType::RAIL:
		mRegistry.emplace<Tag_Tile>(tile);
		mRegistry.emplace<Tag_RailTile>(tile);
		break;

	case TileType::START_POINT:
		mRegistry.emplace<Tag_Tile>(tile);
		mRegistry.emplace<Tag_RailTile>(tile);
		mRegistry.emplace<NameComponent>(tile, "StartPoint");
		break;

	case TileType::END_POINT:
		mRegistry.emplace<Tag_Tile>(tile);
		mRegistry.emplace<Tag_RailTile>(tile);
		mRegistry.emplace<NameComponent>(tile, "EndPoint");
		break;

	case TileType::HOUSE:
		mRegistry.emplace<Tag_Tile>(tile);
		mRegistry.emplace<Tag_HouseTile>(tile);
		break;

	default:
		ASSERT(false, "Unknown tile type!");
		break;
	}
}

void Room::clearGame()
{
	mEntityID = 3;	// Entity ID �ʱ�ȭ
	mEnemySystem->SetGenerate(false);
	mCollisionSystem->SetStart(false);
	mRoomState = RoomState::Waiting;
	mRegistry.clear();
}

Vector3 Room::getCellStartPosition(INT32 index)
{
	auto startPoint = GetEntityByName(mRegistry, "StartPoint");
	ASSERT(mRegistry.valid(startPoint), "Invalid entity!");

	const auto& startPosition = mRegistry.get<TransformComponent>(startPoint).Position;
	auto cellStartPosition = Vector3{ 0.0f, 0.0f, startPosition.z };

	switch (index)
	{
	case 0:
		cellStartPosition.z += 400.0f;
		return cellStartPosition;

	case 1:
		cellStartPosition.x += 400.0f;
		cellStartPosition.z += 400.0f;
		return cellStartPosition;

	case 2:
		cellStartPosition.x += 800.0f;
		cellStartPosition.z += 400.0f;
		return cellStartPosition;
		
	case 3:
		cellStartPosition.z -= 400.0f;
		return cellStartPosition;

	case 4:
		cellStartPosition.x += 400.0f;
		cellStartPosition.z -= 400.0f;
		return cellStartPosition;

	case 5:
		cellStartPosition.x += 800.0f;
		cellStartPosition.z -= 400.0f;
		return cellStartPosition;

	default:
		LOG("Unknown cell index: {0}", index);
		return Vector3::Zero;
	}
}

float GetTileYPos(TileType ttype)
{
	switch (ttype)
	{
	case TileType::BLOCKED:
	case TileType::FAT:
	case TileType::TANK_FAT:
	case TileType::HOUSE:
		return 0.0f;

	case TileType::MOVABLE:
	case TileType::RAIL:
	case TileType::SCAR:
	case TileType::START_POINT:
	case TileType::END_POINT:
		return -Values::TileSide;

	default:
		ASSERT(false, "Unknown tile type!");
		return 0.0f;
	}
}
