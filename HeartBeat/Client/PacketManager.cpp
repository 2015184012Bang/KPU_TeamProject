#include "ClientPCH.h"
#include "PacketManager.h"

PacketManager::~PacketManager()
{
	if (!mbShutdown)
	{
		Shutdown();
	}
}

void PacketManager::Init()
{
	WSAData wsa;

	int retVal = WSAStartup(MAKEWORD(2, 2), &wsa);

	if (0 != retVal)
	{
		HB_LOG("WSAStartup Error: {0}", WSAGetLastError());
		return;
	}

	mSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (INVALID_SOCKET == mSocket)
	{
		HB_LOG("Failed to create client socket: {0}", WSAGetLastError());
		return;
	}
}

void PacketManager::Shutdown()
{
	mbShutdown = true;

	if (INVALID_SOCKET != mSocket)
	{
		closesocket(mSocket);
	}

	WSACleanup();
}

bool PacketManager::Connect(const string& ip, const UINT16 port)
{
	SOCKADDR_IN serverAddr;
	ZeroMemory(&serverAddr, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	inet_pton(AF_INET, ip.data(), &serverAddr.sin_addr);

	int retVal = connect(mSocket, reinterpret_cast<SOCKADDR*>(&serverAddr), sizeof(serverAddr));

	if (SOCKET_ERROR == retVal)
	{
		HB_LOG("Failed to connect server: {0}", WSAGetLastError());
		return false;
	}
	
	mbConnected = true;

	return true;
}

void PacketManager::Recv()
{
	if (!mbConnected)
	{
		return;
	}

	int retVal = recv(mSocket, &mRecvBuffer[mWritePos], PACKET_BUFFER_SIZE, 0);

	if (SOCKET_ERROR == retVal)
	{
		int errorCode = WSAGetLastError();

		if (WSAEWOULDBLOCK != errorCode)
		{
			HB_LOG("Recv Error: {0}", WSAGetLastError());
		}
		return;
	}

	if (0 == retVal)
	{
		HB_LOG("Server disconnected!");
		return;
	}

	mWritePos += retVal;
	UINT32 remainBytes = mWritePos - mReadPos;

	while (remainBytes > 0)
	{
		UINT8 packetSize = mRecvBuffer[mReadPos];

		if (remainBytes >= packetSize)
		{
			PACKET packet;
			packet.DataSize = packetSize;
			packet.PacketID = mRecvBuffer[mReadPos + 1];
			packet.DataPtr = new char[packetSize]; // 동적 할당한 메모리는 씬에서 사용 후 삭제!
			CopyMemory(packet.DataPtr, &mRecvBuffer[mReadPos], packetSize);
			mPackets.push(packet);

			mReadPos += packetSize;
			remainBytes -= packetSize;
		}
		else
		{
			break;
		}
	}

	if (mWritePos >= PACKET_BUFFER_SIZE)
	{
		if (remainBytes > 0)
		{
			CopyMemory(&mRecvBuffer[0], &mRecvBuffer[mReadPos], remainBytes);
			mWritePos = remainBytes;
		}
		else
		{
			mWritePos = 0;
		}
		mReadPos = 0;
	}

	HB_LOG("Recv Bytes: {0} writePos: {1} readPos: {2}", retVal, mWritePos, mReadPos);
}


bool PacketManager::GetPacket(PACKET& packet)
{
	if (mPackets.empty())
	{
		return false;
	}

	packet = mPackets.front();
	mPackets.pop();

	return true;
}

void PacketManager::SetNonblocking(bool opt)
{
	u_long val = opt ? 1 : 0;
	ioctlsocket(mSocket, FIONBIO, &val);
}

void PacketManager::Send(char* packet, UINT32 dataSize)
{
	int retVal = send(mSocket, packet, dataSize, 0);

	if ((SOCKET_ERROR == retVal) && (WSAGetLastError() != WSAEWOULDBLOCK))
	{
		HB_LOG("Send Error: {0}", WSAGetLastError());
	}
}

