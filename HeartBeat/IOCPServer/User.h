#pragma once

#include "Protocol.h"

constexpr UINT32 DATA_BUFFER_SIZE = 8192;
constexpr float PLAYER_MAX_SPEED = 300.0f;

class User
{
public:
	User() = default;
	~User();

	void Init(const INT32 index);

	void Reset();

	void SetLogin(string_view userName);

	void SetData(const UINT32 dataSize, char* pData);

	PACKET_INFO GetPacket();

	void Update();

public:
	INT32 GetIndex() const { return mIndex; }
	string GetUserName() const { return mUserName; }
	bool IsConnected() const { return mConnected; }
	Vector3 GetPosition() const { return mPosition; }
	float GetYaw() const { return mYaw; }
	Vector3 GetMoveDirection() const { return mMoveDirection; }

	void SetPosition(const Vector3& position) { mPosition = position; }
	void SetYaw(float yaw) { mYaw = yaw; }
	void SetMoveDirection(const Vector3& direction);

private:
	INT32 mIndex = -1;
	string mUserName = "";

	bool mConnected = false;

	UINT32 mWritePos = 0;
	UINT32 mReadPos = 0;
	char* mDataBuffer = nullptr;

	// TODO: 플레이어 정보 추가(ex. 체력 등)
	Vector3 mPosition = Vector3::Zero;
	float mYaw = 0.0f;
	Vector3 mMoveDirection = Vector3::Zero;
};

