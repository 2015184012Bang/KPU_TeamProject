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
	// ���� Ŭ���̾�Ʈ ���̵� ����(0~2)
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
		// Ŭ���̾�Ʈ ���̵� ��ȯ
		mClientIDs.push_back((*iter)->GetClientID());

		// ��ȯ �� �ʱ�ȭ
		(*iter)->SetClientID(-1);
		(*iter)->SetRoomIndex(-1);
		(*iter)->SetUserState(User::UserState::IN_LOBBY);
		mUsers.erase(iter);

		if (mRoomState == RoomState::Waiting_Full)
		{
			mRoomState = RoomState::Waiting;
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
