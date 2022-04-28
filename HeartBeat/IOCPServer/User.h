#pragma once

#include "Protocol.h"
#include "Entity.h"

constexpr UINT32 DATA_BUFFER_SIZE = 8192;
constexpr float PLAYER_MAX_SPEED = 300.0f;

class User
{
	const float BASE_ATTACK_COOLDOWN = 1.0f; // 기본 공격 재사용 대기시간

public:
	enum class UpgradePreset : UINT8
	{
		
	};

	User() = default;
	~User();

	void Init(const INT32 index);

	void Reset();

	void SetLogin(string_view userName);

	void SetData(const UINT32 dataSize, char* pData);

	PACKET_INFO GetPacket();

public:
	INT32 GetIndex() const { return mIndex; }
	string GetUserName() const { return mUserName; }
	bool IsConnected() const { return mConnected; }

	Entity& GetCharacter() { return mCharacter; }

	const Vector3& GetPosition();
	const Vector3& GetMoveDirection();

private:
	INT32 mIndex = -1;
	string mUserName = "";

	bool mConnected = false;

	UINT32 mWritePos = 0;
	UINT32 mReadPos = 0;
	char* mDataBuffer = nullptr;
	
	Entity mCharacter = {};
};

