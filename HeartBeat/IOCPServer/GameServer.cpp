#include "GameServer.h"
#include "Log.h"


void GameServer::OnConnect(const INT32 sessionIndex)
{
	LOG("[OnConnect] Session Index: {0}", sessionIndex);
}

void GameServer::OnClose(const INT32 sessionIndex)
{
	LOG("[OnClose] Session Index: {0}", sessionIndex);
}

void GameServer::OnRecv(const INT32 sessionIndex, const UINT32 dataSize, char* msg)
{
	LOG("[OnConnect] Session Index: {0}, Data Size: {1}", sessionIndex, dataSize);
}

void GameServer::Run(const UINT32 maxSessionCount)
{
	StartServer(maxSessionCount);
}

void GameServer::End()
{
	IOCPServer::End();
}
