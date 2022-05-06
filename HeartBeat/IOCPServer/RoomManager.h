#pragma once

class Room;

class RoomManager
{
public:
	void Init(const UINT8 maxRoomCount);

	void SendAvailableRoom(const INT32 sessionIndex);
	
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

