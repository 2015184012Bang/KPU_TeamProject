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

void UserManager::AddUser(const INT32 sessionIndex, string_view userName /*= "default"sv*/)
{
	ASSERT((UINT32)sessionIndex >= 0 && (UINT32)sessionIndex < mMaxUserCount, "Invalid session index!");

	mUsers[sessionIndex]->SetLogin(userName);
	++mCurrentUserCount;
}

void UserManager::DeleteUser(User* user)
{
	ASSERT(user, "user is nullptr");
	user->Reset();
	--mCurrentUserCount;
}

User* UserManager::GetUserByIndex(const INT32 sessionIndex)
{
	ASSERT((UINT32)sessionIndex >= 0 && (UINT32)sessionIndex < mMaxUserCount, "Invalid session index!");
	return mUsers[sessionIndex];
}

vector<INT32> UserManager::GetAllConnectedUsersIndex()
{
	vector<INT32> indices;
	for (auto user : mUsers)
	{
		if (user->IsConnected())
		{
			indices.push_back(user->GetIndex());
		}
	}

	return indices;
}

UserManager gUserManager;