#include "ClientPCH.h"
#include "PacketManager.h"

#include "Client.h"

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

	int opt = 1;
	retVal = setsockopt(mSocket, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<char*>(&opt), sizeof(opt));

	if (SOCKET_ERROR == retVal)
	{
		HB_LOG("Failed to turn off nagle: {0}", WSAGetLastError());
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

bool PacketManager::Connect(string_view ip, const UINT16 port)
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

	int retVal = recv(mSocket, mRecvBuffer, RECV_BUFFER_SIZE, 0);

	if (SOCKET_ERROR == retVal)
	{
		int errorCode = WSAGetLastError();

		if (WSAEWOULDBLOCK != errorCode)
		{
			HB_LOG("Recv Error: {0}", WSAGetLastError());
			gShouldClose = true;
		}
		return;
	}

	if (0 == retVal)
	{
		HB_LOG("Server disconnected!");
		gShouldClose = true;
		return;
	}

	// 받은 바이트 수 만큼 WritePos 전진
	CopyMemory(&mPacketBuffer[mWritePos], mRecvBuffer, retVal);
	mWritePos += retVal;
	UINT32 remainBytes = mWritePos - mReadPos;

	if (mWritePos >= PACKET_BUFFER_SIZE)
	{
		if (remainBytes > 0)
		{
			CopyMemory(&mPacketBuffer[0], &mPacketBuffer[mReadPos], remainBytes);
			mWritePos = remainBytes;
		}
		else
		{
			mWritePos = 0;
		}

		mReadPos = 0;
	}
	
	while (remainBytes > 0)
	{
		UINT8 packetSize = mPacketBuffer[mReadPos];

		if (remainBytes >= packetSize)
		{
			PACKET packet;
			packet.DataSize = packetSize;
			packet.PacketID = mPacketBuffer[mReadPos + 1];
			packet.DataPtr = &mPacketBuffer[mReadPos];
			mPackets.push(packet);

			mReadPos += packetSize;
			remainBytes -= packetSize;
		}
		else
		{
			break;
		}
	}
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

