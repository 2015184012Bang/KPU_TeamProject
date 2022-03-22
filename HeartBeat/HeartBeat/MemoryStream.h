#pragma once

constexpr int PACKET_SIZE = 1400;

class MemoryStream
{
public:
	MemoryStream();
	
	void WriteByte(int8 data);
	void WriteShort(int16 data);
	void WriteInt(int32 data);
	void WriteInt64(int64 data);
	void WriteUByte(uint8 data);
	void WriteUShort(uint16 data);
	void WriteUInt(uint32 data);
	void WriteUInt64(uint64 data);
	void WriteBool(bool data);
	void WriteFloat(float data);
	void WriteVector2(const Vector2& data);
	void WriteVector3(const Vector3& data);

	void ReadByte(int8* outData);
	void ReadShort(int16* outData);
	void ReadInt(int32* outData);
	void ReadInt64(int64* outData);
	void ReadUByte(uint8* outData);
	void ReadUShort(uint16* outData);
	void ReadUInt(uint32* outData);
	void ReadUInt64(uint64* outData);
	void ReadBool(bool* outData);
	void ReadFloat(float* outData);
	void ReadVector2(Vector2* outData);
	void ReadVector3(Vector3* outData);

	void Reset();
	
	uint16 GetLength() const { return mLength; }
	void SetLength(uint16 len) { mLength = len; }

private:
	uint8 mBuffer[PACKET_SIZE];
	uint16 mLength;
};

