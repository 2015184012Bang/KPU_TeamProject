#pragma once

class Room
{
public:
	enum class RoomState
	{
		Waiting,
		Playing,
	};

public:
	RoomState GetState() { return mRoomState; }

private:
	RoomState mRoomState = RoomState::Waiting;

};

