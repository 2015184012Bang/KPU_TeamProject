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

void Room::Update()
{
	if (bGameStart)
	{
		mPlayTimeSec += Timer::GetDeltaTime();
	}

	mScriptSystem->Update();
	mCombatSystem->Update();
	mMovementSystem->Update();
	mCollisionSystem->Update();
	mEnemySystem->Update();
	mPathSystem->Update();

	checkGameState();
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

void Room::DoEnterRoom(User* user)
{
	if (!canEnterRoom())
	{
		return;
	}

	AddUser(user);

	ANSWER_ENTER_ROOM_PACKET packet = {};
	packet.PacketID = ANSWER_ENTER_ROOM;
	packet.PacketSize = sizeof(packet);
	packet.ClientID = user->GetClientID();
	packet.Result = RESULT_CODE::ROOM_ENTER_SUCCESS;
	SendPacketFunction(user->GetIndex(), sizeof(packet), reinterpret_cast<char*>(&packet));

	notifyNewbie(user);
}

void Room::DoLeaveRoom(User* user)
{
	NOTIFY_LEAVE_ROOM_PACKET nlrPacket = {};
	nlrPacket.ClientID = user->GetClientID();
	nlrPacket.PacketID = NOTIFY_LEAVE_ROOM;
	nlrPacket.PacketSize = sizeof(nlrPacket);

	Broadcast(sizeof(nlrPacket), reinterpret_cast<char*>(&nlrPacket));

	RemoveUser(user);
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

	// 기본 장비 프리셋 설정(ATTACK)
	for (auto user : mUsers)
	{
		DoSetPreset(user, UpgradePreset::ATTACK);
	}
}

void Room::DoEnterGame()
{
	bGameStart = true;

	NOTIFY_ENTER_GAME_PACKET negPacket = {};
	negPacket.PacketID = NOTIFY_ENTER_GAME;
	negPacket.PacketSize = sizeof(negPacket);
	negPacket.Result = RESULT_CODE::SUCCESS;
	Broadcast(sizeof(negPacket), reinterpret_cast<char*>(&negPacket));

	createTiles("../Assets/Maps/Map01.csv");

	createTankAndCart();

	createCells();

	// 게임 상태(산소, 이산화탄소 정보 등등)를 기록할 매니저 생성
	createGameState();

	// 플레이어들의 시작 위치를 랜덤하게 설정
	mMovementSystem->SetPlayersStartPos();

	// 스테이지 파일 읽어서 적 생성 시작
	mEnemySystem->LoadStageFile("../Assets/Stages/Stage1.csv");
	mEnemySystem->SetGenerate(true);

	// 충돌 체크 시작
	mCollisionSystem->SetStart(true);

	mCombatSystem->Start();
}

void Room::DoSetDirection(User* user, const Vector3& direction)
{
	auto clientID = user->GetClientID();

	mMovementSystem->SetDirection(clientID, direction);

	auto entity = GetEntityByID(mRegistry, clientID);
	ASSERT(mRegistry.valid(entity), "Invalid entity!");

	NOTIFY_MOVE_PACKET anmPacket = {};
	anmPacket.PacketID = NOTIFY_MOVE;
	anmPacket.PacketSize = sizeof(NOTIFY_MOVE_PACKET);
	anmPacket.Direction = user->GetMoveDirection();
	anmPacket.Position = user->GetPosition();
	anmPacket.EntityID = clientID;
	Broadcast(sizeof(anmPacket), reinterpret_cast<char*>(&anmPacket));
}

void Room::DoSetPreset(User* user, UpgradePreset preset)
{
	auto clientID = user->GetClientID();

	mCombatSystem->SetPreset(clientID, preset);

	NOTIFY_UPGRADE_PACKET nuPacket = {};
	nuPacket.PacketID = NOTIFY_UPGRADE;
	nuPacket.PacketSize = sizeof(nuPacket);
	nuPacket.EntityID = clientID;
	nuPacket.UpgradePreset = static_cast<UINT8>(preset);
	Broadcast(sizeof(nuPacket), reinterpret_cast<char*>(&nuPacket));
}

void Room::DoAttack(User* user)
{
	auto clientID = user->GetClientID();

	if (!mCombatSystem->CanBaseAttack(clientID))
	{
		return;
	}

	bool bHit = mCollisionSystem->CheckAttackHit(clientID);

	NOTIFY_ATTACK_PACKET packet = {};
	packet.EntityID = clientID;
	packet.PacketID = NOTIFY_ATTACK;
	packet.PacketSize = sizeof(packet);
	packet.Result = bHit ? RESULT_CODE::ATTACK_SUCCESS : RESULT_CODE::ATTACK_MISS;
	Broadcast(sizeof(packet), reinterpret_cast<char*>(&packet));
}

void Room::DoSkill(User* user)
{
	auto clientID = user->GetClientID();

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
		mCombatSystem->DoHeal(clientID);
		break;

	case UpgradePreset::SUPPORT:
		mCombatSystem->DoBuff(clientID);
		break;
		
	default:
		ASSERT(false, "Unknown upgrade preset!");
	}
}

