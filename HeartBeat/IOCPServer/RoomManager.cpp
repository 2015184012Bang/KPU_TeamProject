#include "pch.h"
#include "RoomManager.h"

#include "Room.h"
#include "Protocol.h"

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
