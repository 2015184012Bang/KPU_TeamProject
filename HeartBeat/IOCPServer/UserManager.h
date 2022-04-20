#pragma once

#include "Protocol.h"
#include "User.h"

class UserManager
{
public:
	~UserManager();

	void Init(const UINT32 maxUserCount);

	void AddUser(const string& userID, const INT32 sessionIndex);

	void DeleteUser(User* user);

	User* FindUserByIndex(const INT32 sessionIndex);

public:
	UINT32 GetCurrentUserCount() { return mCurrentUserCount; }
	UINT32 GetMaxUserCount() { return mMaxUserCount; }

private:
	UINT32 mCurrentUserCount = 0;
	UINT32 mMaxUserCount = 0;

	vector<User*> mUsers;
};