void Room::DoGameOver()
{
	NOTIFY_GAME_OVER_PACKET packet = {};
	packet.CO2 = mRegistry.get<PlayStateComponent>(mPlayState).CO2;
	packet.O2 = mRegistry.get<PlayStateComponent>(mPlayState).O2;
	packet.PlayTimeSec = static_cast<UINT64>(mPlayTimeSec);
	packet.PacketID = NOTIFY_GAME_OVER;
	packet.PacketSize = sizeof(packet);
	Broadcast(packet.PacketSize, reinterpret_cast<char*>(&packet));

	clearAllUser();
	clearGame();
}

void Room::ChangeTileToRoad(INT32 row, INT32 col)
{
	mPathSystem->ChangeTileToRoad(row, col);
}

void Room::checkGameState()
{
	if (!mRegistry.valid(mPlayState))
	{
		return;
	}

	auto& gameState = mRegistry.get<PlayStateComponent>(mPlayState);
	if (gameState.bChanged)
	{
		gameState.bChanged = false;

		NOTIFY_STATE_CHANGE_PACKET packet = {};
		packet.CO2 = gameState.CO2;
		packet.O2 = gameState.O2;
		packet.TankHealth = gameState.TankHealth;
		packet.PacketID = NOTIFY_STATE_CHANGE;
		packet.PacketSize = sizeof(packet);
		Broadcast(packet.PacketSize, reinterpret_cast<char*>(&packet));
	}
}

bool Room::canEnterRoom()
{
	if (mRoomState == RoomState::Waiting && mUsers.size() < ROOM_MAX_USER)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void Room::notifyNewbie(User* newbie)
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
	NOTIFY_CREATE_ENTITY_PACKET packet = {};
	packet.PacketSize = sizeof(packet);
	packet.PacketID = NOTIFY_CREATE_ENTITY;

	// 탱크 생성
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

	// 카트 생성
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

void Room::createGameState()
{
	mPlayState = mRegistry.create();
	mRegistry.emplace<NameComponent>(mPlayState, "PlayState");
	auto& state = mRegistry.emplace<PlayStateComponent>(mPlayState);
	state.P0HP = Values::PlayerHealth;
	state.P1HP = Values::PlayerHealth;
	state.P2HP = Values::PlayerHealth;
	state.TankHealth = Values::TankHealth;
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

void Room::clearAllUser()
{
	while (!mUsers.empty())
	{
		auto user = mUsers.front();
		auto clientID = user->GetClientID();
		mClientIDs.push_back(clientID);
		user->SetClientID(-1);
		user->SetRoomIndex(-1);
		user->SetUserState(User::UserState::IN_LOBBY);
		mUsers.pop_front();
	}
}

void Room::clearGame()
{
	mEntityID = 3;	// Entity ID 초기화
	mEnemySystem->SetGenerate(false);
	mCollisionSystem->SetStart(false);
	mPathSystem->ResetGraph();
	mRoomState = RoomState::Waiting;
	mPlayTimeSec = 0.0f;
	bGameStart = false;
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
