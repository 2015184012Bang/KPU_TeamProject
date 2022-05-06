#include "pch.h"
#include "Room.h"

#include "User.h"

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
	mUsers.push_back(user);
}

void Room::RemoveUser(User* user)
{
	if (auto iter = find(mUsers.begin(), mUsers.end(), user); iter != mUsers.end())
	{
		mUsers.erase(iter);
	}
	else
	{
		LOG("There is no user named: {0}", user->GetUserName());
	}
}
