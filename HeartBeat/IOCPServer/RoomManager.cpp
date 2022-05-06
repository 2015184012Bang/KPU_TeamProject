#include "pch.h"
#include "RoomManager.h"

#include "Room.h"
#include "Protocol.h"
#include "User.h"

void RoomManager::Init(const UINT8 maxRoomCount)
{
	mMaxRoomCount = maxRoomCount;

	mRooms.reserve(mMaxRoomCount);
	for (UINT8 i = 0; i < mMaxRoomCount; ++i)
	{
		mRooms.push_back(move(make_shared<Room>()));
	}
}

void RoomManager::SendAvailableRoom(const INT32 sessionIndex)
{
	NOTIFY_ROOM_PACKET packet = {};
	packet.PacketID = NOTIFY_ROOM;
	packet.PacketSize = sizeof(packet);
	
	for (auto i = 0; i < mMaxRoomCount; ++i)
	{
		if (mRooms[i]->GetState() == Room::RoomState::Waiting)
		{
			packet.Room[i] = AVAILABLE;
		}
		else
		{
			packet.Room[i] = CANNOT;
		}
	}

	packet.Room[mMaxRoomCount] = END;

	SendPacketFunction(sessionIndex, sizeof(packet), reinterpret_cast<char*>(&packet));
}

bool RoomManager::CanEnter(const INT32 roomIndex)
{
	auto& room = GetRoom(roomIndex);
	if (room->GetState() == Room::RoomState::Waiting &&
		room->ExistsFreeSlot())
	{
		return true;
	}
	else
	{
		return false;
	}
}

void RoomManager::AddUser(const INT32 roomIndex, User* user)
{
	auto& room = GetRoom(roomIndex);
	room->AddUser(user);
}

shared_ptr<Room>& RoomManager::GetRoom(const INT32 roomIndex)
{
	ASSERT(roomIndex >= 0 && roomIndex < static_cast<INT32>(mMaxRoomCount), "Invalid room index!");
	return mRooms[roomIndex];
}
