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
	// 유저 클라이언트 아이디 설정(0~2)
	auto id = mClientIDs.back();
	mClientIDs.pop_back();
	user->SetClientID(id);

	// 유저 룸 인덱스 설정
	user->SetRoomIndex(mRoomIndex);

	// 룸 상태로 설정
	user->SetUserState(User::UserState::IN_ROOM);

	mUsers.push_back(user);
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
