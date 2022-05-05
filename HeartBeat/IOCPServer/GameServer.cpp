#include "pch.h"
#include "GameServer.h"

void GameServer::OnConnect(const INT32 sessionIndex)
{
	LOG("[OnConnect] Session Index: {0}", sessionIndex);
	PACKET_INFO info{ sessionIndex, SYS_USER_CONNECT, 0, nullptr };
	mGameManager->PushSystemPacket(info);
}

void GameServer::OnClose(const INT32 sessionIndex)
{
	LOG("[OnClose] Session Index: {0}", sessionIndex);
	PACKET_INFO info{ sessionIndex, SYS_USER_DISCONNECT, 0, nullptr };
	mGameManager->PushSystemPacket(info);
}

void GameServer::OnRecv(const INT32 sessionIndex, const UINT32 dataSize, char* msg)
{
	LOG("[OnRecv] Session Index: {0}, Data Size: {1}", sessionIndex, dataSize);
	mGameManager->PushUserData(sessionIndex, dataSize, msg);
}

void GameServer::Run(const UINT32 maxSessionCount)
{
	auto sendPacketFunction = [this](INT32 sessionIndex, UINT32 packetSize, char* packet) {
		SendMsg(sessionIndex, packetSize, packet);
	};

	mGameManager = make_shared<GameManager>();
	mGameManager->SendPacketFunction = sendPacketFunction;
	mGameManager->Init(maxSessionCount);
	mGameManager->Run();

	StartServer(maxSessionCount);
}

void GameServer::End()
{
	mGameManager->End();

	IOCPServer::End();
}
