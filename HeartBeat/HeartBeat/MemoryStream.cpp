#include "PCH.h"
#include "MemoryStream.h"

MemoryStream::MemoryStream()
	: mLength(0)
{
	memset(mBuffer, 0, PACKET_SIZE);
}

void MemoryStream::WriteByte(int8 data)
{
	bool isSizeUnder = (mLength + sizeof(int8)) <= PACKET_SIZE;

	HB_ASSERT(isSizeUnder, "MemoryStream is full.");

	memcpy(mBuffer + mLength, &data, sizeof(int8));
	mLength += sizeof(int8);
}

void MemoryStream::WriteShort(int16 data)
{
	bool isSizeUnder = (mLength + sizeof(int16)) <= PACKET_SIZE;

	HB_ASSERT(isSizeUnder, "MemoryStream is full.");

	memcpy(mBuffer + mLength, &data, sizeof(int16));
	mLength += sizeof(int16);
}

void MemoryStream::WriteInt(int32 data)
{
	bool isSizeUnder = (mLength + sizeof(int32)) <= PACKET_SIZE;

	HB_ASSERT(isSizeUnder, "MemoryStream is full.");

	memcpy(mBuffer + mLength, &data, sizeof(int32));
	mLength += sizeof(int32);
}

void MemoryStream::WriteInt64(int64 data)
{
	bool isSizeUnder = (mLength + sizeof(int64)) <= PACKET_SIZE;

	HB_ASSERT(isSizeUnder, "MemoryStream is full.");

	memcpy(mBuffer + mLength, &data, sizeof(int64));
	mLength += sizeof(int64);
}

void MemoryStream::WriteUByte(uint8 data)
{
	bool isSizeUnder = (mLength + sizeof(uint8)) <= PACKET_SIZE;

	HB_ASSERT(isSizeUnder, "MemoryStream is full.");

	memcpy(mBuffer + mLength, &data, sizeof(uint8));
	mLength += sizeof(uint8);
}

void MemoryStream::WriteUShort(uint16 data)
{
	bool isSizeUnder = (mLength + sizeof(uint16)) <= PACKET_SIZE;

	HB_ASSERT(isSizeUnder, "MemoryStream is full.");

	memcpy(mBuffer + mLength, &data, sizeof(uint16));
	mLength += sizeof(uint16);
}

void MemoryStream::WriteUInt(uint32 data)
{
	bool isSizeUnder = (mLength + sizeof(uint32)) <= PACKET_SIZE;

	HB_ASSERT(isSizeUnder, "MemoryStream is full.");

	memcpy(mBuffer + mLength, &data, sizeof(uint32));
	mLength += sizeof(uint32);
}

void MemoryStream::WriteUInt64(uint64 data)
{
	bool isSizeUnder = (mLength + sizeof(uint64)) <= PACKET_SIZE;

	HB_ASSERT(isSizeUnder, "MemoryStream is full.");

	memcpy(mBuffer + mLength, &data, sizeof(uint64));
	mLength += sizeof(uint64);
}

void MemoryStream::WriteBool(bool data)
{
	bool isSizeUnder = (mLength + sizeof(bool)) <= PACKET_SIZE;

	HB_ASSERT(isSizeUnder, "MemoryStream is full.");

	memcpy(mBuffer + mLength, &data, sizeof(bool));
	mLength += sizeof(bool);
}

void MemoryStream::WriteFloat(float data)
{
	bool isSizeUnder = (mLength + sizeof(float)) <= PACKET_SIZE;

	HB_ASSERT(isSizeUnder, "MemoryStream is full.");

	memcpy(mBuffer + mLength, &data, sizeof(float));
	mLength += sizeof(float);
}

void MemoryStream::WriteVector2(const Vector2& data)
{
	bool isSizeUnder = (mLength + sizeof(Vector2)) <= PACKET_SIZE;

	HB_ASSERT(isSizeUnder, "MemoryStream is full.");

	memcpy(mBuffer + mLength, &data, sizeof(Vector2));
	mLength += sizeof(Vector2);
}

void MemoryStream::WriteVector3(const Vector3& data)
{
	bool isSizeUnder = (mLength + sizeof(Vector3)) <= PACKET_SIZE;

	HB_ASSERT(isSizeUnder, "MemoryStream is full.");

	memcpy(mBuffer + mLength, &data, sizeof(Vector3));
	mLength += sizeof(Vector3);
}

void MemoryStream::ReadByte(int8* outData)
{
	memcpy(outData, mBuffer + mLength, sizeof(int8));
	mLength += sizeof(int8);
}

void MemoryStream::ReadShort(int16* outData)
{
	memcpy(outData, mBuffer + mLength, sizeof(int16));
	mLength += sizeof(int16);
}

void MemoryStream::ReadInt(int32* outData)
{
	memcpy(outData, mBuffer + mLength, sizeof(int32));
	mLength += sizeof(int32);
}

void MemoryStream::ReadInt64(int64* outData)
{
	memcpy(outData, mBuffer + mLength, sizeof(int64));
	mLength += sizeof(int64);
}

void MemoryStream::ReadUByte(uint8* outData)
{
	memcpy(outData, mBuffer + mLength, sizeof(uint8));
	mLength += sizeof(uint8);
}

void MemoryStream::ReadUShort(uint16* outData)
{
	memcpy(outData, mBuffer + mLength, sizeof(uint16));
	mLength += sizeof(uint16);
}

void MemoryStream::ReadUInt(uint32* outData)
{
	memcpy(outData, mBuffer + mLength, sizeof(uint32));
	mLength += sizeof(uint32);
}

void MemoryStream::ReadUInt64(uint64* outData)
{
	memcpy(outData, mBuffer + mLength, sizeof(uint64));
	mLength += sizeof(uint64);
}

void MemoryStream::ReadBool(bool* outData)
{
	memcpy(outData, mBuffer + mLength, sizeof(bool));
	mLength += sizeof(bool);
}

void MemoryStream::ReadFloat(float* outData)
{
	memcpy(outData, mBuffer + mLength, sizeof(float));
	mLength += sizeof(float);
}

void MemoryStream::ReadVector2(Vector2* outData)
{
	memcpy(outData, mBuffer + mLength, sizeof(Vector2));
	mLength += sizeof(Vector2);
}

void MemoryStream::ReadVector3(Vector3* outData)
{
	memcpy(outData, mBuffer + mLength, sizeof(Vector3));
	mLength += sizeof(Vector3);
}

void MemoryStream::Reset()
{
	memset(mBuffer, 0, PACKET_SIZE);
	mLength = 0;
}
