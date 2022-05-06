#pragma once

class Room;

class MovementSystem
{
public:
	MovementSystem(entt::registry& registry, shared_ptr<Room>&& room);

	void Update();

	void SetDirection(const UINT32 eid, const Vector3& direction);

	void SendNotifyMovePackets();

	void SetPlayersStartPos();

private:
	entt::registry& mRegistry;
	shared_ptr<Room> mOwner = nullptr;
};

