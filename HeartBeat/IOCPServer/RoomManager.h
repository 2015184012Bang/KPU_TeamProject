#pragma once

class Room;
class User;

class RoomManager
{
public:
	void Init(const UINT8 maxRoomCount);

	// �� ������ ���� ���� ���θ� �������� �۽�
	void SendAvailableRoom(const INT32 sessionIndex);

	shared_ptr<Room>& GetRoom(const INT32 roomIndex);
	
	function<void(INT32, UINT32, char*)> SendPacketFunction;

	void Update();

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

