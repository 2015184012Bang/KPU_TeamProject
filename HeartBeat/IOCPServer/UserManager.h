#pragma once

#include "Protocol.h"
#include "User.h"

class UserManager
{
public:
	~UserManager();

	void Init(const UINT32 maxUserCount);

	void AddUser(const INT32 sessionIndex, string_view userName = "default"sv);

	void DeleteUser(User* user);

	User* GetUserByIndex(const INT32 sessionIndex);

	vector<INT32> GetAllConnectedUsersIndex();

public:
	UINT32 GetCurrentUserCount() { return mCurrentUserCount; }
	UINT32 GetMaxUserCount() { return mMaxUserCount; }

private:
	UINT32 mCurrentUserCount = 0;
	UINT32 mMaxUserCount = 0;

	vector<User*> mUsers;
};
