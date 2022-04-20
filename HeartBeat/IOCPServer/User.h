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

	void SetLogin(const string& userID);

	void SetData(const UINT32 dataSize, char* pData);

	PACKET_INFO GetPacket();

public:
	INT32 GetIndex() { return mIndex; }
	string GetUserID() { return mUserID; }

private:
	INT32 mIndex = -1;
	string mUserID = "";

	UINT32 mWritePos = 0;
	UINT32 mReadPos = 0;
	char* mDataBuffer = nullptr;

	// TODO: 플레이어 정보 추가(ex. 체력 등)
};

