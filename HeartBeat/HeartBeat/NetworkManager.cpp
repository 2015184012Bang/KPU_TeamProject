#include "PCH.h"

NetworkManager::NetworkManager() :
	mByteSentThisFrame(0),
	mDropPacketChane(0.f),
	mSimulatedLatency(0.f)
{
}

NetworkManager::~NetworkManager()
{
}

bool NetworkManager::Init(uint16 inPort)
{
	mSocket = SocketUtil::CreateTCPSocket();
	SocketAddress ownAddress(INADDR_ANY, inPort);
	mSocket->Bind(ownAddress);

	HB_LOG("Initailizing NewworkManager at port &d", inPort);

	if (mSocket == nullptr)
	{
		return false;
	}

	return true;
}

void NetworkManager::ProcessIncomingPackets()
{
	ReadIncomingPacketsIntoQueue();

	ProcessQueuedPackets();

	UpdateBytesSentLastFrame();
}

void NetworkManager::SendPacket(const OutputMemoryBitStream& inOutputStream, const SocketAddress& inFromAddress)
{
	int sentByteCount = mSocket->Send(inOutputStream.GetBufferPtr(), inOutputStream.GetByteLength());
	if (sentByteCount > 0)
	{
		mByteSentThisFrame += sentByteCount;
	}
}

void NetworkManager::UpdateBytesSentLastFrame()
{

}

void NetworkManager::ReadIncomingPacketsIntoQueue()
{
	char packetMem[1500];
	int packetSize = sizeof(packetMem);
	InputMemoryBitStream inputStream(packetMem, packetSize * 8);
	SocketAddress fromAddress;

	int receivedPacketCount = 0;
	int totalReadByteCount = 0;

	while (receivedPacketCount < kMaxPacketsPerFrameCount)
	{
		int readByteCount = mSocket->Receive(packetMem, packetSize);
		if (readByteCount == 0)
		{
			break;
		}
		else if (readByteCount == -WSAECONNRESET)
		{
			HandleConnectionReset(fromAddress);
		}
		else if (readByteCount > 0)
		{
			inputStream.ResetToCapacity(readByteCount);
			++receivedPacketCount;
			totalReadByteCount += readByteCount;

			
		}
		else
		{
			
		}
	}

	if (totalReadByteCount > 0)
	{
		
	}
}

void NetworkManager::ProcessQueuedPackets()
{
	while (!mPacketQueue.empty())
	{
		ReceivedPacket& nextPacket = mPacketQueue.front();
		if (Timing::sInstance.GetTimef() > nextPacket.GetReceivedTime())
		{
			ProcessPacket(nextPacket.GetPacketBuffer(), nextPacket.GetFreomAddress());
			mPacketQueue.pop();
		}
		else
		{
			break;
		}
	}

}

NetworkManager::ReceivedPacket::ReceivedPacket(float inReceivedTime, InputMemoryBitStream& inInputMemoryBitStream, const SocketAddress& inAddress):
	mReceivedTime(inReceivedTime),
	mFromAddress(mFromAddress),
	mPacketBuffer(inInputMemoryBitStream)
{
}
