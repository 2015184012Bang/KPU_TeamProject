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
		auto room = make_shared<Room>();
		room->Init(i, SendPacketFunction);
		mRooms.push_back(move(room));
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
			packet.NumUsers[i] = static_cast<char>(mRooms[i]->GetCurrentUsers());
		}
		else
		{
			packet.Room[i] = CANNOT;
			packet.NumUsers[i] = static_cast<char>(mRooms[i]->GetCurrentUsers());
		}
	}

	packet.Room[mMaxRoomCount] = END;

	SendPacketFunction(sessionIndex, sizeof(packet), reinterpret_cast<char*>(&packet));
}

shared_ptr<Room>& RoomManager::GetRoom(const INT32 roomIndex)
{
	ASSERT(roomIndex >= 0 && roomIndex < static_cast<INT32>(mMaxRoomCount), "Invalid room index!");
	return mRooms[roomIndex];
}

void RoomManager::Update()
{
	for (auto& room : mRooms)
	{
		if (room->GetState() == Room::RoomState::Playing)
		{
			room->Update();
		}
	}
}
