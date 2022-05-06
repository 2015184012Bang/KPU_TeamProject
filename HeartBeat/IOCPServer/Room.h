#pragma once

class User;

constexpr INT32 ROOM_MAX_USER = 3;

class Room
{
public:
	enum class RoomState
	{
		Waiting,
		Playing,
	};

	bool ExistsFreeSlot();

	void AddUser(User* user);

	void RemoveUser(User* user);

public:
	RoomState GetState() { return mRoomState; }

private:
	RoomState mRoomState = RoomState::Waiting;
	
	list<User*> mUsers;
};

