#include "PCH.h"

void OutputMemoryBitStream::WriteBits(uint8 inData, uint32 inBitCount)
{
	uint32 nextBitHead = mBitHead + static_cast<uint32>(inBitCount);

	if (nextBitHead > mBitCapacity)
	{
		ReallocBuffer(std::max(mBitCapacity * 2, nextBitHead));
	}

	uint32 byteOffset = mBitHead >> 3;
	uint32 bitOffset = mBitHead & 0x7;

	uint8 currentMask = ~(0xff << bitOffset);
	mBuffer[byteOffset] = (mBuffer[byteOffset] & currentMask) | (inData << bitOffset);

	uint32 bitsFreeThisByte = 8 - bitOffset;

	if (bitsFreeThisByte < inBitCount)
	{
		mBuffer[byteOffset + 1] = inData >> bitsFreeThisByte;
	}
	
	mBitHead = nextBitHead;

}

void OutputMemoryBitStream::WriteBits(const void* inData, uint32 inBitCount)
{
	const char* srcByte = static_cast<const char*>(inData);

	while (inBitCount > 8)
	{
		WriteBits(*srcByte, 8);
		++srcByte;
		inBitCount -= 8;
	}

	if (inBitCount > 0)
	{
		WriteBits(*srcByte, inBitCount);
	}
}

void OutputMemoryBitStream::ReallocBuffer(uint32 inNewBitCapacity)
{
	if (mBuffer == nullptr)
	{
		mBuffer = static_cast<char*>(malloc(inNewBitCapacity >> 3));
		memset(mBuffer, 0, inNewBitCapacity >> 3);
	}
	else
	{
		char* tempBuffer = static_cast<char*>(malloc(inNewBitCapacity >> 3));
		memset(tempBuffer, 0, inNewBitCapacity >> 3);
		memcpy(tempBuffer, mBuffer, mBitCapacity >> 3);
		free(mBuffer);
		mBuffer = tempBuffer;
	}

	mBitCapacity = inNewBitCapacity;
}

void InputMemoryBitStream::ReadBits(uint8& outData, uint32 inBitCount)
{
	uint32 byteOffset = mBitHead >> 3;
	uint32 bitOffset = mBitHead & 0x7;

	outData = static_cast<uint8>(mBuffer[byteOffset]) >> bitOffset;

	uint32_t bitsFreeThisByte = 8 - bitOffset;
	if (bitsFreeThisByte < inBitCount)
	{
		outData |= static_cast<uint8>(mBuffer[byteOffset + 1]) << bitsFreeThisByte;
	}

	outData &= (~(0x00ff << inBitCount));

	mBitHead += inBitCount;
}

void InputMemoryBitStream::ReadBits(void* outData, uint32 inBitCount)
{
	uint8* destByte = reinterpret_cast<uint8*>(outData);
	while (inBitCount > 8)
	{
		ReadBits(*destByte, 8);
		++destByte;
		inBitCount -= 8;
	}

	if (inBitCount > 0)
	{
		ReadBits(*destByte, inBitCount);
	}
}
