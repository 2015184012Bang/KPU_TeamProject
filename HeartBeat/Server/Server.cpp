#include "ServerPCH.h"
#include "Server.h"


Server::Server()
{

}

bool Server::Init()
{
	HB_LOG("SERVER INIT");

	SocketUtil::Init();

	TCPSocketPtr listenSock = SocketUtil::CreateTCPSocket();
	SocketAddress serveraddr(9000);

	if (listenSock->Bind(serveraddr) == SOCKET_ERROR)
	{
		HB_ASSERT(false, "ASSERTION FAILED");
	}

	if (listenSock->Listen() == SOCKET_ERROR)
	{
		HB_ASSERT(false, "ASSERTION FAILED");
	}
	
	SocketAddress clientaddr;
	int clientNum = 0;

	// serializtion test
	static const uint32 kHelloCC = 'HELO';
	static const uint32 kWelcomCC = 'WLCM';

	char packetMem[1500];
	int packetSize = sizeof(packetMem);
	InputMemoryBitStream inputStream(packetMem, packetSize * 8);


	while (clientNum != 1)
	{
		TCPSocketPtr clientSocket = listenSock->Accept(&clientaddr);
		
		HB_LOG("클라이언트 접속: {0}", clientaddr.ToString());

		clientNum++;

		mClientSockets.push_back(clientSocket);

		// serialization test
		int retval = RecvPacket(inputStream, clientSocket);

		if (retval == 0)
		{
			HB_LOG("Nothing in buffer");
		}
		else if (retval == SOCKET_ERROR)
		{
			HB_LOG("Server::RecvPacketFromClient ERROR");
		}
		else
		{
			uint32 packetType;
			inputStream.Read(packetType);

			switch (packetType)
			{
			case kHelloCC:
				HB_LOG("HELLO packet Recv");
				break;
			default:
				HB_LOG("Fail");
				break;
			}
		}
	}

	return true;
}

void Server::Shutdown()
{
	HB_LOG("SERVER SHUTDOWN");

	SocketUtil::Shutdown();
}

void Server::Run()
{
	while (true)
	{
		//TEST
		
		//HB_LOG("SERVER RUN");
	}
}

void Server::SendPacket(const OutputMemoryBitStream& inOutputStream, const TCPSocketPtr& target)
{
	if (target)
	{
		target->Send(inOutputStream.GetBufferPtr(), inOutputStream.GetByteLength());
	}
	else
	{
		for (const auto& sock : mClientSockets)
		{
			sock->Send(inOutputStream.GetBufferPtr(), inOutputStream.GetByteLength());
		}
	}
}

int Server::RecvPacket(InputMemoryBitStream& inInputStream, const TCPSocketPtr& target)
{
	int retval = target->Recv(inInputStream.GetBufferPtr(), inInputStream.GetRemainingBitCount());

	return retval;
}
