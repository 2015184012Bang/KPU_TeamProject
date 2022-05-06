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
	mUserState = UserState::IN_LOGIN;
	mConnected = false;
	mUserName = "";
	mReadPos = 0;
	mWritePos = 0;
	mClientID = -1;
	mRoomIndex = -1;
	ZeroMemory(mDataBuffer, DATA_BUFFER_SIZE);

	if (mRegistry && mRegistry->valid(mCharacter) )
	{
		mRegistry->destroy(mCharacter);
		mRegistry = nullptr;
		mCharacter = entt::null;
	}
}

void User::SetLogin(string_view userName)
{
	// 유저가 로그인했다는 것은 로그인씬을 벗어나 로비씬에 있다는 걸 의미.
	mUserState = UserState::IN_LOBBY;

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

void User::CreatePlayerEntity()
{
	// 엔티티 생성
	mCharacter = mRegistry->create();

	// 컴포넌트 부착
	auto& transform = mRegistry->emplace<TransformComponent>(mCharacter);
	mRegistry->emplace<IDComponent>(mCharacter, mClientID);
	mRegistry->emplace<NameComponent>(mCharacter, "Player" + to_string(mClientID));
	mRegistry->emplace<MovementComponent>(mCharacter, Vector3::Zero, Values::PlayerSpeed);
	mRegistry->emplace<CombatComponent>(mCharacter);
	mRegistry->emplace<BoxComponent>(mCharacter, &Box::GetBox("../Assets/Boxes/Character.box"),
		transform.Position, transform.Yaw);
	mRegistry->emplace<Tag_Player>(mCharacter);
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

const Vector3& User::GetPosition()
{
	ASSERT(mRegistry->valid(mCharacter), "Invalid entity!");
	return mRegistry->get<TransformComponent>(mCharacter).Position;
}

const Vector3& User::GetMoveDirection()
{
	ASSERT(mRegistry->valid(mCharacter), "Invalid entity!");
	return mRegistry->get<MovementComponent>(mCharacter).Direction;
}
