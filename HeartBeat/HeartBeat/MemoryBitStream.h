#pragma once
class OutputMemoryBitStream
{
public:
	OutputMemoryBitStream() :
		mBitHead(0),
		mBuffer(nullptr)
	{
		ReallocBuffer(1500 * 8);
	}

	~OutputMemoryBitStream() { std::free(mBuffer); }

	void WriteBits(uint8 inData, uint32 inBitCount);
	void WriteBits(const void* inData, uint32 inBitCount);

	const char* GetBufferPtr() const { return mBuffer; }
	uint32 GetBitLength() const { return mBitHead; }
	uint32 GetByteLength() const { return (mBitHead + 7) >> 3; }

	template<typename T>
	void Write(T inData, uint32 inBitCount = sizeof(T) * 8)
	{
		static_assert(std::is_arithmetic<T>::value ||
			std::is_enum<T>::value,
			"Generic Write only supports primitive data types");
		WriteBits(&inData, inBitCount);
	}

	void Write(bool inData) { WriteBits(&inData, 1); }

	void Write(const string& inString)
	{
		uint32 elementCount = static_cast<uint32>(inString.size());
		Write(elementCount);
		for (const auto& element : inString)
		{
			Write(element);
		}
	}

private:
	void ReallocBuffer(uint32 inNewBitCapacity);
	
	char* mBuffer;
	uint32 mBitHead;
	uint32 mBitCapacity;
};

class InputMemoryBitStream
{
public:

	InputMemoryBitStream(char* inBuffer, uint32 inBitCount) :
		mBuffer(inBuffer),
		mBitCapacity(inBitCount),
		mBitHead(0),
		mIsBufferOwner(false) {}

	InputMemoryBitStream(const InputMemoryBitStream& inOther) :
		mBitCapacity(inOther.mBitCapacity),
		mBitHead(inOther.mBitHead),
		mIsBufferOwner(true)
	{
		int byteCount = mBitCapacity / 8;
		mBuffer = static_cast<char*>(malloc(byteCount));
		memcpy(mBuffer, inOther.mBuffer, byteCount);
	}

	~InputMemoryBitStream() { if (mIsBufferOwner) { free(mBuffer); }; }
	
	const char* GetBufferPtr() const { return mBuffer; }
	uint32 GetRemainingBitCount() const { return mBitCapacity - mBitHead; };

	void ReadBits(uint8& outData, uint32 inBitCount);
	void ReadBits(void* outData, uint32 inBitCount);

	void ReadBytes(void* outData, uint32 inByteCount) { ReadBits(outData, inByteCount << 3); }

	template< typename T >
	void Read(T& inData, uint32 inBitCount = sizeof(T) * 8)
	{
		static_assert(std::is_arithmetic< T >::value ||
			std::is_enum< T >::value,
			"Generic Read only supports primitive data types");
		ReadBits(&inData, inBitCount);
	}

	void Read(uint32& outData, uint32 inBitCount = 32) {ReadBits(&outData, inBitCount);	}
	void Read(int& outData, uint32 inBitCount = 32) { ReadBits(&outData, inBitCount); }
	void Read(float& outData) { ReadBits(&outData, 32); }

	void Read(uint16& outData, uint32 inBitCount = 16) { ReadBits(&outData, inBitCount); }
	void Read(int16& outData, uint32 inBitCount = 16) { ReadBits(&outData, inBitCount); }

	void Read(uint8& outData, uint32 inBitCount = 8) { ReadBits(&outData, inBitCount); }
	void Read(bool& outData) { ReadBits(&outData, 1); }

	void ResetToCapacity(uint32 inByteCapacity) 
	{ 
		mBitCapacity = inByteCapacity << 3;
		mBitHead = 0;
	}

	void Read(string& inString)
	{
		uint32 elementCount;
		Read(elementCount);
		inString.resize(elementCount);
		for (auto& element : inString)
		{
			Read(element);
		}
	}


private:
	char* mBuffer;
	uint32 mBitHead;
	uint32 mBitCapacity;
	bool mIsBufferOwner;
};