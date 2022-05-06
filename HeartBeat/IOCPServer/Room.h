#pragma once

class User;

constexpr INT32 ROOM_MAX_USER = 3;

class Room
{
public:
	void Init(const INT32 index, function<void(INT32, UINT32, char*)> sendFunc);

	enum class RoomState
	{
		Waiting,
		Playing,
	};

	bool ExistsFreeSlot();

	void AddUser(User* user);

	void RemoveUser(User* user);

	void Broadcast(const UINT32 packetSize, char* packet);

	function<void(INT32, UINT32, char*)> SendPacketFunction;

public:
	RoomState GetState() { return mRoomState; }

private:
	INT32 mRoomIndex = -1;

	RoomState mRoomState = RoomState::Waiting;
	
	list<INT8> mClientIDs;
	list<User*> mUsers;
};

