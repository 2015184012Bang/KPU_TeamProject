#pragma once

#include "Protocol.h"
#include "Entity.h"

constexpr UINT32 DATA_BUFFER_SIZE = 8192;

class User
{
public:
	enum class UserState
	{
		IN_LOGIN,
		IN_ROOM,
		IN_LOBBY,
		IN_UPGRADE,
		IN_GAME,
	};

	User() = default;
	~User();

	void Init(const INT32 index);

	void Reset();

	// 로그인 성공했을 때 호출
	void SetLogin(string_view userName);

	void SetData(const UINT32 dataSize, char* pData);

	void CreatePlayerEntity();

	PACKET_INFO GetPacket();

public:
	UserState GetUserState() const { return mUserState; }
	INT32 GetIndex() const { return mIndex; }
	string GetUserName() const { return mUserName; }
	bool IsConnected() const { return mConnected; }
	Entity& GetCharacter() { return mCharacter; }
	const Vector3& GetPosition();
	const Vector3& GetMoveDirection();

	void SetUserState(UserState state) { mUserState = state; }

private:
	UserState mUserState = UserState::IN_LOGIN;

	INT32 mIndex = -1;
	string mUserName = "";

	bool mConnected = false;

	UINT32 mWritePos = 0;
	UINT32 mReadPos = 0;
	char* mDataBuffer = nullptr;
	
	Entity mCharacter = {};
};

