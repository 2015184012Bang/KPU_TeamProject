#pragma once

#include "Protocol.h"

constexpr UINT32 DATA_BUFFER_SIZE = 8192;

class User
{
public:
	enum class UserState
	{
		IN_LOGIN,
		IN_LOBBY,
		IN_ROOM,
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
	entt::entity GetCharacterEntity() { return mCharacter; }
	const Vector3& GetPosition();
	const Vector3& GetMoveDirection();
	INT8 GetClientID() const { return mClientID; }
	INT32 GetRoomIndex() const { return mRoomIndex; }

	void SetClientID(const INT8 id) { mClientID = id; }
	void SetUserState(UserState state) { mUserState = state; }
	void SetRoomIndex(const INT32 roomIndex) { mRoomIndex = roomIndex; }
	void SetRegistry(entt::registry* registry) { mRegistry = registry; }

private:
	UserState mUserState = UserState::IN_LOGIN;

	INT32 mIndex = -1;
	INT32 mRoomIndex = -1;
	string mUserName = "";
	INT8 mClientID = -1;

	bool mConnected = false;

	UINT32 mWritePos = 0;
	UINT32 mReadPos = 0;
	char* mDataBuffer = nullptr;
	
	entt::entity mCharacter = entt::null;

	entt::registry* mRegistry;
};

