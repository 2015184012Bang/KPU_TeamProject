#pragma once

#include "Protocol.h"

constexpr UINT32 DATA_BUFFER_SIZE = 8192;

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

public:
	INT32 GetIndex() const { return mIndex; }
	string GetUserName() const { return mUserName; }
	bool IsConnected() const { return mConnected; }

private:
	INT32 mIndex = -1;
	string mUserName = "";

	bool mConnected = false;

	UINT32 mWritePos = 0;
	UINT32 mReadPos = 0;
	char* mDataBuffer = nullptr;

	// TODO: 플레이어 정보 추가(ex. 체력 등)
};

