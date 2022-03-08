#include "PCH.h"
#include "MemoryBitStream.h"

void OutputMemoryBitStream::WriteBits( uint8 inData,
									  uint32 inBitCount )
{
	uint32 nextBitHead = mBitHead + static_cast< uint32 >( inBitCount );
	
	if( nextBitHead > mBitCapacity )
	{
		ReallocBuffer( std::max( mBitCapacity * 2, nextBitHead ) );
	}
	

	uint32 byteOffset = mBitHead >> 3;
	uint32 bitOffset = mBitHead & 0x7;
	
	uint8 currentMask = ~( 0xff << bitOffset );
	mBuffer[ byteOffset ] = ( mBuffer[ byteOffset ] & currentMask ) | ( inData << bitOffset );
	

	uint32_t bitsFreeThisByte = 8 - bitOffset;
	
	if( bitsFreeThisByte < inBitCount )
	{
		mBuffer[ byteOffset + 1 ] = inData >> bitsFreeThisByte;
	}
	
	mBitHead = nextBitHead;
}

void OutputMemoryBitStream::WriteBits( const void* inData, uint32 inBitCount )
{
	const char* srcByte = static_cast< const char* >( inData );

	while( inBitCount > 8 )
	{
		WriteBits( *srcByte, 8 );
		++srcByte;
		inBitCount -= 8;
	}

	if( inBitCount > 0 )
	{
		WriteBits( *srcByte, inBitCount );
	}
}

//void OutputMemoryBitStream::Write( const Vector3& inVector )
//{
//	Write( inVector.mX );
//	Write( inVector.mY );
//	Write( inVector.mZ );
//}
//
//void InputMemoryBitStream::Read( Vector3& outVector )
//{
//	Read( outVector.mX );
//	Read( outVector.mY );
//	Read( outVector.mZ );
//}
//
//void OutputMemoryBitStream::Write( const Quaternion& inQuat )
//{
//	float precision = ( 2.f / 65535.f );
//	Write( ConvertToFixed( inQuat.mX, -1.f, precision ), 16 );
//	Write( ConvertToFixed( inQuat.mY, -1.f, precision ), 16 );
//	Write( ConvertToFixed( inQuat.mZ, -1.f, precision ), 16 );
//	Write( inQuat.mW < 0 );
//}



void OutputMemoryBitStream::ReallocBuffer( uint32 inNewBitLength )
{
	if( mBuffer == nullptr )
	{
		mBuffer = static_cast<char*>( std::malloc( inNewBitLength >> 3 ) );
		memset( mBuffer, 0, inNewBitLength >> 3 );
	}
	else
	{
		char* tempBuffer = static_cast<char*>( std::malloc( inNewBitLength >> 3 ) );
		memset( tempBuffer, 0, inNewBitLength >> 3 );
		memcpy( tempBuffer, mBuffer, mBitCapacity >> 3 );
		std::free( mBuffer );
		mBuffer = tempBuffer;
	}
	

	mBitCapacity = inNewBitLength;
}

void InputMemoryBitStream::ReadBits( uint8& outData, uint32 inBitCount )
{
	uint32 byteOffset = mBitHead >> 3;
	uint32 bitOffset = mBitHead & 0x7;
	
	outData = static_cast< uint8 >( mBuffer[ byteOffset ] ) >> bitOffset;
	
	uint32 bitsFreeThisByte = 8 - bitOffset;
	if( bitsFreeThisByte < inBitCount )
	{
		outData |= static_cast< uint8 >( mBuffer[ byteOffset + 1 ] ) << bitsFreeThisByte;
	}
	
	outData &= ( ~( 0x00ff << inBitCount ) );
	
	mBitHead += inBitCount;
}

void InputMemoryBitStream::ReadBits( void* outData, uint32 inBitCount )
{
	uint8* destByte = reinterpret_cast< uint8* >( outData );
	while( inBitCount > 8 )
	{
		ReadBits( *destByte, 8 );
		++destByte;
		inBitCount -= 8;
	}
	//write anything left
	if( inBitCount > 0 )
	{
		ReadBits( *destByte, inBitCount );
	}
}

//void InputMemoryBitStream::Read( Quaternion& outQuat )
//{
//	float precision = ( 2.f / 65535.f );
//	
//	uint32 f = 0;
//	
//	Read( f, 16 );
//	outQuat.mX = ConvertFromFixed( f, -1.f, precision );
//	Read( f, 16 );
//	outQuat.mY = ConvertFromFixed( f, -1.f, precision );
//	Read( f, 16 );
//	outQuat.mZ = ConvertFromFixed( f, -1.f, precision );
//	
//	outQuat.mW = sqrtf( 1.f -
//					   outQuat.mX * outQuat.mX +
//					   outQuat.mY * outQuat.mY +
//					   outQuat.mZ * outQuat.mZ );
//	bool isNegative;
//	Read( isNegative );
//	
//	if( isNegative )
//	{
//		outQuat.mW *= -1;
//	}
//}
