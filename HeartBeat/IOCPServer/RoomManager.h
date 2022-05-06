#pragma once

class Room;
class User;

class RoomManager
{
public:
	void Init(const UINT8 maxRoomCount);

	void SendAvailableRoom(const INT32 sessionIndex);

	bool CanEnter(const INT32 roomIndex);

	void AddUser(const INT32 roomIndex, User* user);

	void RemoveUser(User* user);

	shared_ptr<Room>& GetRoom(const INT32 roomIndex);

	void Broadcast(const INT32 roomIndex, const UINT32 packetSize, char* packet);
	
	function<void(INT32, UINT32, char*)> SendPacketFunction;

private:
	enum
	{
		AVAILABLE,
		CANNOT,
		END,
	};

	UINT8 mMaxRoomCount = 0;

	vector<shared_ptr<Room>> mRooms;
};

