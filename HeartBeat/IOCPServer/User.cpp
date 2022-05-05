#include "pch.h"
#include "User.h"

#include "Components.h"
#include "Timer.h"
#include "Tags.h"
#include "GameManager.h"
#include "Values.h"

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
	ZeroMemory(mDataBuffer, DATA_BUFFER_SIZE);
	gRegistry.destroy(mCharacter);
	mCharacter = {};
}

void User::SetLogin(string_view userName)
{
	mConnected = true;
	mUserName = userName.data();

	// ��ƼƼ ����
	mCharacter = Entity{ gRegistry.create() };

	// ������Ʈ ����
	auto& transform = mCharacter.AddComponent<TransformComponent>();
	mCharacter.AddComponent<IDComponent>(mIndex);
	mCharacter.AddComponent<NameComponent>("Player" + to_string(mIndex));
	mCharacter.AddComponent<MovementComponent>(Vector3::Zero, Values::PlayerSpeed);
	mCharacter.AddComponent<CombatComponent>();
	mCharacter.AddComponent<BoxComponent>(&Box::GetBox("../Assets/Boxes/Character.box"),
		transform.Position, transform.Yaw);
	mCharacter.AddTag<Tag_Player>();
}

void User::SetData(const UINT32 dataSize, char* pData)
{
	if (mWritePos + dataSize >= DATA_BUFFER_SIZE)
	{
		auto remain = mWritePos - mReadPos;

		if (remain > 0)
		{
			// ���� ���� �����Ͱ� �ִٸ�, ������ ó������ �����ش�.
			CopyMemory(&mDataBuffer[0], &mDataBuffer[mReadPos], remain);
			mWritePos = remain;
		}
		else
		{
			// WritePos == ReadPos ��� ���̹Ƿ�, ���� ��ġ�� ó������ ������.
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

	// ��Ŷ ��� ũ�⺸�� �۳�?
	if (remain < PACKET_HEADER_SIZE)
	{
		return PACKET_INFO();
	}

	PACKET_HEADER* header = reinterpret_cast<PACKET_HEADER*>(&mDataBuffer[mReadPos]);

	// ��Ŷ ũ�⺸�� �۳�?
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

const Vector3& User::GetPosition()
{
	return mCharacter.GetComponent<TransformComponent>().Position;
}

const Vector3& User::GetMoveDirection()
{
	return mCharacter.GetComponent<MovementComponent>().Direction;
}
