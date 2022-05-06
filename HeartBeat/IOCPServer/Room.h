#pragma once

#include "Entity.h"

class User;

constexpr INT32 ROOM_MAX_USER = 3;

class Room
{
public:
	void Init(const INT32 index, function<void(INT32, UINT32, char*)> sendFunc);

	enum class RoomState
	{
		Waiting,
		Waiting_Full,
		Playing,
	};

	bool ExistsFreeSlot();

	bool CanEnter();

	void AddUser(User* user);

	void RemoveUser(User* user);

	void Broadcast(const UINT32 packetSize, char* packet);

	void DoEnterUpgrade();

	void NotifyNewbie(User* newbie);

	function<void(INT32, UINT32, char*)> SendPacketFunction;

public:
	RoomState GetState() { return mRoomState; }
	list<User*>& GetUsers() { return mUsers; }

	void SetState(RoomState state) { mRoomState = state; }

private:
	INT32 mRoomIndex = -1;

	RoomState mRoomState = RoomState::Waiting;
	
	list<INT8> mClientIDs;
	list<User*> mUsers;

	entt::registry mRegistry;
};
