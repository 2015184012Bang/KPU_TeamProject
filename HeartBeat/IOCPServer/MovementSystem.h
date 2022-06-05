#pragma once

class Room;

class MovementSystem
{
public:
	MovementSystem(entt::registry& registry, shared_ptr<Room>&& room);

	void Update();

	void SetDirection(const INT8 clientID, const Vector3& direction);

	void SendNotifyMovePackets();

	void Start();

private:
	void checkArriveAtMidPoint();
	entt::entity getClosestDoor(const Vector3& midPointPos);

private:
	entt::registry& mRegistry;
	shared_ptr<Room> mOwner = nullptr;
};

