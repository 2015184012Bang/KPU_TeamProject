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

void RoomManager::RemoveUser(User* user)
{
	// ������ �濡 �� ���� �ʴٸ� ����
	if (user->GetRoomIndex() == -1)
	{
		return;
	}

	auto roomIndex = user->GetRoomIndex();
	auto& room = GetRoom(roomIndex);
	room->RemoveUser(user);
}

shared_ptr<Room>& RoomManager::GetRoom(const INT32 roomIndex)
{
	ASSERT(roomIndex >= 0 && roomIndex < static_cast<INT32>(mMaxRoomCount), "Invalid room index!");
	return mRooms[roomIndex];
}

void RoomManager::Broadcast(const INT32 roomIndex, const UINT32 packetSize, char* packet)
{
	auto& room = GetRoom(roomIndex);
	room->Broadcast(packetSize, packet);
}

void RoomManager::NotifyNewbie(const INT32 roomIndex, User* newbie)
{
	auto& room = GetRoom(roomIndex);

	NOTIFY_ENTER_ROOM_PACKET nerPacket = {};
	nerPacket.ClientID = newbie->GetClientID();
	nerPacket.PacketID = NOTIFY_ENTER_ROOM;
	nerPacket.PacketSize = sizeof(nerPacket);

	// ���� �����鿡�� �� ������ ������ �˸�
	for (auto user : room->GetUsers())
	{
		if (newbie == user)
		{
			continue;
		}

		SendPacketFunction(user->GetIndex(), sizeof(nerPacket), reinterpret_cast<char*>(&nerPacket));
	}

	// �� �������� ���� �������� �˸�
	for (auto user : room->GetUsers())
	{
		if (newbie == user)
		{
			continue;
		}

		nerPacket.ClientID = user->GetClientID();
		SendPacketFunction(newbie->GetIndex(), sizeof(nerPacket), reinterpret_cast<char*>(&nerPacket));
	}
}

void RoomManager::EnterUpgrade(const INT32 roomIndex)
{
	auto& room = GetRoom(roomIndex);

	if (room->GetState() == Room::RoomState::Playing)
	{
		LOG("This room is playing.");
		return;
	}

	room->DoEnterUpgrade();

	NOTIFY_ENTER_UPGRADE_PACKET ansPacket;
	ansPacket.PacketID = NOTIFY_ENTER_UPGRADE;
	ansPacket.PacketSize = sizeof(NOTIFY_ENTER_UPGRADE_PACKET);
	ansPacket.Result = RESULT_CODE::SUCCESS;
	room->Broadcast(sizeof(ansPacket), reinterpret_cast<char*>(&ansPacket));
}
