#include "pch.h"
#include "Room.h"

#include "User.h"


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

bool Room::ExistsFreeSlot()
{
	if (mUsers.size() < ROOM_MAX_USER)
	{
		return true;
	}

	return false;
}

bool Room::CanEnter()
{
	if (mRoomState == RoomState::Waiting && ExistsFreeSlot())
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
		// 클라이언트 아이디 반환
		mClientIDs.push_back((*iter)->GetClientID());

		// 반환 후 초기화
		(*iter)->SetClientID(-1);
		(*iter)->SetRoomIndex(-1);
		(*iter)->SetUserState(User::UserState::IN_LOBBY);
		mUsers.erase(iter);

		if (mRoomState == RoomState::Waiting_Full)
		{
			mRoomState = RoomState::Waiting;
		}

		// 방 안의 모든 유저가 나가면 룸 상태를 Waiting으로 변경하고
		// 레지스트리를 초기화한다.
		if (mUsers.size() == 0)
		{
			mRoomState = RoomState::Waiting;
			mRegistry.clear();
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

	NOTIFY_ENTER_UPGRADE_PACKET ansPacket;
	ansPacket.PacketID = NOTIFY_ENTER_UPGRADE;
	ansPacket.PacketSize = sizeof(NOTIFY_ENTER_UPGRADE_PACKET);
	ansPacket.Result = RESULT_CODE::SUCCESS;
	Broadcast(sizeof(ansPacket), reinterpret_cast<char*>(&ansPacket));
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

void Room::createSystems()
{
	mMovementSystem = make_unique<MovementSystem>(mRegistry, shared_from_this());
	mScriptSystem = make_unique<ScriptSystem>(mRegistry, shared_from_this());
	mEnemySystem = make_unique<EnemySystem>(mRegistry, shared_from_this());
	mCombatSystem = make_unique<CombatSystem>(mRegistry, shared_from_this());
	mCollisionSystem = make_unique<CollisionSystem>(mRegistry, shared_from_this());
}
