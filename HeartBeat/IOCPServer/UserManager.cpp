#include "pch.h"
#include "UserManager.h"

UserManager::~UserManager()
{
	for (auto user : mUsers)
	{
		delete user;
	}
}

void UserManager::Init(const UINT32 maxUserCount)
{
	mMaxUserCount = maxUserCount;

	for (UINT32 i = 0; i < maxUserCount; ++i)
	{
		auto user = new User;
		user->Init(i);
		mUsers.push_back(user);
	}
}


void UserManager::AddUser(const INT32 sessionIndex, string_view userID /*= "default"sv*/)
{
	ASSERT(sessionIndex >= 0 && sessionIndex < mMaxUserCount, "Invalid session index!");

	mUsers[sessionIndex]->SetLogin(userID);
	++mCurrentUserCount;
}

void UserManager::DeleteUser(User* user)
{
	ASSERT(user, "user is nullptr");
	user->Reset();
	--mCurrentUserCount;
}

User* UserManager::FindUserByIndex(const INT32 sessionIndex)
{
	ASSERT(sessionIndex >= 0 && sessionIndex < mMaxUserCount, "Invalid session index!");
	return mUsers[sessionIndex];
}
