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
