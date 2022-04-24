#include "pch.h"
#include "User.h"

#include "Timer.h"

User::~User()
{
	if (mDataBuffer)
	{
		delete[] mDataBuffer;
		mDataBuffer = nullptr;
	}
}

void User::Init(const INT32 index)
{
	mIndex = index;
	mDataBuffer = new char[DATA_BUFFER_SIZE];
	ZeroMemory(mDataBuffer, DATA_BUFFER_SIZE);
}

void User::Reset()
{
	mConnected = false;
	mUserName = "";
	mReadPos = 0;
	mWritePos = 0;
	mPosition = Vector3::Zero;
	mYaw = 0.0f;
	mMoveDirection = Vector3::Zero;
	ZeroMemory(mDataBuffer, DATA_BUFFER_SIZE);
}

void User::SetLogin(string_view userName)
{
	mConnected = true;
	mUserName = userName.data();
}

void User::SetData(const UINT32 dataSize, char* pData)
{
	if (mWritePos + dataSize >= DATA_BUFFER_SIZE)
	{
		auto remain = mWritePos - mReadPos;

		if (remain > 0)
		{
			// 만약 남은 데이터가 있다면, 버퍼의 처음으로 땡겨준다.
			CopyMemory(&mDataBuffer[0], &mDataBuffer[mReadPos], remain);
			mWritePos = remain;
		}
		else
		{
			// WritePos == ReadPos 라는 것이므로, 쓰기 위치를 처음으로 돌린다.
			mWritePos = 0;
		}

		mReadPos = 0;
	}

	CopyMemory(&mDataBuffer[mWritePos], pData, dataSize);
	mWritePos += dataSize;
}

PACKET_INFO User::GetPacket()
{
	auto remain = mWritePos - mReadPos;

	// 패킷 헤더 크기보다 작나?
	if (remain < PACKET_HEADER_SIZE)
	{
		return PACKET_INFO();
	}

	PACKET_HEADER* header = reinterpret_cast<PACKET_HEADER*>(&mDataBuffer[mReadPos]);
	
	// 패킷 크기보다 작나?
	if (remain < header->PacketSize)
	{
		return PACKET_INFO();
	}

	PACKET_INFO info;
	info.DataPtr = &mDataBuffer[mReadPos];
	info.DataSize = header->PacketSize;
	info.PacketID = header->PacketID;
	info.SessionIndex = mIndex;

	mReadPos += header->PacketSize;

	return info;
}

void User::Update()
{
	if (mMoveDirection == Vector3::Zero)
	{
		return;
	}

	mPosition = mPosition + mMoveDirection * PLAYER_MAX_SPEED * Timer::GetDeltaTime();

	LOG("Position: x-{0} z-{0}", mPosition.x, mPosition.z);
}

void User::SetMoveDirection(const Vector3& direction)
{
	mMoveDirection = direction;

	if (mMoveDirection == Vector3::Zero)
	{
		return;
	}

	Vector3 rotation = XMVector3AngleBetweenVectors(Vector3::UnitZ, mMoveDirection);
	float scalar = 1.0f;
	if (mMoveDirection.x < 0.0f)
	{
		scalar = -1.0f;
	}

	mYaw = scalar * XMConvertToDegrees(rotation.y);
}
