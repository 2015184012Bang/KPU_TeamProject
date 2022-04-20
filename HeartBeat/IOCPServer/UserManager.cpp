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
	for (UINT32 i = 0; i < maxUserCount; ++i)
	{
		auto user = new User;
		user->Init(i);
		mUsers.push_back(user);
	}
}

void UserManager::AddUser(const string& userID, const INT32 sessionIndex)
{
	mUsers[sessionIndex]->SetLogin(userID);
}

void UserManager::DeleteUser(User* user)
{
	user->Reset();
}

User* UserManager::FindUserByIndex(const INT32 sessionIndex)
{
	return mUsers[sessionIndex];
}
