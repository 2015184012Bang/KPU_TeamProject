#include "pch.h"
#include "Room.h"

#include "User.h"
#include "Values.h"
#include "Tags.h"
#include "Tank.h"
#include "Random.h"

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
	// 유저 클라이언트 아이디 설정(0~2)
	// HOST_ID(2)가 먼저 부여되도록 하기 위해 정렬한다.
	mClientIDs.sort();
	auto id = mClientIDs.back();
	mClientIDs.pop_back();

	user->SetClientID(id);

	// 유저 룸 인덱스 설정
	user->SetRoomIndex(mRoomIndex);

	// 룸 상태로 설정
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

		// 클라이언트 아이디 반환
		mClientIDs.push_back((*iter)->GetClientID());

		// 반환 후 초기화
		(*iter)->SetClientID(-1);
		(*iter)->SetRoomIndex(-1);
		(*iter)->SetUserState(User::UserState::IN_LOBBY);
		mUsers.erase(iter);

		if (mRoomState == RoomState::Playing)
		{
			// 나머지 유저들에게 엔티티 삭제 패킷 송신
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

		// 방 안의 모든 유저가 나가면 게임 상태를 초기화한다.
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

	// 플레이어 엔티티 생성
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

	// 플레이어들의 시작 위치를 랜덤하게 설정
	mMovementSystem->SetPlayersStartPos();

	// 스테이지 파일 읽어서 적 생성 시작
	mEnemySystem->LoadStageFile("../Assets/Stages/Stage1.csv");
	mEnemySystem->SetGenerate(true);

	// 충돌 체크 시작
	mCollisionSystem->SetStart(true);
}

void Room::NotifyNewbie(User* newbie)
{
	NOTIFY_ENTER_ROOM_PACKET nerPacket = {};
	nerPacket.ClientID = newbie->GetClientID();
	nerPacket.PacketID = NOTIFY_ENTER_ROOM;
	nerPacket.PacketSize = sizeof(nerPacket);

	// 기존 유저들에게 새 유저의 접속을 알림
	for (auto user : mUsers)
	{
		if (newbie == user)
		{
			continue;
		}

		SendPacketFunction(user->GetIndex(), sizeof(nerPacket), reinterpret_cast<char*>(&nerPacket));
	}

	// 새 유저에게 기존 유저들을 알림
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
}

void Room::SetPreset(const INT8 clientID, CombatSystem::UpgradePreset preset)
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

void Room::createSystems()
{
	mMovementSystem = make_unique<MovementSystem>(mRegistry, shared_from_this());
	mScriptSystem = make_unique<ScriptSystem>(mRegistry, shared_from_this());
	mEnemySystem = make_unique<EnemySystem>(mRegistry, shared_from_this());
	mCombatSystem = make_unique<CombatSystem>(mRegistry, shared_from_this());
	mCollisionSystem = make_unique<CollisionSystem>(mRegistry, shared_from_this());
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

		// 충돌 처리가 필요한 타일의 경우에만 BoxComponent를 부착
		if (tile.TType == TileType::BLOCKED ||
			tile.TType == TileType::FAT ||
			tile.TType == TileType::TANK_FAT)
		{
			mRegistry.emplace<BoxComponent>(obj, &Box::GetBox("../Assets/Boxes/Cube.box"), transform.Position, transform.Yaw);
		}
		else if (tile.TType == TileType::HOUSE)
		{
			// 산소 공급소의 경우, 디폴트 방향이 +z축을 향하고 있다.
			// 클라에서 180도 돌리므로 서버에서도 돌려준다.
			transform.Yaw = 180.0f;
			mRegistry.emplace<BoxComponent>(obj, &Box::GetBox("../Assets/Boxes/House.box"), transform.Position, transform.Yaw);
		}

		// TANK_FAT 타일의 경우에는 아래 쪽에 RAIL_TILE을 깔아야 
		// 탱크가 경로 인식이 가능하다.
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

	NOTIFY_CREATE_ENTITY_PACKET packet = {};
	packet.EntityID = id.ID;
	packet.EntityType = static_cast<UINT8>(EntityType::TANK);
	packet.PacketID = NOTIFY_CREATE_ENTITY;
	packet.PacketSize = sizeof(packet);
	packet.Position = transform.Position;

	Broadcast(sizeof(packet), reinterpret_cast<char*>(&packet));
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
	mEntityID = 3;	// Entity ID 초기화
	mEnemySystem->SetGenerate(false);
	mCollisionSystem->SetStart(false);
	mRoomState = RoomState::Waiting;
	mRegistry.clear();
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