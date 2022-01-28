#pragma once
class NetworkManager
{
public:
	static const uint32 kHelloCC = 'HELO';
	static const uint32 kWelcomeCC = 'WLCM';
	static const uint32 kStateCC = 'STAT';
	static const uint32 kInputCC = 'INPT';
	static const int kMaxPacketsPerFrameCount = 10;

	NetworkManager();
	virtual ~NetworkManager();

	bool Init(uint16 inPort);
	void ProcessIncomingPackets();

	virtual void ProcessPacket(InputMemoryBitStream& inInputStream, const SocketAddress& inFromAddress) = 0;
	virtual void HandleConnectionReset(const SocketAddress& inFromAddress) { (void)inFromAddress; };

	void SendPacket(const OutputMemoryBitStream& inOutputStream, const SocketAddress& inFromAddress);


private:
	class ReceivedPacket
	{
	public:
		ReceivedPacket(float inReceivedTime, InputMemoryBitStream& inInputMemoryBitStream, const SocketAddress& inAddress);

		const SocketAddress& GetFreomAddress() const { return mFromAddress; }
		float GetReceivedTime() const { return mReceivedTime; }
		InputMemoryBitStream& GetPacketBuffer() { return mPacketBuffer; }

	private:
		float mReceivedTime;
		InputMemoryBitStream mPacketBuffer;
		SocketAddress mFromAddress;
	};

	void UpdateBytesSentLastFrame();
	void ReadIncomingPacketsIntoQueue();
	void ProcessQueuedPackets();

	queue< ReceivedPacket, list<ReceivedPacket> > mPacketQueue;

	TCPSocketPtr mSocket;

	int mByteSentThisFrame;

	float mDropPacketChane;
	float mSimulatedLatency;
};